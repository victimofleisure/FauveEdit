// Copyleft 2022 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00		26nov22	initial version
		01		18jan25	disable thread auto-delete to avoid race

*/

#include "stdafx.h"
#include "LoopTiler.h"

CLoopTiler::CLoopTiler()
{
	m_nBusyThreads = 0;
	m_bIsExiting = false;
	m_bIsFlipped = false;
}

CLoopTiler::~CLoopTiler()
{
	Destroy();
}

bool CLoopTiler::Create(int nThreads)
{
	ASSERT(nThreads >= 0 && nThreads <= MAXIMUM_WAIT_OBJECTS);
	// create events and initialize members
	if (!m_evtStart[0].Create(NULL, TRUE, FALSE, NULL))	// manual reset
		return false;
	if (!m_evtStart[1].Create(NULL, TRUE, FALSE, NULL))	// manual reset
		return false;
	if (!m_evtIdle.Create(NULL, FALSE, FALSE, NULL))	// auto reset
		return false;
	m_nBusyThreads = 0;
	m_bIsExiting = false;
	m_bIsFlipped = false;
	m_arrThreadPtr.SetSize(nThreads);
	m_arrThreadHandle.SetSize(nThreads);
	m_arrThreadParam.SetSize(nThreads);
	for (int iThread = 0; iThread < nThreads; iThread++) {	// for each thread
		// each thread gets its own parameter object; avoids race condition
		CThreadParam&	param = m_arrThreadParam[iThread];
		param.m_pThis = this;	// pointer to our instance
		param.m_iThread = iThread;	// index of this thread
		CWinThread	*pThread = AfxBeginThread(ThreadFunc, &param);	// launch thread
		if (pThread == NULL) {	// if thread launch failed
			m_arrThreadHandle.SetSize(iThread);	// truncate handle array
			return false;	// thread launch failure is a serious error
		}
		pThread->m_bAutoDelete = false;	// prevent thread from deleting its object
		m_arrThreadPtr[iThread] = pThread;	// store pointer to thread object
		m_arrThreadHandle[iThread] = pThread->m_hThread;	// store thread handle
	}
	return true;
}

bool CLoopTiler::Destroy()
{
	if (m_arrThreadHandle.GetSize()) {	// if threads were launched
		m_bIsExiting = true;	// set exit flag first; order matters
		m_evtStart[0].Set();	// set both start events so we can't lose
		m_evtStart[1].Set();
		// wait for all threads to exit
		DWORD	nRet = WaitForMultipleObjects(m_arrThreadHandle.GetSize(), 
			m_arrThreadHandle.GetData(), TRUE, INFINITE);
		if (nRet != WAIT_OBJECT_0) {	// if wait failed
			ASSERT(0);	// if this ever happens, our state is invalid
			return false;
		}
	}
	// clean up
	m_evtStart[0].Close();
	m_evtStart[1].Close();
	m_evtIdle.Close();
	int	nThreads = m_arrThreadHandle.GetSize();
	for (int iThread = 0; iThread < nThreads; iThread++) {
		CWinThread	*pThread = static_cast<CWinThread*>(m_arrThreadPtr[iThread]);
		delete pThread;	// delete thread object
	}
	m_arrThreadPtr.RemoveAll();
	m_arrThreadHandle.RemoveAll();
	m_arrThreadParam.RemoveAll();
	return true;
}

void CLoopTiler::Run()
{
	m_nBusyThreads = GetThreadCount();	// set number of busy threads
	m_evtStart[m_bIsFlipped].Set();	// set current start event
	DWORD	nRet = WaitForSingleObject(m_evtIdle, INFINITE);	// wait for idle signal
	if (nRet != WAIT_OBJECT_0) {	// if wait failed
		ASSERT(0);	// if this ever happens, our state is invalid
	}
	m_evtStart[m_bIsFlipped].Reset();	// reset current start event
	m_bIsFlipped ^= 1;	// toggle index to access other start event; see WorkerThread
}

UINT CLoopTiler::ThreadFunc(LPVOID pParam)
{
	CThreadParam	*pThreadParam = static_cast<CThreadParam*>(pParam);
	pThreadParam->m_pThis->WorkerThread(pThreadParam->m_iThread);
	return 0;
}

void CLoopTiler::WorkerThread(int iThread)
{
// Don't rely on the m_bIsFlipped member variable to determine which start
// event is current, because otherwise a race condition would occur between
// the worker thread signaling idle and waiting on one of the start events,
// due to Run toggling m_bIsFlipped asynchronously in the master thread.
//
	bool	bIsFlipped = false;	// keep track of current start event locally
	OnThreadBegin(iThread);	// do derived callback
	while (1) {
		WaitForSingleObject(m_evtStart[bIsFlipped], INFINITE);	// wait for start signal
		if (m_bIsExiting)	// if thread exit requested
			break;
		OnThreadRun(iThread);	// do derived callback; thread is busy
		UINT	nBusy = InterlockedDecrement(&m_nBusyThreads);	// decrement busy count
		if (!nBusy)	// if we're the last thread to finish working
			m_evtIdle.Set();	// set idle event, signaling that all threads are idle
		bIsFlipped ^= 1;	// toggle index to access other start event
	}
	OnThreadExit(iThread);	// do derived callback
}
