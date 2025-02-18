#include "x68sound_config.h"

#include "x68sound_global.h"
#include "x68sound_lfo.h"
#include "x68sound_context.internal.h"
#if defined(__cplusplus)
extern "C" {
#endif
#if X68SOUND_ENABLE_PORTABLE_CODE
const int	PMSMUL[8]={ 0,1,2,4,8,16,32,32 };
const int	PMSSHL[8]={ 0,0,0,0,0, 0, 1, 2 };
#else
int	PMSMUL[8]={ 0,1,2,4,8,16,32,32 };
int	PMSSHL[8]={ 0,0,0,0,0, 0, 1, 2 };
#endif
#if X68SOUND_ENABLE_PORTABLE_CODE
void Lfo_ConstructWithX68SoundContextImpl(LFO* pThis,
	X68SoundContextImpl *contextImpl) {
	pThis->m_contextImpl = contextImpl;
#else
void Lfo_Construct(LFO* pThis) {
#endif
	int i;

	for (i=0; i<N_CH; ++i) {
		pThis->Pmsmul[i] = 0;
		pThis->Pmsshl[i] = 0;
		pThis->Ams[i] = 31;
		pThis->PmdPmsmul[i] = 0;

		pThis->PmValue[i] = 0;
		pThis->AmValue[i] = 0;
	}
	pThis->Pmd = 0;
	pThis->Amd = 0;

	pThis->LfoStartingFlag = 0;
	pThis->LfoOverFlow = 0;
	pThis->LfoTime = 0;
	pThis->LfoTimeAdd = 0;
	pThis->LfoIdx = 0;
	pThis->LfoSmallCounter = 0;
	pThis->LfoSmallCounterStep = 0;
	pThis->Lfrq = 0;
	pThis->LfoWaveForm = 0;

	pThis->PmTblValue = 0;
	pThis->AmTblValue = 255;

	// PM Wave Form 0,3
	for (i=0; i<=127; ++i) {
		pThis->PmTbl0[i] = i;
		pThis->PmTbl0[i+128] = i-127;
		pThis->PmTbl0[i+256] = i;
		pThis->PmTbl0[i+384] = i-127;
	}
	// AM Wave Form 0,3
	for (i=0; i<=255; ++i) {
		pThis->AmTbl0[i] = 255-i;
		pThis->AmTbl0[i+256] = 255-i;
	}

	// PM Wave Form 2
	for (i=0; i<=127; ++i) {
		pThis->PmTbl2[i] = i;
		pThis->PmTbl2[i+128] = 127-i;
		pThis->PmTbl2[i+256] = -i;
		pThis->PmTbl2[i+384] = i-127;
	}
	// AM Wave Form 2
	for (i=0; i<=255; ++i) {
		pThis->AmTbl2[i] = 255-i;
		pThis->AmTbl2[i+256] = i;
	}

};

void Lfo_Init(LFO* pThis) {
	pThis->LfoTimeAdd = LFOPRECISION*62500/Samprate;

	pThis->LfoSmallCounter = 0;

	Lfo_SetLFRQ(pThis, 0);
	Lfo_SetPMDAMD(pThis, 0);
	Lfo_SetPMDAMD(pThis, 128+0);
	Lfo_SetWaveForm(pThis, 0);
	{
		int ch;
		for (ch=0; ch<N_CH; ++ch) {
			Lfo_SetPMSAMS(pThis, ch, 0);
		}
	}
	Lfo_LfoReset(pThis);
	Lfo_LfoStart(pThis);
}
void Lfo_InitSamprate(LFO* pThis) {
	pThis->LfoTimeAdd = LFOPRECISION*62500/Samprate;
}

void Lfo_LfoReset(LFO* pThis) {
	pThis->LfoStartingFlag = 0;

//	LfoTime はリセットされない！！
	pThis->LfoIdx = 0;

	Lfo_CulcTblValue(pThis);
	Lfo_CulcAllPmValue(pThis);
	Lfo_CulcAllAmValue(pThis);
}
void Lfo_LfoStart(LFO* pThis) {
	pThis->LfoStartingFlag = 1;
}
void Lfo_SetLFRQ(LFO* pThis, int n) {
	pThis->Lfrq = n & 255;

	pThis->LfoSmallCounterStep = 16+(pThis->Lfrq&15);
	int	shift = 15-(pThis->Lfrq>>4);
	if (shift == 0) {
		shift = 1;
		pThis->LfoSmallCounterStep <<= 1;
	}
	pThis->LfoOverFlow = (8<<shift) * LFOPRECISION;

//	LfoTime はリセットされる
	pThis->LfoTime = 0;
}
void Lfo_SetPMDAMD(LFO* pThis, int n) {
	if (n & 0x80) {
		pThis->Pmd = n&0x7F;
		int ch;
		for (ch=0; ch<N_CH; ++ch) {
			pThis->PmdPmsmul[ch] = pThis->Pmd * pThis->Pmsmul[ch];
		}
		Lfo_CulcAllPmValue(pThis);
	} else {
		pThis->Amd = n&0x7F;
		Lfo_CulcAllAmValue(pThis);
	}
}
void	Lfo_SetWaveForm(LFO* pThis, int n) {
	pThis->LfoWaveForm = n&3;

	Lfo_CulcTblValue(pThis);
	Lfo_CulcAllPmValue(pThis);
	Lfo_CulcAllAmValue(pThis);
}
void	Lfo_SetPMSAMS(LFO* pThis, int ch, int n) {
	int pms = (n>>4)&7;
	pThis->Pmsmul[ch] = PMSMUL[pms];
	pThis->Pmsshl[ch] = PMSSHL[pms];
	pThis->PmdPmsmul[ch] = pThis->Pmd*pThis->Pmsmul[ch];
	Lfo_CulcPmValue(pThis, ch);

	pThis->Ams[ch] = ((n&3)-1) & 31;
	Lfo_CulcAmValue(pThis, ch);
}

void	Lfo_Update(LFO* pThis) {
	if (pThis->LfoStartingFlag == 0) {
		return;
	}

	pThis->LfoTime += pThis->LfoTimeAdd;
	if (pThis->LfoTime >= pThis->LfoOverFlow) {
		pThis->LfoTime = 0;
		pThis->LfoSmallCounter += pThis->LfoSmallCounterStep;
		switch (pThis->LfoWaveForm) {
		case 0:
			{
				int idxadd = pThis->LfoSmallCounter>>4;
				pThis->LfoIdx = (pThis->LfoIdx+idxadd) & (SIZELFOTBL-1);
				pThis->PmTblValue = pThis->PmTbl0[pThis->LfoIdx];
				pThis->AmTblValue = pThis->AmTbl0[pThis->LfoIdx];
				break;
			}
		case 1:
			{
				int idxadd = pThis->LfoSmallCounter>>4;
				pThis->LfoIdx = (pThis->LfoIdx+idxadd) & (SIZELFOTBL-1);
				if ((pThis->LfoIdx&(SIZELFOTBL/2-1)) < SIZELFOTBL/4) {
					pThis->PmTblValue = 128;
					pThis->AmTblValue = 256;
				} else {
					pThis->PmTblValue = -128;
					pThis->AmTblValue = 0;
				}
			}
			break;
		case 2:
			{
				int idxadd = pThis->LfoSmallCounter>>4;
				pThis->LfoIdx = (pThis->LfoIdx+idxadd+idxadd) & (SIZELFOTBL-1);
				pThis->PmTblValue = pThis->PmTbl2[pThis->LfoIdx];
				pThis->AmTblValue = pThis->AmTbl2[pThis->LfoIdx];
				break;
			}
		case 3:
			{
#if X68SOUND_ENABLE_PORTABLE_CODE
				pThis->LfoIdx =
					irnd(pThis->m_contextImpl) >> (32-SIZELFOTBL_BITS);
#else
				pThis->LfoIdx = irnd() >> (32-SIZELFOTBL_BITS);
#endif
				pThis->PmTblValue = pThis->PmTbl0[pThis->LfoIdx];
				pThis->AmTblValue = pThis->AmTbl0[pThis->LfoIdx];
				break;
			}
		}
		pThis->LfoSmallCounter &= 15;

		Lfo_CulcAllPmValue(pThis);
		Lfo_CulcAllAmValue(pThis);
	}
}


int Lfo_GetPmValue(LFO* pThis, int ch) {
	return pThis->PmValue[ch];
}
int Lfo_GetAmValue(LFO* pThis, int ch) {
	return pThis->AmValue[ch];
}

void Lfo_CulcTblValue(LFO* pThis) {
	switch (pThis->LfoWaveForm) {
	case 0:
		pThis->PmTblValue = pThis->PmTbl0[pThis->LfoIdx];
		pThis->AmTblValue = pThis->AmTbl0[pThis->LfoIdx];
		break;
	case 1:
		if ((pThis->LfoIdx&(SIZELFOTBL/2-1)) < SIZELFOTBL/4) {
			pThis->PmTblValue = 128;
			pThis->AmTblValue = 256;
		} else {
			pThis->PmTblValue = -128;
			pThis->AmTblValue = 0;
		}
		break;
	case 2:
		pThis->PmTblValue = pThis->PmTbl2[pThis->LfoIdx];
		pThis->AmTblValue = pThis->AmTbl2[pThis->LfoIdx];
		break;
	case 3:
		pThis->PmTblValue = pThis->PmTbl0[pThis->LfoIdx];
		pThis->AmTblValue = pThis->AmTbl0[pThis->LfoIdx];
		break;
	}
}
void	Lfo_CulcPmValue(LFO* pThis, int ch) {
	if (pThis->PmTblValue >= 0) {
		pThis->PmValue[ch] =
			((pThis->PmTblValue*pThis->PmdPmsmul[ch])>>(7+5))
				<<pThis->Pmsshl[ch];
	} else {
		pThis->PmValue[ch] =
			-( (((-pThis->PmTblValue)*pThis->PmdPmsmul[ch])
				>>(7+5))<<pThis->Pmsshl[ch] );
	}
}
void	Lfo_CulcAmValue(LFO* pThis, int ch) {
	pThis->AmValue[ch] =
		(((pThis->AmTblValue*pThis->Amd)>>7)<<pThis->Ams[ch])&(int)0x7FFFFFFF;
}

void Lfo_CulcAllPmValue(LFO* pThis) {
	int ch;
	for (ch=0; ch<N_CH; ++ch) {
		Lfo_CulcPmValue(pThis, ch);
	}
}
void Lfo_CulcAllAmValue(LFO* pThis) {
	int ch;
	for (ch=0; ch<N_CH; ++ch) {
		Lfo_CulcAmValue(pThis, ch);
	}
}
#if defined(__cplusplus)
}
#endif
