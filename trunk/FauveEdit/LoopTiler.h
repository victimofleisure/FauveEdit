// Copyleft 2022 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00		26nov22	initial version
		01		18jan25	add array of pointers to thread objects

*/

#pragma once

#include "Event.h"

class CLoopTiler {
public:
// Construction
	CLoopTiler();
	virtual ~CLoopTiler();

// Accessors
	int		GetThreadCount() const;
	bool	IsExiting() const;
	bool	IsFlipped() const;

// Operations
	bool	Create(int nThreads);
	bool	Destroy();
	void	Run();

// Overrideables
	virtual	void OnThreadBegin(int iThread) { UNREFERENCED_PARAMETER(iThread); }
	virtual	void OnThreadRun(int iThread) { UNREFERENCED_PARAMETER(iThread); }
	virtual	void OnThreadExit(int iThread) { UNREFERENCED_PARAMETER(iThread); }

protected:
// Types
	class CThreadParam {
	public:
		CLoopTiler	*m_pThis;	// pointer to our instance
		int		m_iThread;		// index of this thread
	};

// Member data
	CPtrArrayEx	m_arrThreadPtr;	// array of pointers to CWinThread objects
	CArrayEx<HANDLE, HANDLE&> m_arrThreadHandle;	// array of thread handles
	CArrayEx<CThreadParam, CThreadParam&> m_arrThreadParam;	// array of thread parameter objects
	WEvent	m_evtStart[2];		// pair of alternating start events
	WEvent	m_evtIdle;			// signaled when all threads are idle
	UINT	m_nBusyThreads;		// number of threads that are busy
	bool	m_bIsExiting;		// true if threads should exit
	bool	m_bIsFlipped;		// index of current start event

// Helpers
	static	UINT	ThreadFunc(LPVOID pParam);
	void	WorkerThread(int iThread);
};

inline int CLoopTiler::GetThreadCount() const
{
	return m_arrThreadHandle.GetSize();
}

inline bool CLoopTiler::IsExiting() const
{
	return m_bIsExiting;
}

inline bool CLoopTiler::IsFlipped() const
{
	return m_bIsFlipped;
}
