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
		02		15jan25	move thread launch before command line processing

*/

// FauveEdit.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "afxwinappex.h"
#include "afxdialogex.h"
#include "FauveEdit.h"
#include "MainFrm.h"
#include "AboutDlg.h"

#include "ChildFrm.h"
#include "FauveDoc.h"
#include "FauveView.h"

#include "Win32Console.h"
#include "Gdiplus.h"
#include "LoopTiler.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CFauveEditApp

BEGIN_MESSAGE_MAP(CFauveEditApp, CWinAppCK)
	ON_COMMAND(ID_APP_ABOUT, &CFauveEditApp::OnAppAbout)
	// Standard file based document commands
	ON_COMMAND(ID_FILE_NEW, &CWinAppEx::OnFileNew)
	ON_COMMAND(ID_FILE_OPEN, &CWinAppEx::OnFileOpen)
	// Standard print setup command
	ON_COMMAND(ID_FILE_PRINT_SETUP, &CWinAppEx::OnFilePrintSetup)
END_MESSAGE_MAP()


// CFauveEditApp construction

CFauveEditApp::CFauveEditApp()
{
	// prevent ReloadWindowPlacement from showing main window; it's shown at end of InitInstance
	m_bDeferShowOnFirstWindowPlacementLoad = true;	// eliminates startup flicker
	m_bHiColorIcons = TRUE;

	// TODO: replace application ID string below with unique ID string; recommended
	// format for string is CompanyName.ProductName.SubProduct.VersionInformation
	SetAppID(_T("AnalSoftware.FauveEdit.Beta.1.0"));

	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
	m_bIsOpeningDoc = false;
	m_bDragPreview = true;
}

// The one and only CFauveEditApp object

CFauveEditApp theApp;


// CFauveEditApp initialization

BOOL CFauveEditApp::InitInstance()
{
#ifdef _DEBUG
	Win32Console::Create();
#endif

	// InitCommonControlsEx() is required on Windows XP if an application
	// manifest specifies use of ComCtl32.dll version 6 or later to enable
	// visual styles.  Otherwise, any window creation will fail.
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	// Set this to include all the common control classes you want to use
	// in your application.
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);

	CWinAppCK::InitInstance();


	// Initialize OLE libraries
	if (!AfxOleInit())
	{
		AfxMessageBox(IDP_OLE_INIT_FAILED);
		return FALSE;
	}

	AfxEnableControlContainer();

	EnableTaskbarInteraction();

	// AfxInitRichEdit2() is required to use RichEdit control	
	// AfxInitRichEdit2();

	// Standard initialization
	// If you are not using these features and wish to reduce the size
	// of your final executable, you should remove from the following
	// the specific initialization routines you do not need
	// Change the registry key under which our settings are stored
	// TODO: You should modify this string to be something appropriate
	// such as the name of your company or organization
	SetRegistryKey(_T("Anal Software"));
	LoadStdProfileSettings(4);  // Load standard INI file options (including MRU)

	InitContextMenuManager();

	InitKeyboardManager();

	InitTooltipManager();
	CMFCToolTipInfo ttParams;
	ttParams.m_bVislManagerTheme = TRUE;
	theApp.GetTooltipManager()->SetTooltipParams(AFX_TOOLTIP_TYPE_ALL,
		RUNTIME_CLASS(CMFCToolTipCtrl), &ttParams);

	// load options and optionally launch worker threads BEFORE parsing command line;
	// command line can open an existing document, which may utilize worker threads
	m_options.Load();
	if (m_options.UseThreads()) {
		if (!m_LoopTiler.Create(m_options.GetThreadCount())) {	// create loop tiler
			AfxMessageBox(IDS_APP_ERR_CANT_CREATE_THREADS);
		}
	}

	// Register the application's document templates.  Document templates
	//  serve as the connection between documents, frame windows and views
	CMultiDocTemplate* pDocTemplate;
	pDocTemplate = new CMultiDocTemplate(IDR_FauveEditTYPE,
		RUNTIME_CLASS(CFauveDoc),
		RUNTIME_CLASS(CChildFrame), // custom MDI child frame
		RUNTIME_CLASS(CFauveView));
	if (!pDocTemplate)
		return FALSE;
	AddDocTemplate(pDocTemplate);

	// create main MDI Frame window
	CMainFrame* pMainFrame = new CMainFrame;
	if (!pMainFrame || !pMainFrame->LoadFrame(IDR_MAINFRAME))
	{
		// The stock code deletes pMainFrame here, but doing so crashes the app
		// because the instance was already deleted by CFrameWnd::PostNcDestroy()
		return FALSE;
	}
	m_pMainWnd = pMainFrame;


	// Parse command line for standard shell commands, DDE, file open
	CCommandLineInfo cmdInfo;
	ParseCommandLine(cmdInfo);
	if (cmdInfo.m_nShellCommand == CCommandLineInfo::FileNew)
		cmdInfo.m_nShellCommand = CCommandLineInfo::FileNothing;
	
	// Dispatch commands specified on the command line.  Will return FALSE if
	// app was launched with /RegServer, /Register, /Unregserver or /Unregister.
	if (!ProcessShellCommand(cmdInfo))
		return FALSE;

	// The stock code shows and updates the main window here, but this makes the
	// view flicker due to being painted twice; it's solved by moving show/update
	// to CMainFrame::OnDelayedCreate which runs after the window sizes stabilize

	return TRUE;
}

int CFauveEditApp::ExitInstance()
{
	m_options.Store();

	AfxOleTerm(FALSE);

	return CWinAppCK::ExitInstance();
}

CDocument* CFauveEditApp::OpenDocumentFile(LPCTSTR lpszFileName)
{
	m_bIsOpeningDoc = true;
	CDocument	*pDoc = CWinAppCK::OpenDocumentFile(lpszFileName);
	m_bIsOpeningDoc = false;
	return pDoc;
}

CDocument* CFauveEditApp::OpenDocumentFile(LPCTSTR lpszFileName, BOOL bAddToMRU)
{
	m_bIsOpeningDoc = true;
	CDocument	*pDoc = CWinAppCK::OpenDocumentFile(lpszFileName, bAddToMRU);
	m_bIsOpeningDoc = false;
	return pDoc;
}

// CFauveEditApp message handlers

// App command to run the dialog
void CFauveEditApp::OnAppAbout()
{
	CAboutDlg aboutDlg;
	aboutDlg.DoModal();
}

// CFauveEditApp customization load/save methods

void CFauveEditApp::PreLoadState()
{
	BOOL bNameValid;
	CString strName;
	bNameValid = strName.LoadString(IDS_EDIT_MENU);
	ASSERT(bNameValid);
	GetContextMenuManager()->AddMenu(strName, IDR_POPUP_EDIT);
}

void CFauveEditApp::LoadCustomState()
{
}

void CFauveEditApp::SaveCustomState()
{
}

void CFauveEditApp::CFauveLoopTiler::OnThreadRun(int iThread)
{
	ASSERT(m_pTargetDoc != NULL);	// else logic error
	m_pTargetDoc->FauveCallback(iThread);
}

// CFauveEditApp message handlers

