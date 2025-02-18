/* Copyright (C) 2018 Yosshin(@yosshin4004) */

#ifndef __X68SOUND_CONTEXT_INTERNAL_H__
#define __X68SOUND_CONTEXT_INTERNAL_H__

#include <x68sound_context.h>
#include "x68sound_config.h"
#include "x68sound_opm.h"

typedef struct tagX68SoundContextImpl {
	void *m_dmaBase;

	void (*m_OpmFir)(const short *, const short *, const short *, int *);

	int m_DebugValue;
	int m_ErrorCode;
	int m_Samprate;
	int m_WaveOutSamp;
	int m_OpmWait;
	int m_OpmRate;

	int m_STEPTBL[11*12*64];
	#define ALPHAZERO	(SIZEALPHATBL*3)
	unsigned short m_ALPHATBL[ALPHAZERO+SIZEALPHATBL+1];
	short m_SINTBL[SIZESINTBL];

	int m_D1LTBL[16];
	int m_DT1TBL[128+4];

	unsigned short	m_NOISEALPHATBL[ALPHAZERO+SIZEALPHATBL+1];

	int (*m_MemRead)(unsigned char *adrs);

	int m_TotalVolume;	// 音量 x/256

	volatile int m_Semapho;
	volatile int m_TimerSemapho;

	int m_OPMLPF_ROW;
	const short (*m_OPMLOWPASS)[OPMLPF_COL];

	int m_Betw_Time;		// 5 ms
	int m_Late_Time;		// (200+Bet_time) ms
	int m_Late_Samples;	// (44100*Late_Time/1000)
	int m_Blk_Samples;	// 44100/N_waveblk
	int m_Betw_Samples_Slower;	// floor(44100.0*5/1000.0-rev)
	int m_Betw_Samples_Faster;	// ceil(44100.0*5/1000.0+rev)
	int m_Betw_Samples_VerySlower;	// floor(44100.0*5/1000.0-rev)/4.0
	int m_Slower_Limit,m_Faster_Limit;
	unsigned int	m_TimerResolution;
	//unsigned int	m_SamplesCounter=0;
	//unsigned int	m_SamplesCounterRev=0;
	unsigned int	m_nSamples;

	//const int m_N_waveblk=8;
	int m_N_waveblk;
	int m_waveblk;
	int m_playingblk, m_playingblk_next;
	volatile int m_setPcmBufPtr;

	int m_RandSeed;

	OPM m_opm;	/* 配置 new delete が必要 */
} X68SoundContextImpl;

static inline DWORD X68SoundContextImpl_ToOfs(
	const X68SoundContextImpl *contextImpl,
	const volatile void *ptr
){
	uintptr_t ofs = (uintptr_t)ptr - (uintptr_t)contextImpl->m_dmaBase;
#if X68SOUND_ENABLE_PORTABLE_CODE
	assert(ofs < 0x100000000LL);
#endif
	return (ptr ? (DWORD)ofs : 0);
}

static inline BYTE *X68SoundContextImpl_ToPtr(
	const X68SoundContextImpl *contextImpl,
	DWORD ofs
){
	return (ofs ? ((BYTE *)(((uintptr_t)contextImpl->m_dmaBase) + (ofs))) : NULL);
}

#if X68SOUND_ENABLE_PORTABLE_CODE
static inline unsigned int irnd(X68SoundContextImpl *contextImpl){
	contextImpl->m_RandSeed = contextImpl->m_RandSeed * 1566083941UL + 1;
	return contextImpl->m_RandSeed;
}
#endif

#define TO_OFS( ptr ) X68SoundContextImpl_ToOfs( pThis->m_contextImpl , ptr )
#define TO_PTR( ofs ) X68SoundContextImpl_ToPtr( pThis->m_contextImpl , ofs )

#define OpmFir					(pThis->m_contextImpl->m_OpmFir)
#define DebugValue				(pThis->m_contextImpl->m_DebugValue)
#define ErrorCode				(pThis->m_contextImpl->m_ErrorCode)
#define Samprate				(pThis->m_contextImpl->m_Samprate)
#define WaveOutSamp				(pThis->m_contextImpl->m_WaveOutSamp)
#define OpmWait					(pThis->m_contextImpl->m_OpmWait)
#define OpmRate					(pThis->m_contextImpl->m_OpmRate)
#define STEPTBL					(pThis->m_contextImpl->m_STEPTBL)
#define ALPHATBL				(pThis->m_contextImpl->m_ALPHATBL)
#define SINTBL					(pThis->m_contextImpl->m_SINTBL)
#define D1LTBL					(pThis->m_contextImpl->m_D1LTBL)
#define DT1TBL					(pThis->m_contextImpl->m_DT1TBL)
#define NOISEALPHATBL			(pThis->m_contextImpl->m_NOISEALPHATBL)
#define MemRead					(pThis->m_contextImpl->m_MemRead)
#define TotalVolume				(pThis->m_contextImpl->m_TotalVolume)
#define Semapho					(pThis->m_contextImpl->m_Semapho)
#define TimerSemapho			(pThis->m_contextImpl->m_TimerSemapho)
#define OPMLPF_ROW				(pThis->m_contextImpl->m_OPMLPF_ROW)
#define OPMLOWPASS				(pThis->m_contextImpl->m_OPMLOWPASS)
#define Betw_Time				(pThis->m_contextImpl->m_Betw_Time)
#define Late_Time				(pThis->m_contextImpl->m_Late_Time)
#define Late_Samples			(pThis->m_contextImpl->m_Late_Samples)
#define Blk_Samples				(pThis->m_contextImpl->m_Blk_Samples)
#define Betw_Samples_Slower		(pThis->m_contextImpl->m_Betw_Samples_Slower)
#define Betw_Samples_Faster		(pThis->m_contextImpl->m_Betw_Samples_Faster)
#define Betw_Samples_VerySlower	(pThis->m_contextImpl->m_Betw_Samples_VerySlower)
#define Slower_Limit			(pThis->m_contextImpl->m_Slower_Limit)
#define Faster_Limit			(pThis->m_contextImpl->m_Faster_Limit)
#define TimerResolution			(pThis->m_contextImpl->m_TimerResolution)
#define nSamples				(pThis->m_contextImpl->m_nSamples)
#define N_waveblk				(pThis->m_contextImpl->m_N_waveblk)
#define waveblk					(pThis->m_contextImpl->m_waveblk)
#define playingblk				(pThis->m_contextImpl->m_playingblk)
#define playingblk_next			(pThis->m_contextImpl->m_playingblk_next)
#define setPcmBufPtr			(pThis->m_contextImpl->m_setPcmBufPtr)

#endif //__X68SOUND_CONTEXT_INTERNAL_H__
