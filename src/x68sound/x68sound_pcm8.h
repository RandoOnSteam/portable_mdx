#ifndef __X68SOUND_PCM8_H__
#define __X68SOUND_PCM8_H__

#include "x68sound_config.h"

typedef struct PCM8 {
#if X68SOUND_ENABLE_PORTABLE_CODE
/*public:*/
	struct tagX68SoundContextImpl *m_contextImpl;
/*private:*/
#endif
	int	Scale;		//
	int Pcm;		// 16bit PCM Data
	int Pcm16Prev;	// 16bit,8bitPCMの1つ前のデータ
	int InpPcm,InpPcm_prev,OutPcm;		// HPF用 16bit PCM Data
	int OutInpPcm,OutInpPcm_prev;		// HPF用
	int	AdpcmRate;	// 187500(15625*12), 125000(10416.66*12), 93750(7812.5*12), 62500(5208.33*12), 46875(3906.25*12), ...
	int	RateCounter;
	int	N1Data;	// ADPCM 1サンプルのデータの保存
	int N1DataFlag;	// 0 or 1

	volatile int	Mode;
	volatile int	Volume;	// x/16
	volatile int	PcmKind;	// 0～4:ADPCM  5:16bitPCM  6:8bitPCM  7:謎

/*public:*/
	unsigned char	DmaLastValue;
	unsigned char	AdpcmReg;

	volatile unsigned char *DmaMar;
	volatile unsigned int DmaMtc;
	volatile unsigned char *DmaBar;
	volatile unsigned int DmaBtc;
	volatile int	DmaOcr;				// 0:チェイン動作なし 0x08:アレイチェイン 0x0C:リンクアレイチェイン
} PCM8;


	void Pcm8_adpcm2pcm(PCM8* pThis, unsigned char adpcm);
	void Pcm8_pcm16_2pcm(PCM8* pThis, int pcm16);

	int	Pcm8_DmaGetByte(PCM8* pThis);
	int Pcm8_DmaArrayChainSetNextMtcMar(PCM8* pThis);
	int Pcm8_DmaLinkArrayChainSetNextMtcMar(PCM8* pThis);

#if X68SOUND_ENABLE_PORTABLE_CODE
	void Pcm8_ConstructWithX68SoundContextImpl(PCM8* pThis,
		struct tagX68SoundContextImpl *contextImpl);
#endif
	void Pcm8_Construct(PCM8* pThis);
#define Pcm8_Destruct(pThis)
	void	Pcm8_Init(PCM8* pThis);
	void	Pcm8_InitSamprate(PCM8* pThis);
	void	Pcm8_Reset(PCM8* pThis);
	int		Pcm8_GetPcm(PCM8* pThis);
	int		Pcm8_GetPcm62(PCM8* pThis);

	int		Pcm8_Out(PCM8* pThis, void *adrs, int mode, int len);
	int		Pcm8_Aot(PCM8* pThis, void *tbl, int mode, int cnt);
	int		Pcm8_Lot(PCM8* pThis, void *tbl, int mode);
	int		Pcm8_SetMode(PCM8* pThis, int mode);
	int		Pcm8_GetRest(PCM8* pThis);
	int		Pcm8_GetMode(PCM8* pThis);
#endif //__X68SOUND_PCM8_H__
