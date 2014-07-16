/*
Copyright (C) 2001 Quake done Quick

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/
// movie_avi.c

#ifdef _WIN32

#include "quakedef.h"

#ifndef RQM_SV_ONLY

#include "movie_avi.h"

#ifdef QWINAVI

#include <windows.h>
#include <vfw.h>

extern	int	soundtime;

static void (CALLBACK *qAVIFileInit)(void);
static HRESULT (CALLBACK *qAVIFileOpen)(PAVIFILE *, LPCTSTR, UINT, LPCLSID);
static HRESULT (CALLBACK *qAVIFileCreateStream)(PAVIFILE, PAVISTREAM *, AVISTREAMINFO *);
static HRESULT (CALLBACK *qAVIMakeCompressedStream)(PAVISTREAM *, PAVISTREAM, AVICOMPRESSOPTIONS *, CLSID *);
static HRESULT (CALLBACK *qAVIStreamSetFormat)(PAVISTREAM, LONG, LPVOID, LONG);
static HRESULT (CALLBACK *qAVIStreamWrite)(PAVISTREAM, LONG, LONG, LPVOID, LONG, DWORD, LONG *, LONG *);
static ULONG (CALLBACK *qAVIStreamRelease)(PAVISTREAM);
static ULONG (CALLBACK *qAVIFileRelease)(PAVIFILE);
static void (CALLBACK *qAVIFileExit)(void);

// mingw version of msacm.h doesn't define these, and isn't included by vfw.h:
#ifndef ACMAPI
  #include <msacm.h>
  
  #define ACMAPI              WINAPI
  #define ACMERR_BASE         (512)
  #define ACMERR_NOTPOSSIBLE  (ACMERR_BASE + 0)
  #define ACMDRIVERDETAILS_SUPPORTF_CODEC     0x00000001L
  #define ACM_FORMATTAGDETAILSF_INDEX         0x00000000L
  #define ACM_STREAMSIZEF_SOURCE          0x00000000L
  #define ACM_STREAMCONVERTF_BLOCKALIGN   0x00000004
  
  DECLARE_HANDLE(HACMSTREAM);
  typedef HACMSTREAM         *PHACMSTREAM;
  typedef HACMSTREAM     FAR *LPHACMSTREAM;
  
  typedef struct tACMSTREAMHEADER
  {
    DWORD           cbStruct;               // sizeof(ACMSTREAMHEADER)
    DWORD           fdwStatus;              // ACMSTREAMHEADER_STATUSF_*
    DWORD           dwUser;                 // user instance data for hdr
    LPBYTE          pbSrc;
    DWORD           cbSrcLength;
    DWORD           cbSrcLengthUsed;
    DWORD           dwSrcUser;              // user instance data for src
    LPBYTE          pbDst;
    DWORD           cbDstLength;
    DWORD           cbDstLengthUsed;
    DWORD           dwDstUser;              // user instance data for dst
    DWORD           dwReservedDriver[10];   // driver reserved work space

  } ACMSTREAMHEADER, *PACMSTREAMHEADER, FAR *LPACMSTREAMHEADER;
#endif

// mingw version of mmreg.h doesn't define these:
#ifndef _ACM_WAVEFILTER
  #define WAVE_FORMAT_MPEGLAYER3 0x0055  /*  ISO/MPEG Layer3 Format Tag */
  #define MPEGLAYER3_WFX_EXTRA_BYTES   12
  #define MPEGLAYER3_ID_MPEG               1
  #define MPEGLAYER3_FLAG_PADDING_OFF      0x00000002
  typedef void *LPWAVEFILTER;
  
  typedef struct mpeglayer3waveformat_tag 
  {
	  WAVEFORMATEX  wfx;
	  WORD          wID;
	  DWORD         fdwFlags;
	  WORD          nBlockSize;
	  WORD          nFramesPerBlock;
	  WORD          nCodecDelay;
  } MPEGLAYER3WAVEFORMAT;
