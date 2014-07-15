
/* System-independent AVI output; adapted from DarkPlaces */

#include "quakedef.h"

#ifndef RQM_SV_ONLY

#include "movie_avi.h"

#if !defined(QWINAVI) || !defined(_WIN32)
/*
#ifdef WIN32
//typedef long fs_offset_t; // 32bit
  typedef __int64 fs_offset_t; // 64bit (lots of warnings, and read/write still don't take 64bit on win64)
#else
  typedef long long fs_offset_t;
#endif
*/

#define QAVI64


#ifdef QAVI64
# ifdef _WIN32
	typedef HANDLE fs_fileref_t;
	typedef QINT64 fs_offset_t;
# else
//#   define __USE_FILE_OFFSET64
//#   define __USE_LARGEFILE64
//#   include <stdio.h>
	typedef FILE     *fs_fileref_t;
	typedef __off64_t fs_offset_t;
# endif
#else
  typedef FILE  *fs_fileref_t;
  typedef QINT64 fs_offset_t;
#endif

#define FILE_BUFF_SIZE 2048
#define AVI_MASTER_INDEX_SIZE 640 // GB ought to be enough for anyone

typedef enum capturevideoformat_e
{
	CAPTUREVIDEOFORMAT_AVI_I420
}
capturevideoformat_t;

typedef struct capturevideostate_s
{
	double starttime;
	double framerate;
	// for AVI saving some values have to be written after capture ends
	fs_offset_t firstchunkframes_offset;
	fs_offset_t totalframes_offset1;
	fs_offset_t totalframes_offset2;
	fs_offset_t totalsampleframes_offset;
	int ix_master_audio_inuse;
	fs_offset_t ix_master_audio_inuse_offset;
	fs_offset_t ix_master_audio_start_offset;
	int ix_master_video_inuse;
	fs_offset_t ix_master_video_inuse_offset;
	fs_offset_t ix_master_video_start_offset;
	fs_offset_t ix_movistart;
	fs_fileref_t videofile;
	qboolean active;
	qboolean realtime;
	qboolean error;
	capturevideoformat_t format;
	int soundrate;
	int frame;
	int soundsampleframe; // for AVI saving
	unsigned char *screenbuffer;
	unsigned char *outbuffer;
	sizebuf_t riffbuffer;
	unsigned char riffbufferdata[FILE_BUFF_SIZE];
	sizebuf_t riffindexbuffer;
		// note: riffindex buffer has an allocated ->data member, not static like most!
	int riffstacklevel;
	fs_offset_t riffstackstartoffset[4];
	short rgbtoyuvscaletable[3][3][256];
	unsigned char yuvnormalizetable[3][256];
//	char basename[MAX_FILELENGTH];
	int width, height;
}
capturevideostate_t;

capturevideostate_t capturevideo;

cvar_t	capture_realtime = {"capture_realtime",   "0"};
//cvar_t	capture_number   = {"capture_number",     "0"};

cvar_t	vid_pixelheight  = {NULL, "1", 0, NULL, 1};
//qboolean	cl_capturevideo;


#ifdef QAVI64
# ifdef _WIN32
   fs_fileref_t FS_OpenNewFile (const char *name);
   void FS_Close (fs_fileref_t ref);
   qboolean FS_Write (fs_fileref_t ref, const void *data, size_t datasize);
   fs_offset_t FS_Tell (fs_fileref_t ref);
   void FS_Seek (fs_fileref_t ref, fs_offset_t ofs, int from);
# else
#  define FS_OpenNewFile(n) fopen64((n), "wb")
#  define FS_Close(f) fclose((f))
#  define FS_Write(f,p,s) fwrite((p), (s), 1, (f))
#  define FS_Tell(f) ftello64((f))
#  define FS_Seek(f, o, w) fseeko64((f), (o), (w))
# endif
#else
#  define FS_OpenNewFile(n) fopen((n), "wb")
#  define FS_Close(f) fclose((f))
#  define FS_Write(f,p,s) fwrite((p), (s), 1, (f))
#  define FS_Tell(f) ftell((f))
#  define FS_Seek(f, o, w) fseek((f), LOW32(o), (w))
#endif

#define LOW32(qw)  ((int) ((qw) & 0xFFFFFFFF))
#define HIGH32(qw) ((int) (((qw) >> 32) & 0xFFFFFFFF))

//extern cvar_t	capture_fps;
extern	int	soundtime;


/*---------------------------------------------------------------------------*/
/* JDH: 64-bit file calls to replace DarkPlaces functions */

// TODO: check for (and handle) errors

#if defined(QAVI64) && defined(_WIN32)

fs_fileref_t FS_OpenNewFile (const char *name)
{
	return CreateFileA (name, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL );
}

void FS_Close (fs_fileref_t ref)
{
	CloseHandle (ref);
}

qboolean FS_Write (fs_fileref_t ref, const void *data, size_t datasize)
{
	DWORD count;

	return WriteFile (ref, data, datasize, &count, NULL);
}

fs_offset_t FS_Tell (fs_fileref_t ref)
{
	LONG low32, high32 = 0;

	low32 = SetFilePointer (ref, 0, &high32, FILE_CURRENT);
	return (((fs_offset_t) high32) << 32) | low32;
}

void FS_Seek (fs_fileref_t ref, fs_offset_t ofs, int from)
{
	LONG high32 = HIGH32(ofs);

	SetFilePointer (ref, LOW32(ofs), &high32, from);
		// Windows uses the same constants as the C runtime functions for the "from" arg
}

#endif		// QAVI64 && _WIN32

/*---------------------------------------------------------------------------*/

void MSG_WriteUnterminatedString (sizebuf_t *sb, const char *s)
{
	if (s && *s)
		SZ_Write (sb, (unsigned char *)s, (int)strlen(s));
}

qboolean Capture_InitAVI (void)
{
//	Cvar_RegisterBool (&capture_realtime);

	return true;
}

qboolean Capture_InitACM (void)
{
	return true;
}

