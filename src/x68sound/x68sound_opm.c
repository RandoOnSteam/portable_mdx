#include <math.h>
#include <x68sound_context.h>
#include "x68sound_config.h"
#include "x68sound_global.h"
#include "x68sound_opm.h"
#include "x68sound.h"
#include "x68sound_context.internal.h"
#if defined(__cplusplus)
extern "C" {
#endif
void Opm_SetAdpcmRate(OPM* pThis) {
	Adpcm_SetAdpcmRate(&pThis->adpcm,
		ADPCMRATETBL[pThis->AdpcmBaseClock][(pThis->PpiReg>>2)&3]);
}




void Opm_SetConnection(OPM* pThis, int ch, int alg) {
	switch (alg) {
	case 0:
		pThis->op[ch][0].out = &pThis->op[ch][1].inp;
		pThis->op[ch][0].out2 = &pThis->OpOutDummy;
		pThis->op[ch][0].out3 = &pThis->OpOutDummy;
		pThis->op[ch][1].out = &pThis->op[ch][2].inp;
		pThis->op[ch][2].out = &pThis->op[ch][3].inp;
		pThis->op[ch][3].out = &pThis->OpOut[ch];
		break;
	case 1:
		pThis->op[ch][0].out = &pThis->op[ch][2].inp;
		pThis->op[ch][0].out2 = &pThis->OpOutDummy;
		pThis->op[ch][0].out3 = &pThis->OpOutDummy;
		pThis->op[ch][1].out = &pThis->op[ch][2].inp;
		pThis->op[ch][2].out = &pThis->op[ch][3].inp;
		pThis->op[ch][3].out = &pThis->OpOut[ch];
		break;
	case 2:
		pThis->op[ch][0].out = &pThis->op[ch][3].inp;
		pThis->op[ch][0].out2 = &pThis->OpOutDummy;
		pThis->op[ch][0].out3 = &pThis->OpOutDummy;
		pThis->op[ch][1].out = &pThis->op[ch][2].inp;
		pThis->op[ch][2].out = &pThis->op[ch][3].inp;
		pThis->op[ch][3].out = &pThis->OpOut[ch];
		break;
	case 3:
		pThis->op[ch][0].out = &pThis->op[ch][1].inp;
		pThis->op[ch][0].out2 = &pThis->OpOutDummy;
		pThis->op[ch][0].out3 = &pThis->OpOutDummy;
		pThis->op[ch][1].out = &pThis->op[ch][3].inp;
		pThis->op[ch][2].out = &pThis->op[ch][3].inp;
		pThis->op[ch][3].out = &pThis->OpOut[ch];
		break;
	case 4:
		pThis->op[ch][0].out = &pThis->op[ch][1].inp;
		pThis->op[ch][0].out2 = &pThis->OpOutDummy;
		pThis->op[ch][0].out3 = &pThis->OpOutDummy;
		pThis->op[ch][1].out = &pThis->OpOut[ch];
		pThis->op[ch][2].out = &pThis->op[ch][3].inp;
		pThis->op[ch][3].out = &pThis->OpOut[ch];
		break;
	case 5:
		pThis->op[ch][0].out = &pThis->op[ch][1].inp;
		pThis->op[ch][0].out2 = &pThis->op[ch][2].inp;
		pThis->op[ch][0].out3 = &pThis->op[ch][3].inp;
		pThis->op[ch][1].out = &pThis->OpOut[ch];
		pThis->op[ch][2].out = &pThis->OpOut[ch];
		pThis->op[ch][3].out = &pThis->OpOut[ch];
		break;
	case 6:
		pThis->op[ch][0].out = &pThis->op[ch][1].inp;
		pThis->op[ch][0].out2 = &pThis->OpOutDummy;
		pThis->op[ch][0].out3 = &pThis->OpOutDummy;
		pThis->op[ch][1].out = &pThis->OpOut[ch];
		pThis->op[ch][2].out = &pThis->OpOut[ch];
		pThis->op[ch][3].out = &pThis->OpOut[ch];
		break;
	case 7:
		pThis->op[ch][0].out = &pThis->OpOut[ch];
		pThis->op[ch][0].out2 = &pThis->OpOutDummy;
		pThis->op[ch][0].out3 = &pThis->OpOutDummy;
		pThis->op[ch][1].out = &pThis->OpOut[ch];
		pThis->op[ch][2].out = &pThis->OpOut[ch];
		pThis->op[ch][3].out = &pThis->OpOut[ch];
		break;
	}
}


int Opm_SetOpmWait(OPM* pThis, int wait) {
	if (wait != -1) {
		OpmWait = wait;
		Opm_CulcCmndRate(pThis);
	}
	return OpmWait;
}
void Opm_CulcCmndRate(OPM* pThis) {
	if (OpmWait != 0) {
		pThis->CmndRate = (4096*160/OpmWait);
		if (pThis->CmndRate == 0) {
			pThis->CmndRate = 1;
		}
	} else {
		pThis->CmndRate = 4096*CMNDBUFSIZE;
	}
}

void Opm_Reset(OPM* pThis) {
	// OPMコマンドバッファを初期化
	{
		MutexWrapper_lock(&pThis->m_mtxCmnd);
		pThis->NumCmnd = 0;
		pThis->CmndReadIdx = pThis->CmndWriteIdx = 0;
		Opm_CulcCmndRate(pThis);
		MutexWrapper_unlock(&pThis->m_mtxCmnd);
	}

	// 高音フィルター用バッファをクリア
	pThis->InpInpOpm[0] = pThis->InpInpOpm[1] =
	pThis->InpInpOpm_prev[0] = pThis->InpInpOpm_prev[1] = 0;
	pThis->InpInpOpm_prev2[0] = pThis->InpInpOpm_prev2[1] = 0;
	pThis->InpOpm[0] = pThis->InpOpm[1] =
	pThis->InpOpm_prev[0] = pThis->InpOpm_prev[1] =
	pThis->InpOpm_prev2[0] = pThis->InpOpm_prev2[1] =
	pThis->OutOpm[0] = pThis->OutOpm[1] = 0;
	{
#if X68SOUND_ENABLE_PORTABLE_CODE
		int i;
#else
		int i,j;
#endif
		for (i=0; i<OPMLPF_COL*2; ++i) {
			pThis->InpOpmBuf0[i]=pThis->InpOpmBuf1[i]=0;
		}
		pThis->InpOpm_idx = 0;
		pThis->OpmLPFidx = 0;
		pThis->OpmLPFp = OPMLOWPASS[0];
	}
	pThis->OpmHpfInp[0] = pThis->OpmHpfInp[1] =
	pThis->OpmHpfInp_prev[0] = pThis->OpmHpfInp_prev[1] =
	pThis->OpmHpfOut[0] = pThis->OpmHpfOut[1] = 0;
/*	{
		int i,j;
		for (i=0; i<ADPCMLPF_COL*2; ++i) {
			InpAdpcmBuf0[i]=InpAdpcmBuf1[i]=0;
		}
		InpAdpcm_idx = 0;
		AdpcmLPFidx = 0;
		AdpcmLPFp = ADPCMLOWPASS[0];
	}
*/
	pThis->OutInpAdpcm[0] = pThis->OutInpAdpcm[1] =
	pThis->OutInpAdpcm_prev[0] = pThis->OutInpAdpcm_prev[1] =
	pThis->OutInpAdpcm_prev2[0] = pThis->OutInpAdpcm_prev2[1] =
	pThis->OutOutAdpcm[0] = pThis->OutOutAdpcm[1] =
	pThis->OutOutAdpcm_prev[0] = pThis->OutOutAdpcm_prev[1] =
	pThis->OutOutAdpcm_prev2[0] = pThis->OutOutAdpcm_prev2[1] =
	0;
	pThis->OutInpOutAdpcm[0] = pThis->OutInpOutAdpcm[1] =
	pThis->OutInpOutAdpcm_prev[0] = pThis->OutInpOutAdpcm_prev[1] =
	pThis->OutInpOutAdpcm_prev2[0] = pThis->OutInpOutAdpcm_prev2[1] =
	pThis->OutOutInpAdpcm[0] = pThis->OutOutInpAdpcm[1] =
	pThis->OutOutInpAdpcm_prev[0] = pThis->OutOutInpAdpcm_prev[1] =
	0;

	// 全オペレータを初期化
	{
		int	ch;
		for (ch=0; ch<N_CH; ++ch) {
			Op_Init(&pThis->op[ch][0]);
			Op_Init(&pThis->op[ch][1]);
			Op_Init(&pThis->op[ch][2]);
			Op_Init(&pThis->op[ch][3]);
//			con[ch] = 0;
			Opm_SetConnection(pThis,ch,0);
			pThis->pan[0][ch] = pThis->pan[1][ch] = 0;
		}
	}

	// エンベロープ用カウンタを初期化
	{
		pThis->EnvCounter1 = 0;
		pThis->EnvCounter2 = 3;
	}


	// LFO初期化
	Lfo_Init(&pThis->lfo);

	// pThis->PcmBufポインターをリセット
	pThis->PcmBufPtr=0;
//	pThis->PcmBufSize = PCMBUFSIZE;


	// タイマー関係の初期化
	pThis->TimerAreg10 = 0;
	pThis->TimerAreg11 = 0;
	pThis->TimerA = 1024-0;
	pThis->TimerAcounter = 0;
	pThis->TimerB = (256-0) << (10-6);
	pThis->TimerBcounter = 0;
	pThis->TimerReg = 0;
	pThis->StatReg = 0;
	pThis->OpmIntProc = NULL;
#if X68SOUND_ENABLE_PORTABLE_CODE
	pThis->OpmIntArg = NULL;
#endif

	pThis->PpiReg = 0x0B;
	pThis->AdpcmBaseClock = 0;


	Adpcm_Init(&pThis->adpcm);

	{
		int	i;
		for (i=0; i<PCM8_NCH; ++i) {
			Pcm8_Init(&pThis->pcm8[i]);
		}
	}

	TotalVolume = 256;
//	TotalVolume = 192;


	pThis->OpmRegNo = 0;
	pThis->BetwIntProc = NULL;
#if X68SOUND_ENABLE_PORTABLE_CODE
	pThis->BetwIntArg = NULL;
#endif
	pThis->WaveFunc = NULL;
#if X68SOUND_ENABLE_PORTABLE_CODE
	pThis->WaveFuncArg = NULL;
#endif

	MemRead = MemReadDefault;


//	pThis->UseOpmFlag = 0;
//	pThis->UseAdpcmFlag = 0;
}
void Opm_ResetSamprate(OPM* pThis) {
	Opm_CulcCmndRate(pThis);

	// 高音フィルター用バッファをクリア
	pThis->InpInpOpm[0] = pThis->InpInpOpm[1] =
	pThis->InpInpOpm_prev[0] = pThis->InpInpOpm_prev[1] = 0;
	pThis->InpInpOpm_prev2[0] = pThis->InpInpOpm_prev2[1] = 0;
	pThis->InpOpm[0] = pThis->InpOpm[1] =
	pThis->InpOpm_prev[0] = pThis->InpOpm_prev[1] =
	pThis->InpOpm_prev2[0] = pThis->InpOpm_prev2[1] =
	pThis->OutOpm[0] = pThis->OutOpm[1] = 0;
	{
#if X68SOUND_ENABLE_PORTABLE_CODE
		int i;
#else
		int i,j;
#endif
		for (i=0; i<OPMLPF_COL*2; ++i) {
			pThis->InpOpmBuf0[i]=pThis->InpOpmBuf1[i]=0;
		}
		pThis->InpOpm_idx = 0;
		pThis->OpmLPFidx = 0;
		pThis->OpmLPFp = OPMLOWPASS[0];
	}
	pThis->OpmHpfInp[0] = pThis->OpmHpfInp[1] =
	pThis->OpmHpfInp_prev[0] = pThis->OpmHpfInp_prev[1] =
	pThis->OpmHpfOut[0] = pThis->OpmHpfOut[1] = 0;
/*	{
		int i,j;
		for (i=0; i<ADPCMLPF_COL*2; ++i) {
			InpAdpcmBuf0[i]=InpAdpcmBuf1[i]=0;
		}
		InpAdpcm_idx = 0;
		AdpcmLPFidx = 0;
		AdpcmLPFp = ADPCMLOWPASS[0];
	}
*/
	pThis->OutInpAdpcm[0] = pThis->OutInpAdpcm[1] =
	pThis->OutInpAdpcm_prev[0] = pThis->OutInpAdpcm_prev[1] =
	pThis->OutInpAdpcm_prev2[0] = pThis->OutInpAdpcm_prev2[1] =
	pThis->OutOutAdpcm[0] = pThis->OutOutAdpcm[1] =
	pThis->OutOutAdpcm_prev[0] = pThis->OutOutAdpcm_prev[1] =
	pThis->OutOutAdpcm_prev2[0] = pThis->OutOutAdpcm_prev2[1] =
	0;
	pThis->OutInpOutAdpcm[0] = pThis->OutInpOutAdpcm[1] =
	pThis->OutInpOutAdpcm_prev[0] = pThis->OutInpOutAdpcm_prev[1] =
	pThis->OutInpOutAdpcm_prev2[0] = pThis->OutInpOutAdpcm_prev2[1] =
	pThis->OutOutInpAdpcm[0] = pThis->OutOutInpAdpcm[1] =
	pThis->OutOutInpAdpcm_prev[0] = pThis->OutOutInpAdpcm_prev[1] =
	0;

	// 全オペレータを初期化
	{
		int	ch;
		for (ch=0; ch<N_CH; ++ch) {
			Op_InitSamprate(&pThis->op[ch][0]);
			Op_InitSamprate(&pThis->op[ch][1]);
			Op_InitSamprate(&pThis->op[ch][2]);
			Op_InitSamprate(&pThis->op[ch][3]);
		}
	}

	// LFOを初期化
	Lfo_InitSamprate(&pThis->lfo);

	// pThis->PcmBufポインターをリセット
	pThis->PcmBufPtr=0;
//	pThis->PcmBufSize = PCMBUFSIZE;

//	pThis->PpiReg = 0x0B;
//	pThis->AdpcmBaseClock = 0;

	Adpcm_InitSamprate(&pThis->adpcm);
}

#if X68SOUND_ENABLE_PORTABLE_CODE
void Opm_ConstructWithX68SoundContextImpl(OPM* pThis,
	X68SoundContextImpl *contextImpl)
{
	Lfo_ConstructWithX68SoundContextImpl(&pThis->lfo, contextImpl);
	Adpcm_ConstructWithX68SoundContextImpl(&pThis->adpcm, contextImpl);

	Op_ConstructWithX68SoundContextImpl(&pThis->op[0][0], contextImpl);
	Op_ConstructWithX68SoundContextImpl(&pThis->op[0][1], contextImpl);
	Op_ConstructWithX68SoundContextImpl(&pThis->op[0][2], contextImpl);
	Op_ConstructWithX68SoundContextImpl(&pThis->op[0][3], contextImpl);
	Op_ConstructWithX68SoundContextImpl(&pThis->op[1][0], contextImpl);
	Op_ConstructWithX68SoundContextImpl(&pThis->op[1][1], contextImpl);
	Op_ConstructWithX68SoundContextImpl(&pThis->op[1][2], contextImpl);
	Op_ConstructWithX68SoundContextImpl(&pThis->op[1][3], contextImpl);
	Op_ConstructWithX68SoundContextImpl(&pThis->op[2][0], contextImpl);
	Op_ConstructWithX68SoundContextImpl(&pThis->op[2][1], contextImpl);
	Op_ConstructWithX68SoundContextImpl(&pThis->op[2][2], contextImpl);
	Op_ConstructWithX68SoundContextImpl(&pThis->op[2][3], contextImpl);
	Op_ConstructWithX68SoundContextImpl(&pThis->op[3][0], contextImpl);
	Op_ConstructWithX68SoundContextImpl(&pThis->op[3][1], contextImpl);
	Op_ConstructWithX68SoundContextImpl(&pThis->op[3][2], contextImpl);
	Op_ConstructWithX68SoundContextImpl(&pThis->op[3][3], contextImpl);
	Op_ConstructWithX68SoundContextImpl(&pThis->op[4][0], contextImpl);
	Op_ConstructWithX68SoundContextImpl(&pThis->op[4][1], contextImpl);
	Op_ConstructWithX68SoundContextImpl(&pThis->op[4][2], contextImpl);
	Op_ConstructWithX68SoundContextImpl(&pThis->op[4][3], contextImpl);
	Op_ConstructWithX68SoundContextImpl(&pThis->op[5][0], contextImpl);
	Op_ConstructWithX68SoundContextImpl(&pThis->op[5][1], contextImpl);
	Op_ConstructWithX68SoundContextImpl(&pThis->op[5][2], contextImpl);
	Op_ConstructWithX68SoundContextImpl(&pThis->op[5][3], contextImpl);
	Op_ConstructWithX68SoundContextImpl(&pThis->op[6][0], contextImpl);
	Op_ConstructWithX68SoundContextImpl(&pThis->op[6][1], contextImpl);
	Op_ConstructWithX68SoundContextImpl(&pThis->op[6][2], contextImpl);
	Op_ConstructWithX68SoundContextImpl(&pThis->op[6][3], contextImpl);
	Op_ConstructWithX68SoundContextImpl(&pThis->op[7][0], contextImpl);
	Op_ConstructWithX68SoundContextImpl(&pThis->op[7][1], contextImpl);
	Op_ConstructWithX68SoundContextImpl(&pThis->op[7][2], contextImpl);
	Op_ConstructWithX68SoundContextImpl(&pThis->op[7][3], contextImpl);

	Pcm8_ConstructWithX68SoundContextImpl(&pThis->pcm8[0], contextImpl);
	Pcm8_ConstructWithX68SoundContextImpl(&pThis->pcm8[1], contextImpl);
	Pcm8_ConstructWithX68SoundContextImpl(&pThis->pcm8[2], contextImpl);
	Pcm8_ConstructWithX68SoundContextImpl(&pThis->pcm8[3], contextImpl);
	Pcm8_ConstructWithX68SoundContextImpl(&pThis->pcm8[4], contextImpl);
	Pcm8_ConstructWithX68SoundContextImpl(&pThis->pcm8[5], contextImpl);
	Pcm8_ConstructWithX68SoundContextImpl(&pThis->pcm8[6], contextImpl);
	Pcm8_ConstructWithX68SoundContextImpl(&pThis->pcm8[7], contextImpl);

	pThis->m_contextImpl = contextImpl;
#else
void Opm_Construct(OPM* pThis) {
#endif
	pThis->Author = "m_puusan";

#if X68SOUND_ENABLE_PORTABLE_CODE
	pThis->RateForExecuteCmnd = 0;
	pThis->RateForPcmset62 = 0;
	pThis->RateForPcmset22 = 0;
	pThis->Rate2ForPcmset22 = 0;
#endif

#if !X68SOUND_ENABLE_PORTABLE_CODE
	hwo = NULL;
#endif
	pThis->PcmBuf = NULL;
#if !X68SOUND_ENABLE_PORTABLE_CODE
	pThis->TimerID = 0;
#endif

#if X68SOUND_ENABLE_PORTABLE_CODE
	/* clang の未使用メンバ警告を抑制するため */
	pThis->inpopmbuf_dummy = 0;
	pThis->inpadpcmbuf_dummy = 0;
#endif

	pThis->Dousa_mode = 0;

}

void Opm_MakeTable(OPM* pThis) {


	// sinテーブルを作成
	{
		int	i;
		for (i=0; i<SIZESINTBL; ++i) {
#if X68SOUND_ENABLE_PORTABLE_CODE
			SINTBL[i] = (short)(sin(2.0*PI*(i+0.0)/SIZESINTBL)*(MAXSINVAL) + 0.5);
#else
			SINTBL[i] = sin(2.0*PI*(i+0.0)/SIZESINTBL)*(MAXSINVAL) + 0.5;
#endif
		}
//		for (i=0; i<SIZESINTBL/4; ++i) {
//			short s = sin(2.0*PI*i/(SIZESINTBL-0))*(MAXSINVAL+0.0) +0.9;
//			SINTBL[i] = s;
//			SINTBL[SIZESINTBL/2-1-i] = s;
//			SINTBL[i+SIZESINTBL/2] = -s;
//			SINTBL[SIZESINTBL-1-i] = -s;
//		}

	}


	// エンベロープ値 → α 変換テーブルを作成
	{
		int	i;
		for (i=0; i<=ALPHAZERO+SIZEALPHATBL; ++i) {
			ALPHATBL[i] = 0;
		}
		for (i=17; i<=SIZEALPHATBL; ++i) {
#if X68SOUND_ENABLE_PORTABLE_CODE
			ALPHATBL[ALPHAZERO+i] = (unsigned short)floor(
#else
			ALPHATBL[ALPHAZERO+i] = floor(
#endif
				pow(2.0, -((SIZEALPHATBL)-i)*(128.0/8.0)/(SIZEALPHATBL))
				*1.0*1.0*PRECISION +0.0);
		}
	}
	// エンベロープ値 → Noiseα 変換テーブルを作成
	{
		int	i;
		for (i=0; i<=ALPHAZERO+SIZEALPHATBL; ++i) {
			NOISEALPHATBL[i] = 0;
		}
		for (i=17; i<=SIZEALPHATBL; ++i) {
#if X68SOUND_ENABLE_PORTABLE_CODE
			NOISEALPHATBL[ALPHAZERO+i] = (unsigned short)floor(
#else
			NOISEALPHATBL[ALPHAZERO+i] = floor(
#endif
				i*1.0/(SIZEALPHATBL)
				*1.0*0.25*PRECISION +0.0); // Noise音量はOpの1/4
		}
	}

	// D1L → D1l 変換テーブルを作成
	{
		int i;
		for (i=0; i<15; ++i) {
			D1LTBL[i] = i*2;
		}
		D1LTBL[15] = (15+16)*2;
	}


	// C1 <-> M2 入れ替えテーブルを作成
	{
		int	slot;
		for (slot=0; slot<8; ++slot) {
			pThis->SLOTTBL[slot] = slot*4;
			pThis->SLOTTBL[slot+8] = slot*4+2;
			pThis->SLOTTBL[slot+16] = slot*4+1;
			pThis->SLOTTBL[slot+24] = slot*4+3;
		}
	}

	// Pitch→Δt変換テーブルを作成
	{
		int	oct,notekf,step;

		for (oct=0; oct<=10; ++oct) {
			for (notekf=0; notekf<12*64; ++notekf) {
				if (oct >= 3) {
					step = STEPTBL_O2[notekf] << (oct-3);
				} else {
					step = STEPTBL_O2[notekf] >> (3-oct);
				}
#if X68SOUND_ENABLE_PORTABLE_CODE
				STEPTBL[oct*12*64+notekf] = (int)(step * 64 * (int64_t)(OpmRate)/Samprate);
#else
				STEPTBL[oct*12*64+notekf] = (int)(step * 64 * (__int64)(OpmRate)/Samprate);
#endif
			}
		}
//		for (notekf=0; notekf<11*12*64; ++notekf) {
//			STEPTBL3[notekf] = STEPTBL[notekf]/3.0+0.5;
//		}
	}


	{
		int i;
		for (i=0; i<=128+4-1; ++i) {
#if X68SOUND_ENABLE_PORTABLE_CODE
			DT1TBL[i] = (int)(DT1TBL_org[i] * 64 * (int64_t)(OpmRate)/Samprate);
#else
			DT1TBL[i] = (int)(DT1TBL_org[i] * 64 * (__int64)(OpmRate)/Samprate);
#endif
		}

	}


}

unsigned char Opm_OpmPeek(OPM* pThis) {
	return (unsigned char)pThis->StatReg;
}
void Opm_OpmReg(OPM* pThis, unsigned char no) {
	pThis->OpmRegNo = no;
}
void Opm_OpmPoke(OPM* pThis, unsigned char data) {
	{
		MutexWrapper_lock(&pThis->m_mtxCmnd);
		if (pThis->NumCmnd < CMNDBUFSIZE) {
			pThis->CmndBuf[pThis->CmndWriteIdx][0] = pThis->OpmRegNo;
			pThis->CmndBuf[pThis->CmndWriteIdx][1] = data;
			++pThis->CmndWriteIdx; pThis->CmndWriteIdx&=CMNDBUFSIZE;
			++pThis->NumCmnd;
		}
		MutexWrapper_unlock(&pThis->m_mtxCmnd);
	}

	switch (pThis->OpmRegNo) {
	case 0x10: case 0x11:
	// pThis->TimerA
		{
			if (pThis->OpmRegNo == 0x10) {
				pThis->TimerAreg10 = data;
			} else {
				pThis->TimerAreg11 = data & 3;
			}
			pThis->TimerA = 1024-((pThis->TimerAreg10<<2)+pThis->TimerAreg11);
		}
		break;

	case 0x12:
	// pThis->TimerB
		{
			pThis->TimerB = (256-(int)data) << (10-6);
		}
		break;

	case 0x14:
	// タイマー制御レジスタ
		{
#if X68SOUND_ENABLE_PORTABLE_CODE
			for(;;){
				int eax = 0;
				if(	TimerSemapho == eax ){
					int ecx = 1;
					TimerSemapho = ecx;
					break;
				} else {
					eax = TimerSemapho;
				}
			}
#else
			__asm {
				lp1:
					xor		eax,eax
					mov		ecx,1
					lock cmpxchg	TimerSemapho,ecx
				jnz		lp1
			}
#endif

			pThis->TimerReg = data & 0x0F;
			pThis->StatReg &= 0xFF-((data>>4)&3);

			TimerSemapho = 0;
		}
		break;

	case 0x1B:
	// WaveForm
		{
			pThis->AdpcmBaseClock = data >> 7;
			Opm_SetAdpcmRate(pThis);
		}
		break;
	}
}
void Opm_ExecuteCmnd(OPM* pThis) {

#if X68SOUND_ENABLE_PORTABLE_CODE
	pThis->RateForExecuteCmnd -= pThis->CmndRate;
	while (pThis->RateForExecuteCmnd < 0) {
	pThis->RateForExecuteCmnd += 4096;
#else
	static int	rate=0;
	rate -= pThis->CmndRate;
	while (rate < 0) {
	rate += 4096;
#endif

	if (pThis->NumCmnd != 0) {

	unsigned char regno, data;
	{
		MutexWrapper_lock(&pThis->m_mtxCmnd);
		regno = pThis->CmndBuf[pThis->CmndReadIdx][0];
		data = pThis->CmndBuf[pThis->CmndReadIdx][1];
		++pThis->CmndReadIdx;
		pThis->CmndReadIdx &= CMNDBUFSIZE;
		--pThis->NumCmnd;
		MutexWrapper_unlock(&pThis->m_mtxCmnd);
	}
	switch (regno) {
	case 0x01:
	// LFO RESET
		{
			if (data & 0x02) {
				Lfo_LfoReset(&pThis->lfo);
			} else {
				Lfo_LfoStart(&pThis->lfo);
			}
		}
		break;

	case 0x08:
	// KON
		{
			int	ch,s,bit;
			ch = data & 7;
			for (s=0,bit=8; s<4; ++s,bit+=bit) {
				if (data & bit) {
					Op_KeyON(&pThis->op[ch][s]);
				} else {
					Op_KeyOFF(&pThis->op[ch][s]);
				}
			}
		}
		break;

	case 0x0F:
	// NE,NFRQ
		{
			Op_SetNFRQ(&pThis->op[7][3], data&0xFF);
		}
		break;


	case 0x18:
	// LFRQ
		{
			Lfo_SetLFRQ(&pThis->lfo,  data&0xFF);
		}
		break;
	case 0x19:
	// PMD/AMD
		{
			Lfo_SetPMDAMD(&pThis->lfo,  data&0xFF);
		}
		break;
	case 0x1B:
	// WaveForm
		{
			Lfo_SetWaveForm(&pThis->lfo,  data&0xFF);
		}
		break;

	case 0x20: case 0x21: case 0x22: case 0x23: case 0x24: case 0x25: case 0x26: case 0x27:
	// PAN/FL/CON
		{
			int ch = regno-0x20;
//			con[ch] = data & 7;
			Opm_SetConnection(pThis, ch, data&7);
//			pThis->pan[ch] = data>>6;
			pThis->pan[0][ch] = ((data&0x40) ? -1 : 0);
			pThis->pan[1][ch] = ((data&0x80) ? -1 : 0);
			Op_SetFL(&pThis->op[ch][0], data);
		}
		break;

	case 0x28: case 0x29: case 0x2A: case 0x2B: case 0x2C: case 0x2D: case 0x2E: case 0x2F:
	// KC
		{
			int ch = regno-0x28;
			Op_SetKC(&pThis->op[ch][0], data);
			Op_SetKC(&pThis->op[ch][1], data);
			Op_SetKC(&pThis->op[ch][2], data);
			Op_SetKC(&pThis->op[ch][3], data);
		}
		break;

	case 0x30: case 0x31: case 0x32: case 0x33: case 0x34: case 0x35: case 0x36: case 0x37:
	// KF
		{
			int ch = regno-0x30;
			Op_SetKF(&pThis->op[ch][0], data);
			Op_SetKF(&pThis->op[ch][1], data);
			Op_SetKF(&pThis->op[ch][2], data);
			Op_SetKF(&pThis->op[ch][3], data);
		}
		break;

	case 0x38: case 0x39: case 0x3A: case 0x3B: case 0x3C: case 0x3D: case 0x3E: case 0x3F:
	// PMS/AMS
		{
			int ch = regno-0x38;
			Lfo_SetPMSAMS(&pThis->lfo,  ch, data&0xFF);
		}
		break;

	case 0x40: case 0x41: case 0x42: case 0x43: case 0x44: case 0x45: case 0x46: case 0x47:
	case 0x48: case 0x49: case 0x4A: case 0x4B: case 0x4C: case 0x4D: case 0x4E: case 0x4F:
	case 0x50: case 0x51: case 0x52: case 0x53: case 0x54: case 0x55: case 0x56: case 0x57:
	case 0x58: case 0x59: case 0x5A: case 0x5B: case 0x5C: case 0x5D: case 0x5E: case 0x5F:
	// DT1/MUL
		{
			int slot = regno-0x40;
			Op_SetDT1MUL(&pThis->op[0][pThis->SLOTTBL[slot]], data);
		}
		break;

	case 0x60: case 0x61: case 0x62: case 0x63: case 0x64: case 0x65: case 0x66: case 0x67:
	case 0x68: case 0x69: case 0x6A: case 0x6B: case 0x6C: case 0x6D: case 0x6E: case 0x6F:
	case 0x70: case 0x71: case 0x72: case 0x73: case 0x74: case 0x75: case 0x76: case 0x77:
	case 0x78: case 0x79: case 0x7A: case 0x7B: case 0x7C: case 0x7D: case 0x7E: case 0x7F:
	// TL
		{
			int slot = regno-0x60;
			Op_SetTL(&pThis->op[0][pThis->SLOTTBL[slot]], data);
		}
		break;

	case 0x80: case 0x81: case 0x82: case 0x83: case 0x84: case 0x85: case 0x86: case 0x87:
	case 0x88: case 0x89: case 0x8A: case 0x8B: case 0x8C: case 0x8D: case 0x8E: case 0x8F:
	case 0x90: case 0x91: case 0x92: case 0x93: case 0x94: case 0x95: case 0x96: case 0x97:
	case 0x98: case 0x99: case 0x9A: case 0x9B: case 0x9C: case 0x9D: case 0x9E: case 0x9F:
	// KS/AR
		{
			int slot = regno-0x80;
			Op_SetKSAR(&pThis->op[0][pThis->SLOTTBL[slot]], data);
		}
		break;

	case 0xA0: case 0xA1: case 0xA2: case 0xA3: case 0xA4: case 0xA5: case 0xA6: case 0xA7:
	case 0xA8: case 0xA9: case 0xAA: case 0xAB: case 0xAC: case 0xAD: case 0xAE: case 0xAF:
	case 0xB0: case 0xB1: case 0xB2: case 0xB3: case 0xB4: case 0xB5: case 0xB6: case 0xB7:
	case 0xB8: case 0xB9: case 0xBA: case 0xBB: case 0xBC: case 0xBD: case 0xBE: case 0xBF:
	// AME/D1R
		{
			int slot = regno-0xA0;
			Op_SetAMED1R(&pThis->op[0][pThis->SLOTTBL[slot]], data);
		}
		break;

	case 0xC0: case 0xC1: case 0xC2: case 0xC3: case 0xC4: case 0xC5: case 0xC6: case 0xC7:
	case 0xC8: case 0xC9: case 0xCA: case 0xCB: case 0xCC: case 0xCD: case 0xCE: case 0xCF:
	case 0xD0: case 0xD1: case 0xD2: case 0xD3: case 0xD4: case 0xD5: case 0xD6: case 0xD7:
	case 0xD8: case 0xD9: case 0xDA: case 0xDB: case 0xDC: case 0xDD: case 0xDE: case 0xDF:
	// DT2/D2R
		{
			int slot = regno-0xC0;
			Op_SetDT2D2R(&pThis->op[0][pThis->SLOTTBL[slot]], data);
		}
		break;

	case 0xE0: case 0xE1: case 0xE2: case 0xE3: case 0xE4: case 0xE5: case 0xE6: case 0xE7:
	case 0xE8: case 0xE9: case 0xEA: case 0xEB: case 0xEC: case 0xED: case 0xEE: case 0xEF:
	case 0xF0: case 0xF1: case 0xF2: case 0xF3: case 0xF4: case 0xF5: case 0xF6: case 0xF7:
	case 0xF8: case 0xF9: case 0xFA: case 0xFB: case 0xFC: case 0xFD: case 0xFE: case 0xFF:
	// D1L/RR
		{
			int slot = regno-0xE0;
			Op_SetD1LRR(&pThis->op[0][pThis->SLOTTBL[slot]], data);
		}
		break;

	}

	}
	}

}

void Opm_pcmset62(OPM* pThis, int ndata) {

#if !X68SOUND_ENABLE_PORTABLE_CODE
	Opm_DetectMMX(pThis);
#endif

	int	i;
	for (i=0; i<ndata; ++i) {
		int	Out[2];
		Out[0] = Out[1] = 0;

		pThis->OpmLPFidx += Samprate;
		while (pThis->OpmLPFidx >= WaveOutSamp) {
			pThis->OpmLPFidx -= WaveOutSamp;

			int OutInpOpm[2];
			OutInpOpm[0] = OutInpOpm[1] = 0;
			if (pThis->UseOpmFlag) {
#if X68SOUND_ENABLE_PORTABLE_CODE
				pThis->RateForPcmset62 -= OpmRate;
				while (pThis->RateForPcmset62 < 0) {
					pThis->RateForPcmset62 += 62500;
#else
				static int rate=0;
				rate -= OpmRate;
				while (rate < 0) {
					rate += 62500;
#endif

					Opm_timer(pThis);
					Opm_ExecuteCmnd(pThis);
					if ((--pThis->EnvCounter2) == 0) {
						pThis->EnvCounter2 = 3;
						++pThis->EnvCounter1;
						int slot;
						for (slot=0; slot<32; ++slot) {
#if X68SOUND_ENABLE_PORTABLE_CODE
							Op_Envelope(&pThis->op[slot&7][slot>>3],
								pThis->EnvCounter1);
#else
							Op_Envelope(&pThis->op[0][slot], pThis->EnvCounter1);
#endif
						}
					}
				}

					{
						Lfo_Update(&pThis->lfo);

						int lfopitch[8];
						int lfolevel[8];
						int	ch;
						for (ch=0; ch<8; ++ch) {
							pThis->op[ch][1].inp=pThis->op[ch][2].inp=pThis->op[ch][3].inp=pThis->OpOut[ch]=0;

							lfopitch[ch] = Lfo_GetPmValue(&pThis->lfo,  ch);
							lfolevel[ch] = Lfo_GetAmValue(&pThis->lfo,  ch);
						}
						for (ch=0; ch<8; ++ch) {
							Op_Output0(&pThis->op[ch][0], lfopitch[ch], lfolevel[ch]);
						}
						for (ch=0; ch<8; ++ch) {
							Op_Output(&pThis->op[ch][1], lfopitch[ch], lfolevel[ch]);
						}
						for (ch=0; ch<8; ++ch) {
							Op_Output(&pThis->op[ch][2], lfopitch[ch], lfolevel[ch]);
						}
						for (ch=0; ch<7; ++ch) {
							Op_Output(&pThis->op[ch][3], lfopitch[ch], lfolevel[ch]);
						}
						Op_Output32(&pThis->op[7][3], lfopitch[7], lfolevel[7]);
					}

					// pThis->OpmHpfInp[] に OPM の出力PCMをステレオ加算
					pThis->OpmHpfInp[0] =    (pThis->OpOut[0] & pThis->pan[0][0])
									+ (pThis->OpOut[1] & pThis->pan[0][1])
									+ (pThis->OpOut[2] & pThis->pan[0][2])
									+ (pThis->OpOut[3] & pThis->pan[0][3])
									+ (pThis->OpOut[4] & pThis->pan[0][4])
									+ (pThis->OpOut[5] & pThis->pan[0][5])
									+ (pThis->OpOut[6] & pThis->pan[0][6])
									+ (pThis->OpOut[7] & pThis->pan[0][7]);
					pThis->OpmHpfInp[1] =    (pThis->OpOut[0] & pThis->pan[1][0])
									+ (pThis->OpOut[1] & pThis->pan[1][1])
									+ (pThis->OpOut[2] & pThis->pan[1][2])
									+ (pThis->OpOut[3] & pThis->pan[1][3])
									+ (pThis->OpOut[4] & pThis->pan[1][4])
									+ (pThis->OpOut[5] & pThis->pan[1][5])
									+ (pThis->OpOut[6] & pThis->pan[1][6])
									+ (pThis->OpOut[7] & pThis->pan[1][7]);

					pThis->OpmHpfInp[0] = (pThis->OpmHpfInp[0]&(int)0xFFFFFC00) << 4;
					pThis->OpmHpfInp[1] = (pThis->OpmHpfInp[1]&(int)0xFFFFFC00) << 4;

					pThis->OpmHpfOut[0] = pThis->OpmHpfInp[0]-pThis->OpmHpfInp_prev[0]
									+pThis->OpmHpfOut[0]-(pThis->OpmHpfOut[0]>>10)-(pThis->OpmHpfOut[0]>>12);
					pThis->OpmHpfOut[1] = pThis->OpmHpfInp[1]-pThis->OpmHpfInp_prev[1]
									+pThis->OpmHpfOut[1]-(pThis->OpmHpfOut[1]>>10)-(pThis->OpmHpfOut[1]>>12);
					pThis->OpmHpfInp_prev[0] = pThis->OpmHpfInp[0];
					pThis->OpmHpfInp_prev[1] = pThis->OpmHpfInp[1];

					pThis->InpInpOpm[0] = pThis->OpmHpfOut[0] >> (4+5);
					pThis->InpInpOpm[1] = pThis->OpmHpfOut[1] >> (4+5);

//					pThis->InpInpOpm[0] = (pThis->InpInpOpm[0]&(int)0xFFFFFC00)
//									>> ((SIZESINTBL_BITS+PRECISION_BITS)-10-5); // 8*-2^17 ～ 8*+2^17
//					pThis->InpInpOpm[1] = (pThis->InpInpOpm[1]&(int)0xFFFFFC00)
//									>> ((SIZESINTBL_BITS+PRECISION_BITS)-10-5); // 8*-2^17 ～ 8*+2^17

					pThis->InpInpOpm[0] = pThis->InpInpOpm[0]*29;
					pThis->InpInpOpm[1] = pThis->InpInpOpm[1]*29;
					pThis->InpOpm[0] = (pThis->InpInpOpm[0] + pThis->InpInpOpm_prev[0]
						+ pThis->InpOpm[0]*70) >> 7;
					pThis->InpOpm[1] = (pThis->InpInpOpm[1] + pThis->InpInpOpm_prev[1]
						+ pThis->InpOpm[1]*70) >> 7;
					pThis->InpInpOpm_prev[0] = pThis->InpInpOpm[0];
					pThis->InpInpOpm_prev[1] = pThis->InpInpOpm[1];

					OutInpOpm[0] = pThis->InpOpm[0] >> 5; // 8*-2^12 ～ 8*+2^12
					OutInpOpm[1] = pThis->InpOpm[1] >> 5; // 8*-2^12 ～ 8*+2^12
//					OutInpOpm[0] = (pThis->InpOpm[0]*521) >> (5+9); // 8*-2^12 ～ 8*+2^12
//					OutInpOpm[1] = (pThis->InpOpm[1]*521) >> (5+9); // OPMとADPCMの音量バランス調整
			}	// pThis->UseOpmFlag

			if (pThis->UseAdpcmFlag) {
				pThis->OutInpAdpcm[0] = pThis->OutInpAdpcm[1] = 0;
				// pThis->OutInpAdpcm[] に Adpcm の出力PCMを加算
				{
					int	o;
					o = Adpcm_GetPcm62(&pThis->adpcm);
#if X68SOUND_ENABLE_PORTABLE_CODE
					if (o != (int)0x80000000) {
#else
					if (o != 0x80000000) {
#endif
							pThis->OutInpAdpcm[0] += ((((int)(pThis->PpiReg)>>1)&1)-1) & o;
							pThis->OutInpAdpcm[1] += (((int)(pThis->PpiReg)&1)-1) & o;
					}
				}

				// pThis->OutInpAdpcm[] に Pcm8 の出力PCMを加算
				{
					int ch;
					for (ch=0; ch<PCM8_NCH; ++ch) {
						int	o;
						/* FIXME: Should this be pan of Opm or renamed
							to avoid inner declaration warnings? */
						int pan;
						pan = Pcm8_GetMode(&pThis->pcm8[ch]);
						o = Pcm8_GetPcm62(&pThis->pcm8[ch]);
#if X68SOUND_ENABLE_PORTABLE_CODE
						if (o != (int)0x80000000) {
#else
						if (o != 0x80000000) {
#endif
								pThis->OutInpAdpcm[0] += (-(pan&1)) & o;
								pThis->OutInpAdpcm[1] += (-((pan>>1)&1)) & o;
						}
					}
				}


//				pThis->OutInpAdpcm[0] >>= 4;
//				pThis->OutInpAdpcm[1] >>= 4;

				// 音割れ防止
#if X68SOUND_ENABLE_PORTABLE_CODE
				#undef LIMITS
#endif
				#define	LIMITS	((1<<(15+4))-1)
				if ((unsigned int)(pThis->OutInpAdpcm[0]+LIMITS) > (unsigned int)(LIMITS*2)) {
					if ((int)(pThis->OutInpAdpcm[0]+LIMITS) >= (int)(LIMITS*2)) {
						pThis->OutInpAdpcm[0] = LIMITS;
					} else {
						pThis->OutInpAdpcm[0] = -LIMITS;
					}
				}
				if ((unsigned int)(pThis->OutInpAdpcm[1]+LIMITS) > (unsigned int)(LIMITS*2)) {
					if ((int)(pThis->OutInpAdpcm[1]+LIMITS) >= (int)(LIMITS*2)) {
						pThis->OutInpAdpcm[1] = LIMITS;
					} else {
						pThis->OutInpAdpcm[1] = -LIMITS;
					}
				}

				pThis->OutInpAdpcm[0] *= 26;
				pThis->OutInpAdpcm[1] *= 26;
				pThis->OutInpOutAdpcm[0] = (pThis->OutInpAdpcm[0] + pThis->OutInpAdpcm_prev[0]+pThis->OutInpAdpcm_prev[0] + pThis->OutInpAdpcm_prev2[0]
					- pThis->OutInpOutAdpcm_prev[0]*(-1537) - pThis->OutInpOutAdpcm_prev2[0]*617) >> 10;
				pThis->OutInpOutAdpcm[1] = (pThis->OutInpAdpcm[1] + pThis->OutInpAdpcm_prev[1]+pThis->OutInpAdpcm_prev[1] + pThis->OutInpAdpcm_prev2[1]
					- pThis->OutInpOutAdpcm_prev[1]*(-1537) - pThis->OutInpOutAdpcm_prev2[1]*617) >> 10;

				pThis->OutInpAdpcm_prev2[0] = pThis->OutInpAdpcm_prev[0];
				pThis->OutInpAdpcm_prev2[1] = pThis->OutInpAdpcm_prev[1];
				pThis->OutInpAdpcm_prev[0] = pThis->OutInpAdpcm[0];
				pThis->OutInpAdpcm_prev[1] = pThis->OutInpAdpcm[1];
				pThis->OutInpOutAdpcm_prev2[0] = pThis->OutInpOutAdpcm_prev[0];
				pThis->OutInpOutAdpcm_prev2[1] = pThis->OutInpOutAdpcm_prev[1];
				pThis->OutInpOutAdpcm_prev[0] = pThis->OutInpOutAdpcm[0];
				pThis->OutInpOutAdpcm_prev[1] = pThis->OutInpOutAdpcm[1];

				pThis->OutOutInpAdpcm[0] = pThis->OutInpOutAdpcm[0] * (356);
				pThis->OutOutInpAdpcm[1] = pThis->OutInpOutAdpcm[1] * (356);
				pThis->OutOutAdpcm[0] = (pThis->OutOutInpAdpcm[0] + pThis->OutOutInpAdpcm_prev[0]
					- pThis->OutOutAdpcm_prev[0]*(-312)) >> 10;
				pThis->OutOutAdpcm[1] = (pThis->OutOutInpAdpcm[1] + pThis->OutOutInpAdpcm_prev[1]
					- pThis->OutOutAdpcm_prev[1]*(-312)) >> 10;

				pThis->OutOutInpAdpcm_prev[0] = pThis->OutOutInpAdpcm[0];
				pThis->OutOutInpAdpcm_prev[1] = pThis->OutOutInpAdpcm[1];
				pThis->OutOutAdpcm_prev[0] = pThis->OutOutAdpcm[0];
				pThis->OutOutAdpcm_prev[1] = pThis->OutOutAdpcm[1];

//				OutInpOpm[0] += pThis->OutOutAdpcm[0] >> 4;	// -2048*16～+2048*16
//				OutInpOpm[1] += pThis->OutOutAdpcm[1] >> 4;	// -2048*16～+2048*16
				OutInpOpm[0] += (pThis->OutOutAdpcm[0]*506) >> (4+9);	// -2048*16～+2048*16
				OutInpOpm[1] += (pThis->OutOutAdpcm[1]*506) >> (4+9);	// OPMとADPCMの音量バランス調整
			}	// pThis->UseAdpcmFlag



			// 音割れ防止
#if X68SOUND_ENABLE_PORTABLE_CODE
			#undef LIMITS
#endif
			#define	LIMITS	((1<<15)-1)
			if ((unsigned int)(OutInpOpm[0]+LIMITS) > (unsigned int)(LIMITS*2)) {
				if ((int)(OutInpOpm[0]+LIMITS) >= (int)(LIMITS*2)) {
					OutInpOpm[0] = LIMITS;
				} else {
					OutInpOpm[0] = -LIMITS;
				}
			}
			if ((unsigned int)(OutInpOpm[1]+LIMITS) > (unsigned int)(LIMITS*2)) {
				if ((int)(OutInpOpm[1]+LIMITS) >= (int)(LIMITS*2)) {
					OutInpOpm[1] = LIMITS;
				} else {
					OutInpOpm[1] = -LIMITS;
				}
			}

			--pThis->InpOpm_idx;
			if (pThis->InpOpm_idx < 0) pThis->InpOpm_idx=OPMLPF_COL-1;
			pThis->InpOpmBuf0[pThis->InpOpm_idx] =
			pThis->InpOpmBuf0[pThis->InpOpm_idx+OPMLPF_COL] = (short)OutInpOpm[0];
			pThis->InpOpmBuf1[pThis->InpOpm_idx] =
			pThis->InpOpmBuf1[pThis->InpOpm_idx+OPMLPF_COL] = (short)OutInpOpm[1];
		}

		OpmFir(pThis->OpmLPFp, &pThis->InpOpmBuf0[pThis->InpOpm_idx], &pThis->InpOpmBuf1[pThis->InpOpm_idx], pThis->OutOpm);

		pThis->OpmLPFp += OPMLPF_COL;
		if (pThis->OpmLPFp >= OPMLOWPASS[OPMLPF_ROW]) {
			pThis->OpmLPFp = OPMLOWPASS[0];
		}

		// 全体の音量を調整
		pThis->OutOpm[0] = (pThis->OutOpm[0]*TotalVolume) >> 8;
		pThis->OutOpm[1] = (pThis->OutOpm[1]*TotalVolume) >> 8;

		Out[0] -= pThis->OutOpm[0];	// -4096 ～ +4096
		Out[1] -= pThis->OutOpm[1];


		// pThis->WaveFunc()の出力値を加算
		if (pThis->WaveFunc != NULL) {
			int	ret;
#if X68SOUND_ENABLE_PORTABLE_CODE
			ret = pThis->WaveFunc(pThis->WaveFuncArg);
#else
			ret = pThis->WaveFunc();
#endif
			Out[0] += (int)(short)ret;
			Out[1] += (ret>>16);
		}


		// 音割れ防止
		if ((unsigned int)(Out[0]+32767) > (unsigned int)(32767*2)) {
			if ((int)(Out[0]+32767) >= (int)(32767*2)) {
				Out[0] = 32767;
			} else {
				Out[0] = -32767;
			}
		}
		if ((unsigned int)(Out[1]+32767) > (unsigned int)(32767*2)) {
			if ((int)(Out[1]+32767) >= (int)(32767*2)) {
				Out[1] = 32767;
			} else {
				Out[1] = -32767;
			}
		}

		pThis->PcmBuf[pThis->PcmBufPtr][0] = Out[0];
		pThis->PcmBuf[pThis->PcmBufPtr][1] = Out[1];

		++pThis->PcmBufPtr;
		if (pThis->PcmBufPtr >= pThis->PcmBufSize) {
			pThis->PcmBufPtr = 0;
		}
	}

}

void Opm_pcmset22(OPM* pThis, int ndata) {

	int	i;
	for (i=0; i<ndata; ++i) {
		int	Out[2];
		Out[0] = Out[1] = 0;

		if (pThis->UseOpmFlag) {
#if X68SOUND_ENABLE_PORTABLE_CODE
			pThis->RateForPcmset22-=OpmRate;
			while (pThis->RateForPcmset22<0) {
				pThis->RateForPcmset22+=22050;
#else
			static int rate=0,rate2=0;

//			rate-=62500;
			rate-=OpmRate;
			while (rate<0) {
				rate+=22050;
#endif

				Opm_timer(pThis);
				Opm_ExecuteCmnd(pThis);
				if ((--pThis->EnvCounter2) == 0) {
					pThis->EnvCounter2 = 3;
					++pThis->EnvCounter1;
					int slot;
					for (slot=0; slot<32; ++slot) {
#if X68SOUND_ENABLE_PORTABLE_CODE
						Op_Envelope(&pThis->op[slot&7][slot>>3], pThis->EnvCounter1);
#else
						Op_Envelope(&pThis->op[0][slot], pThis->EnvCounter1);
#endif
					}
				}
			}

					{
						Lfo_Update(&pThis->lfo);

						int lfopitch[8];
						int lfolevel[8];
						int	ch;
						for (ch=0; ch<8; ++ch) {
							pThis->op[ch][1].inp=pThis->op[ch][2].inp=pThis->op[ch][3].inp=pThis->OpOut[ch]=0;

							lfopitch[ch] = Lfo_GetPmValue(&pThis->lfo,  ch);
							lfolevel[ch] = Lfo_GetAmValue(&pThis->lfo,  ch);
						}
						for (ch=0; ch<8; ++ch) {
							Op_Output0(&pThis->op[ch][0], lfopitch[ch], lfolevel[ch]);
						}
						for (ch=0; ch<8; ++ch) {
							Op_Output(&pThis->op[ch][1], lfopitch[ch], lfolevel[ch]);
						}
						for (ch=0; ch<8; ++ch) {
							Op_Output(&pThis->op[ch][2], lfopitch[ch], lfolevel[ch]);
						}
						for (ch=0; ch<7; ++ch) {
							Op_Output(&pThis->op[ch][3], lfopitch[ch], lfolevel[ch]);
						}
						Op_Output32(&pThis->op[7][3], lfopitch[7], lfolevel[7]);
					}


				// pThis->InpInpOpm[] に OPM の出力PCMをステレオ加算
				pThis->InpInpOpm[0] =    (pThis->OpOut[0] & pThis->pan[0][0])
								+ (pThis->OpOut[1] & pThis->pan[0][1])
								+ (pThis->OpOut[2] & pThis->pan[0][2])
								+ (pThis->OpOut[3] & pThis->pan[0][3])
								+ (pThis->OpOut[4] & pThis->pan[0][4])
								+ (pThis->OpOut[5] & pThis->pan[0][5])
								+ (pThis->OpOut[6] & pThis->pan[0][6])
								+ (pThis->OpOut[7] & pThis->pan[0][7]);
				pThis->InpInpOpm[1] =    (pThis->OpOut[0] & pThis->pan[1][0])
								+ (pThis->OpOut[1] & pThis->pan[1][1])
								+ (pThis->OpOut[2] & pThis->pan[1][2])
								+ (pThis->OpOut[3] & pThis->pan[1][3])
								+ (pThis->OpOut[4] & pThis->pan[1][4])
								+ (pThis->OpOut[5] & pThis->pan[1][5])
								+ (pThis->OpOut[6] & pThis->pan[1][6])
								+ (pThis->OpOut[7] & pThis->pan[1][7]);

				{

					pThis->InpInpOpm[0] = (pThis->InpInpOpm[0]&(int)0xFFFFFC00)
									>> ((SIZESINTBL_BITS+PRECISION_BITS)-10-5); // 8*-2^17 ～ 8*+2^17
					pThis->InpInpOpm[1] = (pThis->InpInpOpm[1]&(int)0xFFFFFC00)
									>> ((SIZESINTBL_BITS+PRECISION_BITS)-10-5); // 8*-2^17 ～ 8*+2^17
				}

				pThis->InpInpOpm[0] += (pThis->InpInpOpm[0]<<4)+pThis->InpInpOpm[0];	// * 18
				pThis->InpInpOpm[1] += (pThis->InpInpOpm[1]<<4)+pThis->InpInpOpm[1];	// * 18
				pThis->InpOpm[0] = (pThis->InpInpOpm[0] + pThis->InpInpOpm_prev[0]+pThis->InpInpOpm_prev[0] + pThis->InpInpOpm_prev2[0]
					+ pThis->InpOpm_prev[0]+pThis->InpOpm_prev[0]+pThis->InpOpm_prev[0] - pThis->InpOpm_prev2[0]*11) >> 6;
				pThis->InpOpm[1] = (pThis->InpInpOpm[1] + pThis->InpInpOpm_prev[1]+pThis->InpInpOpm_prev[1] + pThis->InpInpOpm_prev2[1]
					+ pThis->InpOpm_prev[1]+pThis->InpOpm_prev[1]+pThis->InpOpm_prev[1] - pThis->InpOpm_prev2[1]*11) >> 6;

				pThis->InpInpOpm_prev2[0] = pThis->InpInpOpm_prev[0];
				pThis->InpInpOpm_prev2[1] = pThis->InpInpOpm_prev[1];
				pThis->InpInpOpm_prev[0] = pThis->InpInpOpm[0];
				pThis->InpInpOpm_prev[1] = pThis->InpInpOpm[1];
				pThis->InpOpm_prev2[0] = pThis->InpOpm_prev[0];
				pThis->InpOpm_prev2[1] = pThis->InpOpm_prev[1];
				pThis->InpOpm_prev[0] = pThis->InpOpm[0];
				pThis->InpOpm_prev[1] = pThis->InpOpm[1];

//			}

			// 全体の音量を調整
			pThis->OutOpm[0] = (pThis->InpOpm[0]*TotalVolume) >> 8;
			pThis->OutOpm[1] = (pThis->InpOpm[1]*TotalVolume) >> 8;

			Out[0] -= pThis->OutOpm[0]>>(5);	// -4096 ～ +4096
			Out[1] -= pThis->OutOpm[1]>>(5);
		}

		if (pThis->UseAdpcmFlag) {
#if X68SOUND_ENABLE_PORTABLE_CODE
				pThis->Rate2ForPcmset22 -= 15625;
				if (pThis->Rate2ForPcmset22 < 0) {
					pThis->Rate2ForPcmset22 += 22050;
#else
			static int rate=0,rate2=0;

				rate2 -= 15625;
				if (rate2 < 0) {
					rate2 += 22050;
#endif

					pThis->OutInpAdpcm[0] = pThis->OutInpAdpcm[1] = 0;
					// pThis->OutInpAdpcm[] に Adpcm の出力PCMを加算
					{
						int	o;
						o = Adpcm_GetPcm(&pThis->adpcm);
#if X68SOUND_ENABLE_PORTABLE_CODE
						if (o != (int)0x80000000) {
#else
						if (o != 0x80000000) {
#endif
							pThis->OutInpAdpcm[0] += ((((int)(pThis->PpiReg)>>1)&1)-1) & o;
							pThis->OutInpAdpcm[1] += (((int)(pThis->PpiReg)&1)-1) & o;
						}
					}

					// pThis->OutInpAdpcm[] に Pcm8 の出力PCMを加算
					{
						int ch;
						for (ch=0; ch<PCM8_NCH; ++ch) {
							/* FIXME: Should this be pan of Opm or renamed
								to avoid inner declaration warnings? */
							int	o;
							int pan;
							pan = Pcm8_GetMode(&pThis->pcm8[ch]);

							o = Pcm8_GetPcm(&pThis->pcm8[ch]);
#if X68SOUND_ENABLE_PORTABLE_CODE
							if (o != (int)0x80000000) {
#else
							if (o != 0x80000000) {
#endif
								pThis->OutInpAdpcm[0] += (-(pan&1)) & o;
								pThis->OutInpAdpcm[1] += (-((pan>>1)&1)) & o;
							}
						}
					}

					// 全体の音量を調整
//					pThis->OutInpAdpcm[0] = (pThis->OutInpAdpcm[0]*TotalVolume) >> 8;
//					pThis->OutInpAdpcm[1] = (pThis->OutInpAdpcm[1]*TotalVolume) >> 8;

					// 音割れ防止
#if X68SOUND_ENABLE_PORTABLE_CODE
					#undef LIMITS
#endif
					#define	LIMITS	((1<<19)-1)
					if ((unsigned int)(pThis->OutInpAdpcm[0]+LIMITS) > (unsigned int)(LIMITS*2)) {
						if ((int)(pThis->OutInpAdpcm[0]+LIMITS) >= (int)(LIMITS*2)) {
							pThis->OutInpAdpcm[0] = LIMITS;
						} else {
							pThis->OutInpAdpcm[0] = -LIMITS;
						}
					}
					if ((unsigned int)(pThis->OutInpAdpcm[1]+LIMITS) > (unsigned int)(LIMITS*2)) {
						if ((int)(pThis->OutInpAdpcm[1]+LIMITS) >= (int)(LIMITS*2)) {
							pThis->OutInpAdpcm[1] = LIMITS;
						} else {
							pThis->OutInpAdpcm[1] = -LIMITS;
						}
					}

					pThis->OutInpAdpcm[0] *= 40;
					pThis->OutInpAdpcm[1] *= 40;
				}

				pThis->OutOutAdpcm[0] = (pThis->OutInpAdpcm[0] + pThis->OutInpAdpcm_prev[0]+pThis->OutInpAdpcm_prev[0] + pThis->OutInpAdpcm_prev2[0]
								- pThis->OutOutAdpcm_prev[0]*(-157) - pThis->OutOutAdpcm_prev2[0]*(61)) >> 8;
				pThis->OutOutAdpcm[1] = (pThis->OutInpAdpcm[1] + pThis->OutInpAdpcm_prev[1]+pThis->OutInpAdpcm_prev[1] + pThis->OutInpAdpcm_prev2[1]
								- pThis->OutOutAdpcm_prev[1]*(-157) - pThis->OutOutAdpcm_prev2[1]*(61)) >> 8;

				pThis->OutInpAdpcm_prev2[0] = pThis->OutInpAdpcm_prev[0];
				pThis->OutInpAdpcm_prev2[1] = pThis->OutInpAdpcm_prev[1];
				pThis->OutInpAdpcm_prev[0] = pThis->OutInpAdpcm[0];
				pThis->OutInpAdpcm_prev[1] = pThis->OutInpAdpcm[1];
				pThis->OutOutAdpcm_prev2[0] = pThis->OutOutAdpcm_prev[0];
				pThis->OutOutAdpcm_prev2[1] = pThis->OutOutAdpcm_prev[1];
				pThis->OutOutAdpcm_prev[0] = pThis->OutOutAdpcm[0];
				pThis->OutOutAdpcm_prev[1] = pThis->OutOutAdpcm[1];



//			Out[0] += OutAdpcm[0] >> 4;	// -2048*16～+2048*16
//			Out[1] += OutAdpcm[1] >> 4;
			Out[0] -= pThis->OutOutAdpcm[0] >> 4;	// -2048*16～+2048*16
			Out[1] -= pThis->OutOutAdpcm[1] >> 4;
		}

//		// 全体の音量を調整
//		Out[0] = (Out[0]*TotalVolume) >> 8;
//		Out[1] = (Out[1]*TotalVolume) >> 8;


		// pThis->WaveFunc()の出力値を加算
		if (pThis->WaveFunc != NULL) {
			int	ret;
#if X68SOUND_ENABLE_PORTABLE_CODE
			ret = pThis->WaveFunc(pThis->WaveFuncArg);
#else
			ret = pThis->WaveFunc();
#endif
			Out[0] += (int)(short)ret;
			Out[1] += (ret>>16);
		}


		// 音割れ防止
		if ((unsigned int)(Out[0]+32767) > (unsigned int)(32767*2)) {
			if ((int)(Out[0]+32767) >= (int)(32767*2)) {
				Out[0] = 32767;
			} else {
				Out[0] = -32767;
			}
		}
		if ((unsigned int)(Out[1]+32767) > (unsigned int)(32767*2)) {
			if ((int)(Out[1]+32767) >= (int)(32767*2)) {
				Out[1] = 32767;
			} else {
				Out[1] = -32767;
			}
		}

		pThis->PcmBuf[pThis->PcmBufPtr][0] = Out[0];
		pThis->PcmBuf[pThis->PcmBufPtr][1] = Out[1];

		++pThis->PcmBufPtr;
		if (pThis->PcmBufPtr >= pThis->PcmBufSize) {
			pThis->PcmBufPtr = 0;
		}
	}
}

int Opm_GetPcm(OPM* pThis, void *buf, int ndata) {
	if (pThis->Dousa_mode != 2) {
		return X68SNDERR_NOTACTIVE;
	}
	pThis->PcmBuf = (short (*)[2])buf;
	pThis->PcmBufPtr=0;
	if (Samprate != 22050) {
		Opm_pcmset62(pThis, ndata);
	} else {
		Opm_pcmset22(pThis, ndata);
	}
	pThis->PcmBuf = NULL;
	return 0;
}

void Opm_timer(OPM* pThis) {
	char	res;
#if X68SOUND_ENABLE_PORTABLE_CODE
	{
		int eax = 0;
		int ecx = 1;
		if(	TimerSemapho == eax ){
			TimerSemapho = ecx;
			res = 0;
		} else {
			eax = TimerSemapho;
			res = 1;
		}
	}
#else
	__asm {
		xor		eax,eax
		mov		ecx,1
		lock cmpxchg	TimerSemapho,ecx
		setnz	res
	}
#endif
	if (res) {
		return;
	}

	int	prev_stat = pThis->StatReg;
	int flag_set = 0;
	if (pThis->TimerReg & 0x01) {	// pThis->TimerA 動作中
		++pThis->TimerAcounter;
		if (pThis->TimerAcounter >= pThis->TimerA) {
			flag_set |= ((pThis->TimerReg>>2) & 0x01);
			pThis->TimerAcounter = 0;
		}
	}
	if (pThis->TimerReg & 0x02) {	// pThis->TimerB 動作中
		++pThis->TimerBcounter;
		if (pThis->TimerBcounter >= pThis->TimerB) {
			flag_set |= ((pThis->TimerReg>>2) & 0x02);
			pThis->TimerBcounter = 0;
		}
	}

//	int	next_stat = pThis->StatReg;

	pThis->StatReg |= flag_set;

	TimerSemapho = 0;

	if (flag_set != 0) {
		if (
			prev_stat == 0) {
			if (pThis->OpmIntProc != NULL) {
#if X68SOUND_ENABLE_PORTABLE_CODE
				pThis->OpmIntProc(pThis->OpmIntArg);
#else
				pThis->OpmIntProc();
#endif
			}
		}
	}

}


int Opm_Start(OPM* pThis, int samprate, int opmflag, int adpcmflag,
						int betw, int pcmbuf, int late, double rev) {
	if (pThis->Dousa_mode != 0) {
		return X68SNDERR_ALREADYACTIVE;
	}
	pThis->Dousa_mode = 1;

	if (rev < 0.1) rev = 0.1;

	pThis->UseOpmFlag = opmflag;
	pThis->UseAdpcmFlag = adpcmflag;
	pThis->_betw = betw;
	pThis->_pcmbuf = pcmbuf;
	pThis->_late = late;
#if X68SOUND_ENABLE_PORTABLE_CODE
	pThis->_rev = (int)rev;
#else
	pThis->_rev = rev;
#endif

	if (samprate == 44100) {
		Samprate = 62500;
		WaveOutSamp = 44100;
		OPMLPF_ROW = OPMLPF_ROW_44;
		OPMLOWPASS = OPMLOWPASS_44;
	} else if (samprate == 48000) {
		Samprate = 62500;
		WaveOutSamp = 48000;
		OPMLPF_ROW = OPMLPF_ROW_48;
		OPMLOWPASS = OPMLOWPASS_48;
	} else {
		Samprate = 22050;
		WaveOutSamp = 22050;
	}

	Opm_MakeTable(pThis);
	Opm_Reset(pThis);

	return Opm_WaveAndTimerStart(pThis);
}

int Opm_StartPcm(OPM* pThis, int samprate, int opmflag, int adpcmflag, int pcmbuf) {
	if (pThis->Dousa_mode != 0) {
		return X68SNDERR_ALREADYACTIVE;
	}
	pThis->Dousa_mode = 2;

	pThis->UseOpmFlag = opmflag;
	pThis->UseAdpcmFlag = adpcmflag;
	pThis->_betw = 5;
	pThis->_pcmbuf = pcmbuf;
	pThis->_late = 200;
#if X68SOUND_ENABLE_PORTABLE_CODE
	pThis->_rev = 1;
#else
	pThis->_rev = (int)1.0;
#endif

	if (samprate == 44100) {
		Samprate = 62500;
		WaveOutSamp = 44100;
		OPMLPF_ROW = OPMLPF_ROW_44;
		OPMLOWPASS = OPMLOWPASS_44;
	} else if (samprate == 48000) {
		Samprate = 62500;
		WaveOutSamp = 48000;
		OPMLPF_ROW = OPMLPF_ROW_48;
		OPMLOWPASS = OPMLOWPASS_48;
	} else {
		Samprate = 22050;
		WaveOutSamp = 22050;
	}

	Opm_MakeTable(pThis);
	Opm_Reset(pThis);

	pThis->PcmBufSize = 0xFFFFFFFF;

	return Opm_WaveAndTimerStart(pThis);
}

int Opm_SetSamprate(OPM* pThis, int samprate) {
	if (pThis->Dousa_mode == 0) {
		return X68SNDERR_NOTACTIVE;
	}
	int dousa_mode_bak = pThis->Dousa_mode;

	Opm_Free(pThis);

	if (samprate == 44100) {
		Samprate = 62500;
		WaveOutSamp = 44100;
		OPMLPF_ROW = OPMLPF_ROW_44;
		OPMLOWPASS = OPMLOWPASS_44;
	} else if (samprate == 48000) {
		Samprate = 62500;
		WaveOutSamp = 48000;
		OPMLPF_ROW = OPMLPF_ROW_48;
		OPMLOWPASS = OPMLOWPASS_48;
	} else {
		Samprate = 22050;
		WaveOutSamp = 22050;
	}

	Opm_MakeTable(pThis);
	Opm_ResetSamprate(pThis);

	pThis->Dousa_mode = dousa_mode_bak;
	return Opm_WaveAndTimerStart(pThis);
}

int Opm_SetOpmClock(OPM* pThis, int clock) {
	int rate = clock >> 6;
	if (rate <= 0) {
		return X68SNDERR_BADARG;
	}
	if (pThis->Dousa_mode == 0) {
		OpmRate = rate;
		return 0;
	}
	int dousa_mode_bak = pThis->Dousa_mode;

	Opm_Free(pThis);

	OpmRate = rate;

	Opm_MakeTable(pThis);
	Opm_ResetSamprate(pThis);

	pThis->Dousa_mode = dousa_mode_bak;
	return Opm_WaveAndTimerStart(pThis);
}

int Opm_WaveAndTimerStart(OPM* pThis) {

	Betw_Time = pThis->_betw;
	TimerResolution = pThis->_betw;
	Late_Time = pThis->_late+pThis->_betw;
#if X68SOUND_ENABLE_PORTABLE_CODE
	Betw_Samples_Slower = (int)floor((double)(WaveOutSamp)*pThis->_betw/1000.0-pThis->_rev);
	Betw_Samples_Faster = (int)ceil((double)(WaveOutSamp)*pThis->_betw/1000.0+pThis->_rev);
	Betw_Samples_VerySlower = (int)(floor((double)(WaveOutSamp)*pThis->_betw/1000.0-pThis->_rev)/8.0);
#else
	Betw_Samples_Slower = floor((double)(WaveOutSamp)*pThis->_betw/1000.0-pThis->_rev);
	Betw_Samples_Faster = ceil((double)(WaveOutSamp)*pThis->_betw/1000.0+pThis->_rev);
	Betw_Samples_VerySlower = floor((double)(WaveOutSamp)*pThis->_betw/1000.0-pThis->_rev)/8.0;
#endif
	Late_Samples = WaveOutSamp*Late_Time/1000;

//	Blk_Samples = WaveOutSamp/N_waveblk;
	Blk_Samples = Late_Samples;

//	Faster_Limit = Late_Samples;
//	Faster_Limit = WaveOutSamp*50/1000;
	if (Late_Samples >= WaveOutSamp*175/1000) {
		Faster_Limit = Late_Samples - WaveOutSamp*125/1000;
	} else {
		Faster_Limit = WaveOutSamp*50/1000;
	}
	if (Faster_Limit > Late_Samples) Faster_Limit = Late_Samples;
	Slower_Limit = Faster_Limit;
//	Slower_Limit = WaveOutSamp*100/1000;
//	Slower_Limit = Late_Samples;
	if (Slower_Limit > Late_Samples) Slower_Limit = Late_Samples;

	if (pThis->Dousa_mode != 1) {
		return 0;
	}



	pThis->PcmBufSize = Blk_Samples * N_waveblk;
	nSamples = (unsigned int)(Betw_Samples_Faster);


#if !X68SOUND_ENABLE_PORTABLE_CODE
	thread_handle = CreateThread(NULL, 0, waveOutThread, NULL, 0, &thread_id);
	if (thread_handle == NULL) {
		Opm_Free(pThis);
		ErrorCode = 5;
		return X68SNDERR_TIMER;
	}
//	SetThreadPriority(thread_handle, THREAD_PRIORITY_ABOVE_NORMAL );
//	SetThreadPriority(thread_handle, THREAD_PRIORITY_BELOW_NORMAL );
	SetThreadPriority(thread_handle, THREAD_PRIORITY_HIGHEST );
	while (!thread_flag) Sleep(100);




	MMRESULT	ret;


	wfx.wFormatTag = WAVE_FORMAT_PCM;
	wfx.nChannels = 2;
	wfx.nSamplesPerSec = WaveOutSamp;
	wfx.wBitsPerSample = 16;
	wfx.nBlockAlign = wfx.nChannels*(wfx.wBitsPerSample/8);
	wfx.nAvgBytesPerSec = wfx.nSamplesPerSec*wfx.nBlockAlign;
	wfx.cbSize = 0;

	timer_start_flag = 0;
	if ((ret=waveOutOpen(&hwo, WAVE_MAPPER, &wfx, (DWORD)waveOutProc, NULL, CALLBACK_FUNCTION ))
		!= MMSYSERR_NOERROR) {
		hwo = NULL;
		Opm_Free(pThis);
		ErrorCode = 0x10000000+ret;
		return X68SNDERR_PCMOUT;
	}
//	if (waveOutReset(hwo) != MMSYSERR_NOERROR) {
//		waveOutClose(hwo);
//		hwo = NULL;
//		return X68SNDERR_PCMOUT;
//	}


	pThis->PcmBuf = (short (*)[2])GlobalAllocPtr(GMEM_MOVEABLE | GMEM_SHARE, pThis->PcmBufSize*2*2 );
	if (!pThis->PcmBuf) {
		Opm_Free(pThis);
		ErrorCode = 2;
		return X68SNDERR_MEMORY;
	}
	lpwh = (LPWAVEHDR)GlobalAllocPtr(GMEM_MOVEABLE | GMEM_SHARE,
                  (DWORD) sizeof(WAVEHDR)*N_waveblk);
	if (!lpwh) {
		Opm_Free(pThis);
		ErrorCode = 3;
		return X68SNDERR_MEMORY;
	}

//	pcmset(Late_Samples);
	{
		unsigned int i;
		for (i=0; i<pThis->PcmBufSize; ++i) {
			pThis->PcmBuf[i][0] = pThis->PcmBuf[i][1] = 0;
		}
	}

	{
		int i;
		N_wavehdr = 0;
		for (i=0; i<N_waveblk; ++i) {
			(lpwh+i)->lpData = (LPSTR)pThis->PcmBuf[Blk_Samples*i];
			(lpwh+i)->dwBufferLength = Blk_Samples*2*2;
			(lpwh+i)->dwFlags = 0;

			if ((ret=waveOutPrepareHeader(hwo, lpwh+i, sizeof(WAVEHDR))) != MMSYSERR_NOERROR) {
				Opm_Free(pThis);
				ErrorCode = 0x20000000+ret;
				return X68SNDERR_PCMOUT;
			}
			++N_wavehdr;
		}
	}

	pThis->PcmBufPtr = Blk_Samples + Late_Samples + Betw_Samples_Faster;
	while (pThis->PcmBufPtr >= pThis->PcmBufSize) pThis->PcmBufPtr -= pThis->PcmBufSize;
	waveblk = 0;
	playingblk = 0;
//	playingblk_next = playingblk+1;
	{
		int i;
		for (i=0; i<N_waveblk; ++i) {
			PostThreadMessage(thread_id, THREADMES_WAVEOUTDONE, 0, 0);
		}
	}

	timeBeginPeriod(TimerResolution);
	pThis->TimerID=timeSetEvent(Betw_Time, TimerResolution, OpmTimeProc, NULL, TIME_PERIODIC);
	if (pThis->TimerID == NULL) {
		Opm_Free(pThis);
		ErrorCode = 4;
		return X68SNDERR_TIMER;
	}

	while (!timer_start_flag) Sleep(200);	// マルチメディアタイマーの処理が開始されるまで待つ
#endif

	return 0;
}


void Opm_Free(OPM* pThis) {
#if !X68SOUND_ENABLE_PORTABLE_CODE
	if (pThis->TimerID != NULL) {	// マルチメディアタイマー停止
		timeKillEvent(pThis->TimerID);
		pThis->TimerID = NULL;
		timeEndPeriod(TimerResolution);
	}
	if (thread_handle) {	// スレッド停止
		PostThreadMessage(thread_id, THREADMES_KILL, 0, 0);
		WaitForSingleObject(thread_handle, INFINITE);
		CloseHandle(thread_handle);
		thread_handle = NULL;
	}
	timer_start_flag = 0;	// マルチメディアタイマーの処理を停止
	if (hwo != NULL) {		// ウェーブ再生停止
//		waveOutReset(hwo);
		if (lpwh) {
			int i;
			for (i=0; i<N_wavehdr; ++i) {
				waveOutUnprepareHeader(hwo, lpwh+i, sizeof(WAVEHDR));
			}
			N_wavehdr = 0;
			GlobalFreePtr( lpwh );
			lpwh=NULL;
		}
		waveOutClose(hwo);
		if (pThis->PcmBuf) { GlobalFreePtr( pThis->PcmBuf ); pThis->PcmBuf=NULL; }
		hwo = NULL;
	}
#endif


	pThis->Dousa_mode = 0;
}

void Opm_Destruct(OPM* pThis) {
	Opm_Free(pThis);
}


#if X68SOUND_ENABLE_PORTABLE_CODE
void Opm_OpmInt(OPM* pThis, void (*proc)(void *), void *arg) {
	pThis->OpmIntProc = proc;
	pThis->OpmIntArg = arg;
#else
void Opm_OpmInt(OPM* pThis, void (CALLBACK *proc)()) {
	pThis->OpmIntProc = proc;
#endif
}

unsigned char Opm_AdpcmPeek(OPM* pThis) {
	return pThis->adpcm.AdpcmReg;
}
void Opm_AdpcmPoke(OPM* pThis, unsigned char data) {
//#ifdef ADPCM_POLY
#if 0
	// PCM8テスト
	if (data & 0x02) {	// ADPCM再生開始
		static int pcm8_pantbl[4]={3,1,2,0};
		int minch=0,ch;
		unsigned int minlen=0xFFFFFFFF;
		for (ch=0; ch<PCM8_NCH; ++ch) {
			if ((unsigned int)Pcm8_GetRest(&pThis->pcm8[ch]) < minlen) {
				minlen = Pcm8_GetRest(&pThis->pcm8[ch]);
				minch = ch;
			}
		}
		if (pThis->adpcm.DmaReg[0x05] & 0x08) {	// チェイニング動作
			if (!(pThis->adpcm.DmaReg[0x05] & 0x04)) {	// アレイチェイン
				Pcm8_Aot(&pThis->pcm8[minch], bswapl(*(unsigned char **)&(pThis->adpcm.DmaReg[0x1C])),
					(8<<16)+(ADPCMRATETBL[pThis->AdpcmBaseClock][(pThis->PpiReg>>2)&3]<<8)+pcm8_pantbl[pThis->PpiReg&3],
					bswapw(*(unsigned short *)&(pThis->adpcm.DmaReg[0x1A])));
			} else {						// リンクアレイチェイン
				Pcm8_Lot(&pThis->pcm8[minch], bswapl(*(unsigned char **)&(pThis->adpcm.DmaReg[0x1C])),
					(8<<16)+(ADPCMRATETBL[pThis->AdpcmBaseClock][(pThis->PpiReg>>2)&3]<<8)+pcm8_pantbl[pThis->PpiReg&3]);
			}
		} else {	// ノーマル転送
			Pcm8_Out(&pThis->pcm8[minch], bswapl(*(unsigned char **)&(pThis->adpcm.DmaReg[0x0C])),
				(8<<16)+(ADPCMRATETBL[pThis->AdpcmBaseClock][(pThis->PpiReg>>2)&3]<<8)+pcm8_pantbl[pThis->PpiReg&3],
				bswapw(*(unsigned short *)&(pThis->adpcm.DmaReg[0x0A])));
		}
		if (pThis->adpcm.IntProc != NULL) {
			Adpcm_IntProc(&pThis->adpcm);
		}
	} else if (data & 0x01) {	// 再生動作停止
	}
	return;
#endif

	// オリジナル
	if (data & 0x02) {	// ADPCM再生開始
		pThis->adpcm.AdpcmReg &= 0x7F;
	} else if (data & 0x01) {	// 再生動作停止
		pThis->adpcm.AdpcmReg |= 0x80;
		Adpcm_Reset(&pThis->adpcm);
	}
}
unsigned char Opm_PpiPeek(OPM* pThis) {
	return pThis->PpiReg;
}
void Opm_PpiPoke(OPM* pThis, unsigned char data) {
	pThis->PpiReg = data;
	Opm_SetAdpcmRate(pThis);
}
void Opm_PpiCtrl(OPM* pThis, unsigned char data) {
	if (!(data&0x80)) {
		if (data&0x01) {
			pThis->PpiReg |= 1<<((data>>1)&7);
		} else {
			pThis->PpiReg &= 0xFF^(1<<((data>>1)&7));
		}
		Opm_SetAdpcmRate(pThis);
	}
}
unsigned char Opm_DmaPeek(OPM* pThis, unsigned char adrs) {
	if (adrs >= 0x40) return 0;
	if (adrs == 0x00) {
		if ((pThis->adpcm.AdpcmReg & 0x80) == 0) {	// ADPCM 再生中
#if X68SOUND_ENABLE_PORTABLE_CODE
			pThis->adpcm.DmaReg.asUint8[0x00] |= 0x02;
			return pThis->adpcm.DmaReg.asUint8[0x00] | 0x01;
#else
			pThis->adpcm.DmaReg[0x00] |= 0x02;
			return pThis->adpcm.DmaReg[0x00] | 0x01;
#endif
		}
	}
#if X68SOUND_ENABLE_PORTABLE_CODE
	return pThis->adpcm.DmaReg.asUint8[adrs];
#else
	return pThis->adpcm.DmaReg[adrs];
#endif
}
void Opm_DmaPoke(OPM* pThis, unsigned char adrs, unsigned char data) {
	if (adrs >= 0x40) return;
	switch (adrs) {
	case 0x00:	// CSR
		data &= 0xF6;					// ACTとPCSはクリアしない
#if X68SOUND_ENABLE_PORTABLE_CODE
		pThis->adpcm.DmaReg.asUint8[adrs] &= ~data;
#else
		pThis->adpcm.DmaReg[adrs] &= ~data;
#endif
		if (data & 0x10) {
#if X68SOUND_ENABLE_PORTABLE_CODE
			pThis->adpcm.DmaReg.asUint8[0x01] = 0;
#else
			pThis->adpcm.DmaReg[0x01] = 0;
#endif
		}
		return;
		break;
	case 0x01:	// CER
		return;
		break;
	case 0x04: // DCR
	case 0x05: // OCR
	case 0x06: // SCR
	case 0x0A: // MTC
	case 0x0B: // MTC
	case 0x0C: // MAR
	case 0x0D: // MAR
	case 0x0E: // MAR
	case 0x0F: // MAR
	case 0x14: // DAR
	case 0x15: // DAR
	case 0x16: // DAR
	case 0x17: // DAR
	case 0x29: // MFC
	case 0x31: // DFC
#if X68SOUND_ENABLE_PORTABLE_CODE
		if (pThis->adpcm.DmaReg.asUint8[0x00] & 0x08) {	// ACT==1 ?
#else
		if (pThis->adpcm.DmaReg[0x00] & 0x08) {	// ACT==1 ?
#endif
			Adpcm_DmaError(&pThis->adpcm, 0x02);	// 動作タイミングエラー
			break;
		}
	case 0x1A:	// BTC
	case 0x1B:	// BTC
	case 0x1C:	// BAR
	case 0x1D:	// BAR
	case 0x1E:	// BAR
	case 0x1F:	// BAR
	case 0x25:	// NIV
	case 0x27:	// EIV
	case 0x2D:	// CPR
	case 0x39:	// BFC
	case 0x3F:	// GCR
#if X68SOUND_ENABLE_PORTABLE_CODE
		pThis->adpcm.DmaReg.asUint8[adrs] = data;
#else
		pThis->adpcm.DmaReg[adrs] = data;
#endif
		break;

	case 0x07:
#if X68SOUND_ENABLE_PORTABLE_CODE
		pThis->adpcm.DmaReg.asUint8[0x07] = data & 0x78;
#else
		pThis->adpcm.DmaReg[0x07] = data & 0x78;
#endif
		if (data&0x80) {		// STR == 1 ?

#if X68SOUND_ENABLE_PORTABLE_CODE
			if (pThis->adpcm.DmaReg.asUint8[0x00] & 0xF8) {	// COC|BTC|NDT|ERR|ACT == 1 ?
				Adpcm_DmaError(&pThis->adpcm, 0x02);	// 動作タイミングエラー
				pThis->adpcm.DmaReg.asUint8[0x07] = data & 0x28;
#else
			if (pThis->adpcm.DmaReg[0x00] & 0xF8) {	// COC|BTC|NDT|ERR|ACT == 1 ?
				Adpcm_DmaError(&pThis->adpcm, 0x02);	// 動作タイミングエラー
				pThis->adpcm.DmaReg[0x07] = data & 0x28;
#endif
				break;
			}
#if X68SOUND_ENABLE_PORTABLE_CODE
			pThis->adpcm.DmaReg.asUint8[0x00] |= 0x08;	// ACT=1
#else
			pThis->adpcm.DmaReg[0x00] |= 0x08;	// ACT=1
#endif
//pThis->adpcm.FinishFlag=0;
#if X68SOUND_ENABLE_PORTABLE_CODE
			if (  pThis->adpcm.DmaReg.asUint8[0x04] & 0x08		// DPS != 0 ?
				||pThis->adpcm.DmaReg.asUint8[0x06] & 0x03		// DAC != 00 ?
				||bswapl(pThis->adpcm.DmaReg.asUint32[0x14/4]) != 0x00E92003) {
				Adpcm_DmaError(&pThis->adpcm, 0x0A);	// バスエラー(デバイスアドレス)
				pThis->adpcm.DmaReg.asUint8[0x07] = data & 0x28;
#else
			if (  pThis->adpcm.DmaReg[0x04] & 0x08		// DPS != 0 ?
				||pThis->adpcm.DmaReg[0x06] & 0x03		// DAC != 00 ?
				||bswapl(*(unsigned char **)&pThis->adpcm.DmaReg[0x14]) != (unsigned char *)0x00E92003) {
				Adpcm_DmaError(&pThis->adpcm, 0x0A);	// バスエラー(デバイスアドレス)
				pThis->adpcm.DmaReg[0x07] = data & 0x28;
#endif
				break;
			}
			unsigned char ocr;
#if X68SOUND_ENABLE_PORTABLE_CODE
			ocr = pThis->adpcm.DmaReg.asUint8[0x05] & 0xB0;
#else
			ocr = pThis->adpcm.DmaReg[0x05] & 0xB0;
#endif
			if (ocr != 0x00 && ocr != 0x30) {	// DIR==1 || SIZE!=00&&SIZE!=11 ?
				Adpcm_DmaError(&pThis->adpcm, 0x01);	// コンフィグレーションエラー
#if X68SOUND_ENABLE_PORTABLE_CODE
				pThis->adpcm.DmaReg.asUint8[0x07] = data & 0x28;
#else
				pThis->adpcm.DmaReg[0x07] = data & 0x28;
#endif
				break;
			}

		}
		if (data&0x40) {	// CNT == 1 ?
#if X68SOUND_ENABLE_PORTABLE_CODE
			if ( (pThis->adpcm.DmaReg.asUint8[0x00] & 0x48) != 0x08 ) {	// !(BTC==0&&ACT==1) ?
				Adpcm_DmaError(&pThis->adpcm, 0x02);	// 動作タイミングエラー
				pThis->adpcm.DmaReg.asUint8[0x07] = data & 0x28;
#else
			if ( (pThis->adpcm.DmaReg[0x00] & 0x48) != 0x08 ) {	// !(BTC==0&&ACT==1) ?
				Adpcm_DmaError(&pThis->adpcm, 0x02);	// 動作タイミングエラー
				pThis->adpcm.DmaReg[0x07] = data & 0x28;
#endif
				break;
			}

#if X68SOUND_ENABLE_PORTABLE_CODE
			if (pThis->adpcm.DmaReg.asUint8[0x05] & 0x08) {	// CHAIN == 10 or 11 ?
				Adpcm_DmaError(&pThis->adpcm, 0x01);	// コンフィグレーションエラー
				pThis->adpcm.DmaReg.asUint8[0x07] = data & 0x28;
#else
			if (pThis->adpcm.DmaReg[0x05] & 0x08) {	// CHAIN == 10 or 11 ?
				Adpcm_DmaError(&pThis->adpcm, 0x01);	// コンフィグレーションエラー
				pThis->adpcm.DmaReg[0x07] = data & 0x28;
#endif
				break;
			}

		}
		if (data&0x10) {	// SAB == 1 ?
#if X68SOUND_ENABLE_PORTABLE_CODE
			if (pThis->adpcm.DmaReg.asUint8[0x00] & 0x08) {	// ACT == 1 ?
				Adpcm_DmaError(&pThis->adpcm, 0x11);	// ソフトウェア強制停止
				pThis->adpcm.DmaReg.asUint8[0x07] = data & 0x28;
#else
			if (pThis->adpcm.DmaReg[0x00] & 0x08) {	// ACT == 1 ?
				Adpcm_DmaError(&pThis->adpcm, 0x11);	// ソフトウェア強制停止
				pThis->adpcm.DmaReg[0x07] = data & 0x28;
#endif
				break;
			}
		}
		if (data&0x80) {	// STR == 1 ?
			data &= 0x7F;

#if X68SOUND_ENABLE_PORTABLE_CODE
			if (pThis->adpcm.DmaReg.asUint8[0x05] & 0x08) {	// チェイニング動作
				if (!(pThis->adpcm.DmaReg.asUint8[0x05] & 0x04)) {	// アレイチェイン
					if (Adpcm_DmaArrayChainSetNextMtcMar(&pThis->adpcm)) {
						pThis->adpcm.DmaReg.asUint8[0x07] = data & 0x28;
#else
			if (pThis->adpcm.DmaReg[0x05] & 0x08) {	// チェイニング動作
				if (!(pThis->adpcm.DmaReg[0x05] & 0x04)) {	// アレイチェイン
					if (Adpcm_DmaArrayChainSetNextMtcMar(&pThis->adpcm)) {
						pThis->adpcm.DmaReg[0x07] = data & 0x28;
#endif
						break;
					}
				} else {						// リンクアレイチェイン
					if (Adpcm_DmaLinkArrayChainSetNextMtcMar(&pThis->adpcm)) {
#if X68SOUND_ENABLE_PORTABLE_CODE
						pThis->adpcm.DmaReg.asUint8[0x07] = data & 0x28;
#else
						pThis->adpcm.DmaReg[0x07] = data & 0x28;
#endif
						break;
					}
				}
			}

#if X68SOUND_ENABLE_PORTABLE_CODE
			if ( pThis->adpcm.DmaReg.asUint16[0x0A/2] == 0 ) {	// MTC == 0 ?
#else
			if ( (*(unsigned short *)&pThis->adpcm.DmaReg[0x0A]) == 0 ) {	// MTC == 0 ?
#endif
				Adpcm_DmaError(&pThis->adpcm, 0x0D);	// カウントエラー(メモリアドレス/メモリカウンタ)
				data &= 0x28;
				break;
			}

		}
		break;
	}
}
#if X68SOUND_ENABLE_PORTABLE_CODE
void Opm_DmaInt(OPM* pThis, void (*proc)(void *), void *arg) {
	pThis->adpcm.IntProc = proc;
	pThis->adpcm.IntArg = arg;
#else
void Opm_DmaInt(OPM* pThis, void (CALLBACK *proc)()) {
	pThis->adpcm.IntProc = proc;
#endif
}
#if X68SOUND_ENABLE_PORTABLE_CODE
void Opm_DmaErrInt(OPM* pThis, void (*proc)(void *), void *arg) {
	pThis->adpcm.ErrIntProc = proc;
	pThis->adpcm.ErrIntArg = arg;
#else
void Opm_DmaErrInt(OPM* pThis, void (CALLBACK *proc)()) {
	pThis->adpcm.ErrIntProc = proc;
#endif
}


int Opm_Pcm8_Out(OPM* pThis, int ch, void *adrs, int mode, int len) {
	return Pcm8_Out(&pThis->pcm8[ch & (PCM8_NCH-1)], adrs, mode, len);
}
int Opm_Pcm8_Aot(OPM* pThis, int ch, void *tbl, int mode, int cnt) {
	return Pcm8_Aot(&pThis->pcm8[ch & (PCM8_NCH-1)], tbl, mode, cnt);
}
int Opm_Pcm8_Lot(OPM* pThis, int ch, void *tbl, int mode) {
	return Pcm8_Lot(&pThis->pcm8[ch & (PCM8_NCH-1)], tbl, mode);
}
int Opm_Pcm8_SetMode(OPM* pThis, int ch, int mode) {
	return Pcm8_SetMode(&pThis->pcm8[ch & (PCM8_NCH-1)], mode);
}
int Opm_Pcm8_GetRest(OPM* pThis, int ch) {
	return Pcm8_GetRest(&pThis->pcm8[ch & (PCM8_NCH-1)]);
}
int Opm_Pcm8_GetMode(OPM* pThis, int ch) {
	return Pcm8_GetMode(&pThis->pcm8[ch & (PCM8_NCH-1)]);
}
int Opm_Pcm8_Abort(OPM* pThis) {
	int ch;
	for (ch=0; ch<PCM8_NCH; ++ch) {
		Pcm8_Init(&pThis->pcm8[ch]);
	}
	return 0;
}

int Opm_SetTotalVolume(OPM* pThis, int v) {
	if ((unsigned int)v <= 65535) {
		TotalVolume = v;
	}
	return TotalVolume;
}

int Opm_GetTotalVolume(OPM* pThis) {
	return TotalVolume;
}

#if X68SOUND_ENABLE_PORTABLE_CODE
void Opm_BetwInt(OPM* pThis, void (*proc)(void *), void *arg) {
	pThis->BetwIntProc = proc;
	pThis->BetwIntArg = arg;
#else
void Opm_BetwInt(OPM* pThis, void (CALLBACK *proc)()) {
	pThis->BetwIntProc = proc;
#endif
}
void Opm_betwint(OPM* pThis) {
	if (pThis->BetwIntProc != NULL) {
#if X68SOUND_ENABLE_PORTABLE_CODE
		pThis->BetwIntProc(pThis->BetwIntArg);
#endif
	}
}

#if X68SOUND_ENABLE_PORTABLE_CODE
void Opm_SetWaveFunc(OPM* pThis, int (*func)(void *), void *arg) {
	pThis->WaveFunc = func;
	pThis->WaveFuncArg = arg;
#else
void Opm_SetWaveFunc(OPM* pThis, int (CALLBACK *func)()) {
	pThis->WaveFunc = func;
#endif
}


void Opm_PushRegs(OPM* pThis) {
	pThis->OpmRegNo_backup = pThis->OpmRegNo;
}
void Opm_PopRegs(OPM* pThis) {
	pThis->OpmRegNo = pThis->OpmRegNo_backup;
}



#if X68SOUND_ENABLE_PORTABLE_CODE
void Opm_MemReadFunc(OPM* pThis, int (*func)(unsigned char *)) {
#else
void Opm_MemReadFunc(OPM* pThis, int (CALLBACK *func)(unsigned char *)) {
#endif
	if (func == NULL) {
		MemRead = MemReadDefault;
	} else {
		MemRead = func;
	}
}
#if defined(__cplusplus)
}
#endif
