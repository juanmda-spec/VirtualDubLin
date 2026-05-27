//	VirtualDub - Video processing and capture application
//	System library component
//	Copyright (C) 1998-2004 Avery Lee, All Rights Reserved.
//
//	Beginning with 1.6.0, the VirtualDub system library is licensed
//	differently than the remainder of VirtualDub.  This particular file is
//	thus licensed as follows (the "zlib" license):
//
//	This software is provided 'as-is', without any express or implied
//	warranty.  In no event will the authors be held liable for any
//	damages arising from the use of this software.
//
//	Permission is granted to anyone to use this software for any purpose,
//	including commercial applications, and to alter it and redistribute it
//	freely, subject to the following restrictions:
//
//	1.	The origin of this software must not be misrepresented; you must
//		not claim that you wrote the original software. If you use this
//		software in a product, an acknowledgment in the product
//		documentation would be appreciated but is not required.
//	2.	Altered source versions must be plainly marked as such, and must
//		not be misrepresented as being the original software.
//	3.	This notice may not be removed or altered from any source
//		distribution.

#include "stdafx.h"
#ifndef _LINUX_PORT
#include <windows.h>
#include <process.h>
#endif

#ifdef _LINUX_PORT
#include <pthread.h>
#include <unistd.h>
#include <sys/syscall.h>
#endif

#include <vd2/system/vdtypes.h>
#include <vd2/system/thread.h>
#include <vd2/system/tls.h>
#include <vd2/system/protscope.h>
#include <vd2/system/bitmath.h>

namespace {
	//
	// This apparently came from one a talk by one of the Visual Studio
	// developers, i.e. I didn't write it.
	//
	#define MS_VC_EXCEPTION 0x406d1388

	typedef struct tagTHREADNAME_INFO
	{
		DWORD dwType;        // must be 0x1000
		LPCSTR szName;       // pointer to name (in same addr space)
		DWORD dwThreadID;    // thread ID (-1 caller thread)
		DWORD dwFlags;       // reserved for future use, most be zero
	} THREADNAME_INFO;
}

VDThreadID VDGetCurrentThreadID() {
#ifndef _LINUX_PORT
	return (VDThreadID)GetCurrentThreadId();
#else
	return (VDThreadID)syscall(SYS_gettid);
#endif
}

VDProcessId VDGetCurrentProcessId() {
#ifndef _LINUX_PORT
	return (VDProcessId)GetCurrentProcessId();
#else
	return (VDProcessId)getpid();
#endif
}

uint32 VDGetLogicalProcessorCount() {
#ifndef _LINUX_PORT
	DWORD_PTR processAffinityMask;
	DWORD_PTR systemAffinityMask;
	if (!::GetProcessAffinityMask(::GetCurrentProcess(), &processAffinityMask, &systemAffinityMask))
		return 1;

	// avoid unnecessary WTFs
	if (!processAffinityMask)
		return 1;

	// We use the process affinity mask as that's the number of logical processors we'll
	// actually be working with.
	return VDCountBits(processAffinityMask);
#else
	return (uint32)sysconf(_SC_NPROCESSORS_ONLN);
#endif
}

void VDSetThreadDebugName(VDThreadID tid, const char *name) {
	#ifdef VD_COMPILER_MSVC
		THREADNAME_INFO info;
		info.dwType		= 0x1000;
		info.szName		= name;
		info.dwThreadID	= tid;
		info.dwFlags	= 0;

		__try {
			RaiseException(MS_VC_EXCEPTION, 0, sizeof(info) / sizeof(DWORD), (ULONG_PTR *)&info);
		} __except (EXCEPTION_CONTINUE_EXECUTION) {
		}
	#endif
}

void VDThreadSleep(int milliseconds) {
	if (milliseconds > 0) {
#ifndef _LINUX_PORT
		::Sleep(milliseconds);
#else
		usleep(milliseconds * 1000);
#endif
	}
}