static void SCR_CaptureVideo_RIFF_Start(void)
{
	memset(&capturevideo.riffbuffer, 0, sizeof(sizebuf_t));
	capturevideo.riffbuffer.maxsize = sizeof(capturevideo.riffbufferdata);
	capturevideo.riffbuffer.data = capturevideo.riffbufferdata;
}

static void SCR_CaptureVideo_RIFF_Flush(void)
{
	if (capturevideo.riffbuffer.cursize > 0)
	{
#if defined(_DEBUG) && (!defined(QAVI64) || !defined(_WIN32))
		if (ferror(capturevideo.videofile))
			soundtime *= 1;
#endif
		if (!FS_Write(capturevideo.videofile, capturevideo.riffbuffer.data, capturevideo.riffbuffer.cursize))
			capturevideo.error = true;
		capturevideo.riffbuffer.cursize = 0;
		capturevideo.riffbuffer.overflowed = false;
	}
}

static void SCR_CaptureVideo_RIFF_WriteBytes(const byte *data, size_t size)
{
	SCR_CaptureVideo_RIFF_Flush();
	if (!FS_Write(capturevideo.videofile, data, size))
		capturevideo.error = true;
}

static void SCR_CaptureVideo_RIFF_Write32(int n)
{
	if (capturevideo.riffbuffer.cursize + 4 > capturevideo.riffbuffer.maxsize)
		SCR_CaptureVideo_RIFF_Flush();
	MSG_WriteLong(&capturevideo.riffbuffer, n);
}

static void SCR_CaptureVideo_RIFF_Write16(int n)
{
	if (capturevideo.riffbuffer.cursize + 2 > capturevideo.riffbuffer.maxsize)
		SCR_CaptureVideo_RIFF_Flush();
	MSG_WriteShort(&capturevideo.riffbuffer, n);
}

static void SCR_CaptureVideo_RIFF_WriteFourCC(const char *chunkfourcc)
{
	if (capturevideo.riffbuffer.cursize + (int)strlen(chunkfourcc) > capturevideo.riffbuffer.maxsize)
		SCR_CaptureVideo_RIFF_Flush();
	MSG_WriteUnterminatedString(&capturevideo.riffbuffer, chunkfourcc);
}

static void SCR_CaptureVideo_RIFF_WriteTerminatedString(const char *string)
{
	if (capturevideo.riffbuffer.cursize + (int)strlen(string) > capturevideo.riffbuffer.maxsize)
		SCR_CaptureVideo_RIFF_Flush();
	MSG_WriteString(&capturevideo.riffbuffer, string);
}

static fs_offset_t SCR_CaptureVideo_RIFF_GetPosition(void)
{
	SCR_CaptureVideo_RIFF_Flush();
	return FS_Tell(capturevideo.videofile);
}

static void SCR_CaptureVideo_RIFF_Push(const char *chunkfourcc, const char *listtypefourcc)
{
	SCR_CaptureVideo_RIFF_WriteFourCC(chunkfourcc);
	SCR_CaptureVideo_RIFF_Write32(0);
	SCR_CaptureVideo_RIFF_Flush();
	capturevideo.riffstackstartoffset[capturevideo.riffstacklevel++] = SCR_CaptureVideo_RIFF_GetPosition();
	if (listtypefourcc)
		SCR_CaptureVideo_RIFF_WriteFourCC(listtypefourcc);
}

static void SCR_CaptureVideo_RIFF_Pop(void)
{
	fs_offset_t offset;
	int x;
	unsigned char sizebytes[4];

	// write out the chunk size and then return to the current file position
	capturevideo.riffstacklevel--;
	offset = SCR_CaptureVideo_RIFF_GetPosition();
	x = (int)(offset - (capturevideo.riffstackstartoffset[capturevideo.riffstacklevel]));
	sizebytes[0] = (x) & 0xff;
	sizebytes[1] = (x >> 8) & 0xff;
	sizebytes[2] = (x >> 16) & 0xff;
	sizebytes[3] = (x >> 24) & 0xff;
	FS_Seek(capturevideo.videofile, -(x + 4), SEEK_END);
	FS_Write(capturevideo.videofile, sizebytes, 4);
	FS_Seek(capturevideo.videofile, 0, SEEK_END);
	if (offset & 1)
	{
		unsigned char c = 0;
		FS_Write(capturevideo.videofile, &c, 1);
	}
}

static void GrowBuf(sizebuf_t *buf, int extralen)
{
	if(buf->cursize + extralen > buf->maxsize)
	{
		int oldsize = buf->maxsize;
		unsigned char *olddata;
		olddata = buf->data;
		buf->maxsize = max(buf->maxsize * 2, 4096);
		buf->data = Q_malloc (buf->maxsize);
		if(olddata)
		{
			memcpy(buf->data, olddata, oldsize);
			free(olddata);
		}
	}
}

static void SCR_CaptureVideo_RIFF_IndexEntry(const char *chunkfourcc, int chunksize, int flags)
{
	fs_offset_t ofs;

	if (capturevideo.riffstacklevel != 2)
		Sys_Error("SCR_Capturevideo_RIFF_IndexEntry: RIFF stack level is %i (should be 2)\n", capturevideo.riffstacklevel);

	GrowBuf(&capturevideo.riffindexbuffer, 16);
	SCR_CaptureVideo_RIFF_Flush();

	MSG_WriteUnterminatedString(&capturevideo.riffindexbuffer, chunkfourcc);
	MSG_WriteLong(&capturevideo.riffindexbuffer, flags);

	ofs = FS_Tell(capturevideo.videofile) - capturevideo.riffstackstartoffset[1];
	MSG_WriteLong(&capturevideo.riffindexbuffer, LOW32(ofs));

	MSG_WriteLong(&capturevideo.riffindexbuffer, chunksize);
}