#endif

  
static MMRESULT (ACMAPI *qacmDriverOpen)(LPHACMDRIVER, HACMDRIVERID, DWORD);
static MMRESULT (ACMAPI *qacmDriverDetails)(HACMDRIVERID, LPACMDRIVERDETAILS, DWORD);
static MMRESULT (ACMAPI *qacmDriverEnum)(ACMDRIVERENUMCB, DWORD, DWORD);
static MMRESULT (ACMAPI *qacmFormatTagDetails)(HACMDRIVER, LPACMFORMATTAGDETAILS, DWORD);
static MMRESULT (ACMAPI *qacmStreamOpen)(LPHACMSTREAM, HACMDRIVER, LPWAVEFORMATEX, LPWAVEFORMATEX, LPWAVEFILTER, DWORD, DWORD, DWORD);
static MMRESULT (ACMAPI *qacmStreamSize)(HACMSTREAM, DWORD, LPDWORD, DWORD);
static MMRESULT (ACMAPI *qacmStreamPrepareHeader)(HACMSTREAM, LPACMSTREAMHEADER, DWORD);
static MMRESULT (ACMAPI *qacmStreamUnprepareHeader)(HACMSTREAM, LPACMSTREAMHEADER, DWORD);
static MMRESULT (ACMAPI *qacmStreamConvert)(HACMSTREAM, LPACMSTREAMHEADER, DWORD);
static MMRESULT (ACMAPI *qacmStreamClose)(HACMSTREAM, DWORD);
static MMRESULT (ACMAPI *qacmDriverClose)(HACMDRIVER, DWORD);

static HINSTANCE handle_avi = NULL, handle_acm = NULL;

PAVIFILE	m_file;
PAVISTREAM	m_uncompressed_video_stream;
PAVISTREAM	m_compressed_video_stream;
PAVISTREAM	m_audio_stream;

unsigned long	m_codec_fourcc;
int		m_video_frame_counter;
int		m_video_frame_size;
static	float	hack_ctr;
static	int		movie_width, movie_height;
static	byte	*movie_buffer_in, *movie_buffer_out;
static	float	movie_fps;

qboolean	m_audio_is_mp3;
int		m_audio_frame_counter;
WAVEFORMATEX	m_wave_format;
MPEGLAYER3WAVEFORMAT mp3_format;
qboolean	mp3_driver;
HACMDRIVER	had;
HACMSTREAM	hstr;
ACMSTREAMHEADER	strhdr;

extern qboolean OnChange_capturevar (cvar_t *var, const char *string);
cvar_t	capture_codec    = {"capture_codec",      "0", CVAR_FLAG_ARCHIVE, OnChange_capturevar};
cvar_t	capture_mp3	     = {"capture_mp3",        "0", CVAR_FLAG_ARCHIVE, OnChange_capturevar};
cvar_t	capture_mp3_kbps = {"capture_mp3_kbps", "128", CVAR_FLAG_ARCHIVE, OnChange_capturevar};
cvar_t	capture_hack     = {"capture_hack",       "0", CVAR_FLAG_ARCHIVE};


#define AVI_GETFUNC(f) (qAVI##f = (void *)GetProcAddress(handle_avi, "AVI" #f))
#define ACM_GETFUNC(f) (qacm##f = (void *)GetProcAddress(handle_acm, "acm" #f))
#define ACM_GETFUNCA(f) (qacm##f = (void *)GetProcAddress(handle_acm, "acm" #f "A"))

qboolean Capture_InitAVI (void)
{
	qboolean movie_avi_loaded = false;

	if (!(handle_avi = LoadLibrary("avifil32.dll")))
	{
		Con_Print ("\x02" "Avi capturing module not found\n");
		goto fail;
	}

	AVI_GETFUNC(FileInit);
	AVI_GETFUNC(FileOpen);
	AVI_GETFUNC(FileCreateStream);
	AVI_GETFUNC(MakeCompressedStream);
	AVI_GETFUNC(StreamSetFormat);
	AVI_GETFUNC(StreamWrite);
	AVI_GETFUNC(StreamRelease);
	AVI_GETFUNC(FileRelease);
	AVI_GETFUNC(FileExit);

	movie_avi_loaded = qAVIFileInit && qAVIFileOpen && qAVIFileCreateStream && 
			qAVIMakeCompressedStream && qAVIStreamSetFormat && qAVIStreamWrite && 
			qAVIStreamRelease && qAVIFileRelease && qAVIFileExit;

	if (!movie_avi_loaded)
	{
		Con_Print ("\x02" "Avi capturing module not initialized\n");
		goto fail;
	}

	Cvar_RegisterString (&capture_codec);
	Cvar_Register (&capture_hack);

	//	Con_Print ("Avi capturing module initialized\n");
	return true;

fail:
	if (handle_avi)
	{
		FreeLibrary (handle_avi);
		handle_avi = NULL;
	}
	return false;
}

