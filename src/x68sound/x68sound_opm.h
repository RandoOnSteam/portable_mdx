#ifndef __X68SOUND_OPM_H__
#define __X68SOUND_OPM_H__

#include "x68sound_config.h"

#if X68SOUND_ENABLE_PORTABLE_CODE
	#include "x68sound_global.h"
#endif
#include "x68sound_op.h"
#include "x68sound_lfo.h"
#include "x68sound_adpcm.h"
#include "x68sound_pcm8.h"

#include "mutexwrapper.h"

#define	CMNDBUFSIZE	65535
#define	FlagBufSize	7

//#define	RES	(20)
//#define	NDATA	(44100/5)
#define	PCMBUFSIZE	65536
//#define	DELAY	(1000/5)

typedef struct OPM {
#if X68SOUND_ENABLE_PORTABLE_CODE
/*public:*/
	struct tagX68SoundContextImpl *m_contextImpl;
/*private:*/
#endif
#if X68SOUND_ENABLE_PORTABLE_CODE
	const char *Author;
#else
	char *Author;
#endif

	OP	op[8][4];	// オペレータ0～31
	int	EnvCounter1;	// エンベロープ用カウンタ1 (0,1,2,3,4,5,6,...)
	int	EnvCounter2;	// エンベロープ用カウンタ2 (3,2,1,3,2,1,3,2,...)
//	int	con[N_CH];	// アルゴリズム 0～7
	int	pan[2][N_CH];	// 0:無音 -1:出力
//	int pms[N_CH];	// 0, 1, 2, 4, 10, 20, 80, 140
//	int	ams[N_CH];	// 右シフト回数 31(0), 2(1), 1(2), 0(3)
//	int	pmd;
//	int	amd;
//	int	pmspmd[N_CH];	// pms[]*pmd
	LFO	lfo;
	int	SLOTTBL[8*4];

	unsigned char	CmndBuf[CMNDBUFSIZE+1][2];
	volatile int	NumCmnd;
	int	CmndReadIdx,CmndWriteIdx;
	int CmndRate;

#if X68SOUND_ENABLE_PORTABLE_CODE
	int RateForExecuteCmnd;
	int RateForPcmset62;
	int RateForPcmset22;
	int Rate2ForPcmset22;
#endif

	//	short	PcmBuf[PCMBUFSIZE][2];
	short	(*PcmBuf)[2];
/*public:*/
	unsigned int PcmBufSize;
	volatile unsigned int PcmBufPtr;
/*private:*/
#if !X68SOUND_ENABLE_PORTABLE_CODE
	unsigned int	TimerID;
#endif

//	int	LfoOverTime;	// LFO tのオーバーフロー値
//	int	LfoTime;	// LFO専用 t
//	int LfoRndTime;	// LFOランダム波専用t
//	int
//	int	Lfrq;		// LFO周波数設定値 LFRQ
//	int	LfoWaveForm;	// LFO wave form
//	inline void	CulcLfoStep();

	volatile int	OpOut[8];
	int	OpOutDummy;

	int	TimerAreg10;	// OPMreg$10の値
	int	TimerAreg11;	// OPMreg$11の値
	int	TimerA;			// タイマーAのオーバーフロー設定値
	int	TimerAcounter;	// タイマーAのカウンター値
	int	TimerB;			// タイマーBのオーバーフロー設定値
	int	TimerBcounter;	// タイマーBのカウンター値
	volatile int	TimerReg;		// タイマー制御レジスタ (OPMreg$14の下位4ビット)
	volatile int	StatReg;		// OPMステータスレジスタ ($E90003の下位2ビット)
#if X68SOUND_ENABLE_PORTABLE_CODE
	void (*OpmIntProc)(void *);		// OPM割り込みコールバック関数
	void *OpmIntArg;
#else
	void (CALLBACK *OpmIntProc)();	// OPM割り込みコールバック関数
#endif

	double inpopmbuf_dummy;
	short InpOpmBuf0[OPMLPF_COL*2],InpOpmBuf1[OPMLPF_COL*2];
	int InpOpm_idx;
#if X68SOUND_ENABLE_PORTABLE_CODE
	int OpmLPFidx; const short *OpmLPFp;
#else
	int OpmLPFidx; short *OpmLPFp;
#endif
	double inpadpcmbuf_dummy;
//	short InpAdpcmBuf0[ADPCMLPF_COL*2],InpAdpcmBuf1[ADPCMLPF_COL*2];
//	int InpAdpcm_idx;
//	int	AdpcmLPFidx; short *AdpcmLPFp;
	int	OutOpm[2];
	int InpInpOpm[2],InpOpm[2];
	int	InpInpOpm_prev[2],InpOpm_prev[2];
	int	InpInpOpm_prev2[2],InpOpm_prev2[2];
	int OpmHpfInp[2], OpmHpfInp_prev[2], OpmHpfOut[2];
	int OutInpAdpcm[2],OutInpAdpcm_prev[2],OutInpAdpcm_prev2[2],
		OutOutAdpcm[2],OutOutAdpcm_prev[2],OutOutAdpcm_prev2[2];	// 高音フィルター２用バッファ
	int OutInpOutAdpcm[2],OutInpOutAdpcm_prev[2],OutInpOutAdpcm_prev2[2],
		OutOutInpAdpcm[2],OutOutInpAdpcm_prev[2];			// 高音フィルター３用バッファ

	volatile unsigned char	PpiReg;
	unsigned char	AdpcmBaseClock;	// ADPCMクロック切り替え(0:8MHz 1:4Mhz)


	unsigned char	OpmRegNo;		// 現在指定されているOPMレジスタ番号
	unsigned char	OpmRegNo_backup;		// バックアップ用OPMレジスタ番号
#if X68SOUND_ENABLE_PORTABLE_CODE
	void (*BetwIntProc)(void *);	// マルチメディアタイマー割り込み
	void *BetwIntArg;
	int (*WaveFunc)(void *);		// WaveFunc
	void *WaveFuncArg;
#else
	void (CALLBACK *BetwIntProc)();	// マルチメディアタイマー割り込み
	int (CALLBACK *WaveFunc)();		// WaveFunc
#endif

	int	UseOpmFlag;		// OPMを利用するかどうかのフラグ
	int	UseAdpcmFlag;	// ADPCMを利用するかどうかのフラグ
	int _betw;
	int _pcmbuf;
	int _late;
	int _rev;

	int Dousa_mode;		// 0:非動作 1:X68Sound_Start中  2:X68Sound_PcmStart中

	/* CmndBufアクセスロック用 */
	MUTEXWRAPPER m_mtxCmnd;

//public:
	ADPCM	adpcm;
//private:
	PCM8	pcm8[PCM8_NCH];

//	int	TotalVolume;	// 音量 x/256
/*public:*/
} OPM;

	void Opm_SetAdpcmRate(OPM* pThis);
	void Opm_SetConnection(OPM* pThis, int ch, int alg);