///////////////////////////////////////////////////////////////////////////

VDThread::VDThread(const char *pszDebugName)
	: mpszDebugName(pszDebugName)
	, mhThread(0)
	, mThreadID(0)
	, mThreadPriority(INT_MIN)
{
}

VDThread::~VDThread() throw() {
	if (isThreadAttached())
		ThreadWait();
}

bool VDThread::ThreadStart() {
	VDASSERT(!isThreadAttached());

	if (!isThreadAttached()) {
	#ifndef _LINUX_PORT
		mhThread = (void *)_beginthreadex(NULL, 0, StaticThreadStart, this, 0, &mThreadID);

		if (mhThread && mThreadPriority != INT_MIN)
			::SetThreadPriority(mhThread, mThreadPriority);
	#else
		pthread_t *pt = new pthread_t;
		if (pthread_create(pt, NULL, (void* (*)(void*))StaticThreadStart, this) == 0) {
			mhThread = (void *)pt;
			mThreadID = (VDThreadID)*pt;
		} else {
			delete pt;
		}
	#endif
	}

	return mhThread != 0;
}

void VDThread::ThreadDetach() {
	if (isThreadAttached()) {
#ifndef _LINUX_PORT
		CloseHandle((HANDLE)mhThread);
#else
		pthread_detach(*(pthread_t*)mhThread);
		delete (pthread_t*)mhThread;
#endif
		mhThread = NULL;
		mThreadID = 0;
	}
}

void VDThread::ThreadWait() {
	if (isThreadAttached()) {
#ifndef _LINUX_PORT
		WaitForSingleObject((HANDLE)mhThread, INFINITE);
#else
		pthread_join(*(pthread_t*)mhThread, NULL);
#endif
		ThreadDetach();
		mThreadID = 0;
	}
}

void VDThread::ThreadSetPriority(int priority) {
	if (mThreadPriority != priority) {
		mThreadPriority = priority;

#ifndef _LINUX_PORT
		if (mhThread && priority != INT_MIN)
			::SetThreadPriority(mhThread, priority);
#endif
	}
}

bool VDThread::isThreadActive() {
	if (isThreadAttached()) {
#ifndef _LINUX_PORT
		if (WAIT_TIMEOUT == WaitForSingleObject((HANDLE)mhThread, 0))
			return true;
#endif

		ThreadDetach();
		mThreadID = 0;
	}
	return false;
}

void VDThread::ThreadFinish() {
#ifndef _LINUX_PORT
	_endthreadex(0);
#endif
}

void *VDThread::ThreadLocation() const {
	if (!isThreadAttached())
		return NULL;

#ifndef _LINUX_PORT
	CONTEXT ctx;

	ctx.ContextFlags = CONTEXT_CONTROL;

	SuspendThread(mhThread);
	GetThreadContext(mhThread, &ctx);
	ResumeThread(mhThread);

#if defined(VD_CPU_AMD64)
	return (void *)ctx.Rip;
#elif defined(VD_CPU_X86)
	return (void *)ctx.Eip;
#elif defined(VD_CPU_ARM)
	return (void *)ctx.Pc;
#endif
#else
	return NULL;
#endif
}

///////////////////////////////////////////////////////////////////////////

unsigned __stdcall VDThread::StaticThreadStart(void *pThisAsVoid) {
	VDThread *pThis = static_cast<VDThread *>(pThisAsVoid);

	// We cannot use mThreadID here because it might already have been
	// invalidated by a detach in the main thread.
	if (pThis->mpszDebugName)
		VDSetThreadDebugName(VDGetCurrentThreadID(), pThis->mpszDebugName);

	VDInitThreadData(pThis->mpszDebugName);

	vdprotected1("running thread \"%.64s\"", const char *, pThis->mpszDebugName) {
		pThis->ThreadRun();
	}

	// NOTE: Do not put anything referencing this here, since our object
	//       may have been destroyed by the threaded code.

	VDDeinitThreadData();

	return 0;
}