static void SCR_CaptureVideo_RIFF_MakeIxChunk(const char *fcc, const char *dwChunkId, fs_offset_t masteridx_counter,
												int *masteridx_count, fs_offset_t masteridx_start)
{
	int nMatching;
	int i;
	fs_offset_t ix = SCR_CaptureVideo_RIFF_GetPosition();
	fs_offset_t pos;

	if (*masteridx_count >= AVI_MASTER_INDEX_SIZE)
		return;

	nMatching = 0; // go through index and enumerate them
	for(i = 0; i < capturevideo.riffindexbuffer.cursize; i += 16)
		if(!memcmp(capturevideo.riffindexbuffer.data + i, dwChunkId, 4))
			++nMatching;

	SCR_CaptureVideo_RIFF_Push(fcc, NULL);
	SCR_CaptureVideo_RIFF_Write16(2); // wLongsPerEntry
	SCR_CaptureVideo_RIFF_Write16(0x0100); // bIndexType=1, bIndexSubType=0
	SCR_CaptureVideo_RIFF_Write32(nMatching); // nEntriesInUse
	SCR_CaptureVideo_RIFF_WriteFourCC(dwChunkId); // dwChunkId
	SCR_CaptureVideo_RIFF_Write32(LOW32(capturevideo.ix_movistart));
	SCR_CaptureVideo_RIFF_Write32(HIGH32(capturevideo.ix_movistart));
	SCR_CaptureVideo_RIFF_Write32(0); // dwReserved

	for(i = 0; i < capturevideo.riffindexbuffer.cursize; i += 16)
		if(!memcmp(capturevideo.riffindexbuffer.data + i, dwChunkId, 4))
		{
			unsigned int *p = (unsigned int *) (capturevideo.riffindexbuffer.data + i);
			unsigned int flags = p[1];
			unsigned int rpos = p[2];
			unsigned int size = p[3];
			size &= ~0x80000000;
			if(!(flags & 0x10)) // no keyframe?
				size |= 0x80000000;
			SCR_CaptureVideo_RIFF_Write32(rpos + 8);
			SCR_CaptureVideo_RIFF_Write32(size);
		}

	SCR_CaptureVideo_RIFF_Pop();
	pos = SCR_CaptureVideo_RIFF_GetPosition();
	SCR_CaptureVideo_RIFF_Flush();

	FS_Seek(capturevideo.videofile, masteridx_start + 16 * *masteridx_count, SEEK_SET);
	SCR_CaptureVideo_RIFF_Write32(LOW32(ix));
	SCR_CaptureVideo_RIFF_Write32(HIGH32(ix));
	SCR_CaptureVideo_RIFF_Write32(LOW32(pos - ix));
	SCR_CaptureVideo_RIFF_Write32(nMatching);
	SCR_CaptureVideo_RIFF_Flush();

	FS_Seek(capturevideo.videofile, masteridx_counter, SEEK_SET);
	SCR_CaptureVideo_RIFF_Write32(++*masteridx_count);
	SCR_CaptureVideo_RIFF_Flush();

	FS_Seek(capturevideo.videofile, 0, SEEK_END);
}

static void SCR_CaptureVideo_RIFF_Finish(qboolean final)
{
	// close the "movi" list
	SCR_CaptureVideo_RIFF_Pop();

	if(capturevideo.ix_master_video_inuse_offset)
	{
		SCR_CaptureVideo_RIFF_MakeIxChunk("ix00", "00dc", capturevideo.ix_master_video_inuse_offset,
					&capturevideo.ix_master_video_inuse, capturevideo.ix_master_video_start_offset);
	}

	if(capturevideo.ix_master_audio_inuse_offset)
	{
		SCR_CaptureVideo_RIFF_MakeIxChunk("ix01", "01wb", capturevideo.ix_master_audio_inuse_offset,
					&capturevideo.ix_master_audio_inuse, capturevideo.ix_master_audio_start_offset);
	}

	// write the idx1 chunk that we've been building while saving the frames (for old style players)
	if(final && capturevideo.firstchunkframes_offset)
	// TODO replace index creating by OpenDML ix##/##ix/indx chunk so it works for more than one AVI part too
	{
		SCR_CaptureVideo_RIFF_Push("idx1", NULL);
		SCR_CaptureVideo_RIFF_WriteBytes(capturevideo.riffindexbuffer.data, capturevideo.riffindexbuffer.cursize);
		SCR_CaptureVideo_RIFF_Pop();
	}
	capturevideo.riffindexbuffer.cursize = 0;
	// pop the RIFF chunk itself
	while (capturevideo.riffstacklevel > 0)
		SCR_CaptureVideo_RIFF_Pop();
	SCR_CaptureVideo_RIFF_Flush();
	if(capturevideo.firstchunkframes_offset)
	{
		Con_DPrintf("Finishing first chunk (%d frames)\n", capturevideo.frame);
		FS_Seek(capturevideo.videofile, capturevideo.firstchunkframes_offset, SEEK_SET);
		SCR_CaptureVideo_RIFF_Write32(capturevideo.frame);
		SCR_CaptureVideo_RIFF_Flush();
		FS_Seek(capturevideo.videofile, 0, SEEK_END);
		capturevideo.firstchunkframes_offset = 0;
	}
	else
		Con_DPrintf("Finishing another chunk (%d total frames)\n", capturevideo.frame);
}

static void SCR_CaptureVideo_RIFF_OverflowCheck(int framesize)
{
	fs_offset_t cursize, curfilesize;
	if (capturevideo.riffstacklevel != 2)
		Sys_Error("SCR_CaptureVideo_RIFF_OverflowCheck: chunk stack leakage!\n");
	// check where we are in the file
	SCR_CaptureVideo_RIFF_Flush();
	cursize = SCR_CaptureVideo_RIFF_GetPosition() - capturevideo.riffstackstartoffset[0];
	curfilesize = SCR_CaptureVideo_RIFF_GetPosition();

	// if this would overflow the windows limit of 1GB per RIFF chunk, we need
	// to close the current RIFF chunk and open another for future frames
	// (note that the Ix buffer takes less space... I just don't dare to / 2 here now... sorry, maybe later)
	if (8 + cursize + framesize + capturevideo.riffindexbuffer.cursize +
		8 + capturevideo.riffindexbuffer.cursize + 64 > 1<<30)
	{
		SCR_CaptureVideo_RIFF_Finish(false);
		// begin a new 1GB extended section of the AVI
		SCR_CaptureVideo_RIFF_Push("RIFF", "AVIX");
		SCR_CaptureVideo_RIFF_Push("LIST", "movi");
		capturevideo.ix_movistart = capturevideo.riffstackstartoffset[1];
	}
}