#if X68SOUND_ENABLE_PORTABLE_CODE
	void Opm_ConstructWithX68SoundContextImpl(OPM* pThis,
		struct tagX68SoundContextImpl *contextImpl);
#endif
	void Opm_Construct(OPM* pThis);
    void Opm_Destruct(OPM* pThis);
	void Opm_pcmset62(OPM* pThis, int ndata);
	void Opm_pcmset22(OPM* pThis, int ndata);

	int Opm_GetPcm(OPM* pThis, void *buf, int ndata);

	void Opm_timer(OPM* pThis);
	void Opm_betwint(OPM* pThis);

	void Opm_MakeTable(OPM* pThis);
	int Opm_Start(OPM* pThis, int samprate, int opmflag, int adpcmflag,
				int betw, int pcmbuf, int late, double rev);
	int Opm_StartPcm(OPM* pThis,
		int samprate, int opmflag, int adpcmflag, int pcmbuf);
	int Opm_SetSamprate(OPM* pThis, int samprate);
	int Opm_SetOpmClock(OPM* pThis, int clock);
	int Opm_WaveAndTimerStart(OPM* pThis);
	int Opm_SetOpmWait(OPM* pThis, int wait);
	void Opm_CulcCmndRate(OPM* pThis);
	void Opm_Reset(OPM* pThis);
	void Opm_ResetSamprate(OPM* pThis);
	void Opm_Free(OPM* pThis);
#if X68SOUND_ENABLE_PORTABLE_CODE
	void Opm_BetwInt(OPM* pThis, void (*proc)(void *), void *arg);
#else
	void Opm_BetwInt(OPM* pThis, void (CALLBACK *proc)());
#endif

	unsigned char Opm_OpmPeek(OPM* pThis);
	void Opm_OpmReg(OPM* pThis, unsigned char no);
	void Opm_OpmPoke(OPM* pThis, unsigned char data);
	void Opm_ExecuteCmnd(OPM* pThis);
#if X68SOUND_ENABLE_PORTABLE_CODE
	void Opm_OpmInt(OPM* pThis, void (*proc)(void *), void *arg);
#else
	void Opm_OpmInt(OPM* pThis, void (CALLBACK *proc)());
#endif

	unsigned char Opm_AdpcmPeek(OPM* pThis);
	void Opm_AdpcmPoke(OPM* pThis, unsigned char data);
	unsigned char Opm_PpiPeek(OPM* pThis);
	void Opm_PpiPoke(OPM* pThis, unsigned char data);
	void Opm_PpiCtrl(OPM* pThis, unsigned char data);

	unsigned char Opm_DmaPeek(OPM* pThis, unsigned char adrs);
	void Opm_DmaPoke(OPM* pThis, unsigned char adrs, unsigned char data);
#if X68SOUND_ENABLE_PORTABLE_CODE
	void Opm_DmaInt(OPM* pThis, void (*proc)(void *), void *arg);
	void Opm_DmaErrInt(OPM* pThis, void (*proc)(void *), void *arg);
	void Opm_MemReadFunc(OPM* pThis, int (*func)(unsigned char *));
#else
	void Opm_DmaInt(OPM* pThis, void (CALLBACK *proc)());
	void Opm_DmaErrInt(OPM* pThis, void (CALLBACK *proc)());
	void Opm_MemReadFunc(OPM* pThis, int (CALLBACK *func)(unsigned char *));
#endif

#if X68SOUND_ENABLE_PORTABLE_CODE
	void Opm_SetWaveFunc(OPM* pThis, int (*func)(void *), void *arg);
#else
	void Opm_SetWaveFunc(OPM* pThis, int (CALLBACK *func)());
#endif

	int Opm_Pcm8_Out(OPM* pThis, int ch, void *adrs, int mode, int len);
	int Opm_Pcm8_Aot(OPM* pThis, int ch, void *tbl, int mode, int cnt);
	int Opm_Pcm8_Lot(OPM* pThis, int ch, void *tbl, int mode);
	int Opm_Pcm8_SetMode(OPM* pThis, int ch, int mode);
	int Opm_Pcm8_GetRest(OPM* pThis, int ch);
	int Opm_Pcm8_GetMode(OPM* pThis, int ch);
	int Opm_Pcm8_Abort(OPM* pThis);

	int Opm_SetTotalVolume(OPM* pThis, int v);
	int Opm_GetTotalVolume(OPM* pThis);

	void Opm_PushRegs(OPM* pThis);
	void Opm_PopRegs(OPM* pThis);

#endif //__X68SOUND_OPM_H__