///////////////////////////////////////////////////////////////////////////

void VDCriticalSection::StructCheck() {
#ifndef _LINUX_PORT
	VDASSERTCT(sizeof(CritSec) == sizeof(CRITICAL_SECTION));
#endif
}

///////////////////////////////////////////////////////////////////////////

VDSignal::VDSignal() {
	#ifndef _LINUX_PORT
	hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
#endif
}

VDSignalPersistent::VDSignalPersistent() {
	#ifndef _LINUX_PORT
	hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
#endif
}

VDSignalBase::~VDSignalBase() {
	#ifndef _LINUX_PORT
	CloseHandle(hEvent);
#endif
}

void VDSignalBase::signal() {
	#ifndef _LINUX_PORT
	SetEvent(hEvent);
#endif
}

void VDSignalBase::wait() {
	#ifndef _LINUX_PORT
	WaitForSingleObject(hEvent, INFINITE);
#endif
}

bool VDSignalBase::check() {
	#ifndef _LINUX_PORT
	return WAIT_OBJECT_0 == WaitForSingleObject(hEvent, 0);
#else
	return false;
#endif
}

int VDSignalBase::wait(VDSignalBase *second) {
#ifndef _LINUX_PORT
	HANDLE		hArray[16];
	DWORD		dwRet;

	hArray[0] = hEvent;
	hArray[1] = second->hEvent;

	dwRet = WaitForMultipleObjects(2, hArray, FALSE, INFINITE);

	return dwRet == WAIT_FAILED ? -1 : dwRet - WAIT_OBJECT_0;
#else
	return -1;
#endif
}

int VDSignalBase::wait(VDSignalBase *second, VDSignalBase *third) {
#ifndef _LINUX_PORT
	HANDLE		hArray[3];
	DWORD		dwRet;

	hArray[0] = hEvent;
	hArray[1] = second->hEvent;
	hArray[2] = third->hEvent;

	dwRet = WaitForMultipleObjects(3, hArray, FALSE, INFINITE);

	return dwRet == WAIT_FAILED ? -1 : dwRet - WAIT_OBJECT_0;
#else
	return -1;
#endif
}

int VDSignalBase::waitMultiple(const VDSignalBase **signals, int count) {
	VDASSERT(count <= 16);

#ifndef _LINUX_PORT
	HANDLE handles[16];
	int active = 0;

	for(int i=0; i<count; ++i) {
		HANDLE h = signals[i]->hEvent;

		if (h)
			handles[active++] = h;
	}

	if (!active)
		return -1;

	DWORD dwRet = WaitForMultipleObjects(active, handles, FALSE, INFINITE);

	return dwRet == WAIT_FAILED ? -1 : dwRet - WAIT_OBJECT_0;
#else
	return -1;
#endif
}

bool VDSignalBase::tryWait(uint32 timeoutMillisec) {
	#ifndef _LINUX_PORT
	return WAIT_OBJECT_0 == WaitForSingleObject(hEvent, timeoutMillisec);
#else
	return false;
#endif
}

void VDSignalPersistent::unsignal() {
	#ifndef _LINUX_PORT
	ResetEvent(hEvent);
#endif
}

VDSemaphore::VDSemaphore(int initial)
#ifndef _LINUX_PORT
	: mKernelSema(CreateSemaphore(NULL, initial, 0x0fffffff, NULL))
#else
	: mKernelSema(NULL)
#endif
{
}

VDSemaphore::~VDSemaphore() {
#ifndef _LINUX_PORT
	if (mKernelSema) CloseHandle(mKernelSema);
#endif
}

void VDSemaphore::Reset(int count) {
#ifndef _LINUX_PORT
	// reset semaphore to zero
	while(WAIT_OBJECT_0 == WaitForSingleObject(mKernelSema, 0))
		;

	if (count)
		ReleaseSemaphore(mKernelSema, count, NULL);
#endif
}