static void FindFraction(double val, int *num, int *denom, int denomMax)
{
	int i;
	double bestdiff;
	// initialize
	bestdiff = fabs(val);
	*num = 0;
	*denom = 1;

	for(i = 1; i <= denomMax; ++i)
	{
		int inum = floor(0.5 + val * i);
		double diff = fabs(val - inum / (double)i);
		if(diff < bestdiff)
		{
			bestdiff = diff;
			*num = inum;
			*denom = i;
		}
	}
}

void Capture_UpdateRamps (void)
{
//	double gamma, g;
	int r, g, b;
	unsigned int i;
	extern	unsigned short	ramps[3][256];
#ifdef NEWHWBLEND
	extern	cvar_t gl_hwblend;
#endif

//	gamma = 1.0/*/scr_screenshot_gammaboost.value*/;
	/*
	for (i = 0;i < 256;i++)
	{
		unsigned char j = (unsigned char)bound(0, 255*pow(i/255.0, gamma), 255);
		capturevideo.rgbgammatable[0][i] = j;
		capturevideo.rgbgammatable[1][i] = j;
		capturevideo.rgbgammatable[2][i] = j;
	}
	*/
/*
R = Y + 1.4075 * (Cr - 128);
G = Y + -0.3455 * (Cb - 128) + -0.7169 * (Cr - 128);
B = Y + 1.7790 * (Cb - 128);
Y = R *  .299 + G *  .587 + B *  .114;
Cb = R * -.169 + G * -.332 + B *  .500 + 128.;
Cr = R *  .500 + G * -.419 + B * -.0813 + 128.;
*/

	for (i = 0;i < 256;i++)
	{
		//g = 255*pow(i/255.0, gamma);
		if (V_USING_HWRAMPS())
		{
			r = ramps[0][i] >> 8;
			g = ramps[1][i] >> 8;
			b = ramps[2][i] >> 8;
		}
		else
			r = g = b = i;

		// Y weights from RGB
		capturevideo.rgbtoyuvscaletable[0][0][i] = (short)(r *  0.299);
		capturevideo.rgbtoyuvscaletable[0][1][i] = (short)(g *  0.587);
		capturevideo.rgbtoyuvscaletable[0][2][i] = (short)(b *  0.114);
		// Cb weights from RGB
		capturevideo.rgbtoyuvscaletable[1][0][i] = (short)(r * -0.169);
		capturevideo.rgbtoyuvscaletable[1][1][i] = (short)(g * -0.332);
		capturevideo.rgbtoyuvscaletable[1][2][i] = (short)(b *  0.500);
		// Cr weights from RGB
		capturevideo.rgbtoyuvscaletable[2][0][i] = (short)(r *  0.500);
		capturevideo.rgbtoyuvscaletable[2][1][i] = (short)(g * -0.419);
		capturevideo.rgbtoyuvscaletable[2][2][i] = (short)(b * -0.0813);
		// range reduction of YCbCr to valid signal range
		capturevideo.yuvnormalizetable[0][i] = 16 + i * (236-16) / 256;
		capturevideo.yuvnormalizetable[1][i] = 16 + i * (240-16) / 256;
		capturevideo.yuvnormalizetable[2][i] = 16 + i * (240-16) / 256;
	}
}

// DarkPlaces code uses vid.width & vid.height, but this works better here
#define SCREEN_WIDTH  glwidth
#define SCREEN_HEIGHT glheight

