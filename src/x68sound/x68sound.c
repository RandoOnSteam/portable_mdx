#include <x68sound_context.h>
#include "x68sound_config.h"

#include "x68sound_global.h"
#include "x68sound.h"
#include "x68sound_op.h"
#include "x68sound_lfo.h"
#include "x68sound_adpcm.h"
#include "x68sound_pcm8.h"
#include "x68sound_opm.h"

#if X68SOUND_ENABLE_PORTABLE_CODE
	#include "x68sound_context.internal.h"
#endif

#if defined(__cplusplus)
extern "C" {
#endif

#if X68SOUND_ENABLE_PORTABLE_CODE
	/* X68SoundContext に移動 */
#else
OPM	opm;
#endif
//MMTIME	mmt;

#if !X68SOUND_ENABLE_PORTABLE_CODE
void CALLBACK waveOutProc(HWAVEOUT hwo, UINT uMsg, DWORD dwInstance,
						  DWORD dwParam1, DWORD dwParam2) {
	if (uMsg == WOM_DONE && thread_flag) {
		int playptr;
		int genptr;
		timer_start_flag = 1;	// マルチメディアタイマーの処理を開始


		playingblk = (playingblk+1) & (N_waveblk-1);
		playptr = playingblk * Blk_Samples;

		genptr = opm.PcmBufPtr;
		if (genptr < playptr) {
			genptr += opm.PcmBufSize;
		}
		genptr -= playptr;
		if (genptr <= Late_Samples) {
			if (Late_Samples-Faster_Limit <= genptr) {
				// 音生成が遅れている
				nSamples = Betw_Samples_Faster;
			} else {
				// 音生成が進みすぎている
//				nSamples = Betw_Samples_VerySlower;
				// 音生成が遅れすぎている
//				setPcmBufPtr = ((playingblk+1)&(N_waveblk-1)) * Blk_Samples;
				unsigned int ptr = playptr + Late_Samples + Betw_Samples_Faster;
				while (ptr >= opm.PcmBufSize) ptr -= opm.PcmBufSize;
				setPcmBufPtr = ptr;
			}
		} else {
			if (genptr <= Late_Samples+Slower_Limit) {
				// 音生成が進んでいる
				nSamples = Betw_Samples_Slower;
			} else {
				// 音生成が進みすぎている
//				nSamples = Betw_Samples_VerySlower;
//				setPcmBufPtr = ((playingblk+1)&(N_waveblk-1)) * Blk_Samples;
				unsigned int ptr = playptr + Late_Samples + Betw_Samples_Faster;
				while (ptr >= opm.PcmBufSize) ptr -= opm.PcmBufSize;
				setPcmBufPtr = ptr;
			}
		}

		PostThreadMessage(thread_id, THREADMES_WAVEOUTDONE, 0, 0);
	}
}
#endif

#if !X68SOUND_ENABLE_PORTABLE_CODE
DWORD WINAPI waveOutThread( LPVOID lpContext ) {
	MSG Msg;
	(void)(lpContext);
	thread_flag = 1;

	while (GetMessage( &Msg, NULL, 0, 0)) {
		if (Msg.message == THREADMES_WAVEOUTDONE) {

			waveOutWrite(hwo, lpwh+waveblk, sizeof(WAVEHDR));

			++waveblk;
			if (waveblk >= N_waveblk) {
				waveblk = 0;
			}

		} else if (Msg.message == THREADMES_KILL) {
			waveOutReset(hwo);
			break;
		}
	}

	thread_flag = 0;
	return 0;
}
#endif


#if !X68SOUND_ENABLE_PORTABLE_CODE
// マルチメディアタイマー
void CALLBACK OpmTimeProc(UINT uID, UINT uMsg, DWORD dwUser, DWORD dw1, DWORD dw2) {
		if (!timer_start_flag) return;

//if (opm.PcmBufPtr/Blk_Samples == ((playingblk-1)&(N_waveblk-1))) return;
		if (setPcmBufPtr != -1) {
			opm.PcmBufPtr = setPcmBufPtr;
			setPcmBufPtr = -1;
		}

		Opm_PushRegs(&opm);

		if (Samprate != 22050) {
			Opm_pcmset62(&opm, nSamples);
		} else {
			Opm_pcmset22(&opm, nSamples);
		}

//		Opm_timer(&opm);
		Opm_betwint(&opm);


		Opm_PopRegs(&opm);


/*
		if (opm.adpcm.DmaReg[0x00] & 0x10) {
			if (opm.adpcm.DmaReg[0x07] & 0x08) {	// INT==1?
				if (opm.adpcm.ErrIntProc != NULL) {
					opm.adpcm.ErrIntProc();
				}
			}
		} else if (opm.adpcm.DmaReg[0x00] & 0x10) {
			if (opm.adpcm.DmaReg[0x07] & 0x08) {	// INT==1?
				if (opm.adpcm.IntProc != NULL) {
					opm.adpcm.IntProc();
				}
			}
		}
*/

}
#endif




#if X68SOUND_ENABLE_PORTABLE_CODE
int X68Sound_Start(X68SoundContext *context, int samprate, int opmflag, int adpcmflag,
				  int betw, int pcmbuf, int late, double rev) {
	return Opm_Start(&context->m_impl->m_opm, samprate, opmflag, adpcmflag, betw, pcmbuf, late, rev);
}
int X68Sound_Samprate(X68SoundContext *context, int samprate) {
	return Opm_SetSamprate(&context->m_impl->m_opm, samprate);
}
int X68Sound_OpmClock(X68SoundContext *context, int clock) {
	return Opm_SetOpmClock(&context->m_impl->m_opm, clock);
}
void X68Sound_Reset(X68SoundContext *context) {
	Opm_Reset(&context->m_impl->m_opm);
}
void X68Sound_Free(X68SoundContext *context) {
	Opm_Free(&context->m_impl->m_opm);
}
void X68Sound_BetwInt(X68SoundContext *context, void (*proc)(void *), void *arg) {
	Opm_BetwInt(&context->m_impl->m_opm, proc, arg);
}

int X68Sound_StartPcm(X68SoundContext *context, int samprate, int opmflag, int adpcmflag, int pcmbuf) {
	return Opm_StartPcm(&context->m_impl->m_opm, samprate, opmflag, adpcmflag, pcmbuf);
}
int X68Sound_GetPcm(X68SoundContext *context, void *buf, int len) {
	return Opm_GetPcm(&context->m_impl->m_opm, buf, len);
}

unsigned char X68Sound_OpmPeek(X68SoundContext *context) {
	return Opm_OpmPeek(&context->m_impl->m_opm);
}
void X68Sound_OpmReg(X68SoundContext *context, unsigned char no) {
	Opm_OpmReg(&context->m_impl->m_opm, no);
}
void X68Sound_OpmPoke(X68SoundContext *context, unsigned char data) {
	Opm_OpmPoke(&context->m_impl->m_opm, data);
}
void X68Sound_OpmInt(X68SoundContext *context, void (*proc)(void *), void *arg) {
	Opm_OpmInt(&context->m_impl->m_opm, proc, arg);
}
int X68Sound_OpmWait(X68SoundContext *context, int wait) {
	return Opm_SetOpmWait(&context->m_impl->m_opm, wait);
}

unsigned char X68Sound_AdpcmPeek(X68SoundContext *context) {
	return Opm_AdpcmPeek(&context->m_impl->m_opm);
}
void X68Sound_AdpcmPoke(X68SoundContext *context, unsigned char data) {
	Opm_AdpcmPoke(&context->m_impl->m_opm, data);
}
unsigned char X68Sound_PpiPeek(X68SoundContext *context) {
	return Opm_PpiPeek(&context->m_impl->m_opm);
}
void X68Sound_PpiPoke(X68SoundContext *context, unsigned char data) {
	Opm_PpiPoke(&context->m_impl->m_opm, data);
}
void X68Sound_PpiCtrl(X68SoundContext *context, unsigned char data) {
	Opm_PpiCtrl(&context->m_impl->m_opm, data);
}
unsigned char X68Sound_DmaPeek(X68SoundContext *context, unsigned char adrs) {
	return Opm_DmaPeek(&context->m_impl->m_opm, adrs);
}
void X68Sound_DmaPoke(X68SoundContext *context, unsigned char adrs, unsigned char data) {
	Opm_DmaPoke(&context->m_impl->m_opm, adrs, data);
}
void X68Sound_DmaInt(X68SoundContext *context, void (*proc)(void *), void *arg) {
	Opm_DmaInt(&context->m_impl->m_opm, proc, arg);
}
void X68Sound_DmaErrInt(X68SoundContext *context, void (*proc)(void *), void *arg) {
	Opm_DmaErrInt(&context->m_impl->m_opm, proc, arg);
}
void X68Sound_MemReadFunc(X68SoundContext *context, int (*func)(unsigned char *)) {
	Opm_MemReadFunc(&context->m_impl->m_opm, func);
}

void X68Sound_WaveFunc(X68SoundContext *context, int (*func)(void *), void *arg) {
	Opm_SetWaveFunc(&context->m_impl->m_opm, func, arg);
}

int X68Sound_Pcm8_Out(X68SoundContext *context, int ch, void *adrs, int mode, int len) {
	return Opm_Pcm8_Out(&context->m_impl->m_opm, ch, adrs, mode, len);
}
int X68Sound_Pcm8_Aot(X68SoundContext *context, int ch, void *tbl, int mode, int cnt) {
	return Opm_Pcm8_Aot(&context->m_impl->m_opm, ch, tbl, mode, cnt);
}
int X68Sound_Pcm8_Lot(X68SoundContext *context, int ch, void *tbl, int mode) {
	return Opm_Pcm8_Lot(&context->m_impl->m_opm, ch, tbl, mode);
}
int X68Sound_Pcm8_SetMode(X68SoundContext *context, int ch, int mode) {
	return Opm_Pcm8_SetMode(&context->m_impl->m_opm, ch, mode);
}
int X68Sound_Pcm8_GetRest(X68SoundContext *context, int ch) {
	return Opm_Pcm8_GetRest(&context->m_impl->m_opm, ch);
}
int X68Sound_Pcm8_GetMode(X68SoundContext *context, int ch) {
	return Opm_Pcm8_GetMode(&context->m_impl->m_opm, ch);
}
int X68Sound_Pcm8_Abort(X68SoundContext *context) {
	return Opm_Pcm8_Abort(&context->m_impl->m_opm);
}


int X68Sound_TotalVolume(X68SoundContext *context, int v) {
	return Opm_SetTotalVolume(&context->m_impl->m_opm, v);
}
int X68Sound_GetTotalVolume(X68SoundContext *context) {
	return Opm_GetTotalVolume(&context->m_impl->m_opm);
}


int X68Sound_ErrorCode(X68SoundContext *context) {
	return context->m_impl->m_ErrorCode;
}
int X68Sound_DebugValue(X68SoundContext *context) {
	return context->m_impl->m_DebugValue;
}
#else
int X68Sound_Start(int samprate, int opmflag, int adpcmflag,
				  int betw, int pcmbuf, int late, double rev) {
	return Opm_Start(&opm, samprate, opmflag, adpcmflag, betw, pcmbuf, late, rev);
}
int X68Sound_Samprate(int samprate) {
	return Opm_SetSamprate(&opm, samprate);
}
int X68Sound_OpmClock(int clock) {
	return Opm_SetOpmClock(&opm, clock);
}
void X68Sound_Reset() {
	Opm_Reset(&opm);
}
void X68Sound_Free() {
	Opm_Free(&opm);
}
void X68Sound_BetwInt(void (CALLBACK *proc)()) {
	Opm_BetwInt(&opm, proc);
}

int X68Sound_StartPcm(int samprate, int opmflag, int adpcmflag, int pcmbuf) {
	return Opm_StartPcm(&opm, samprate, opmflag, adpcmflag, pcmbuf);
}
int X68Sound_GetPcm(void *buf, int len) {
	return Opm_GetPcm(&opm, buf, len);
}

unsigned char X68Sound_OpmPeek() {
	return Opm_OpmPeek(&opm);
}
void X68Sound_OpmReg(unsigned char no) {
	Opm_OpmReg(&opm, no);
}
void X68Sound_OpmPoke(unsigned char data) {
	Opm_OpmPoke(&opm, data);
}
void X68Sound_OpmInt(void (CALLBACK *proc)()) {
	Opm_OpmInt(&opm, proc);
}
int X68Sound_OpmWait(int wait) {
	return Opm_SetOpmWait(&opm, wait);
}

unsigned char X68Sound_AdpcmPeek() {
	return Opm_AdpcmPeek(&opm);
}
void X68Sound_AdpcmPoke(unsigned char data) {
	Opm_AdpcmPoke(&opm, data);
}
unsigned char X68Sound_PpiPeek() {
	return Opm_PpiPeek(&opm);
}
void X68Sound_PpiPoke(unsigned char data) {
	Opm_PpiPoke(&opm, data);
}
void X68Sound_PpiCtrl(unsigned char data) {
	Opm_PpiCtrl(&opm, data);
}
unsigned char X68Sound_DmaPeek(unsigned char adrs) {
	return Opm_DmaPeek(&opm, adrs);
}
void X68Sound_DmaPoke(unsigned char adrs, unsigned char data) {
	Opm_DmaPoke(&opm, adrs, data);
}
void X68Sound_DmaInt(void (CALLBACK *proc)()) {
	Opm_DmaInt(&opm, proc);
}
void X68Sound_DmaErrInt(void (CALLBACK *proc)()) {
	Opm_DmaErrInt(&opm, proc);
}
void X68Sound_MemReadFunc(int (CALLBACK *func)(unsigned char *)) {
	Opm_MemReadFunc(&opm, func);
}

void X68Sound_WaveFunc(int (CALLBACK *func)()) {
	Opm_SetWaveFunc(&opm, func);
}

int X68Sound_Pcm8_Out(int ch, void *adrs, int mode, int len) {
	return Opm_Pcm8_Out(&opm, ch, adrs, mode, len);
}
int X68Sound_Pcm8_Aot(int ch, void *tbl, int mode, int cnt) {
	return Opm_Pcm8_Aot(&opm, ch, tbl, mode, cnt);
}
int X68Sound_Pcm8_Lot(int ch, void *tbl, int mode) {
	return Opm_Pcm8_Lot(&opm, ch, tbl, mode);
}
int X68Sound_Pcm8_SetMode(int ch, int mode) {
	return Opm_Pcm8_SetMode(&opm, ch, mode);
}
int X68Sound_Pcm8_GetRest(int ch) {
	return Opm_Pcm8_GetRest(&opm, ch);
}
int X68Sound_Pcm8_GetMode(int ch) {
	return Opm_Pcm8_GetMode(&opm, ch);
}
int X68Sound_Pcm8_Abort() {
	return Opm_Pcm8_Abort(&opm);
}


int X68Sound_TotalVolume(int v) {
	return Opm_SetTotalVolume(&opm, v);
}
int X68Sound_GetTotalVolume(void) {
	return Opm_GetTotalVolume(&opm);
}








int X68Sound_ErrorCode() {
	return ErrorCode;
}
int X68Sound_DebugValue() {
	return DebugValue;
}
#endif
#if defined(__cplusplus)
}
#endif
