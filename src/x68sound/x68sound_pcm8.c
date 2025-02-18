#include "x68sound_config.h"

#include "x68sound_global.h"
#include "x68sound_pcm8.h"
#include "x68sound_context.internal.h"
#if defined(__cplusplus)
extern "C" {
#endif
#if X68SOUND_ENABLE_PORTABLE_CODE
void Pcm8_ConstructWithX68SoundContextImpl(PCM8* pThis,
	X68SoundContextImpl *contextImpl) {
	pThis->m_contextImpl = contextImpl;
	pThis->Mode = 0x00080403;
	Pcm8_SetMode(pThis, pThis->Mode);
}
#endif
void Pcm8_Construct(PCM8* pThis) {
	pThis->Mode = 0x00080403;
	Pcm8_SetMode(pThis, pThis->Mode);
}



void Pcm8_Init(PCM8* pThis) {
	pThis->AdpcmReg = 0xC7;	// ADPCM動作停止

	pThis->Scale = 0;
	pThis->Pcm = 0;
	pThis->Pcm16Prev = 0;
	pThis->InpPcm = pThis->InpPcm_prev = pThis->OutPcm = 0;
	pThis->OutInpPcm = pThis->OutInpPcm_prev = 0;
	pThis->AdpcmRate = 15625*12;
	pThis->RateCounter = 0;
	pThis->N1Data = 0;
	pThis->N1DataFlag = 0;
	pThis->DmaLastValue = 0;

	pThis->DmaMar = NULL;
	pThis->DmaMtc = 0;
	pThis->DmaBar = NULL;
	pThis->DmaBtc = 0;
	pThis->DmaOcr = 0;

}
void Pcm8_InitSamprate(PCM8* pThis) {
	pThis->RateCounter = 0;
}
void Pcm8_Reset(PCM8* pThis) {		// ADPCM キーオン時の処理
	pThis->Scale = 0;
	pThis->Pcm = 0;
	pThis->Pcm16Prev = 0;
	pThis->InpPcm = pThis->InpPcm_prev = pThis->OutPcm = 0;
	pThis->OutInpPcm = pThis->OutInpPcm_prev = 0;

	pThis->N1Data = 0;
	pThis->N1DataFlag = 0;

}



int Pcm8_DmaArrayChainSetNextMtcMar(PCM8* pThis) {
	int	mem0,mem1,mem2,mem3,mem4,mem5;
	if ( pThis->DmaBtc == 0 ) {
		return 1;
	}
	--pThis->DmaBtc;

	mem0 = MemRead((unsigned char *)pThis->DmaBar++);
	mem1 = MemRead((unsigned char *)pThis->DmaBar++);
	mem2 = MemRead((unsigned char *)pThis->DmaBar++);
	mem3 = MemRead((unsigned char *)pThis->DmaBar++);
	mem4 = MemRead((unsigned char *)pThis->DmaBar++);
	mem5 = MemRead((unsigned char *)pThis->DmaBar++);
	if ((mem0|mem1|mem2|mem3|mem4|mem5) == -1) {
		// バスエラー(ベースアドレス/ベースカウンタ)
		return 1;
	}
#if X68SOUND_ENABLE_PORTABLE_CODE
	pThis->DmaMar = (volatile unsigned char *)TO_PTR((mem0<<24)|(mem1<<16)|(mem2<<8)|(mem3));	// MAR
#else
	pThis->DmaMar = (volatile unsigned char *)((mem0<<24)|(mem1<<16)|(mem2<<8)|(mem3));	// MAR
#endif
	pThis->DmaMtc = (mem4<<8)|(mem5);	// MTC

	if ( pThis->DmaMtc == 0 ) {	// pThis->MTC == 0 ?
		// カウントエラー(メモリアドレス/メモリカウンタ)
		return 1;
	}
	return 0;
}
int Pcm8_DmaLinkArrayChainSetNextMtcMar(PCM8* pThis) {
	int	mem0,mem1,mem2,mem3,mem4,mem5;
	int mem6,mem7,mem8,mem9;
	if (pThis->DmaBar == (unsigned char *)0) {
		return 1;
	}

	mem0 = MemRead((unsigned char *)pThis->DmaBar++);
	mem1 = MemRead((unsigned char *)pThis->DmaBar++);
	mem2 = MemRead((unsigned char *)pThis->DmaBar++);
	mem3 = MemRead((unsigned char *)pThis->DmaBar++);
	mem4 = MemRead((unsigned char *)pThis->DmaBar++);
	mem5 = MemRead((unsigned char *)pThis->DmaBar++);
	mem6 = MemRead((unsigned char *)pThis->DmaBar++);
	mem7 = MemRead((unsigned char *)pThis->DmaBar++);
	mem8 = MemRead((unsigned char *)pThis->DmaBar++);
	mem9 = MemRead((unsigned char *)pThis->DmaBar++);
	if ((mem0|mem1|mem2|mem3|mem4|mem5|mem6|mem7|mem8|mem9) == -1) {
		// バスエラー(ベースアドレス/ベースカウンタ)
		return 1;
	}
#if X68SOUND_ENABLE_PORTABLE_CODE
	pThis->DmaMar = (volatile unsigned char *)TO_PTR((mem0<<24)|(mem1<<16)|(mem2<<8)|(mem3));	// MAR
	pThis->DmaMtc = (mem4<<8)|(mem5);	// MTC
	pThis->DmaBar = (volatile unsigned char *)TO_PTR((mem6<<24)|(mem7<<16)|(mem8<<8)|(mem9));	// BAR
#else
	pThis->DmaMar = (volatile unsigned char *)((mem0<<24)|(mem1<<16)|(mem2<<8)|(mem3));	// MAR
	pThis->DmaMtc = (mem4<<8)|(mem5);	// MTC
	pThis->DmaBar = (volatile unsigned char *)((mem6<<24)|(mem7<<16)|(mem8<<8)|(mem9));	// BAR
#endif

	if ( pThis->DmaMtc == 0 ) {	// pThis->MTC == 0 ?
		// カウントエラー(メモリアドレス/メモリカウンタ)
		return 1;
	}
	return 0;
}

int	Pcm8_DmaGetByte(PCM8* pThis) {
	if (pThis->DmaMtc == 0) {
		return 0x80000000;
	}
	{
		int mem;
		mem = MemRead((unsigned char *)pThis->DmaMar);
		if (mem == -1) {
			// バスエラー(メモリアドレス/メモリカウンタ)
			return 0x80000000;
		}
		pThis->DmaLastValue = mem;
		pThis->DmaMar += 1;
	}

	--pThis->DmaMtc;

#if X68SOUND_ENABLE_PORTABLE_CODE
	do {
		if (pThis->DmaMtc == 0) {
			if (pThis->DmaOcr & 0x08) {	// チェイニング動作
				if (!(pThis->DmaOcr & 0x04)) {	// アレイチェイン
					if (Pcm8_DmaArrayChainSetNextMtcMar(pThis)) {
						break;
					}
				} else {						// リンクアレイチェイン
					if (Pcm8_DmaLinkArrayChainSetNextMtcMar(pThis)) {
						break;
					}
				}
			}
		}
	} while(0);
#else
	try {
	if (pThis->DmaMtc == 0) {
		if (pThis->DmaOcr & 0x08) {	// チェイニング動作
			if (!(pThis->DmaOcr & 0x04)) {	// アレイチェイン
				if (Pcm8_DmaArrayChainSetNextMtcMar(pThis)) {
					throw "";
				}
			} else {						// リンクアレイチェイン
				if (Pcm8_DmaLinkArrayChainSetNextMtcMar(pThis)) {
					throw "";
				}
			}
		}
	}
	} catch (void *) {
	}
#endif

	return pThis->DmaLastValue;
}








#define	MAXPCMVAL	(2047)
#if !X68SOUND_ENABLE_PORTABLE_CODE
static int	HPF_shift_tbl[16+1]={ 0, 0, 1, 1, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 3, 3, 4,};
#endif


// adpcmを入力して InpPcm の値を変化させる
// -2047<<(4+4) <= InpPcm <= +2047<<(4+4)
void	Pcm8_adpcm2pcm(PCM8* pThis, unsigned char adpcm) {

	int sign;
	int	dltL;
	dltL = dltLTBL[pThis->Scale];
	dltL = (dltL&(adpcm&4?-1:0)) + ((dltL>>1)&(adpcm&2?-1:0)) + ((dltL>>2)&(adpcm&1?-1:0)) + (dltL>>3);
	sign = adpcm&8?-1:0;
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

// pcm16を入力して InpPcm の値を変化させる
// -2047<<(4+4) <= InpPcm <= +2047<<(4+4)
void	Pcm8_pcm16_2pcm(PCM8* pThis, int pcm16) {
	pThis->Pcm += pcm16-pThis->Pcm16Prev;
	pThis->Pcm16Prev = pcm16;


	if ((unsigned int)(pThis->Pcm+MAXPCMVAL) > (unsigned int)(MAXPCMVAL*2)) {
		if ((int)(pThis->Pcm+MAXPCMVAL) >= (int)(MAXPCMVAL*2)) {
			pThis->Pcm = MAXPCMVAL;
		} else {
			pThis->Pcm = -MAXPCMVAL;
		}
	}

	pThis->InpPcm = (pThis->Pcm&(int)0xFFFFFFFC)<<(4+4);
}

// -32768<<4 <= retval <= +32768<<4
int Pcm8_GetPcm(PCM8* pThis) {
	if (pThis->AdpcmReg & 0x80) {		// ADPCM 停止中
		return 0x80000000;
	}
	pThis->RateCounter -= pThis->AdpcmRate;
	while (pThis->RateCounter < 0) {
		if (pThis->PcmKind == 5) {	// 16bitPCM
			int dataH,dataL;
			dataH = Pcm8_DmaGetByte(pThis);
#if X68SOUND_ENABLE_PORTABLE_CODE
			if (dataH == (int)0x80000000) {
#else
			if (dataH == 0x80000000) {
#endif
				pThis->RateCounter = 0;
				pThis->AdpcmReg = 0xC7;	// ADPCM 停止
				return 0x80000000;
			}
			dataL = Pcm8_DmaGetByte(pThis);
#if X68SOUND_ENABLE_PORTABLE_CODE
			if (dataL == (int)0x80000000) {
#else
			if (dataL == 0x80000000) {
#endif
				pThis->RateCounter = 0;
				pThis->AdpcmReg = 0xC7;	// ADPCM 停止
				return 0x80000000;
			}
			Pcm8_pcm16_2pcm(pThis, (int)(short)((dataH<<8)|dataL));	// OutPcm に値が入る
		} else if (pThis->PcmKind == 6) {	// 8bitPCM
			int data;
			data = Pcm8_DmaGetByte(pThis);
#if X68SOUND_ENABLE_PORTABLE_CODE
			if (data == (int)0x80000000) {
#else
			if (data == 0x80000000) {
#endif
				pThis->RateCounter = 0;
				pThis->AdpcmReg = 0xC7;	// ADPCM 停止
				return 0x80000000;
			}
			Pcm8_pcm16_2pcm(pThis, (int)(char)data);	// InpPcm に値が入る
		} else {
			int	N10Data;	// (N1Data << 4) | N0Data
			if (pThis->N1DataFlag == 0) {		// 次のADPCMデータが内部にない場合
				N10Data = Pcm8_DmaGetByte(pThis);	// DMA転送(1バイト)
#if X68SOUND_ENABLE_PORTABLE_CODE
				if (N10Data == (int)0x80000000) {
#else
				if (N10Data == 0x80000000) {
#endif
					pThis->RateCounter = 0;
					pThis->AdpcmReg = 0xC7;	// ADPCM 停止
					return 0x80000000;
				}
				Pcm8_adpcm2pcm(pThis, N10Data & 0x0F);	// InpPcm に値が入る
				pThis->N1Data = (N10Data >> 4) & 0x0F;
				pThis->N1DataFlag = 1;
			} else {
				Pcm8_adpcm2pcm(pThis, pThis->N1Data);			// InpPcm に値が入る
				pThis->N1DataFlag = 0;
			}
		}
		pThis->RateCounter += 15625*12;
	}
	pThis->OutPcm = ((pThis->InpPcm<<9) - (pThis->InpPcm_prev<<9)
		+ 459*pThis->OutPcm) >> 9;
	pThis->InpPcm_prev = pThis->InpPcm;

	return (((pThis->OutPcm*pThis->Volume)>>4)*TotalVolume)>>8;
}

// -32768<<4 <= retval <= +32768<<4
int Pcm8_GetPcm62(PCM8* pThis) {
	if (pThis->AdpcmReg & 0x80) {		// ADPCM 停止中
		return 0x80000000;
	}
	pThis->RateCounter -= pThis->AdpcmRate;
	while (pThis->RateCounter < 0) {
		if (pThis->PcmKind == 5) {	// 16bitPCM
			int dataH,dataL;
			dataH = Pcm8_DmaGetByte(pThis);
#if X68SOUND_ENABLE_PORTABLE_CODE
			if (dataH == (int)0x80000000) {
#else
			if (dataH == 0x80000000) {
#endif
				pThis->RateCounter = 0;
				pThis->AdpcmReg = 0xC7;	// ADPCM 停止
				return 0x80000000;
			}
			dataL = Pcm8_DmaGetByte(pThis);
#if X68SOUND_ENABLE_PORTABLE_CODE
			if (dataL == (int)0x80000000) {
#else
			if (dataL == 0x80000000) {
#endif
				pThis->RateCounter = 0;
				pThis->AdpcmReg = 0xC7;	// ADPCM 停止
				return 0x80000000;
			}
			Pcm8_pcm16_2pcm(pThis, (int)(short)((dataH<<8)|dataL));	// OutPcm に値が入る
		} else if (pThis->PcmKind == 6) {	// 8bitPCM
			int data;
			data = Pcm8_DmaGetByte(pThis);
#if X68SOUND_ENABLE_PORTABLE_CODE
			if (data == (int)0x80000000) {
#else
			if (data == 0x80000000) {
#endif
				pThis->RateCounter = 0;
				pThis->AdpcmReg = 0xC7;	// ADPCM 停止
				return 0x80000000;
			}
			Pcm8_pcm16_2pcm(pThis, (int)(char)data);	// InpPcm に値が入る
		} else {
			int	N10Data;	// (N1Data << 4) | N0Data
			if (pThis->N1DataFlag == 0) {		// 次のADPCMデータが内部にない場合
				N10Data = Pcm8_DmaGetByte(pThis);	// DMA転送(1バイト)
#if X68SOUND_ENABLE_PORTABLE_CODE
				if (N10Data == (int)0x80000000) {
#else
				if (N10Data == 0x80000000) {
#endif
					pThis->RateCounter = 0;
					pThis->AdpcmReg = 0xC7;	// ADPCM 停止
					return 0x80000000;
				}
				Pcm8_adpcm2pcm(pThis, N10Data & 0x0F);	// InpPcm に値が入る
				pThis->N1Data = (N10Data >> 4) & 0x0F;
				pThis->N1DataFlag = 1;
			} else {
				Pcm8_adpcm2pcm(pThis, pThis->N1Data);			// InpPcm に値が入る
				pThis->N1DataFlag = 0;
			}
		}
		pThis->RateCounter += 15625*12*4;
	}
	pThis->OutInpPcm = (pThis->InpPcm<<9) - (pThis->InpPcm_prev<<9)
		+  pThis->OutInpPcm-(pThis->OutInpPcm>>5)-(pThis->OutInpPcm>>10);
	pThis->InpPcm_prev = pThis->InpPcm;
	pThis->OutPcm = pThis->OutInpPcm - pThis->OutInpPcm_prev
		+ pThis->OutPcm-(pThis->OutPcm>>8)-(pThis->OutPcm>>9)-(pThis->OutPcm>>12);
	pThis->OutInpPcm_prev = pThis->OutInpPcm;
	return ((pThis->OutPcm>>9)*pThis->Volume)>>4;
}







int	Pcm8_Out(PCM8* pThis, void *adrs, int mode, int len) {
	if (len <= 0) {
		if (len < 0) {
			return Pcm8_GetRest(pThis);
		} else {
			pThis->DmaMtc = 0;
			return 0;
		}
	}
	pThis->AdpcmReg = 0xC7;	// ADPCM 停止
	pThis->DmaMtc = 0;
	pThis->DmaMar = (unsigned char *)adrs;
	Pcm8_SetMode(pThis, mode);
	if ((mode&3) != 0) {
		pThis->DmaMtc = len;
		Pcm8_Reset(pThis);
		pThis->AdpcmReg = 0x47;	// ADPCM 動作開始
	}
	return 0;
}
int	Pcm8_Aot(PCM8* pThis, void *tbl, int mode, int cnt) {
	if (cnt <= 0) {
		if (cnt < 0) {
			return Pcm8_GetRest(pThis);
		} else {
			pThis->DmaMtc = 0;
			return 0;
		}
	}
	pThis->AdpcmReg = 0xC7;	// ADPCM 停止
	pThis->DmaMtc = 0;
	pThis->DmaBar = (unsigned char *)tbl;
	pThis->DmaBtc = cnt;
	Pcm8_SetMode(pThis, mode);
	if ((mode&3) != 0) {
		Pcm8_DmaArrayChainSetNextMtcMar(pThis);
		Pcm8_Reset(pThis);
		pThis->AdpcmReg = 0x47;	// ADPCM 動作開始
	}
	return 0;
}
int	Pcm8_Lot(PCM8* pThis, void *tbl, int mode) {
	pThis->AdpcmReg = 0xC7;	// ADPCM 停止
	pThis->DmaMtc = 0;
	pThis->DmaBar = (unsigned char *)tbl;
	Pcm8_SetMode(pThis, mode);
	if ((mode&3) != 0) {
		Pcm8_DmaLinkArrayChainSetNextMtcMar(pThis);
		Pcm8_Reset(pThis);
		pThis->AdpcmReg = 0x47;	// ADPCM 動作開始
	}
	return 0;
}

int	Pcm8_SetMode(PCM8* pThis, int mode) {
	int	m;
	m = (mode>>16) & 0xFF;
	if (m != 0xFF) {
		m &= 15;
		pThis->Volume = PCM8VOLTBL[m];
		pThis->Mode = (pThis->Mode&0xFF00FFFF)|(m<<16);
	}
	m = (mode>>8) & 0xFF;
	if (m != 0xFF) {
		m &= 7;
		pThis->AdpcmRate = ADPCMRATEADDTBL[m];
		pThis->PcmKind = m;
		pThis->Mode = (pThis->Mode&0xFFFF00FF)|(m<<8);
	}
	m = (mode) & 0xFF;
	if (m != 0xFF) {
		m &= 3;
		if (m == 0) {
			pThis->AdpcmReg = 0xC7;	// ADPCM 停止
			pThis->DmaMtc = 0;
		} else {
			pThis->Mode = (pThis->Mode&0xFFFFFF00)|(m);
		}
	}
	return 0;
}

int	Pcm8_GetRest(PCM8* pThis) {
	if (pThis->DmaMtc == 0) {
		return 0;
	}
	if (pThis->DmaOcr & 0x08) {	// チェイニング動作
		if (!(pThis->DmaOcr & 0x04)) {	// アレイチェイン
			return -1;
		} else {						// リンクアレイチェイン
			return -2;
		}
	}
	return pThis->DmaMtc;
}
int	Pcm8_GetMode(PCM8* pThis) {
	return pThis->Mode;
}
#if defined(__cplusplus)
}
#endif