qboolean Capture_Open (const char *path, int width, int height, float fps)		// was SCR_CaptureVideo_BeginVideo
{
	double aspect;
	int n, d;
	unsigned int i;
	extern	unsigned short	ramps[3][256];

	if (capturevideo.active)
		return false;
	memset(&capturevideo, 0, sizeof(capturevideo));
	// soundrate is figured out on the first SoundFrame

	aspect = SCREEN_WIDTH / (SCREEN_HEIGHT * vid_pixelheight.value);

	capturevideo.width = width;
	capturevideo.height = height;
	capturevideo.active = true;
	capturevideo.starttime = realtime;
	capturevideo.framerate = fps;
	capturevideo.soundrate = S_GetSoundRate();
	capturevideo.frame = 0;
	capturevideo.soundsampleframe = 0;
	capturevideo.realtime = capture_realtime.value != 0;
	capturevideo.screenbuffer = (unsigned char *)Q_malloc(SCREEN_WIDTH * SCREEN_HEIGHT * 3);
	capturevideo.outbuffer = (unsigned char *)Q_malloc(width * height * (3+3) + 18);

//	Q_snprintfz(capturevideo.basename, sizeof(capturevideo.basename), "video/dpvideo%03i", capture_number.value);
//	Cvar_SetValueDirect(&capture_number, capture_number.value + 1);
//	COM_FileBase (path, capturevideo.basename, sizeof(capturevideo.basename));

	Capture_UpdateRamps ();

	//if (cl_capturevideo_)
	//{
	//}
	//else
	{
		capturevideo.format = CAPTUREVIDEOFORMAT_AVI_I420;
		capturevideo.videofile = FS_OpenNewFile(path);

		SCR_CaptureVideo_RIFF_Start();
		// enclosing RIFF chunk (there can be multiple of these in >1GB files, the
		// later ones are "AVIX" instead of "AVI " and have no header/stream info)
		SCR_CaptureVideo_RIFF_Push("RIFF", "AVI ");

		// AVI main header
		SCR_CaptureVideo_RIFF_Push("LIST", "hdrl");
		SCR_CaptureVideo_RIFF_Push("avih", NULL);
		SCR_CaptureVideo_RIFF_Write32((int)(1000000.0 / capturevideo.framerate)); // microseconds per frame
		SCR_CaptureVideo_RIFF_Write32(0); // max bytes per second
		SCR_CaptureVideo_RIFF_Write32(0); // padding granularity
		SCR_CaptureVideo_RIFF_Write32(0x910); // flags (AVIF_HASINDEX | AVIF_ISINTERLEAVED | AVIF_TRUSTCKTYPE)
		capturevideo.firstchunkframes_offset = SCR_CaptureVideo_RIFF_GetPosition();
		SCR_CaptureVideo_RIFF_Write32(0); // total frames
		SCR_CaptureVideo_RIFF_Write32(0); // initial frames
		if (capturevideo.soundrate)
			SCR_CaptureVideo_RIFF_Write32(2); // number of streams
		else
			SCR_CaptureVideo_RIFF_Write32(1); // number of streams
		SCR_CaptureVideo_RIFF_Write32(0); // suggested buffer size
		SCR_CaptureVideo_RIFF_Write32(width); // width
		SCR_CaptureVideo_RIFF_Write32(height); // height
		SCR_CaptureVideo_RIFF_Write32(0); // reserved[0]
		SCR_CaptureVideo_RIFF_Write32(0); // reserved[1]
		SCR_CaptureVideo_RIFF_Write32(0); // reserved[2]
		SCR_CaptureVideo_RIFF_Write32(0); // reserved[3]
		SCR_CaptureVideo_RIFF_Pop();

		// video stream info
		SCR_CaptureVideo_RIFF_Push("LIST", "strl");
		SCR_CaptureVideo_RIFF_Push("strh", "vids");
		SCR_CaptureVideo_RIFF_WriteFourCC("I420"); // stream fourcc (I420 colorspace, uncompressed)
		SCR_CaptureVideo_RIFF_Write32(0); // flags
		SCR_CaptureVideo_RIFF_Write16(0); // priority
		SCR_CaptureVideo_RIFF_Write16(0); // language
		SCR_CaptureVideo_RIFF_Write32(0); // initial frames

		// find an ideal divisor for the framerate
		FindFraction(capturevideo.framerate, &n, &d, 1000);
		SCR_CaptureVideo_RIFF_Write32(d); // samples/second divisor
		SCR_CaptureVideo_RIFF_Write32(n); // samples/second multiplied by divisor
		SCR_CaptureVideo_RIFF_Write32(0); // start
		capturevideo.totalframes_offset1 = SCR_CaptureVideo_RIFF_GetPosition();
		SCR_CaptureVideo_RIFF_Write32(0); // length
		SCR_CaptureVideo_RIFF_Write32(width*height+(width/2)*(height/2)*2); // suggested buffer size
		SCR_CaptureVideo_RIFF_Write32(0); // quality
		SCR_CaptureVideo_RIFF_Write32(0); // sample size
		SCR_CaptureVideo_RIFF_Write16(0); // frame left
		SCR_CaptureVideo_RIFF_Write16(0); // frame top
		SCR_CaptureVideo_RIFF_Write16(width); // frame right
		SCR_CaptureVideo_RIFF_Write16(height); // frame bottom
		SCR_CaptureVideo_RIFF_Pop();

		// video stream format
		SCR_CaptureVideo_RIFF_Push("strf", NULL);
		SCR_CaptureVideo_RIFF_Write32(40); // BITMAPINFO struct size
		SCR_CaptureVideo_RIFF_Write32(width); // width
		SCR_CaptureVideo_RIFF_Write32(height); // height
		SCR_CaptureVideo_RIFF_Write16(3); // planes
		SCR_CaptureVideo_RIFF_Write16(12); // bitcount
		SCR_CaptureVideo_RIFF_WriteFourCC("I420"); // compression
		SCR_CaptureVideo_RIFF_Write32(width*height+(width/2)*(height/2)*2); // size of image
		SCR_CaptureVideo_RIFF_Write32(0); // x pixels per meter
		SCR_CaptureVideo_RIFF_Write32(0); // y pixels per meter
		SCR_CaptureVideo_RIFF_Write32(0); // color used
		SCR_CaptureVideo_RIFF_Write32(0); // color important
		SCR_CaptureVideo_RIFF_Pop();

		// master index
		SCR_CaptureVideo_RIFF_Push("indx", NULL);
		SCR_CaptureVideo_RIFF_Write16(4); // wLongsPerEntry
		SCR_CaptureVideo_RIFF_Write16(0); // bIndexSubType=0, bIndexType=0
		capturevideo.ix_master_video_inuse_offset = SCR_CaptureVideo_RIFF_GetPosition();
		SCR_CaptureVideo_RIFF_Write32(0); // nEntriesInUse
		SCR_CaptureVideo_RIFF_WriteFourCC("00dc"); // dwChunkId
		SCR_CaptureVideo_RIFF_Write32(0); // dwReserved1
		SCR_CaptureVideo_RIFF_Write32(0); // dwReserved2
		SCR_CaptureVideo_RIFF_Write32(0); // dwReserved3
		capturevideo.ix_master_video_start_offset = SCR_CaptureVideo_RIFF_GetPosition();
		for(i = 0; i < AVI_MASTER_INDEX_SIZE * 4; ++i)
			SCR_CaptureVideo_RIFF_Write32(0); // fill up later
		SCR_CaptureVideo_RIFF_Pop();

		// extended format (aspect!)
		SCR_CaptureVideo_RIFF_Push("vprp", NULL);
		SCR_CaptureVideo_RIFF_Write32(0); // VideoFormatToken
		SCR_CaptureVideo_RIFF_Write32(0); // VideoStandard
		SCR_CaptureVideo_RIFF_Write32((int)capturevideo.framerate); // dwVerticalRefreshRate (bogus)
		SCR_CaptureVideo_RIFF_Write32(width); // dwHTotalInT
		SCR_CaptureVideo_RIFF_Write32(height); // dwVTotalInLines
		FindFraction(aspect, &n, &d, 1000);
		SCR_CaptureVideo_RIFF_Write32((n << 16) | d); // dwFrameAspectRatio // TODO a word
		SCR_CaptureVideo_RIFF_Write32(width); // dwFrameWidthInPixels
		SCR_CaptureVideo_RIFF_Write32(height); // dwFrameHeightInLines
		SCR_CaptureVideo_RIFF_Write32(1); // nFieldPerFrame
		SCR_CaptureVideo_RIFF_Write32(width); // CompressedBMWidth
		SCR_CaptureVideo_RIFF_Write32(height); // CompressedBMHeight
		SCR_CaptureVideo_RIFF_Write32(width); // ValidBMHeight
		SCR_CaptureVideo_RIFF_Write32(height); // ValidBMWidth
		SCR_CaptureVideo_RIFF_Write32(0); // ValidBMXOffset
		SCR_CaptureVideo_RIFF_Write32(0); // ValidBMYOffset
		SCR_CaptureVideo_RIFF_Write32(0); // ValidBMXOffsetInT
		SCR_CaptureVideo_RIFF_Write32(0); // ValidBMYValidStartLine
		SCR_CaptureVideo_RIFF_Pop();
		SCR_CaptureVideo_RIFF_Pop();

		if (capturevideo.soundrate)
		{
			// audio stream info
			SCR_CaptureVideo_RIFF_Push("LIST", "strl");
			SCR_CaptureVideo_RIFF_Push("strh", "auds");
			SCR_CaptureVideo_RIFF_Write32(1); // stream fourcc (PCM audio, uncompressed)
			SCR_CaptureVideo_RIFF_Write32(0); // flags
			SCR_CaptureVideo_RIFF_Write16(0); // priority
			SCR_CaptureVideo_RIFF_Write16(0); // language
			SCR_CaptureVideo_RIFF_Write32(0); // initial frames
			SCR_CaptureVideo_RIFF_Write32(1); // samples/second divisor
			SCR_CaptureVideo_RIFF_Write32((int)(capturevideo.soundrate)); // samples/second multiplied by divisor
			SCR_CaptureVideo_RIFF_Write32(0); // start
			capturevideo.totalsampleframes_offset = SCR_CaptureVideo_RIFF_GetPosition();
			SCR_CaptureVideo_RIFF_Write32(0); // length
			SCR_CaptureVideo_RIFF_Write32(capturevideo.soundrate * 2); // suggested buffer size (this is a half second)
			SCR_CaptureVideo_RIFF_Write32(0); // quality
			SCR_CaptureVideo_RIFF_Write32(4); // sample size
			SCR_CaptureVideo_RIFF_Write16(0); // frame left
			SCR_CaptureVideo_RIFF_Write16(0); // frame top
			SCR_CaptureVideo_RIFF_Write16(0); // frame right
			SCR_CaptureVideo_RIFF_Write16(0); // frame bottom
			SCR_CaptureVideo_RIFF_Pop();

			// audio stream format
			SCR_CaptureVideo_RIFF_Push("strf", NULL);
			SCR_CaptureVideo_RIFF_Write16(1); // format (uncompressed PCM?)
			SCR_CaptureVideo_RIFF_Write16(2); // channels (stereo)
			SCR_CaptureVideo_RIFF_Write32(capturevideo.soundrate); // sampleframes per second
			SCR_CaptureVideo_RIFF_Write32(capturevideo.soundrate * 4); // average bytes per second
			SCR_CaptureVideo_RIFF_Write16(4); // block align
			SCR_CaptureVideo_RIFF_Write16(16); // bits per sample
			SCR_CaptureVideo_RIFF_Write16(0); // size
			SCR_CaptureVideo_RIFF_Pop();

			// master index
			SCR_CaptureVideo_RIFF_Push("indx", NULL);
			SCR_CaptureVideo_RIFF_Write16(4); // wLongsPerEntry
			SCR_CaptureVideo_RIFF_Write16(0); // bIndexSubType=0, bIndexType=0
			capturevideo.ix_master_audio_inuse_offset = SCR_CaptureVideo_RIFF_GetPosition();
			SCR_CaptureVideo_RIFF_Write32(0); // nEntriesInUse
			SCR_CaptureVideo_RIFF_WriteFourCC("01wb"); // dwChunkId
			SCR_CaptureVideo_RIFF_Write32(0); // dwReserved1
			SCR_CaptureVideo_RIFF_Write32(0); // dwReserved2
			SCR_CaptureVideo_RIFF_Write32(0); // dwReserved3
			capturevideo.ix_master_audio_start_offset = SCR_CaptureVideo_RIFF_GetPosition();
			for(i = 0; i < AVI_MASTER_INDEX_SIZE * 4; ++i)
				SCR_CaptureVideo_RIFF_Write32(0); // fill up later
			SCR_CaptureVideo_RIFF_Pop();
			SCR_CaptureVideo_RIFF_Pop();
		}

		capturevideo.ix_master_audio_inuse = capturevideo.ix_master_video_inuse = 0;

		// extended header (for total #frames)
		SCR_CaptureVideo_RIFF_Push("LIST", "odml");
		SCR_CaptureVideo_RIFF_Push("dmlh", NULL);
		capturevideo.totalframes_offset2 = SCR_CaptureVideo_RIFF_GetPosition();
		SCR_CaptureVideo_RIFF_Write32(0);
		SCR_CaptureVideo_RIFF_Pop();
		SCR_CaptureVideo_RIFF_Pop();

		// close the AVI header list
		SCR_CaptureVideo_RIFF_Pop();
		// software that produced this AVI video file
		SCR_CaptureVideo_RIFF_Push("LIST", "INFO");
		SCR_CaptureVideo_RIFF_Push("ISFT", NULL);
		SCR_CaptureVideo_RIFF_WriteTerminatedString(va("reQuiem %s", VersionString()));
		SCR_CaptureVideo_RIFF_Pop();
		// enable this junk filler if you like the LIST movi to always begin at 4KB in the file (why?)
#if 0
		{
			int x, i;
			const char *junkfiller = "[ DarkPlaces junk data ]";

			SCR_CaptureVideo_RIFF_Push("JUNK", NULL);
			x = 4096 - SCR_CaptureVideo_RIFF_GetPosition();
			while (x > 0)
			{
				i = min(x, (int)strlen(junkfiller));
				SCR_CaptureVideo_RIFF_WriteBytes((const unsigned char *)junkfiller, i);
				x -= i;
			}
			SCR_CaptureVideo_RIFF_Pop();
		}
#endif
		SCR_CaptureVideo_RIFF_Pop();
		// begin the actual video section now
		SCR_CaptureVideo_RIFF_Push("LIST", "movi");
		capturevideo.ix_movistart = capturevideo.riffstackstartoffset[1];
		// we're done with the headers now...
		SCR_CaptureVideo_RIFF_Flush();
		if (capturevideo.riffstacklevel != 2)
			Sys_Error("SCR_CaptureVideo_BeginVideo: broken AVI writing code (stack level is %i (should be 2) at end of headers)\n", capturevideo.riffstacklevel);
	}
/*
	switch(capturevideo.format)
	{
	case CAPTUREVIDEOFORMAT_AVI_I420:
		break;
	default:
		break;
	}
*/
	return true;
}

