#ifndef __X68SOUND_LFO_H__
#define __X68SOUND_LFO_H__

#include "x68sound_config.h"

#define	SIZELFOTBL	512				// 2^9
#define	SIZELFOTBL_BITS	9
#define	LFOPRECISION	4096	// 2^12
//#define	PMTBLMAXVAL	(128)
//#define	PMTBLMAXVAL_BITS	(7)
//#define	AMTBLMAXVAL	(256)
//#define	AMTBLMAXVAL_BITS	(8)
//#define	LFOTIMECYCLE	 1073741824		// 2^30
//#define LFOTIMECYCLE_BITS	30
//#define	LFORNDTIMECYCLE	 (LFOTIMECYCLE>>8)		// 2^22
//#define	CYCLE2PMAM	(30-8)				// log2(LFOTIMECYCLE/SIZEPMAMTBL)
//#define	LFOHZ		0.0009313900811
//int		LFOSTEPTBL[256];
//int		LFOSTEPTBL3[256];		// Wave form 3 用
//short	PMSTBL[8]={ 0,1,2,4,8,16,64,128 };


typedef struct LFO {
#if X68SOUND_ENABLE_PORTABLE_CODE
/*public:*/
	struct tagX68SoundContextImpl *m_contextImpl;
/*private:*/
#endif
	int Pmsmul[N_CH];	// 0, 1, 2, 4, 8, 16, 32, 32
	int Pmsshl[N_CH];	// 0, 0, 0, 0, 0,  0,  1,  2
	int	Ams[N_CH];	// 左シフト回数 31(0), 0(1), 1(2), 2(3)
	int	PmdPmsmul[N_CH];	// Pmd*Pmsmul[]
	int	Pmd;
	int	Amd;

	int LfoStartingFlag;	// 0:LFO停止中  1:LFO動作中
	int	LfoOverFlow;	// LFO tのオーバーフロー値
	int	LfoTime;	// LFO専用 t
	int	LfoTimeAdd;	// LFO専用Δt
	int LfoIdx;	// LFOテーブルへのインデックス値
	int LfoSmallCounter;	// LFO周期微調整カウンタ (0～15の値をとる)
	int LfoSmallCounterStep;	// LFO周期微調整カウンタ用ステップ値 (16～31)
	int	Lfrq;		// LFO周波数設定値 LFRQ
	int	LfoWaveForm;	// LFO wave form

	int	PmTblValue,AmTblValue;
	int	PmValue[N_CH],AmValue[N_CH];

	char	PmTbl0[SIZELFOTBL], PmTbl2[SIZELFOTBL];
	unsigned char	AmTbl0[SIZELFOTBL], AmTbl2[SIZELFOTBL];
} LFO;

/* private */
void Lfo_CulcTblValue(LFO* pThis);
void Lfo_CulcPmValue(LFO* pThis, int ch);
void Lfo_CulcAmValue(LFO* pThis, int ch);
void Lfo_CulcAllPmValue(LFO* pThis);
void Lfo_CulcAllAmValue(LFO* pThis);

/* public */
#if X68SOUND_ENABLE_PORTABLE_CODE
	void Lfo_ConstructWithX68SoundContextImpl(LFO* pThis,
		struct tagX68SoundContextImpl *contextImpl);
#else
	void Lfo_Construct(LFO* pThis);
#endif
#define Lfo_Destruct(pThis)

void Lfo_Init(LFO* pThis);
void Lfo_InitSamprate(LFO* pThis);

void Lfo_LfoReset(LFO* pThis);
void Lfo_LfoStart(LFO* pThis);
void Lfo_SetLFRQ(LFO* pThis, int n);
void Lfo_SetPMDAMD(LFO* pThis, int n);
void Lfo_SetWaveForm(LFO* pThis, int n);
void Lfo_SetPMSAMS(LFO* pThis, int ch, int n);

void Lfo_Update(LFO* pThis);
int	Lfo_GetPmValue(LFO* pThis, int ch);
int	Lfo_GetAmValue(LFO* pThis, int ch);

#endif //__X68SOUND_LFO_H__