qboolean Capture_InitACM (void)
{
	qboolean movie_acm_loaded = false;

	if (!(handle_acm = LoadLibrary("msacm32.dll")))
	{
		Con_Print ("\x02" "ACM module not found\n");
		goto fail;
	}

	ACM_GETFUNC(DriverOpen);
	ACM_GETFUNC(DriverEnum);
	ACM_GETFUNC(StreamOpen);
	ACM_GETFUNC(StreamSize);
	ACM_GETFUNC(StreamPrepareHeader);
	ACM_GETFUNC(StreamUnprepareHeader);
	ACM_GETFUNC(StreamConvert);
	ACM_GETFUNC(StreamClose);
	ACM_GETFUNC(DriverClose);
//	qacmDriverDetails = (void *)GetProcAddress (handle_acm, "acmDriverDetailsA");
//	qacmFormatTagDetails = (void *)GetProcAddress (handle_acm, "acmFormatTagDetailsA");
	ACM_GETFUNCA(DriverDetails);
	ACM_GETFUNCA(FormatTagDetails);

	movie_acm_loaded = qacmDriverOpen && qacmDriverDetails && qacmDriverEnum && 
			qacmFormatTagDetails && qacmStreamOpen && qacmStreamSize && 
			qacmStreamPrepareHeader && qacmStreamUnprepareHeader && 
			qacmStreamConvert && qacmStreamClose && qacmDriverClose;

	if (!movie_acm_loaded)
	{
		Con_Print ("\x02" "ACM module not initialized\n");
		goto fail;
	}

	Cvar_RegisterBool (&capture_mp3);
	Cvar_Register (&capture_mp3_kbps);

	//	Con_Print ("ACM module initialized\n");
	return true;

fail:
	if (handle_acm)
	{
		FreeLibrary (handle_acm);
		handle_acm = NULL;
	}
	return false;
}

PAVISTREAM Capture_VideoStream (void)
{
	return m_codec_fourcc ? m_compressed_video_stream : m_uncompressed_video_stream;
}

BOOL CALLBACK acmDriverEnumCallback (HACMDRIVERID hadid, DWORD dwInstance, DWORD fdwSupport)
{
	if (fdwSupport & ACMDRIVERDETAILS_SUPPORTF_CODEC)
	{
		int	i;
		ACMDRIVERDETAILS details;

		details.cbStruct = sizeof(details);
		qacmDriverDetails (hadid, &details, 0);
		qacmDriverOpen (&had, hadid, 0);

		for (i = 0 ; i < details.cFormatTags ; i++)
		{
			ACMFORMATTAGDETAILS	fmtDetails;

			memset (&fmtDetails, 0, sizeof(fmtDetails));
			fmtDetails.cbStruct = sizeof(fmtDetails);
			fmtDetails.dwFormatTagIndex = i;
			qacmFormatTagDetails (had, &fmtDetails, ACM_FORMATTAGDETAILSF_INDEX);
			if (fmtDetails.dwFormatTag == WAVE_FORMAT_MPEGLAYER3)
			{
				Con_DPrintf ("MP3-capable ACM codec found: %s\n", details.szLongName);
				mp3_driver = true;

				return false;
			}
		}

		qacmDriverClose (had, 0);
	}

	return true;
}