void Capture_Close (void)		// was SCR_CaptureVideo_EndVideo
{
	if (!capturevideo.active)
		return;
	capturevideo.active = false;
	if (capturevideo.videofile)
	{
		switch(capturevideo.format)
		{
		case CAPTUREVIDEOFORMAT_AVI_I420:
			// close any open chunks
			SCR_CaptureVideo_RIFF_Finish(true);
			// go back and fix the video frames and audio samples fields
			Con_DPrintf("Finishing capture (%d frames, %d audio frames)\n\n", capturevideo.frame, capturevideo.soundsampleframe);
			FS_Seek(capturevideo.videofile, capturevideo.totalframes_offset1, SEEK_SET);
			SCR_CaptureVideo_RIFF_Write32(capturevideo.frame);
			SCR_CaptureVideo_RIFF_Flush();
			FS_Seek(capturevideo.videofile, capturevideo.totalframes_offset2, SEEK_SET);
			SCR_CaptureVideo_RIFF_Write32(capturevideo.frame);
			SCR_CaptureVideo_RIFF_Flush();
			if (capturevideo.soundrate)
			{
				FS_Seek(capturevideo.videofile, capturevideo.totalsampleframes_offset, SEEK_SET);
				SCR_CaptureVideo_RIFF_Write32(capturevideo.soundsampleframe);
				SCR_CaptureVideo_RIFF_Flush();
			}
			break;
		default:
			break;
		}
		FS_Close(capturevideo.videofile);
		capturevideo.videofile = NULL;
	}

	if (capturevideo.screenbuffer)
	{
		free (capturevideo.screenbuffer);
		capturevideo.screenbuffer = NULL;
	}

	if (capturevideo.outbuffer)
	{
		free (capturevideo.outbuffer);
		capturevideo.outbuffer = NULL;
	}

	if (capturevideo.riffindexbuffer.data)
	{
		free (capturevideo.riffindexbuffer.data);
		capturevideo.riffindexbuffer.data = NULL;
	}

	memset(&capturevideo, 0, sizeof(capturevideo));
}

