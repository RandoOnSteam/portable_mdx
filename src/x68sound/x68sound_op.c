#include "x68sound_config.h"
#include "x68sound_global.h"
#include "x68sound_op.h"
#include "x68sound_context.internal.h"
#if defined(__cplusplus)
extern "C" {
#endif
const int NEXTSTAT[RELEASE_MAX+1]={
	DECAY, SUSTAIN, SUSTAIN_MAX, SUSTAIN_MAX, RELEASE_MAX, RELEASE_MAX,
};
const int MAXSTAT[RELEASE_MAX+1]={
	ATACK, SUSTAIN_MAX, SUSTAIN_MAX, SUSTAIN_MAX, RELEASE_MAX, RELEASE_MAX,
};

void Op_ConstructWithX68SoundContextImpl(OP* pThis,
	X68SoundContextImpl *contextImpl) {
	pThis->m_contextImpl = contextImpl;
}
void Op_Construct(OP* pThis) {
	(void)(pThis);
}
void Op_Init(OP* pThis) {
	pThis->Note = 5*12+8;
	pThis->Kc = 5*16+8 + 1;
	pThis->Kf = 5;
	pThis->Ar = 10;
	pThis->D1r = 10;
	pThis->D2r = 5;
	pThis->Rr = 12;
	pThis->Ks = 1;
	pThis->Dt2 = 0;
	pThis->Dt1 = 0;

	pThis->ArTime = 0;
	pThis->Fl = 31;
	pThis->Fl_mask = 0;
	pThis->Out2Fb = 0;
	pThis->inp = 0;
	pThis->Inp_last = 0;
	pThis->DeltaT = 0;
	pThis->LfoPitch = CULC_DELTA_T;
	pThis->T = 0;
	pThis->LfoLevel = CULC_ALPHA;
	pThis->Alpha = 0;
	pThis->Tl = (128-127)<<3;
	pThis->Xr_el = 1024;
	pThis->Xr_step = 0;
	pThis->Mul = 2;
	pThis->Ame = 0;

//	pThis->NoiseStep = (__int64)(1<<26)*(__int64)62500/Samprate;
#if X68SOUND_ENABLE_PORTABLE_CODE
	pThis->NoiseStep = (int)((int64_t)(1<<26)*(int64_t)OpmRate/Samprate);
#else
	pThis->NoiseStep = (int)((__int64)(1<<26)*(__int64)OpmRate/Samprate);
#endif
	Op_SetNFRQ(pThis, 0);
	pThis->NoiseValue = 1;

	// 状態推移テーブルを作成
//	pThis->StatTbl[ATACK].nextstat = DECAY;
//	pThis->StatTbl[DECAY].nextstat = SUSTAIN;
//	pThis->StatTbl[SUSTAIN].nextstat = SUSTAIN_MAX;
//	pThis->StatTbl[SUSTAIN_MAX].nextstat = SUSTAIN_MAX;
//	pThis->StatTbl[RELEASE].nextstat = RELEASE_MAX;
//	pThis->StatTbl[RELEASE_MAX].nextstat = RELEASE_MAX;

	pThis->StatTbl[ATACK].limit = 0;
	pThis->StatTbl[DECAY].limit = D1LTBL[0];
	pThis->StatTbl[SUSTAIN].limit = 63;
	pThis->StatTbl[SUSTAIN_MAX].limit = 63;
	pThis->StatTbl[RELEASE].limit = 63;
	pThis->StatTbl[RELEASE_MAX].limit = 63;

#if X68SOUND_ENABLE_PORTABLE_CODE
	pThis->StatTbl[SUSTAIN_MAX].m_and = 4097;
#else
	pThis->StatTbl[SUSTAIN_MAX].and = 4097;
#endif
	pThis->StatTbl[SUSTAIN_MAX].cmp = 2048;
	pThis->StatTbl[SUSTAIN_MAX].add = 0;
#if X68SOUND_ENABLE_PORTABLE_CODE
	pThis->StatTbl[RELEASE_MAX].m_and = 4097;
#else
	pThis->StatTbl[RELEASE_MAX].and = 4097;
#endif
	pThis->StatTbl[RELEASE_MAX].cmp = 2048;
	pThis->StatTbl[RELEASE_MAX].add = 0;

	pThis->Xr_stat = RELEASE_MAX;
#if X68SOUND_ENABLE_PORTABLE_CODE
	pThis->Xr_and = pThis->StatTbl[pThis->Xr_stat].m_and;
#else
	pThis->Xr_and = pThis->StatTbl[pThis->Xr_stat].and;
#endif
	pThis->Xr_cmp = pThis->StatTbl[pThis->Xr_stat].cmp;
	pThis->Xr_add = pThis->StatTbl[pThis->Xr_stat].add;
	pThis->Xr_limit = pThis->StatTbl[pThis->Xr_stat].limit;

	Op_CulcArStep(pThis);
	Op_CulcD1rStep(pThis);
	Op_CulcD2rStep(pThis);
	Op_CulcRrStep(pThis);
	Op_CulcPitch(pThis);
	Op_CulcDt1Pitch(pThis);
}

void Op_InitSamprate(OP* pThis) {
	pThis->LfoPitch = CULC_DELTA_T;

//	pThis->NoiseStep = (__int64)(1<<26)*(__int64)62500/Samprate;
#if X68SOUND_ENABLE_PORTABLE_CODE
	pThis->NoiseStep = (int64_t)(1<<26)*(int64_t)OpmRate/Samprate;
#else
	pThis->NoiseStep = (__int64)(1<<26)*(__int64)OpmRate/Samprate;
#endif
	Op_CulcNoiseCycle(pThis);

	Op_CulcArStep(pThis);
	Op_CulcD1rStep(pThis);
	Op_CulcD2rStep(pThis);
	Op_CulcRrStep(pThis);
	Op_CulcPitch(pThis);
	Op_CulcDt1Pitch(pThis);
}

void	Op_CulcArStep(OP* pThis) {
	if (pThis->Ar != 0) {
		int ks = (pThis->Ar<<1)+(pThis->Kc>>(5-pThis->Ks));
#if X68SOUND_ENABLE_PORTABLE_CODE
			pThis->StatTbl[ATACK].m_and = XRTBL[ks].m_and;
			pThis->StatTbl[ATACK].cmp = XRTBL[ks].m_and>>1;
#else
			pThis->StatTbl[ATACK].and = XRTBL[ks].and;
			pThis->StatTbl[ATACK].cmp = XRTBL[ks].and>>1;
#endif
		if (ks < 62) {
			pThis->StatTbl[ATACK].add = XRTBL[ks].add;
		} else {
			pThis->StatTbl[ATACK].add = 128;
		}
	} else {
#if X68SOUND_ENABLE_PORTABLE_CODE
		pThis->StatTbl[ATACK].m_and = 4097;
#else
		pThis->StatTbl[ATACK].and = 4097;
#endif
		pThis->StatTbl[ATACK].cmp = 2048;
		pThis->StatTbl[ATACK].add = 0;
	}
	if (pThis->Xr_stat == ATACK) {
#if X68SOUND_ENABLE_PORTABLE_CODE
		pThis->Xr_and = pThis->StatTbl[pThis->Xr_stat].m_and;
#else
		pThis->Xr_and = pThis->StatTbl[pThis->Xr_stat].and;
#endif
		pThis->Xr_cmp = pThis->StatTbl[pThis->Xr_stat].cmp;
		pThis->Xr_add = pThis->StatTbl[pThis->Xr_stat].add;
	}
};
void	Op_CulcD1rStep(OP* pThis) {
	if (pThis->D1r != 0) {
		int ks = (pThis->D1r<<1)+(pThis->Kc>>(5-pThis->Ks));
#if X68SOUND_ENABLE_PORTABLE_CODE
		pThis->StatTbl[DECAY].m_and = XRTBL[ks].m_and;
		pThis->StatTbl[DECAY].cmp = XRTBL[ks].m_and>>1;
#else
		pThis->StatTbl[DECAY].and = XRTBL[ks].and;
		pThis->StatTbl[DECAY].cmp = XRTBL[ks].and>>1;
#endif
		pThis->StatTbl[DECAY].add = XRTBL[ks].add;
	} else {
#if X68SOUND_ENABLE_PORTABLE_CODE
		pThis->StatTbl[DECAY].m_and = 4097;
#else
		pThis->StatTbl[DECAY].and = 4097;
#endif
		pThis->StatTbl[DECAY].cmp = 2048;
		pThis->StatTbl[DECAY].add = 0;
	}
	if (pThis->Xr_stat == DECAY) {
#if X68SOUND_ENABLE_PORTABLE_CODE
		pThis->Xr_and = pThis->StatTbl[pThis->Xr_stat].m_and;
#else
		pThis->Xr_and = pThis->StatTbl[pThis->Xr_stat].and;
#endif
		pThis->Xr_cmp = pThis->StatTbl[pThis->Xr_stat].cmp;
		pThis->Xr_add = pThis->StatTbl[pThis->Xr_stat].add;
	}
};
void	Op_CulcD2rStep(OP* pThis) {
	if (pThis->D2r != 0) {
		int ks = (pThis->D2r<<1)+(pThis->Kc>>(5-pThis->Ks));
#if X68SOUND_ENABLE_PORTABLE_CODE
		pThis->StatTbl[SUSTAIN].m_and = XRTBL[ks].m_and;
		pThis->StatTbl[SUSTAIN].cmp = XRTBL[ks].m_and>>1;
#else
		pThis->StatTbl[SUSTAIN].and = XRTBL[ks].and;
		pThis->StatTbl[SUSTAIN].cmp = XRTBL[ks].and>>1;
#endif
		pThis->StatTbl[SUSTAIN].add = XRTBL[ks].add;
	} else {
#if X68SOUND_ENABLE_PORTABLE_CODE
		pThis->StatTbl[SUSTAIN].m_and = 4097;
#else
		pThis->StatTbl[SUSTAIN].and = 4097;
#endif
		pThis->StatTbl[SUSTAIN].cmp = 2048;
		pThis->StatTbl[SUSTAIN].add = 0;
	}
	if (pThis->Xr_stat == SUSTAIN) {
#if X68SOUND_ENABLE_PORTABLE_CODE
		pThis->Xr_and = pThis->StatTbl[pThis->Xr_stat].m_and;
#else
		pThis->Xr_and = pThis->StatTbl[pThis->Xr_stat].and;
#endif
		pThis->Xr_cmp = pThis->StatTbl[pThis->Xr_stat].cmp;
		pThis->Xr_add = pThis->StatTbl[pThis->Xr_stat].add;
	}
};
void	Op_CulcRrStep(OP* pThis) {
	int ks = (pThis->Rr<<2)+2+(pThis->Kc>>(5-pThis->Ks));
#if X68SOUND_ENABLE_PORTABLE_CODE
	pThis->StatTbl[RELEASE].m_and = XRTBL[ks].m_and;
	pThis->StatTbl[RELEASE].cmp = XRTBL[ks].m_and>>1;
#else
	pThis->StatTbl[RELEASE].and = XRTBL[ks].and;
	pThis->StatTbl[RELEASE].cmp = XRTBL[ks].and>>1;
#endif
	pThis->StatTbl[RELEASE].add = XRTBL[ks].add;
	if (pThis->Xr_stat == RELEASE) {
#if X68SOUND_ENABLE_PORTABLE_CODE
		pThis->Xr_and = pThis->StatTbl[pThis->Xr_stat].m_and;
#else
		pThis->Xr_and = pThis->StatTbl[pThis->Xr_stat].and;
#endif
		pThis->Xr_cmp = pThis->StatTbl[pThis->Xr_stat].cmp;
		pThis->Xr_add = pThis->StatTbl[pThis->Xr_stat].add;
	}
};
void Op_CulcPitch(OP* pThis) {
	pThis->Pitch = (pThis->Note<<6)+pThis->Kf+pThis->Dt2;
}
void Op_CulcDt1Pitch(OP* pThis) {
	pThis->Dt1Pitch = DT1TBL[(pThis->Kc&0xFC)+(pThis->Dt1&3)];
	if (pThis->Dt1&0x04) {
		pThis->Dt1Pitch = -pThis->Dt1Pitch;
	}
}

void Op_SetFL(OP* pThis, int n) {
	n = (n>>3) & 7;
	if (n == 0) {
		pThis->Fl = 31;
		pThis->Fl_mask = 0;
	} else {
		pThis->Fl = (7-n+1+1);
		pThis->Fl_mask = -1;
	}
};

void Op_SetKC(OP* pThis, int n) {
	int note;
	pThis->Kc = n & 127;
	note = pThis->Kc & 15;
	pThis->Note = ((pThis->Kc>>4)+1)*12+ note-(note>>2);
	++pThis->Kc;
	Op_CulcPitch(pThis);
	Op_CulcDt1Pitch(pThis);
	pThis->LfoPitch = CULC_DELTA_T;
	Op_CulcArStep(pThis);
	Op_CulcD1rStep(pThis);
	Op_CulcD2rStep(pThis);
	Op_CulcRrStep(pThis);
};

void Op_SetKF(OP* pThis, int n) {
	pThis->Kf = (n&255)>>2;
	Op_CulcPitch(pThis);
	pThis->LfoPitch = CULC_DELTA_T;
};

void Op_SetDT1MUL(OP* pThis, int n) {
	pThis->Dt1 = (n>>4)&7;
	Op_CulcDt1Pitch(pThis);
	pThis->Mul = (n&15)<<1;
	if (pThis->Mul == 0) {
		pThis->Mul = 1;
	}
	pThis->LfoPitch = CULC_DELTA_T;
};

void Op_SetTL(OP* pThis, int n) {
	pThis->Tl = (128-(n&127))<<3;
	pThis->LfoLevel = CULC_ALPHA;
};

void Op_SetKSAR(OP* pThis, int n) {
	pThis->Ks = (n&255)>>6;
	pThis->Ar = n & 31;
	Op_CulcArStep(pThis);
	Op_CulcD1rStep(pThis);
	Op_CulcD2rStep(pThis);
	Op_CulcRrStep(pThis);
};

void Op_SetAMED1R(OP* pThis, int n) {
	pThis->D1r = n & 31;
	Op_CulcD1rStep(pThis);
	pThis->Ame = 0;
	if (n & 0x80) {
		pThis->Ame = -1;
	}
};

void Op_SetDT2D2R(OP* pThis, int n) {
	pThis->Dt2 = DT2TBL[(n&255)>>6];
	Op_CulcPitch(pThis);
	pThis->LfoPitch = CULC_DELTA_T;
	pThis->D2r = n & 31;
	Op_CulcD2rStep(pThis);
};

void Op_SetD1LRR(OP* pThis, int n) {
	pThis->StatTbl[DECAY].limit = D1LTBL[(n&255)>>4];
	if (pThis->Xr_stat == DECAY) {
		pThis->Xr_limit = pThis->StatTbl[DECAY].limit;
	}

	pThis->Rr = n & 15;
	Op_CulcRrStep(pThis);
};

void Op_KeyON(OP* pThis) {
	if (pThis->Xr_stat >= RELEASE) {
		// KEYON
		pThis->T = 0;

		if (pThis->Xr_el == 0) {
			pThis->Xr_stat = DECAY;
#if X68SOUND_ENABLE_PORTABLE_CODE
			pThis->Xr_and = pThis->StatTbl[pThis->Xr_stat].m_and;
#else
			pThis->Xr_and = pThis->StatTbl[pThis->Xr_stat].and;
#endif
			pThis->Xr_cmp = pThis->StatTbl[pThis->Xr_stat].cmp;
			pThis->Xr_add = pThis->StatTbl[pThis->Xr_stat].add;
			pThis->Xr_limit = pThis->StatTbl[pThis->Xr_stat].limit;
			if ((pThis->Xr_el>>4) == pThis->Xr_limit) {
				pThis->Xr_stat = NEXTSTAT[pThis->Xr_stat];
#if X68SOUND_ENABLE_PORTABLE_CODE
				pThis->Xr_and = pThis->StatTbl[pThis->Xr_stat].m_and;
#else
				pThis->Xr_and = pThis->StatTbl[pThis->Xr_stat].and;
#endif
				pThis->Xr_cmp = pThis->StatTbl[pThis->Xr_stat].cmp;
				pThis->Xr_add = pThis->StatTbl[pThis->Xr_stat].add;
				pThis->Xr_limit = pThis->StatTbl[pThis->Xr_stat].limit;
			}
		} else {
			pThis->Xr_stat = ATACK;
#if X68SOUND_ENABLE_PORTABLE_CODE
			pThis->Xr_and = pThis->StatTbl[pThis->Xr_stat].m_and;
#else
			pThis->Xr_and = pThis->StatTbl[pThis->Xr_stat].and;
#endif
			pThis->Xr_cmp = pThis->StatTbl[pThis->Xr_stat].cmp;
			pThis->Xr_add = pThis->StatTbl[pThis->Xr_stat].add;
			pThis->Xr_limit = pThis->StatTbl[pThis->Xr_stat].limit;
		}
	}
};
void Op_KeyOFF(OP* pThis) {
	pThis->Xr_stat = RELEASE;
#if X68SOUND_ENABLE_PORTABLE_CODE
	pThis->Xr_and = pThis->StatTbl[pThis->Xr_stat].m_and;
#else
	pThis->Xr_and = pThis->StatTbl[pThis->Xr_stat].and;
#endif
	pThis->Xr_cmp = pThis->StatTbl[pThis->Xr_stat].cmp;
	pThis->Xr_add = pThis->StatTbl[pThis->Xr_stat].add;
	pThis->Xr_limit = pThis->StatTbl[pThis->Xr_stat].limit;
	if ((pThis->Xr_el>>4) >= 63) {
		pThis->Xr_el = 1024;
		pThis->Xr_stat = MAXSTAT[pThis->Xr_stat];
#if X68SOUND_ENABLE_PORTABLE_CODE
		pThis->Xr_and = pThis->StatTbl[pThis->Xr_stat].m_and;
#else
		pThis->Xr_and = pThis->StatTbl[pThis->Xr_stat].and;
#endif
		pThis->Xr_cmp = pThis->StatTbl[pThis->Xr_stat].cmp;
		pThis->Xr_add = pThis->StatTbl[pThis->Xr_stat].add;
		pThis->Xr_limit = pThis->StatTbl[pThis->Xr_stat].limit;
	}
};

void Op_Envelope(OP* pThis, int env_counter) {
	if ((env_counter&pThis->Xr_and) == pThis->Xr_cmp) {

		if (pThis->Xr_stat==ATACK) {
			// ATACK
			pThis->Xr_step += pThis->Xr_add;
			pThis->Xr_el += ((~pThis->Xr_el)*(pThis->Xr_step>>3)) >> 4;
			pThis->LfoLevel = CULC_ALPHA;
			pThis->Xr_step &= 7;

			if (pThis->Xr_el <= 0) {
				pThis->Xr_el = 0;
				pThis->Xr_stat = DECAY;
#if X68SOUND_ENABLE_PORTABLE_CODE
				pThis->Xr_and = pThis->StatTbl[pThis->Xr_stat].m_and;
#else
				pThis->Xr_and = pThis->StatTbl[pThis->Xr_stat].and;
#endif
				pThis->Xr_cmp = pThis->StatTbl[pThis->Xr_stat].cmp;
				pThis->Xr_add = pThis->StatTbl[pThis->Xr_stat].add;
				pThis->Xr_limit = pThis->StatTbl[pThis->Xr_stat].limit;
				if ((pThis->Xr_el>>4) == pThis->Xr_limit) {
					pThis->Xr_stat = NEXTSTAT[pThis->Xr_stat];
#if X68SOUND_ENABLE_PORTABLE_CODE
					pThis->Xr_and = pThis->StatTbl[pThis->Xr_stat].m_and;
#else
					pThis->Xr_and = pThis->StatTbl[pThis->Xr_stat].and;
#endif
					pThis->Xr_cmp = pThis->StatTbl[pThis->Xr_stat].cmp;
					pThis->Xr_add = pThis->StatTbl[pThis->Xr_stat].add;
					pThis->Xr_limit = pThis->StatTbl[pThis->Xr_stat].limit;
				}
			}
		} else {
			// DECAY, SUSTAIN, RELEASE
			pThis->Xr_step += pThis->Xr_add;
			pThis->Xr_el += pThis->Xr_step>>3;
			pThis->LfoLevel = CULC_ALPHA;
			pThis->Xr_step &= 7;

			int e = pThis->Xr_el>>4;
			if (e == 63) {
				pThis->Xr_el = 1024;
				pThis->Xr_stat = MAXSTAT[pThis->Xr_stat];
#if X68SOUND_ENABLE_PORTABLE_CODE
				pThis->Xr_and = pThis->StatTbl[pThis->Xr_stat].m_and;
#else
				pThis->Xr_and = pThis->StatTbl[pThis->Xr_stat].and;
#endif
				pThis->Xr_cmp = pThis->StatTbl[pThis->Xr_stat].cmp;
				pThis->Xr_add = pThis->StatTbl[pThis->Xr_stat].add;
				pThis->Xr_limit = pThis->StatTbl[pThis->Xr_stat].limit;
			} else if (e == pThis->Xr_limit) {
				pThis->Xr_stat = NEXTSTAT[pThis->Xr_stat];
#if X68SOUND_ENABLE_PORTABLE_CODE
				pThis->Xr_and = pThis->StatTbl[pThis->Xr_stat].m_and;
#else
				pThis->Xr_and = pThis->StatTbl[pThis->Xr_stat].and;
#endif
				pThis->Xr_cmp = pThis->StatTbl[pThis->Xr_stat].cmp;
				pThis->Xr_add = pThis->StatTbl[pThis->Xr_stat].add;
				pThis->Xr_limit = pThis->StatTbl[pThis->Xr_stat].limit;
			}
		}
	}
}

void Op_SetNFRQ(OP* pThis, int nfrq) {
	if ((pThis->Nfrq ^ nfrq) & 0x80) {
		pThis->LfoLevel = CULC_ALPHA;
	}
	pThis->Nfrq = nfrq;
	Op_CulcNoiseCycle(pThis);
}
void Op_CulcNoiseCycle(OP* pThis) {
	if (pThis->Nfrq & 0x80) {
		pThis->NoiseCycle = (32-(pThis->Nfrq&31)) << 25;
		if (pThis->NoiseCycle < pThis->NoiseStep) {
			pThis->NoiseCycle = pThis->NoiseStep;
		}
		pThis->NoiseCounter = pThis->NoiseCycle;
	} else {
		pThis->NoiseCycle = 0;
	}
}


void Op_Output0(OP* pThis, int lfopitch, int lfolevel) {
	if (pThis->LfoPitch != lfopitch) {
//		pThis->DeltaT = ((STEPTBL[pThis->Pitch+lfopitch]+pThis->Dt1Pitch)
// *pThis->Mul)>>1;
		pThis->DeltaT = ((STEPTBL[pThis->Pitch+lfopitch]+pThis->Dt1Pitch)
			*pThis->Mul)>>(6+1);
		pThis->LfoPitch = lfopitch;
	}
	pThis->T += pThis->DeltaT;

	int lfolevelame = lfolevel & pThis->Ame;
	if (pThis->LfoLevel != lfolevelame) {
		pThis->Alpha = (int)(ALPHATBL[
			ALPHAZERO+pThis->Tl-pThis->Xr_el-lfolevelame]);
		pThis->LfoLevel = lfolevelame;
	}
	int o = (pThis->Alpha)
		* (int)(SINTBL[
				(((pThis->T+pThis->Out2Fb)>>PRECISION_BITS))&(SIZESINTBL-1)]);

//	int o2 = (o+Inp_last) >> 1;
//	pThis->Out2Fb = (o+o) >> Fl;
	pThis->Out2Fb = ((o + pThis->Inp_last) & pThis->Fl_mask) >> pThis->Fl;
	pThis->Inp_last = o;

	*pThis->out = o;
	*pThis->out2 = o;	// alg=5用
	*pThis->out3 = o; // alg=5用
//	*pThis->out = o2;
//	*pThis->out2 = o2;	// alg=5用
//	*pThis->out3 = o2; // alg=5用
};

void Op_Output(OP* pThis, int lfopitch, int lfolevel) {
	int lfolevelame;
	int o;
	if (pThis->LfoPitch != lfopitch) {
//		pThis->DeltaT = ((STEPTBL[pThis->Pitch+lfopitch]+pThis->Dt1Pitch)
// *pThis->Mul)>>1;
		pThis->DeltaT = ((STEPTBL[pThis->Pitch+lfopitch]+pThis->Dt1Pitch)
			*pThis->Mul)>>(6+1);
		pThis->LfoPitch = lfopitch;
	}
	pThis->T += pThis->DeltaT;

	lfolevelame = lfolevel & pThis->Ame;
	if (pThis->LfoLevel != lfolevelame) {
		pThis->Alpha = (int)(
			ALPHATBL[ALPHAZERO+pThis->Tl-pThis->Xr_el-lfolevelame]);
		pThis->LfoLevel = lfolevelame;
	}
	o = (pThis->Alpha)
		* (int)(SINTBL[(((pThis->T+pThis->inp)>>PRECISION_BITS))
			&(SIZESINTBL-1)]) ;

	*pThis->out += o;
};

void Op_Output32(OP* pThis, int lfopitch, int lfolevel) {
	int o;
	if (pThis->LfoPitch != lfopitch) {
//		pThis->DeltaT = ((STEPTBL[pThis->Pitch+lfopitch]+pThis->Dt1Pitch)
// *pThis->Mul)>>1;
		pThis->DeltaT = ((STEPTBL[pThis->Pitch+lfopitch]+pThis->Dt1Pitch)
			*pThis->Mul)>>(6+1);
		pThis->LfoPitch = lfopitch;
	}
	pThis->T += pThis->DeltaT;

	if (pThis->NoiseCycle == 0) {
		int lfolevelame = lfolevel & pThis->Ame;
		if (pThis->LfoLevel != lfolevelame) {
			pThis->Alpha = (int)(ALPHATBL[
				ALPHAZERO+pThis->Tl-pThis->Xr_el-lfolevelame]);
			pThis->LfoLevel = lfolevelame;
		}
		o = (pThis->Alpha) * (int)(SINTBL[
			(((pThis->T+pThis->inp)>>PRECISION_BITS))&(SIZESINTBL-1)]) ;
	} else {
		int lfolevelame;
		pThis->NoiseCounter -= pThis->NoiseStep;
		if (pThis->NoiseCounter <= 0) {
#if X68SOUND_ENABLE_PORTABLE_CODE
			pThis->NoiseValue = (int)((irnd(pThis->m_contextImpl)>>30)&2)-1;
#else
			pThis->NoiseValue = (int)((irnd()>>30)&2)-1;
#endif
			pThis->NoiseCounter += pThis->NoiseCycle;
		}

		lfolevelame = lfolevel & pThis->Ame;
		if (pThis->LfoLevel != lfolevelame) {
			pThis->Alpha = (int)(NOISEALPHATBL[
				ALPHAZERO+pThis->Tl-pThis->Xr_el-lfolevelame]);
			pThis->LfoLevel = lfolevelame;
		}
		o = (pThis->Alpha)
			* pThis->NoiseValue * MAXSINVAL;
	}

	*pThis->out += o;
};

void Op_Output0_22(OP* pThis, int lfopitch, int lfolevel) {
	int lfolevelame;
	int o;
	if (pThis->LfoPitch != lfopitch) {
//		pThis->DeltaT = ((STEPTBL[pThis->Pitch+lfopitch]+pThis->Dt1Pitch)
// *pThis->Mul)>>1;
		pThis->DeltaT = ((STEPTBL[pThis->Pitch+lfopitch]+pThis->Dt1Pitch)
			*pThis->Mul)>>(6+1);
		pThis->LfoPitch = lfopitch;
	}
	pThis->T += pThis->DeltaT;

	lfolevelame = lfolevel & pThis->Ame;
	if (pThis->LfoLevel != lfolevelame) {
		pThis->Alpha = (int)(ALPHATBL[
			ALPHAZERO+pThis->Tl-pThis->Xr_el-lfolevelame]);
		pThis->LfoLevel = lfolevelame;
	}
	o = (pThis->Alpha)
		* (int)(SINTBL[(((pThis->T+pThis->Out2Fb)
			>>PRECISION_BITS))&(SIZESINTBL-1)]) ;

	pThis->Out2Fb = ((o + pThis->Inp_last) & pThis->Fl_mask) >> pThis->Fl;
	pThis->Inp_last = o;

//	*pThis->out += o;
//	*pThis->out2 += o;	// alg=5用
//	*pThis->out3 += o; // alg=5用
	*pThis->out = o;
	*pThis->out2 = o;	// alg=5用
	*pThis->out3 = o; // alg=5用
};

void Op_Output_22(OP* pThis, int lfopitch, int lfolevel) {
	int lfolevelame;
	int o;
	if (pThis->LfoPitch != lfopitch) {
//		pThis->DeltaT = ((STEPTBL[pThis->Pitch+lfopitch]+pThis->Dt1Pitch)*pThis->Mul)>>1;
		pThis->DeltaT = ((STEPTBL[pThis->Pitch+lfopitch]+pThis->Dt1Pitch)*pThis->Mul)>>(6+1);
		pThis->LfoPitch = lfopitch;
	}
	pThis->T += pThis->DeltaT;

	lfolevelame = lfolevel & pThis->Ame;
	if (pThis->LfoLevel != lfolevelame) {
		pThis->Alpha = (int)(ALPHATBL[
			ALPHAZERO+pThis->Tl-pThis->Xr_el-lfolevelame]);
		pThis->LfoLevel = lfolevelame;
	}
	o = (pThis->Alpha)
		* (int)(SINTBL[(((pThis->T+pThis->inp)>>PRECISION_BITS))&(SIZESINTBL-1)]) ;

	*pThis->out += o;
};

void Op_Output32_22(OP* pThis, int lfopitch, int lfolevel) {
	int o;
	if (pThis->LfoPitch != lfopitch) {
//		pThis->DeltaT = ((STEPTBL[pThis->Pitch+lfopitch]+pThis->Dt1Pitch)
// *pThis->Mul)>>1;
		pThis->DeltaT = ((STEPTBL[pThis->Pitch+lfopitch]+pThis->Dt1Pitch)
			*pThis->Mul)>>(6+1);
		pThis->LfoPitch = lfopitch;
	}
	pThis->T += pThis->DeltaT;

	if (pThis->NoiseCycle == 0) {
		int lfolevelame = lfolevel & pThis->Ame;
		if (pThis->LfoLevel != lfolevelame) {
			pThis->Alpha = (int)(ALPHATBL[
				ALPHAZERO+pThis->Tl-pThis->Xr_el-lfolevelame]);
			pThis->LfoLevel = lfolevelame;
		}
		o = (pThis->Alpha)
			* (int)(SINTBL[(((pThis->T+pThis->inp)
				>>PRECISION_BITS))&(SIZESINTBL-1)]) ;
	} else {
		int lfolevelame;
		pThis->NoiseCounter -= pThis->NoiseStep;
		if (pThis->NoiseCounter <= 0) {
#if X68SOUND_ENABLE_PORTABLE_CODE
			pThis->NoiseValue = (int)((irnd(pThis->m_contextImpl)>>30)&2)-1;
#else
			pThis->NoiseValue = (int)((irnd()>>30)&2)-1;
#endif
			pThis->NoiseCounter += pThis->NoiseCycle;
		}

		lfolevelame = lfolevel & pThis->Ame;
		if (pThis->LfoLevel != lfolevelame) {
			pThis->Alpha = (int)(
				NOISEALPHATBL[ALPHAZERO+pThis->Tl-pThis->Xr_el-lfolevelame]);
			pThis->LfoLevel = lfolevelame;
		}
#if X68SOUND_ENABLE_PORTABLE_CODE
		o = (pThis->Alpha)
			* pThis->NoiseValue * MAXSINVAL;
#else
		o = (pThis->Alpha)
			* pThis->NoiseValue * MAXSINVAL;
#endif
	}

	*pThis->out += o;
};
#if defined(__cplusplus)
}
#endif