qboolean Capture_Open (const char *filename, int width, int height, float fps)
{
	HRESULT			hr;
	BITMAPINFOHEADER	bitmap_info_header;
	AVISTREAMINFO		stream_header;
	char			*fourcc;

	hack_ctr = capture_hack.value;

	m_video_frame_counter = m_audio_frame_counter = 0;
	m_file = NULL;
	m_codec_fourcc = 0;
	m_uncompressed_video_stream = m_compressed_video_stream = m_audio_stream = NULL;
	m_audio_is_mp3 = (qboolean)capture_mp3.value;

	if (*(fourcc = capture_codec.string) != '0')	// codec fourcc supplied
		m_codec_fourcc = mmioFOURCC (fourcc[0], fourcc[1], fourcc[2], fourcc[3]);

	qAVIFileInit ();
	hr = qAVIFileOpen (&m_file, filename, OF_WRITE | OF_CREATE, NULL);
	if (FAILED(hr))
	{
		Con_Print ("ERROR: Couldn't open AVI file\n");
		return false;
	}

	// initialize video data
	m_video_frame_size = width * height * 3;

	memset (&bitmap_info_header, 0, sizeof(bitmap_info_header));
	bitmap_info_header.biSize = sizeof(BITMAPINFOHEADER);
	bitmap_info_header.biWidth = width;
	bitmap_info_header.biHeight = height;
	bitmap_info_header.biPlanes = 1;
	bitmap_info_header.biBitCount = 24;
	bitmap_info_header.biCompression = BI_RGB;
	bitmap_info_header.biSizeImage = m_video_frame_size;

	memset (&stream_header, 0, sizeof(stream_header));
	stream_header.fccType = streamtypeVIDEO;
	stream_header.fccHandler = m_codec_fourcc;
	stream_header.dwScale = 1;
	stream_header.dwRate = (unsigned long)(0.5 + fps);
	stream_header.dwSuggestedBufferSize = bitmap_info_header.biSizeImage;
	SetRect (&stream_header.rcFrame, 0, 0, bitmap_info_header.biWidth, bitmap_info_header.biHeight);

	hr = qAVIFileCreateStream (m_file, &m_uncompressed_video_stream, &stream_header);
	if (FAILED(hr))
	{
		Con_Print ("ERROR: Couldn't create video stream\n");
		return false;
	}

	if (m_codec_fourcc)
	{
		AVICOMPRESSOPTIONS	opts;

		memset (&opts, 0, sizeof(opts));
		opts.fccType = stream_header.fccType;
		opts.fccHandler = m_codec_fourcc;

		// Make the stream according to compression
		hr = qAVIMakeCompressedStream (&m_compressed_video_stream, m_uncompressed_video_stream, &opts, NULL);
		if (FAILED(hr))
		{
			Con_Print ("ERROR: Couldn't make compressed video stream\n");
			return false;
		}
	}

	hr = qAVIStreamSetFormat (Capture_VideoStream(), 0, &bitmap_info_header, bitmap_info_header.biSize);
	if (FAILED(hr))
	{
		Con_Print ("ERROR: Couldn't set video stream format\n");
		return false;
	}

	if (shm)
	{
		// initialize audio data
		memset (&m_wave_format, 0, sizeof(m_wave_format));
		m_wave_format.wFormatTag = WAVE_FORMAT_PCM;
		m_wave_format.nChannels = 2;		// always stereo in Quake sound engine
		m_wave_format.nSamplesPerSec = shm->speed;
		m_wave_format.wBitsPerSample = 16;	// always 16bit in Quake sound engine
		m_wave_format.nBlockAlign = m_wave_format.wBitsPerSample/8 * m_wave_format.nChannels;
		m_wave_format.nAvgBytesPerSec = m_wave_format.nSamplesPerSec * m_wave_format.nBlockAlign;

		memset (&stream_header, 0, sizeof(stream_header));
		stream_header.fccType = streamtypeAUDIO;
		stream_header.dwScale = m_wave_format.nBlockAlign;
		stream_header.dwRate = stream_header.dwScale * (unsigned long)m_wave_format.nSamplesPerSec;
		stream_header.dwSampleSize = m_wave_format.nBlockAlign;

		hr = qAVIFileCreateStream (m_file, &m_audio_stream, &stream_header);
		if (FAILED(hr))
		{
			Con_Print ("ERROR: Couldn't create audio stream\n");
			return false;
		}

		if (m_audio_is_mp3)
		{
			MMRESULT	mmr;

			// try to find an MP3 codec
			had = NULL;
			mp3_driver = false;
			qacmDriverEnum (acmDriverEnumCallback, 0, 0);
			if (!mp3_driver)
			{
				Con_Print ("ERROR: Couldn't find any MP3 decoder\n");
				return false;
			}

			memset (&mp3_format, 0, sizeof(mp3_format));
			mp3_format.wfx.wFormatTag = WAVE_FORMAT_MPEGLAYER3;
			mp3_format.wfx.nChannels = 2;
			mp3_format.wfx.nSamplesPerSec = shm->speed;
			mp3_format.wfx.wBitsPerSample = 0;
			mp3_format.wfx.nBlockAlign = 1;
			mp3_format.wfx.nAvgBytesPerSec = capture_mp3_kbps.value * 125;
			mp3_format.wfx.cbSize = MPEGLAYER3_WFX_EXTRA_BYTES;
			mp3_format.wID = MPEGLAYER3_ID_MPEG;
			mp3_format.fdwFlags = MPEGLAYER3_FLAG_PADDING_OFF;
			mp3_format.nBlockSize = mp3_format.wfx.nAvgBytesPerSec / fps;
			mp3_format.nFramesPerBlock = 1;
			mp3_format.nCodecDelay = 1393;

			hstr = NULL;
			if ((mmr = qacmStreamOpen(&hstr, had, &m_wave_format, &mp3_format.wfx, NULL, 0, 0, 0)))
			{
				switch (mmr)
				{
				case MMSYSERR_INVALPARAM:
					Con_Print ("ERROR: Invalid parameters passed to acmStreamOpen\n");
					return false;

				case ACMERR_NOTPOSSIBLE:
					Con_Print ("ERROR: No ACM filter found capable of decoding MP3\n");
					return false;

				default:
					Con_Print ("ERROR: Couldn't open ACM decoding stream\n");
					return false;
				}
			}

			hr = qAVIStreamSetFormat (m_audio_stream, 0, &mp3_format, sizeof(MPEGLAYER3WAVEFORMAT));
			if (FAILED(hr))
			{
				Con_Print ("ERROR: Couldn't set audio stream format\n");
				return false;
			}
		}
		else
		{
			hr = qAVIStreamSetFormat (m_audio_stream, 0, &m_wave_format, sizeof(WAVEFORMATEX));
			if (FAILED(hr))
			{
				Con_Print ("ERROR: Couldn't set audio stream format\n");
				return false;
			}
		}
	}

	movie_width = width;
	movie_height = height;
	movie_fps = fps;
	movie_buffer_in = Q_malloc (glwidth * glheight * 3);
	movie_buffer_out = Q_malloc (width * height * 3);
	return true;
}