// converts from BGR24 to I420 colorspace (identical to YV12 except chroma plane order is reversed), this colorspace is handled by the Intel(r) 4:2:0 codec on Windows
void SCR_CaptureVideo_ConvertFrame_BGR_to_I420_flip(int width, int height, unsigned char *instart, unsigned char *outstart)
{
	int x, y;
	int blockr, blockg, blockb;
	int outoffset = (width/2)*(height/2);
	unsigned char *b, *out;

	// process one line at a time, and CbCr every other line at 2 pixel intervals
	for (y = 0;y < height;y++)
	{
		// 1x1 Y
		for (b = instart + (height-1-y)*width*3, out = outstart + y*width, x = 0;x < width;x++, b += 3, out++)
		{
			blockr = b[2];
			blockg = b[1];
			blockb = b[0];
			*out = capturevideo.yuvnormalizetable[0][capturevideo.rgbtoyuvscaletable[0][0][blockr]
					+ capturevideo.rgbtoyuvscaletable[0][1][blockg]
					+ capturevideo.rgbtoyuvscaletable[0][2][blockb]];
		}
		if ((y & 1) == 0)
		{
			// 2x2 Cr and Cb planes
			int inpitch = width*3;
			for (b = instart + (height-2-y)*width*3, out = outstart + width*height + (y/2)*(width/2), x = 0;x < width/2;x++, b += 6, out++)
			{
				blockr = (b[2] + b[5] + b[inpitch+2] + b[inpitch+5]) >> 2;
				blockg = (b[1] + b[4] + b[inpitch+1] + b[inpitch+4]) >> 2;
				blockb = (b[0] + b[3] + b[inpitch+0] + b[inpitch+3]) >> 2;
				// Cr
				out[0] = capturevideo.yuvnormalizetable[1][capturevideo.rgbtoyuvscaletable[1][0][blockr]
							+ capturevideo.rgbtoyuvscaletable[1][1][blockg]
							+ capturevideo.rgbtoyuvscaletable[1][2][blockb] + 128];
				// Cb
				out[outoffset] = capturevideo.yuvnormalizetable[2][capturevideo.rgbtoyuvscaletable[2][0][blockr]
								+ capturevideo.rgbtoyuvscaletable[2][1][blockg]
								+ capturevideo.rgbtoyuvscaletable[2][2][blockb] + 128];
			}
		}
	}
}
/*
#ifdef _DEBUG
#  define MOV_CHECKTIME(t) (t = Sys_DoubleTime ())
#else
#  define MOV_CHECKTIME(t)
#endif
*/
qboolean SCR_CaptureVideo_VideoFrame(int newframenum)
{
	int x = 0, y = 0, width = capturevideo.width, height = capturevideo.height;
	unsigned char *in, *out;
	fs_offset_t pos;
#ifdef _DEBUG
//	double time1, time2, time3, time4, time5;
#endif

	//return SCR_ScreenShot(filename, capturevideo.buffer, capturevideo.buffer + vid.width * vid.height * 3, capturevideo.buffer + vid.width * vid.height * 6, 0, 0, vid.width, vid.height, false, false, false, jpeg, true);
	// speed is critical here, so do saving as directly as possible
	switch (capturevideo.format)
	{
	case CAPTUREVIDEOFORMAT_AVI_I420:
		// if there's no videofile we have to just give up, and abort saving
		if (!capturevideo.videofile)
			return false;
		// FIXME: width/height must be multiple of 2, enforce this?
//MOV_CHECKTIME(time1);
//		Con_Printf ("glwidth = %d, glheight = %d, vid.width = %d, vid.height = %d\n", glwidth, glheight, vid.width, vid.height);
		glReadPixels (x, y, SCREEN_WIDTH, SCREEN_HEIGHT, GL_BGR_EXT, GL_UNSIGNED_BYTE, capturevideo.screenbuffer);
//MOV_CHECKTIME(time2);
		in = Movie_ScaleDownBGR (capturevideo.screenbuffer, SCREEN_WIDTH, SCREEN_HEIGHT, capturevideo.outbuffer, width, height);
		//in = capturevideo.outbuffer;
//MOV_CHECKTIME(time3);
		out = capturevideo.outbuffer + width*height*3;
		SCR_CaptureVideo_ConvertFrame_BGR_to_I420_flip(width, height, in, out);
		x = width*height+(width/2)*(height/2)*2;
		SCR_CaptureVideo_RIFF_OverflowCheck(8 + x);
#ifdef _DEBUG
//		scr_disabled_for_loading = true;
//		Con_Printf ("writing %d frames\n", newframenum - capturevideo.frame);
//		scr_disabled_for_loading = false;
#endif
		for (;capturevideo.frame < newframenum; capturevideo.frame++)
		{
			pos = SCR_CaptureVideo_RIFF_GetPosition ();

			SCR_CaptureVideo_RIFF_IndexEntry("00dc", x, 0x10); // AVIIF_KEYFRAME
			SCR_CaptureVideo_RIFF_Push("00dc", NULL);
//MOV_CHECKTIME(time4);
			SCR_CaptureVideo_RIFF_WriteBytes(out, x);
//MOV_CHECKTIME(time5);
			SCR_CaptureVideo_RIFF_Pop();

			if (capturevideo.error)
			{
				FS_Seek (capturevideo.videofile, pos, SEEK_SET);
				clearerr (capturevideo.videofile);
				break;
			}
		}
#ifdef _DEBUG
/*		if (newframenum <= 5)
		{
			scr_disabled_for_loading = true;
			Con_Printf ("%.03lf +%.03lf +%.03lf +%.03lf +%.03lf %.03lf\n", time1,
						time2-time1, time3-time2, time4-time3, time5-time4, time5);
			scr_disabled_for_loading = false;
		}
*/
#endif
		return true;
	default:
		return false;
	}
}

