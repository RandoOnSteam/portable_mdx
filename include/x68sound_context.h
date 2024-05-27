/* Copyright (C) 2018 Yosshin(@yosshin4004) */

#ifndef __X68SOUND_CONTEXT_H__
#define __X68SOUND_CONTEXT_H__


#if defined(_MSC_VER) && _MSC_VER <= 1400 /* 2005 <= */
	#define nullptr NULL
	#define override
	#define alignas(x) /* not supported */
	#ifndef __cplusplus
		#define bool int
		#define true 1
		#define false 0
	#endif
#else
	#include <stdbool.h>
#endif
#if defined(__cplusplus) && !defined(NOTHROW)
	#include <new.h>
	template <class DATATYPE> void placement_new(
		void* d, const DATATYPE& t) { new (d) DATATYPE(t); }
#endif
#ifdef __cplusplus
extern "C" {
#endif

typedef struct tagX68SoundContext {
	struct tagX68SoundContextImpl *m_impl;
} X68SoundContext;

bool X68SoundContext_Initialize(
	X68SoundContext *context,
	void *dmaBase
);
bool X68SoundContext_Terminate(
	X68SoundContext *context
);

#ifdef __cplusplus
}
#endif

#endif //__X68SOUND_CONTEXT_H__