void Capture_Close (void)
{
	if (m_uncompressed_video_stream)
		qAVIStreamRelease (m_uncompressed_video_stream);
	if (m_compressed_video_stream)
		qAVIStreamRelease (m_compressed_video_stream);
	if (m_audio_stream)
		qAVIStreamRelease (m_audio_stream);
	if (m_audio_is_mp3)
	{
		qacmStreamClose (hstr, 0);
		qacmDriverClose (had, 0);
	}
	if (m_file)
		qAVIFileRelease (m_file);

	if (movie_buffer_in)
		free (movie_buffer_in);
	if (movie_buffer_out)
		free (movie_buffer_out);
	
	qAVIFileExit ();
}

qboolean Capture_WriteVideo (void)
{
//#ifdef GLQUAKE
	int		i, size = glwidth * glheight;
//	byte	temp;
	byte	*outbuf;
//#else
//	int	size = vid.width * vid.height;
//	int	i, j, rowp;
//	byte	*buffer, *p;
//#endif
	HRESULT	hr;
	extern	unsigned short	ramps[3][256];
#ifdef _DEBUG
	double time1 = Sys_DoubleTime(), time2;
#endif

	size *= 3;
	
	// check frame size (TODO: other things too?) hasn't changed
/*	if (m_video_frame_size != size)
	{
		Con_Print ("ERROR: Frame size changed\n");
		return;
	}
*/
	if (capture_hack.value)
	{
		if (hack_ctr != capture_hack.value)
		{
			if (!hack_ctr)
				hack_ctr = capture_hack.value;
			else
				hack_ctr--;
			return;
		}
		hack_ctr--;
	}

//#ifdef GLQUAKE
	glReadPixels (glx, gly, glwidth, glheight, GL_BGR_EXT, GL_UNSIGNED_BYTE, movie_buffer_in);

#ifdef _DEBUG
	time2 = Sys_DoubleTime();
	Con_Printf ("Frame read:  %lf\n", time2-time1);
	time1 = time2;
#endif

	if (vid_hwgamma_enabled)
	{
		for (i=0 ; i<size ; i+=3)
		{
			movie_buffer_in[i+0] = ramps[2][movie_buffer_in[i+0]] >> 8;
			movie_buffer_in[i+1] = ramps[1][movie_buffer_in[i+1]] >> 8;
			movie_buffer_in[i+2] = ramps[0][movie_buffer_in[i+2]] >> 8;
		}
	}

/*	glReadPixels (glx, gly, glwidth, glheight, GL_RGB, GL_UNSIGNED_BYTE, movie_buffer_in);
	ApplyGamma (movie_buffer_in, size);

	for (i = 0 ; i < size ; i += 3)
	{
		temp = movie_buffer_in[i];
		movie_buffer_in[i] = movie_buffer_in[i+2];
		movie_buffer_in[i+2] = temp;
	}
*/
//#else
/*	buffer = Q_malloc (vid.width * vid.height * 3);

	D_EnableBackBufferAccess ();

	p = buffer;
	for (i = vid.height - 1 ; i >= 0 ; i--)
	{
		rowp = i * vid.rowbytes;
		for (j = 0 ; j < vid.width ; j++)
		{
			*p++ = current_pal[vid.buffer[rowp]*3+2];
			*p++ = current_pal[vid.buffer[rowp]*3+1];
			*p++ = current_pal[vid.buffer[rowp]*3+0];
			rowp++;
		}
	}

	D_DisableBackBufferAccess ();
#endif*/

	outbuf = Movie_ScaleDownBGR (movie_buffer_in, glwidth, glheight, movie_buffer_out, movie_width, movie_height);

	if (!Capture_VideoStream())
	{
		Con_Print ("ERROR: Video stream is NULL\n");
		//goto EXITPOINT;
		return false;
	}

	hr = qAVIStreamWrite (Capture_VideoStream(), m_video_frame_counter++, 1, outbuf, m_video_frame_size, AVIIF_KEYFRAME, NULL, NULL);
	if (FAILED(hr))
	{
		Con_Print ("ERROR: Couldn't write video stream\n");
		//goto EXITPOINT;
		return false;
	}

#ifdef _DEBUG
	time2 = Sys_DoubleTime();
	Con_Printf ("Frame write: %lf\n", time2-time1);
#endif

/*EXITPOINT:
	free (buffer);*/
	return true;
}

