#ifndef __X68SOUND_H__
#define __X68SOUND_H__

#ifdef __cplusplus
extern "C" {
#endif

#define X68SNDERR_PCMOUT		(-1)
#define X68SNDERR_TIMER			(-2)
#define X68SNDERR_MEMORY		(-3)
#define X68SNDERR_NOTACTIVE		(-4)
#define X68SNDERR_ALREADYACTIVE	(-5)
#define X68SNDERR_BADARG		(-6)

#define X68SNDERR_DLL			(-1)
#define X68SNDERR_FUNC			(-2)

typedef struct tagX68SoundContext tagX68SoundContext;
int X68Sound_Start(tagX68SoundContext *context, int samprate/*=44100*/, int opmflag/*=1*/, int adpcmflag/*=1*/,
		  int betw/*=5*/, int pcmbuf/*=5*/, int late/*=200*/, double rev/*=1.0*/);
int X68Sound_Samprate(tagX68SoundContext *context, int samprate/*=44100*/);
void X68Sound_Reset(tagX68SoundContext *context);
void X68Sound_Free(tagX68SoundContext *context);
void X68Sound_BetwInt(tagX68SoundContext *context, void (*proc)(void *), void *arg);

int X68Sound_StartPcm(tagX68SoundContext *context, int samprate/*=44100*/, int opmflag/*=1*/, int adpcmflag/*=1*/, int pcmbuf/*=5*/);
int X68Sound_GetPcm(tagX68SoundContext *context, void *buf, int len);

unsigned char X68Sound_OpmPeek(tagX68SoundContext *context);
void X68Sound_OpmReg(tagX68SoundContext *context, unsigned char no);
void X68Sound_OpmPoke(tagX68SoundContext *context, unsigned char data);
void X68Sound_OpmInt(tagX68SoundContext *context, void (*proc)(void *), void *arg);
int X68Sound_OpmWait(tagX68SoundContext *context, int wait/*=240*/);
int X68Sound_OpmClock(tagX68SoundContext *context, int clock/*=4000000*/);

unsigned char X68Sound_AdpcmPeek(tagX68SoundContext *context);
void X68Sound_AdpcmPoke(tagX68SoundContext *context, unsigned char data);
unsigned char X68Sound_PpiPeek(tagX68SoundContext *context);
void X68Sound_PpiPoke(tagX68SoundContext *context, unsigned char data);
void X68Sound_PpiCtrl(tagX68SoundContext *context, unsigned char data);
unsigned char X68Sound_DmaPeek(tagX68SoundContext *context, unsigned char adrs);
void X68Sound_DmaPoke(tagX68SoundContext *context, unsigned char adrs, unsigned char data);
void X68Sound_DmaInt(tagX68SoundContext *context, void (*proc)(void *), void *arg);
void X68Sound_DmaErrInt(tagX68SoundContext *context, void (*Proc)(void *), void *arg);
void X68Sound_MemReadFunc(tagX68SoundContext *context, int (*func)(unsigned char *));

void X68Sound_WaveFunc(tagX68SoundContext *context, int (*func)(void *), void *arg);
int X68Sound_Pcm8_Out(tagX68SoundContext *context, int ch, void *adrs, int mode, int len);
int X68Sound_Pcm8_Aot(tagX68SoundContext *context, int ch, void *tbl, int mode, int cnt);
int X68Sound_Pcm8_Lot(tagX68SoundContext *context, int ch, void *tbl, int mode);
int X68Sound_Pcm8_SetMode(tagX68SoundContext *context, int ch, int mode);
int X68Sound_Pcm8_GetRest(tagX68SoundContext *context, int ch);
int X68Sound_Pcm8_GetMode(tagX68SoundContext *context, int ch);
int X68Sound_Pcm8_Abort(tagX68SoundContext *context);

int X68Sound_TotalVolume(tagX68SoundContext *context, int v);
int X68Sound_GetTotalVolume(tagX68SoundContext *context);

int X68Sound_ErrorCode(tagX68SoundContext *context);
int X68Sound_DebugValue(tagX68SoundContext *context);

#ifdef __cplusplus
}
#endif

#endif //__X68SOUND_H__
