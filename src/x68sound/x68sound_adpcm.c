#include "x68sound_config.h"

#include "x68sound_global.h"
#include "x68sound_adpcm.h"
#include "x68sound_context.internal.h"

#if defined(__cplusplus)
extern "C" {
#endif

#if X68SOUND_ENABLE_PORTABLE_CODE
void Adpcm_ConstructWithX68SoundContextImpl(ADPCM* pThis,
	struct tagX68SoundContextImpl *contextImpl) {
	pThis->m_contextImpl = contextImpl;
}
#else
void Adpcm_Construct(ADPCM* pThis) {
}
#endif

void Adpcm_SetAdpcmRate(ADPCM* pThis, int rate) {
	pThis->AdpcmRate = ADPCMRATEADDTBL[rate & 7];
}

#if X68SOUND_ENABLE_PORTABLE_CODE
static const unsigned char DmaRegInit[0x40] = {
#else
unsigned char DmaRegInit[0x40] = {
#endif
/*+00*/	0x00,0x00,	// CSR/CER
/*+02*/	0xFF,0xFF,
/*+04*/	0x80,0x32,	// DCR/OCR
/*+06*/	0x04,0x08,	// SCR/CCR
/*+08*/	0xFF,0xFF,
/*+0A*/	0x00,0x00,	// MTC
/*+0C*/	0x00,0x00,	// MAR
/*+0E*/	0x00,0x00,	// MAR
/*+10*/	0xFF,0xFF,
/*+12*/	0xFF,0xFF,
/*+14*/	0x00,0xE9,	// DAR
/*+16*/	0x20,0x03,	// DAR
/*+18*/	0xFF,0xFF,
/*+1A*/	0x00,0x00,	// BTC
/*+1C*/	0x00,0x00,	// BAR
/*+1E*/	0x00,0x00,	// BAR
/*+20*/	0xFF,0xFF,
/*+22*/	0xFF,0xFF,
/*+24*/	0xFF,0x6A,	// NIV
/*+26*/	0xFF,0x6B,	// EIV
/*+28*/	0xFF,0x05,	// MFC
/*+2A*/	0xFF,0xFF,
/*+2C*/	0xFF,0x01,	// CPR
/*+2E*/	0xFF,0xFF,
/*+30*/	0xFF,0x05,	// DFC
/*+32*/	0xFF,0xFF,
/*+34*/	0xFF,0xFF,
/*+36*/	0xFF,0xFF,
/*+38*/	0xFF,0x05,	// BFC
/*+3A*/	0xFF,0xFF,
/*+3C*/	0xFF,0xFF,
/*+3E*/	0xFF,0x00,	// GCR
};

void Adpcm_Init(ADPCM* pThis) {
	pThis->Scale = 0;
	pThis->Pcm = 0;
	pThis->InpPcm = pThis->InpPcm_prev = pThis->OutPcm = 0;
	pThis->OutInpPcm = pThis->OutInpPcm_prev = 0;
	pThis->AdpcmRate = 15625*12;
	pThis->RateCounter = 0;
	pThis->N1Data = 0;
	pThis->N1DataFlag = 0;
	pThis->IntProc = NULL;
#if X68SOUND_ENABLE_PORTABLE_CODE
	pThis->IntArg = NULL;
#endif
	pThis->ErrIntProc = NULL;
#if X68SOUND_ENABLE_PORTABLE_CODE
	pThis->ErrIntArg = NULL;
#endif
	pThis->DmaLastValue = 0;
	pThis->AdpcmReg = 0xC7;
	{
		int i;
		for (i=0; i<0x40; ++i) {
#if X68SOUND_ENABLE_PORTABLE_CODE
			pThis->DmaReg.asUint8[i] = DmaRegInit[i];
#else
			pThis->DmaReg[i] = DmaRegInit[i];
#endif
		}
	}
	pThis->FinishCounter = 3;
}
void Adpcm_InitSamprate(ADPCM* pThis) {
	pThis->RateCounter = 0;
}
void Adpcm_Reset(ADPCM* pThis) {	// ADPCM キーオン時の処理

	pThis->Scale = 0;

	pThis->Pcm = 0;
	pThis->InpPcm = pThis->InpPcm_prev = pThis->OutPcm = 0;
	pThis->OutInpPcm = pThis->OutInpPcm_prev = 0;

	pThis->N1Data = 0;
	pThis->N1DataFlag = 0;
}


void Adpcm_DmaError(ADPCM* pThis, unsigned char errcode) {
#if X68SOUND_ENABLE_PORTABLE_CODE
	pThis->DmaReg.asUint8[0x00] &= 0xF7;		// ACT=0
	pThis->DmaReg.asUint8[0x00] |= 0x90;		// COC=ERR=1
	pThis->DmaReg.asUint8[0x01] = errcode;		// CER=errorcode
	if (pThis->DmaReg.asUint8[0x07] & 0x08) {	// INT==1?
		if (pThis->ErrIntProc != NULL) {
			pThis->ErrIntProc(pThis->ErrIntArg);
#else
	pThis->DmaReg[0x00] &= 0xF7;		// ACT=0
	pThis->DmaReg[0x00] |= 0x90;		// COC=ERR=1
	pThis->DmaReg[0x01] = errcode;		// CER=errorcode
	if (pThis->DmaReg[0x07] & 0x08) {	// INT==1?
		if (pThis->ErrIntProc != NULL) {
			pThis->ErrIntProc();
#endif
		}
	}
}
void Adpcm_DmaFinish(ADPCM* pThis) {
#if X68SOUND_ENABLE_PORTABLE_CODE
	pThis->DmaReg.asUint8[0x00] &= 0xF7;		// ACT=0
	pThis->DmaReg.asUint8[0x00] |= 0x80;		// COC=1
	if (pThis->DmaReg.asUint8[0x07] & 0x08) {	// INT==1?
		if (pThis->IntProc != NULL) {
			pThis->IntProc(pThis->IntArg);
#else
	pThis->DmaReg[0x00] &= 0xF7;		// ACT=0
	pThis->DmaReg[0x00] |= 0x80;		// COC=1
	if (pThis->DmaReg[0x07] & 0x08) {	// INT==1?
		if (pThis->IntProc != NULL) {
			pThis->IntProc();
#endif
		}
	}
}

int Adpcm_DmaContinueSetNextMtcMar(ADPCM* pThis) {
#if X68SOUND_ENABLE_PORTABLE_CODE
	pThis->DmaReg.asUint8[0x07] &= (0xFF-0x40);	// CNT=0
	pThis->DmaReg.asUint16[0x0A/2] = pThis->DmaReg.asUint16[0x1A/2];	// BTC -> MTC
	pThis->DmaReg.asUint32[0x0C/4] = pThis->DmaReg.asUint32[0x1C/4];	// BAR -> MAR
	pThis->DmaReg.asUint8[0x29] = pThis->DmaReg.asUint8[0x39];	// BFC -> MFC
	if ( (pThis->DmaReg.asUint16[0x0A/2]) == 0 ) {	// MTC == 0 ?
		Adpcm_DmaError(pThis, 0x0D);	// カウントエラー(メモリアドレス/メモリカウンタ)
		return 1;
	}
	pThis->DmaReg.asUint8[0x00] |= 0x40;		// BTC=1
	if (pThis->DmaReg.asUint8[0x07] & 0x08) {	// INT==1?
		if (pThis->IntProc != NULL) {
			pThis->IntProc(pThis->IntArg);
		}
	}
#else
	pThis->DmaReg[0x07] &= (0xFF-0x40);	// CNT=0
	*(unsigned short *)&pThis->DmaReg[0x0A] =
		*(unsigned short *)&pThis->DmaReg[0x1A];	// BTC -> MTC
	*(unsigned int *)&pThis->DmaReg[0x0C] =
		*(unsigned int *)&pThis->DmaReg[0x1C];	// BAR -> MAR
	pThis->DmaReg[0x29] = pThis->DmaReg[0x39];	// BFC -> MFC
	if ( (*(unsigned short *)&pThis->DmaReg[0x0A]) == 0 ) {	// MTC == 0 ?
		Adpcm_DmaError(pThis, 0x0D);	// カウントエラー(メモリアドレス/メモリカウンタ)
		return 1;
	}
	pThis->DmaReg[0x00] |= 0x40;		// BTC=1
	if (pThis->DmaReg[0x07] & 0x08) {	// INT==1?
		if (pThis->IntProc != NULL) {
			pThis->IntProc();
		}
	}
#endif
	return 0;
}
int Adpcm_DmaArrayChainSetNextMtcMar(ADPCM* pThis) {
	unsigned char *Bar;
	int	mem0,mem1,mem2,mem3,mem4,mem5;
	unsigned short	Btc;
#if X68SOUND_ENABLE_PORTABLE_CODE
	Btc = bswapw(pThis->DmaReg.asUint16[0x1A/2]);
#else
	Btc = bswapw(*(unsigned short *)&pThis->DmaReg[0x1A]);
#endif
	if ( Btc == 0 ) {
		Adpcm_DmaFinish(pThis);
		pThis->FinishCounter = 0;
		return 1;
	}
	--Btc;
#if X68SOUND_ENABLE_PORTABLE_CODE
	pThis->DmaReg.asUint16[0x1A/2] = bswapw(Btc);
#else
	*(unsigned short *)&pThis->DmaReg[0x1A] = bswapw(Btc);
#endif


#if X68SOUND_ENABLE_PORTABLE_CODE
	Bar = TO_PTR(bswapl(pThis->DmaReg.asUint32[0x1C/4]));
#else
	Bar = bswapl(*(unsigned char **)&pThis->DmaReg[0x1C]);
#endif
	mem0 = MemRead(Bar++);
	mem1 = MemRead(Bar++);
	mem2 = MemRead(Bar++);
	mem3 = MemRead(Bar++);
	mem4 = MemRead(Bar++);
	mem5 = MemRead(Bar++);
	if ((mem0|mem1|mem2|mem3|mem4|mem5) == -1) {
		Adpcm_DmaError(pThis, 0x0B);		// バスエラー(ベースアドレス/ベースカウンタ)
		return 1;
	}
#if X68SOUND_ENABLE_PORTABLE_CODE
	pThis->DmaReg.asUint32[0x1C/4] = bswapl(TO_OFS(Bar));
	pThis->DmaReg.asUint8[0x0C] = (uint8_t)mem0;	// MAR
	pThis->DmaReg.asUint8[0x0D] = (uint8_t)mem1;
	pThis->DmaReg.asUint8[0x0E] = (uint8_t)mem2;
	pThis->DmaReg.asUint8[0x0F] = (uint8_t)mem3;
	pThis->DmaReg.asUint8[0x0A] = (uint8_t)mem4;	// MTC
	pThis->DmaReg.asUint8[0x0B] = (uint8_t)mem5;
#else
	*(unsigned char **)&pThis->DmaReg[0x1C] = bswapl(Bar);
	pThis->DmaReg[0x0C] = mem0;	// MAR
	pThis->DmaReg[0x0D] = mem1;
	pThis->DmaReg[0x0E] = mem2;
	pThis->DmaReg[0x0F] = mem3;
	pThis->DmaReg[0x0A] = mem4;	// MTC
	pThis->DmaReg[0x0B] = mem5;
#endif

#if X68SOUND_ENABLE_PORTABLE_CODE
	if ( (pThis->DmaReg.asUint16[0x0A/2]) == 0 ) {	// MTC == 0 ?
#else
	if ( (*(unsigned short *)&pThis->DmaReg[0x0A]) == 0 ) {	// MTC == 0 ?
#endif
		Adpcm_DmaError(pThis, 0x0D); // カウントエラー(メモリアドレス/メモリカウンタ)
		return 1;
	}
	return 0;
}
int Adpcm_DmaLinkArrayChainSetNextMtcMar(ADPCM* pThis) {
	int	mem0,mem1,mem2,mem3,mem4,mem5;
	int mem6,mem7,mem8,mem9;
	unsigned char *Bar;
#if X68SOUND_ENABLE_PORTABLE_CODE
	Bar = TO_PTR(bswapl(pThis->DmaReg.asUint32[0x1C/4]));
#else
	Bar = bswapl(*(unsigned char **)&pThis->DmaReg[0x1C]);
#endif
	if (Bar == (unsigned char *)0) {
		Adpcm_DmaFinish(pThis);
		pThis->FinishCounter = 0;
		return 1;
	}

	mem0 = MemRead(Bar++);
	mem1 = MemRead(Bar++);
	mem2 = MemRead(Bar++);
	mem3 = MemRead(Bar++);
	mem4 = MemRead(Bar++);
	mem5 = MemRead(Bar++);
	mem6 = MemRead(Bar++);
	mem7 = MemRead(Bar++);
	mem8 = MemRead(Bar++);
	mem9 = MemRead(Bar++);
	if ((mem0|mem1|mem2|mem3|mem4|mem5|mem6|mem7|mem8|mem9) == -1) {
		Adpcm_DmaError(pThis, 0x0B);		// バスエラー(ベースアドレス/ベースカウンタ)
		return 1;
	}
#if X68SOUND_ENABLE_PORTABLE_CODE
	pThis->DmaReg.asUint32[0x1C/4] = bswapl(TO_OFS(Bar));
	pThis->DmaReg.asUint8[0x0C] = (uint8_t)mem0;	// MAR
	pThis->DmaReg.asUint8[0x0D] = (uint8_t)mem1;
	pThis->DmaReg.asUint8[0x0E] = (uint8_t)mem2;
	pThis->DmaReg.asUint8[0x0F] = (uint8_t)mem3;
	pThis->DmaReg.asUint8[0x0A] = (uint8_t)mem4;	// MTC
	pThis->DmaReg.asUint8[0x0B] = (uint8_t)mem5;
	pThis->DmaReg.asUint8[0x1C] = (uint8_t)mem6;	// BAR
	pThis->DmaReg.asUint8[0x1D] = (uint8_t)mem7;
	pThis->DmaReg.asUint8[0x1E] = (uint8_t)mem8;
	pThis->DmaReg.asUint8[0x1F] = (uint8_t)mem9;
#else
	*(unsigned char **)&pThis->DmaReg[0x1C] = bswapl(Bar);
	pThis->DmaReg[0x0C] = mem0;	// MAR
	pThis->DmaReg[0x0D] = mem1;
	pThis->DmaReg[0x0E] = mem2;
	pThis->DmaReg[0x0F] = mem3;
	pThis->DmaReg[0x0A] = mem4;	// MTC
	pThis->DmaReg[0x0B] = mem5;
	pThis->DmaReg[0x1C] = mem6;	// BAR
	pThis->DmaReg[0x1D] = mem7;
	pThis->DmaReg[0x1E] = mem8;
	pThis->DmaReg[0x1F] = mem9;
#endif

#if X68SOUND_ENABLE_PORTABLE_CODE
	if ( pThis->DmaReg.asUint16[0x0A/2] == 0 ) {	// MTC == 0 ?
#else
	if ( (*(unsigned short *)&pThis->DmaReg[0x0A]) == 0 ) {	// MTC == 0 ?
#endif
		Adpcm_DmaError(pThis, 0x0D);		// カウントエラー(メモリアドレス/メモリカウンタ)
		return 1;
	}
	return 0;
}

#if X68SOUND_ENABLE_PORTABLE_CODE
static const int MACTBL[4] = {0,1,-1,1};
#else
int	MACTBL[4] = {0,1,-1,1};
#endif

int	Adpcm_DmaGetByte(ADPCM* pThis) {
	unsigned short	Mtc;
#if X68SOUND_ENABLE_PORTABLE_CODE
	if ((!(pThis->DmaReg.asUint8[0x00]&0x08))
			|| (pThis->DmaReg.asUint8[0x07]&0x20)) {	// ACT==0 || HLT==1 ?
#else
	if ((!(pThis->DmaReg[0x00]&0x08))
			|| (pThis->DmaReg[0x07]&0x20)) {	// ACT==0 || HLT==1 ?
#endif
		return 0x80000000;
	}

#if X68SOUND_ENABLE_PORTABLE_CODE
	Mtc = bswapw(pThis->DmaReg.asUint16[0x0A/2]);
#else
	Mtc = bswapw(*(unsigned short *)&pThis->DmaReg[0x0A]);
#endif
	if (Mtc == 0) {
//		if (pThis->DmaReg[0x07] & 0x40) {	// Continue動作
//			if (DmaContinueSetNextMtcMar()) {
//				return 0x80000000;
//			}
//			Mtc = bswapw(*(unsigned short *)&pThis->DmaReg[0x0A]);
//		} else {
			return 0x80000000;
//		}
	}


	{
		int mem;
		unsigned char *Mar;
#if X68SOUND_ENABLE_PORTABLE_CODE
		Mar = TO_PTR(bswapl(pThis->DmaReg.asUint32[0x0C/4]));
#else
		Mar = bswapl(*(unsigned char **)&pThis->DmaReg[0x0C]);
#endif
		mem = MemRead(Mar);
		if (mem == -1) {
			Adpcm_DmaError(pThis, 0x09);	// バスエラー(メモリアドレス/メモリカウンタ)
			return 0x80000000;
		}
		pThis->DmaLastValue = mem;
#if X68SOUND_ENABLE_PORTABLE_CODE
		Mar += MACTBL[(pThis->DmaReg.asUint8[0x06]>>2)&3];
		pThis->DmaReg.asUint32[0x0C/4] = bswapl(TO_OFS(Mar));
#else
		Mar += MACTBL[(pThis->DmaReg[0x06]>>2)&3];
		*(unsigned char **)&pThis->DmaReg[0x0C] = bswapl(Mar);
#endif
	}

	--Mtc;
#if X68SOUND_ENABLE_PORTABLE_CODE
	pThis->DmaReg.asUint16[0x0A/2] = bswapw(Mtc);
#else
	*(unsigned short *)&pThis->DmaReg[0x0A] = bswapw(Mtc);
#endif

#if X68SOUND_ENABLE_PORTABLE_CODE
	do {
		if (Mtc == 0) {
			if (pThis->DmaReg.asUint8[0x07] & 0x40) {	// Continue動作
				if (Adpcm_DmaContinueSetNextMtcMar(pThis)) {
					break;
				}
			} else if (pThis->DmaReg.asUint8[0x05] & 0x08) {	// チェイニング動作
				if (!(pThis->DmaReg.asUint8[0x05] & 0x04)) {	// アレイチェイン
					if (Adpcm_DmaArrayChainSetNextMtcMar(pThis)) {
						break;
					}
				} else {						// リンクアレイチェイン
					if (Adpcm_DmaLinkArrayChainSetNextMtcMar(pThis)) {
						break;
					}
				}
			} else {	// ノーマル転送終了
//				if (!(DmaReg[0x00] & 0x40)) {		// BTC=1 ?
//					if (DmaContinueSetNextMtcMar()) {
//						break;
//					}
//				} else {
					Adpcm_DmaFinish(pThis);
					pThis->FinishCounter = 0;
//				}
			}
		}
	} while(0);
#else
#if defined(__cplusplus)
	try {
#endif
	if (Mtc == 0) {
		if (pThis->DmaReg[0x07] & 0x40) {	// Continue動作
			if (DmaContinueSetNextMtcMar()) {
				throw "";
			}
		} else if (pThis->DmaReg[0x05] & 0x08) {	// チェイニング動作
			if (!(pThis->DmaReg[0x05] & 0x04)) {	// アレイチェイン
				if (DmaArrayChainSetNextMtcMar()) {
					throw "";
				}
			} else {						// リンクアレイチェイン
				if (DmaLinkArrayChainSetNextMtcMar()) {
					throw "";
				}
			}
		} else {	// ノーマル転送終了
//			if (!(DmaReg[0x00] & 0x40)) {		// BTC=1 ?
//				if (DmaContinueSetNextMtcMar()) {
//					throw "";
//				}
//			} else {
				Adpcm_DmaFinish(pThis);
				pThis->FinishCounter = 0;
//			}
		}
	}
#if defined(__cplusplus)
	} catch (void *) {
	}
#endif /* __cplusplus */
#endif

	return pThis->DmaLastValue;
}








#define	MAXPCMVAL	(2047)

// adpcmを入力して InpPcm の値を変化させる
// -2047<<(4+4) <= InpPcm <= +2047<<(4+4)
void	Adpcm_adpcm2pcm(ADPCM* pThis, unsigned char adpcm) {


	int	dltL;
	dltL = dltLTBL[pThis->Scale];
	dltL = (dltL&(adpcm&4?-1:0)) + ((dltL>>1)&(adpcm&2?-1:0))
		+ ((dltL>>2)&(adpcm&1?-1:0)) + (dltL>>3);
	int sign = adpcm&8?-1:0;
	dltL = (dltL^sign)+(sign&1);
	pThis->Pcm += dltL;


	if ((unsigned int)(pThis->Pcm+MAXPCMVAL) > (unsigned int)(MAXPCMVAL*2)) {
		if ((int)(pThis->Pcm+MAXPCMVAL) >= (int)(MAXPCMVAL*2)) {
			pThis->Pcm = MAXPCMVAL;
		} else {
			pThis->Pcm = -MAXPCMVAL;
		}
	}

	pThis->InpPcm = (pThis->Pcm&(int)0xFFFFFFFC)<<(4+4);

	pThis->Scale += DCT[adpcm];
	if ((unsigned int)pThis->Scale > (unsigned int)48) {
		if ((int)pThis->Scale >= (int)48) {
			pThis->Scale = 48;
		} else {
			pThis->Scale = 0;
		}
	}
}

// -32768<<4 <= retval <= +32768<<4
int Adpcm_GetPcm(ADPCM* pThis) {
	if (pThis->AdpcmReg & 0x80) {		// ADPCM 停止中
		return 0x80000000;
	}
	pThis->RateCounter -= pThis->AdpcmRate;
	while (pThis->RateCounter < 0) {
		int	N10Data;	// (N1Data << 4) | N0Data
		if (pThis->N1DataFlag == 0) {		// 次のADPCMデータが内部にない場合
			N10Data = Adpcm_DmaGetByte(pThis);	// DMA転送(1バイト)
#if X68SOUND_ENABLE_PORTABLE_CODE
			if (N10Data == (int)0x80000000) {
#else
			if (N10Data == 0x80000000) {
#endif
				pThis->RateCounter = 0;
				return 0x80000000;
			}
			Adpcm_adpcm2pcm(pThis, N10Data & 0x0F);	// InpPcm に値が入る
			pThis->N1Data = (N10Data >> 4) & 0x0F;
			pThis->N1DataFlag = 1;
		} else {
			Adpcm_adpcm2pcm(pThis, pThis->N1Data);			// InpPcm に値が入る
			pThis->N1DataFlag = 0;
		}
		pThis->RateCounter += 15625*12;
	}
	pThis->OutPcm = ((pThis->InpPcm<<9) - (pThis->InpPcm_prev<<9)
		+ 459*pThis->OutPcm) >> 9;
	pThis->InpPcm_prev = pThis->InpPcm;

	return (pThis->OutPcm*TotalVolume)>>8;
}

// -32768<<4 <= retval <= +32768<<4
int Adpcm_GetPcm62(ADPCM* pThis) {
	if (pThis->AdpcmReg & 0x80) {		// ADPCM 停止中
		return 0x80000000;
	}
	pThis->RateCounter -= pThis->AdpcmRate;
	while (pThis->RateCounter < 0) {
		int	N10Data;	// (N1Data << 4) | N0Data
		if (pThis->N1DataFlag == 0) {		// 次のADPCMデータが内部にない場合
			N10Data = Adpcm_DmaGetByte(pThis);	// DMA転送(1バイト)
#if X68SOUND_ENABLE_PORTABLE_CODE
			if (N10Data == (int)0x80000000) {
#else
			if (N10Data == 0x80000000) {
#endif
				pThis->RateCounter = 0;
				return 0x80000000;
			}
			Adpcm_adpcm2pcm(pThis, N10Data & 0x0F);	// InpPcm に値が入る
			pThis->N1Data = (N10Data >> 4) & 0x0F;
			pThis->N1DataFlag = 1;
		} else {
			Adpcm_adpcm2pcm(pThis, pThis->N1Data);			// InpPcm に値が入る
			pThis->N1DataFlag = 0;
		}
		pThis->RateCounter += 15625*12*4;

	}
	pThis->OutInpPcm = (pThis->InpPcm<<9) - (pThis->InpPcm_prev<<9)
		+ pThis->OutInpPcm-(pThis->OutInpPcm>>5)-(pThis->OutInpPcm>>10);
	pThis->InpPcm_prev = pThis->InpPcm;
	pThis->OutPcm = pThis->OutInpPcm - pThis->OutInpPcm_prev
		+ pThis->OutPcm-(pThis->OutPcm>>8)-(pThis->OutPcm>>9)-(pThis->OutPcm>>12);
	pThis->OutInpPcm_prev = pThis->OutInpPcm;
	return (pThis->OutPcm>>9);
}
#if defined(__cplusplus)
}
#endif
