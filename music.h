
#ifndef __MUSIC_H__
#define __MUSIC_H__

#ifndef QUAKE_GAME
#include "quakedef.h"
#endif

qboolean Music_Init (void);
qboolean Music_IsInitialized (void);
void Music_Shutdown (void);

void Music_ChangeVolume (float value);		// value is 0-1
void Music_ChangeVolume_f (cmd_source_t src);

void Music_Stop_f (cmd_source_t src);
void Music_Pause_f (cmd_source_t src);
void Music_Resume (void);

qboolean Music_PlayTrack (cmd_source_t src, byte track, qboolean loop);

#ifdef HEXEN2_SUPPORT
  void Music_PlayMIDI_f (cmd_source_t src);
  void Music_PlayMIDI (cmd_source_t src, const char *name);

  void Music_Loop_f (cmd_source_t src);
#endif

#endif