qboolean Capture_WriteAudio (const byte *sample_buffer, int samples)				// was SCR_CaptureVideo_SoundFrame
{
	int x;
//	capturevideo.soundrate = rate;
	capturevideo.soundsampleframe += samples;
	switch (capturevideo.format)
	{
	case CAPTUREVIDEOFORMAT_AVI_I420:
		x = samples*4;
		SCR_CaptureVideo_RIFF_OverflowCheck(8 + x);
		SCR_CaptureVideo_RIFF_IndexEntry("01wb", x, 0x10); // AVIIF_KEYFRAME
		SCR_CaptureVideo_RIFF_Push("01wb", NULL);
		SCR_CaptureVideo_RIFF_WriteBytes(sample_buffer, x);
		SCR_CaptureVideo_RIFF_Pop();
		break;
	default:
		break;
	}

	return true;
}

qboolean Capture_WriteVideo (void)		// was SCR_CaptureVideo
{
	int newframenum;
//	if (cl_capturevideo /*&& r_render.integer*/)
	{
//		if (!capturevideo.active)
//			SCR_CaptureVideo_BeginVideo();
		/*if (capturevideo.framerate != capture_fps.value)
		{
			Con_Print("You can not change the video framerate while recording a video.\n");
			Cvar_SetValueDirect(&capture_fps, capturevideo.framerate);
		}*/

		// for AVI saving we have to make sure that sound is saved before video
	/*	if (capturevideo.soundrate && !capturevideo.soundsampleframe)
			return;
		if (capturevideo.realtime)
		{
			// preserve sound sync by duplicating frames when running slow
			newframenum = (int)((realtime - capturevideo.starttime) * capturevideo.framerate);
		}
		else*/
			newframenum = capturevideo.frame + 1;
		// if falling behind more than one second, stop
		/*if (newframenum - capturevideo.frame > 60 * (int)ceil(capturevideo.framerate))
		{
//			Cvar_SetValueQuick(&cl_capturevideo, 0);
			Con_Printf("video saving failed on frame %i, your machine is too slow for this capture speed.\n", capturevideo.frame);
			return false;
		}*/
		// write frames
		SCR_CaptureVideo_VideoFrame(newframenum);
		if (capturevideo.error)
		{
//			Cvar_SetValueQuick(&cl_capturevideo, 0);
			Con_Printf("video saving failed on frame %i, out of disk space? stopping video capture.\n", capturevideo.frame);
			return false;
		}
#ifdef _DEBUG
//		if (capturevideo.frame >= 150)
//			return false;
#endif
	}
//	else if (capturevideo.active)
//		Capture_Close();

	return true;
}

double Capture_FrameTime (void)
{
	double time;
/*	extern double oldrealtime;

	if (capturevideo.realtime)
		time = 1.0 / capturevideo.framerate;
	else
	{
		//clframetime = 1.0 / cls.capturevideo.framerate;
		//cl.realframetime = max(cl_timer, clframetime);
		time = realtime - oldrealtime;
	}
*/
	time = 1.0 / capturevideo.framerate;
	return bound(1.0 / 1000, time, 1.0);
}

qboolean Capture_GetSoundtime (void)
{
/*	if (capturevideo.soundrate && !capturevideo.realtime) // SUPER NASTY HACK to record non-realtime sound
	{
	//	usesoundtimehack = 2;
		soundtime = (unsigned int)((double)capturevideo.frame * (double)shm->speed / (double)capturevideo.framerate);
		return true;
	}

	return false;
*/

//	soundtime += (int)(0.5 + host_frametime * shm->speed * (Capture_FrameTime() / host_frametime));
	soundtime += (int)(0.5 + shm->speed * Capture_FrameTime());
	return true;
}

#endif		//#ifndef QWINAVI

#endif		//#ifndef RQM_SV_ONLY