qboolean Capture_WriteAudio (byte *sample_buffer, int samples)
{
	HRESULT		hr;
	unsigned long	sample_bufsize;

	if (!m_audio_stream)
	{
		Con_Print ("ERROR: Audio stream is NULL\n");
		return;
	}

	sample_bufsize = samples * m_wave_format.nBlockAlign;
	if (m_audio_is_mp3)
	{
		MMRESULT	mmr;
		byte		*mp3_buffer;
		unsigned long	mp3_bufsize;

		if ((mmr = qacmStreamSize(hstr, sample_bufsize, &mp3_bufsize, ACM_STREAMSIZEF_SOURCE)))
		{
			Con_Print ("ERROR: Couldn't get mp3bufsize\n");
			return;
		}
		if (!mp3_bufsize)
		{
			Con_Print ("ERROR: mp3bufsize is zero\n");
			return false;
		}
		mp3_buffer = Q_calloc (mp3_bufsize, 1);

		memset (&strhdr, 0, sizeof(strhdr));
		strhdr.cbStruct = sizeof(strhdr);
		strhdr.pbSrc = sample_buffer;
		strhdr.cbSrcLength = sample_bufsize;
		strhdr.pbDst = mp3_buffer;
		strhdr.cbDstLength = mp3_bufsize;

		if ((mmr = qacmStreamPrepareHeader(hstr, &strhdr, 0)))
		{
			Con_Print ("ERROR: Couldn't prepare header\n");
			free (mp3_buffer);
			return false;
		}

		if ((mmr = qacmStreamConvert(hstr, &strhdr, ACM_STREAMCONVERTF_BLOCKALIGN)))
		{
			Con_Print ("ERROR: Couldn't convert audio stream\n");
		}
		else
		{
			hr = qAVIStreamWrite (m_audio_stream, m_audio_frame_counter++, 1, mp3_buffer, strhdr.cbDstLengthUsed, AVIIF_KEYFRAME, NULL, NULL);
		}

		if ((mmr = qacmStreamUnprepareHeader(hstr, &strhdr, 0)))
		{
			Con_Print ("ERROR: Couldn't unprepare header\n");
			free (mp3_buffer);
			return false;
		}

		free (mp3_buffer);
	}
	else
	{
		hr = qAVIStreamWrite (m_audio_stream, m_audio_frame_counter++, 1, sample_buffer, sample_bufsize, AVIIF_KEYFRAME, NULL, NULL);
	}
	if (FAILED(hr))
	{
		Con_Print ("ERROR: Couldn't write audio stream\n");
		return false;
	}

	return true;
}

double Capture_FrameTime (void)
{
	double	time;

	if (!capture_hack.value)
		time = 1.0 / movie_fps;
	else
		time = 1.0 / (movie_fps * (capture_hack.value + 1.0));

	return bound(1.0 / 1000, time, 1.0);
}

qboolean Capture_GetSoundtime (void)
{
//	soundtime += (int)(0.5 + host_frametime * shm->speed * (Capture_FrameTime() / host_frametime));
	soundtime += (int)(0.5 + shm->speed * Capture_FrameTime());
	return true;
}

#endif		//#ifdef QWINAVI

#endif		//#ifndef RQM_SV_ONLY

#endif		//#ifdef _WIN32
