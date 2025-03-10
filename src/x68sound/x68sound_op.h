﻿#ifndef __X68SOUND_OP_H__
#define __X68SOUND_OP_H__

#include <x68sound_context.h>
#include "x68sound_config.h"

#define	KEYON	-1
#define	ATACK	0
#define	DECAY	1
#define	SUSTAIN	2
#define	SUSTAIN_MAX	3
#define	RELEASE	4
#define	RELEASE_MAX	5

#define CULC_DELTA_T	(0x7FFFFFFF)
#define	CULC_ALPHA		(0x7FFFFFFF)

typedef struct OP {
#if X68SOUND_ENABLE_PORTABLE_CODE
/*public:*/
	struct tagX68SoundContextImpl *m_contextImpl;
/*private:*/
#endif
/*public:*/
	volatile int	inp;			// FM変調の入力
/*private:*/
	int LfoPitch;	// 前回のlfopitch値, CULC_DELTA_T値の時はDeltaTを再計算する。
	int	T;		// 現在時間 (0 <= T < SIZESINTBL*PRECISION)
	int DeltaT;	// Δt
	int	Ame;		// 0(トレモロをかけない), -1(トレモロをかける)
	int	LfoLevel;	// 前回のlfopitch&Ame値, CULC_ALPHA値の時はAlphaを再計算する。
	int	Alpha;	// 最終的なエンベロープ出力値
/*public:*/
	volatile int	*out;			// オペレータの出力先
	volatile int	*out2;			// オペレータの出力先(alg=5時のM1用)
	volatile int	*out3;			// オペレータの出力先(alg=5時のM1用)
/*private:*/
	int	Pitch;	// 0<=pitch<10*12*64
	int	Dt1Pitch;	// Step に対する補正量
	int	Mul;	// 0.5*2 1*2 2*2 3*2 ... 15*2
	int	Tl;		// (128-TL)*8

	int	Out2Fb;	// フィードバックへの出力値
	int	Inp_last;	// 最後の入力値
	int	Fl;		// フィードバックレベルのシフト値(31,7,6,5,4,3,2,1)
	int	Fl_mask;	// フィードバックのマスク(0,-1)
	int	ArTime;	// AR専用 t

	int	NoiseCounter;	// Noise用カウンタ
	int NoiseStep;	// Noise用カウントダウン値
	int NoiseCycle;	// Noise周期 32*2^25(0) ～ 1*2^25(31) NoiseCycle==0の時はノイズオフ
	int NoiseValue;	// ノイズ値  1 or -1

	// エンベロープ関係
	int	Xr_stat;
	int	Xr_el;
	int	Xr_step;
	int	Xr_and;
	int	Xr_cmp;
	int	Xr_add;
	int Xr_limit;


	int	Note;	// 音階 (0 <= Note < 10*12)
	int	Kc;		// 音階 (1 <= Kc <= 128)
	int	Kf;		// 微調整 (0 <= Kf < 64)
	int Ar;		// 0 <= Ar < 31
	int D1r;	// 0 <= D1r < 31
	int	D2r;	// 0 <= D2r < 31
	int	Rr;		// 0 <= Rr < 15
	int	Ks;		// 0 <= Ks <= 3
	int	Dt2;	// Pitch に対する補正量(0, 384, 500, 608)
	int	Dt1;	// DT1の値(0～7)
	int Nfrq;	// Noiseflag,NFRQの値

#if X68SOUND_ENABLE_PORTABLE_CODE
	struct {int m_and,cmp,add, limit;}
#else
	struct {int and,cmp,add, limit;}
#endif
	StatTbl[RELEASE_MAX+1];	// 状態推移テーブル

} OP;

	//           ATACK     DECAY   SUSTAIN     SUSTAIN_MAX RELEASE     RELEASE_MAX
	// and     :                               4097                    4097
	// cmp     :                               2048                    2048
	// add     :                               0                       0
	// limit   : 0         D1l     63          63          63          63
	// nextstat: DECAY     SUSTAIN SUSTAIN_MAX SUSTAIN_MAX RELEASE_MAX RELEASE_MAX

	void Op_CulcArStep(OP* pThis);
	void Op_CulcD1rStep(OP* pThis);
	void Op_CulcD2rStep(OP* pThis);
	void Op_CulcRrStep(OP* pThis);
	void Op_CulcPitch(OP* pThis);
	void Op_CulcDt1Pitch(OP* pThis);
	void Op_CulcNoiseCycle(OP* pThis);

	void Op_ConstructWithX68SoundContextImpl(OP* pThis,
		struct tagX68SoundContextImpl *contextImpl);
	void Op_Construct(OP* pThis);
	#define Op_Destruct(pThis)
	void Op_Init(OP* pThis);
	void Op_InitSamprate(OP* pThis);
	void Op_SetFL(OP* pThis, int n);
	void Op_SetKC(OP* pThis, int n);
	void Op_SetKF(OP* pThis, int n);
	void Op_SetDT1MUL(OP* pThis, int n);
	void Op_SetTL(OP* pThis, int n);
	void Op_SetKSAR(OP* pThis, int n);
	void Op_SetAMED1R(OP* pThis, int n);
	void Op_SetDT2D2R(OP* pThis, int n);
	void Op_SetD1LRR(OP* pThis, int n);
	void Op_KeyON(OP* pThis);
	void Op_KeyOFF(OP* pThis);
	void Op_Envelope(OP* pThis, int env_counter);
	void Op_SetNFRQ(OP* pThis, int nfrq);

	void Op_Output0(OP* pThis, int lfopitch, int lfolevel);		// オペレータ0用
	void Op_Output(OP* pThis, int lfopitch, int lfolevel);		// 一般オペレータ用
	void Op_Output32(OP* pThis, int lfopitch, int lfolevel);		// スロット32用
	void Op_Output0_22(OP* pThis, int lfopitch, int lfolevel);		// オペレータ0用
	void Op_Output_22(OP* pThis, int lfopitch, int lfolevel);		// 一般オペレータ用
	void Op_Output32_22(OP* pThis, int lfopitch, int lfolevel);		// スロット32用

#endif //__X68SOUND_OP_H__
