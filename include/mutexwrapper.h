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
	#include <mutex>
	typedef std::mutex MutexWrapper;
#endif
#endif /* __MUTEXWRAPPER_H__ */