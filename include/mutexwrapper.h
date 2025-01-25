#ifndef __MUTEXWRAPPER_H__
#define __MUTEXWRAPPER_H__
#if defined(_WIN32)
	#include <windows.h>
	typedef struct MUTEXWRAPPER
	{
		void* mH;
	} MUTEXWRAPPER;
	#define MutexWrapper_Construct(pThis) \
		(pThis)->mH = CreateMutex(NULL, FALSE, NULL);
	#define MutexWrapper_Destruct(pThis) \
		CloseHandle((pThis)->mH);
	#define MutexWrapper_lock(pThis) \
		WaitForSingleObject((pThis)->mH, INFINITE);
	#define MutexWrapper_unlock(pThis) \
		ReleaseMutex((pThis)->mH);
#else
	#include <pthread.h>
	typedef struct MUTEXWRAPPER
	{
		pthread_mutex_t mutex;
	} MUTEXWRAPPER;
	#define MutexWrapper_Construct(pThis) \
		pthread_mutex_init(&(pThis)->mutex, NULL);
	#define MutexWrapper_Destruct(pThis) \
		pthread_mutex_destroy(&(pThis)->mutex);
	#define MutexWrapper_lock(pThis) \
		pthread_mutex_lock(&(pThis)->mutex);
	#define MutexWrapper_unlock(pThis) \
		pthread_mutex_unlock(&(pThis)->mutex);
#endif
#endif /* __MUTEXWRAPPER_H__ */