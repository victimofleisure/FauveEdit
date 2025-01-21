// Copyleft 2020 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00		29apr20	initial version
		01		10jan25	remove needless GDI+ init

*/

// FauveEdit.h : main header file for the FauveEdit application
//
#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"       // main symbols
#include "WinAppCK.h"
#include "LoopTiler.h"
#include "Options.h"

class CMainFrame;
class CFauveDoc;

// CFauveEditApp:
// See FauveEdit.cpp for the implementation of this class
//

class CFauveEditApp : public CWinAppCK
{
public:
	CFauveEditApp();

// Types
	class CFauveLoopTiler : public CLoopTiler {
	public:
		CFauveLoopTiler() { m_pTargetDoc = NULL; }
		virtual	void OnThreadRun(int iThread);
		CFauveDoc	*m_pTargetDoc;
	};

// Attributes
	CMainFrame	*GetMainFrame() const;

// Public data
	COptions	m_options;

// Overrides
public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();
	virtual CDocument* OpenDocumentFile(LPCTSTR lpszFileName);
	virtual CDocument* OpenDocumentFile(LPCTSTR lpszFileName, BOOL bAddToMRU);

// Implementation
	UINT  m_nAppLook;
	BOOL  m_bHiColorIcons;
	bool	m_bIsOpeningDoc;
	bool	m_bDragPreview;
	CFauveLoopTiler	m_LoopTiler;

	virtual void PreLoadState();
	virtual void LoadCustomState();
	virtual void SaveCustomState();

// protected:
	
	afx_msg void OnAppAbout();
	DECLARE_MESSAGE_MAP()
};

extern CFauveEditApp theApp;

inline CMainFrame* CFauveEditApp::GetMainFrame() const
{
	return reinterpret_cast<CMainFrame*>(m_pMainWnd);
}
