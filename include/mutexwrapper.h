#ifndef __MUTEXWRAPPER_H__
#define __MUTEXWRAPPER_H__
#if defined(_WIN32)
	#include <windows.h>
	class MutexWrapper
	{
	public:
		MutexWrapper() { mH = CreateMutex(NULL, FALSE, NULL); }
		~MutexWrapper() { CloseHandle(mH); }
		void lock() { WaitForSingleObject(mH, INFINITE); }
		void unlock() { ReleaseMutex(mH); }
		void* mH;
	};
#else
	#include <mutex>
	typedef std::mutex MutexWrapper;
#endif
#endif /* __MUTEXWRAPPER_H__ */