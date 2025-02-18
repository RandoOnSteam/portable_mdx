#ifndef __X68SOUND_ADPCM_H__
#define __X68SOUND_ADPCM_H__

#include "x68sound_config.h"

typedef struct ADPCM {
#if X68SOUND_ENABLE_PORTABLE_CODE
/*public:*/
	struct tagX68SoundContextImpl *m_contextImpl;
/*private:*/
#endif
	int	Scale;		//
	int Pcm;		// 16bit PCM Data
	int InpPcm,InpPcm_prev,OutPcm;		// HPF用 16bit PCM Data
	int OutInpPcm,OutInpPcm_prev;		// HPF用
	volatile int	AdpcmRate;	// 187500(15625*12), 125000(10416.66*12), 93750(7812.5*12), 62500(5208.33*12), 46875(3906.25*12), ...
	int	RateCounter;
	int	N1Data;	// ADPCM 1サンプルのデータの保存
	int N1DataFlag;	// 0 or 1

/*public:*/
#if X68SOUND_ENABLE_PORTABLE_CODE
	void (*IntProc)(void *);	// 割り込みアドレス
	void *IntArg;
	void (*ErrIntProc)(void *);	// 割り込みアドレス
	void *ErrIntArg;
#else
	void (CALLBACK *IntProc)();	// 割り込みアドレス
	void (CALLBACK *ErrIntProc)();	// 割り込みアドレス
#endif
//	int	AdpcmFlag;	// 0:非動作  1:再生中
//	int PpiReg;		// PPI レジスタの内容
//	int	DmaCsr;		// DMA CSR レジスタの内容
//	int	DmaCcr;		// DMA CCR レジスタの内容
//	int	DmaFlag;	// 0:DMA非動作  1:DMA動作中
	unsigned char	DmaLastValue;
	volatile unsigned char	AdpcmReg;
#if X68SOUND_ENABLE_PORTABLE_CODE
	volatile union {
		uint8_t asUint8[0x40];
		uint16_t asUint16[0x20];
		uint32_t asUint32[0x10];
	} DmaReg;
#else
	volatile unsigned char	DmaReg[0x40];
#endif
	int FinishCounter;
} ADPCM;

int	Adpcm_DmaGetByte(ADPCM* pThis);
void  Adpcm_adpcm2pcm(ADPCM* pThis, unsigned char adpcm);

void Adpcm_DmaError(ADPCM* pThis, unsigned char errcode);
void Adpcm_DmaFinish(ADPCM* pThis);
int Adpcm_DmaContinueSetNextMtcMar(ADPCM* pThis);
int Adpcm_DmaArrayChainSetNextMtcMar(ADPCM* pThis);
int Adpcm_DmaLinkArrayChainSetNextMtcMar(ADPCM* pThis);

void Adpcm_ConstructWithX68SoundContextImpl(ADPCM* pThis,
	struct tagX68SoundContextImpl *contextImpl);
void Adpcm_Construct(ADPCM* pThis);
#define Adpcm_Destruct(pThis)
void Adpcm_Init(ADPCM* pThis);
void Adpcm_InitSamprate(ADPCM* pThis);
void Adpcm_Reset(ADPCM* pThis);
int	Adpcm_GetPcm(ADPCM* pThis);
int	Adpcm_GetPcm62(ADPCM* pThis);

void Adpcm_SetAdpcmRate(ADPCM* pThis, int rate);

#endif //__X68SOUND_ADPCM_H__
