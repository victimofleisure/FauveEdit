// Copyleft 2020 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00		29apr20	initial version
		01		15jan25	add update options methods

*/

// MainFrm.cpp : implementation of the CMainFrame class
//

#include "stdafx.h"
#include "FauveEdit.h"

#include "MainFrm.h"
#include "FauveDoc.h"
#include "OptionsDlg.h"
#include "DocIter.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CMainFrame

IMPLEMENT_DYNAMIC(CMainFrame, CMDIFrameWndEx)

const int  iMaxUserToolbars = 10;
const UINT uiFirstUserToolBarId = ID_APP_DOCKING_BAR_LAST + 1;
const UINT uiLastUserToolBarId = uiFirstUserToolBarId + iMaxUserToolbars - 1;

BEGIN_MESSAGE_MAP(CMainFrame, CMDIFrameWndEx)
	ON_WM_CREATE()
	ON_COMMAND(ID_WINDOW_MANAGER, &CMainFrame::OnWindowManager)
	ON_COMMAND(ID_VIEW_CUSTOMIZE, &CMainFrame::OnViewCustomize)
	ON_REGISTERED_MESSAGE(AFX_WM_CREATETOOLBAR, &CMainFrame::OnToolbarCreateNew)
	ON_COMMAND_RANGE(ID_VIEW_APPLOOK_WIN_2000, ID_VIEW_APPLOOK_WINDOWS_7, &CMainFrame::OnApplicationLook)
	ON_UPDATE_COMMAND_UI_RANGE(ID_VIEW_APPLOOK_WIN_2000, ID_VIEW_APPLOOK_WINDOWS_7, &CMainFrame::OnUpdateApplicationLook)
	ON_MESSAGE(UWM_DELAYED_CREATE, OnDelayedCreate)
	ON_COMMAND(ID_TOOLS_OPTIONS, OnToolsOptions)
	ON_COMMAND(ID_WINDOW_FULL_SCREEN, OnWindowFullScreen)
END_MESSAGE_MAP()

static UINT indicators[] =
{
	ID_SEPARATOR,           // status line indicator
	ID_INDICATOR_CAPS,
	ID_INDICATOR_NUM,
	ID_INDICATOR_SCRL,
};

// CMainFrame construction/destruction

CMainFrame::CMainFrame()
{
	theApp.m_nAppLook = theApp.GetInt(_T("ApplicationLook"), ID_VIEW_APPLOOK_VS_2008);
	m_pActiveDoc = NULL;
	m_bIsFullScreenMode = false;
	m_pPreFullScreenViewParent = NULL;
}

CMainFrame::~CMainFrame()
{
}

int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CMDIFrameWndEx::OnCreate(lpCreateStruct) == -1)
		return -1;

	BOOL bNameValid;

	CMDITabInfo mdiTabParams;
	mdiTabParams.m_style = CMFCTabCtrl::STYLE_3D_ONENOTE; // other styles available...
	mdiTabParams.m_bActiveTabCloseButton = TRUE;      // set to FALSE to place close button at right of tab area
	mdiTabParams.m_bTabIcons = FALSE;    // set to TRUE to enable document icons on MDI taba
	mdiTabParams.m_bAutoColor = TRUE;    // set to FALSE to disable auto-coloring of MDI tabs
	mdiTabParams.m_bDocumentMenu = TRUE; // enable the document menu at the right edge of the tab area
	EnableMDITabbedGroups(TRUE, mdiTabParams);

	if (!m_wndMenuBar.Create(this))
	{
		TRACE0("Failed to create menubar\n");
		return -1;      // fail to create
	}

	m_wndMenuBar.SetPaneStyle(m_wndMenuBar.GetPaneStyle() | CBRS_SIZE_DYNAMIC | CBRS_TOOLTIPS | CBRS_FLYBY);

	// prevent the menu bar from taking the focus on activation
	CMFCPopupMenu::SetForceMenuFocus(FALSE);

	if (!m_wndToolBar.CreateEx(this, TBSTYLE_FLAT, WS_CHILD | WS_VISIBLE | CBRS_TOP | CBRS_GRIPPER | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC) ||
		!m_wndToolBar.LoadToolBar(theApp.m_bHiColorIcons ? IDR_MAINFRAME_256 : IDR_MAINFRAME))
	{
		TRACE0("Failed to create toolbar\n");
		return -1;      // fail to create
	}

	CString strToolBarName;
	bNameValid = strToolBarName.LoadString(IDS_TOOLBAR_STANDARD);
	ASSERT(bNameValid);
	m_wndToolBar.SetWindowText(strToolBarName);

	CString strCustomize;
	bNameValid = strCustomize.LoadString(IDS_TOOLBAR_CUSTOMIZE);
	ASSERT(bNameValid);
	m_wndToolBar.EnableCustomizeButton(TRUE, ID_VIEW_CUSTOMIZE, strCustomize);

	// Allow user-defined toolbars operations:
	InitUserToolbars(NULL, uiFirstUserToolBarId, uiLastUserToolBarId);

	if (!m_wndStatusBar.Create(this))
	{
		TRACE0("Failed to create status bar\n");
		return -1;      // fail to create
	}
	m_wndStatusBar.SetIndicators(indicators, sizeof(indicators)/sizeof(UINT));

	// TODO: Delete these five lines if you don't want the toolbar and menubar to be dockable
	m_wndMenuBar.EnableDocking(CBRS_ALIGN_ANY);
	m_wndToolBar.EnableDocking(CBRS_ALIGN_ANY);
	EnableDocking(CBRS_ALIGN_ANY);
	DockPane(&m_wndMenuBar);
	DockPane(&m_wndToolBar);


	// enable Visual Studio 2005 style docking window behavior
	CDockingManager::SetDockingMode(DT_SMART);
	// enable Visual Studio 2005 style docking window auto-hide behavior
	EnableAutoHidePanes(CBRS_ALIGN_ANY);

	// create docking windows
	if (!CreateDockingWindows())
	{
		TRACE0("Failed to create docking windows\n");
		return -1;
	}

	// set the visual manager and style based on persisted value
	OnApplicationLook(theApp.m_nAppLook);

	// Enable enhanced windows management dialog
	EnableWindowsDialog(ID_WINDOW_MANAGER, ID_WINDOW_MANAGER, TRUE);

	// Enable toolbar and docking window menu replacement
	EnablePaneMenu(TRUE, ID_VIEW_CUSTOMIZE, strCustomize, ID_VIEW_TOOLBAR);

	// enable quick (Alt+drag) toolbar customization
	CMFCToolBar::EnableQuickCustomization();

	if (CMFCToolBar::GetUserImages() == NULL)
	{
		// load user-defined toolbar images
		if (m_UserImages.Load(_T(".\\UserImages.bmp")))
		{
			CMFCToolBar::SetUserImages(&m_UserImages);
		}
	}

#if 0	// disable menu personalization
	// enable menu personalization (most-recently used commands)
	// TODO: define your own basic commands, ensuring that each pulldown menu has at least one basic command.
	CList<UINT, UINT> lstBasicCommands;

	lstBasicCommands.AddTail(ID_FILE_NEW);
	lstBasicCommands.AddTail(ID_FILE_OPEN);
	lstBasicCommands.AddTail(ID_FILE_SAVE);
	lstBasicCommands.AddTail(ID_FILE_PRINT);
	lstBasicCommands.AddTail(ID_APP_EXIT);
	lstBasicCommands.AddTail(ID_EDIT_CUT);
	lstBasicCommands.AddTail(ID_EDIT_PASTE);
	lstBasicCommands.AddTail(ID_EDIT_UNDO);
	lstBasicCommands.AddTail(ID_APP_ABOUT);
	lstBasicCommands.AddTail(ID_VIEW_STATUS_BAR);
	lstBasicCommands.AddTail(ID_VIEW_TOOLBAR);
	lstBasicCommands.AddTail(ID_VIEW_APPLOOK_OFF_2003);
	lstBasicCommands.AddTail(ID_VIEW_APPLOOK_VS_2005);
	lstBasicCommands.AddTail(ID_VIEW_APPLOOK_OFF_2007_BLUE);
	lstBasicCommands.AddTail(ID_VIEW_APPLOOK_OFF_2007_SILVER);
	lstBasicCommands.AddTail(ID_VIEW_APPLOOK_OFF_2007_BLACK);
	lstBasicCommands.AddTail(ID_VIEW_APPLOOK_OFF_2007_AQUA);
	lstBasicCommands.AddTail(ID_VIEW_APPLOOK_WINDOWS_7);

	CMFCToolBar::SetBasicCommands(lstBasicCommands);
#endif

	// Switch the order of document name and application name on the window title bar. This
	// improves the usability of the taskbar because the document name is visible with the thumbnail.
	ModifyStyle(0, FWS_PREFIXTITLE);

	DragAcceptFiles();

	PostMessage(UWM_DELAYED_CREATE);

	return 0;
}

BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	if( !CMDIFrameWndEx::PreCreateWindow(cs) )
		return FALSE;
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return TRUE;
}

BOOL CMainFrame::CreateDockingWindows()
{
	CString sTitle;
	DWORD	dwBaseStyle = WS_CHILD | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | CBRS_FLOAT_MULTI;
	// hue bar must be excluded because it's derived from CPaneDialog which has a different constructor
	#define MAINDOCKBARDEF_EXCLUDE_HUE
	#define MAINDOCKBARDEF(name, width, height, style) \
		sTitle.LoadString(IDS_BAR_##name); \
		if (!m_wnd##name##Bar.Create(sTitle, this, CRect(0, 0, width, height), TRUE, ID_BAR_##name, style)) {	\
			TRACE("Failed to create %s bar\n", #name);	\
			return FALSE; \
		} \
		m_wnd##name##Bar.EnableDocking(CBRS_ALIGN_ANY); \
		if (ID_BAR_##name == ID_BAR_Levels) \
			m_wndLevelsBar.DockToWindow(&m_wndCropBar, CBRS_ALIGN_BOTTOM); \
		else \
			DockPane(&m_wnd##name##Bar);
	#include "MainDockBarDef.h"	// generate code to create docking windows
	// create hue bar
	sTitle.LoadString(IDS_BAR_Hue);
	if (!m_wndHueBar.Create(sTitle, this, TRUE, IDD_HUE_PANE, dwBaseStyle | CBRS_RIGHT | WS_VISIBLE, ID_BAR_Hue)) {
		TRACE("Failed to create %s bar\n", sTitle.GetString());
		return FALSE;
	}
	m_wndHueBar.EnableDocking(CBRS_ALIGN_ANY);
	DockPane(&m_wndHueBar);
	SetDockingWindowIcons(theApp.m_bHiColorIcons);
	return TRUE;
}

void CMainFrame::SetDockingWindowIcons(BOOL bHiColorIcons)
{
	UNREFERENCED_PARAMETER(bHiColorIcons);
	UpdateMDITabbedBarsIcons();
}

// CMainFrame diagnostics

#ifdef _DEBUG
void CMainFrame::AssertValid() const
{
	CMDIFrameWndEx::AssertValid();
}

void CMainFrame::Dump(CDumpContext& dc) const
{
	CMDIFrameWndEx::Dump(dc);
}
#endif //_DEBUG


// CMainFrame message handlers

void CMainFrame::OnWindowManager()
{
	ShowWindowsDialog();
}

void CMainFrame::OnViewCustomize()
{
	CMFCToolBarsCustomizeDialog* pDlgCust = new CMFCToolBarsCustomizeDialog(this, TRUE /* scan menus */);
	pDlgCust->EnableUserDefinedToolbars();
	pDlgCust->Create();
}

LRESULT CMainFrame::OnToolbarCreateNew(WPARAM wp,LPARAM lp)
{
	LRESULT lres = CMDIFrameWndEx::OnToolbarCreateNew(wp,lp);
	if (lres == 0)
	{
		return 0;
	}

	CMFCToolBar* pUserToolbar = (CMFCToolBar*)lres;
	ASSERT_VALID(pUserToolbar);

	BOOL bNameValid;
	CString strCustomize;
	bNameValid = strCustomize.LoadString(IDS_TOOLBAR_CUSTOMIZE);
	ASSERT(bNameValid);

	pUserToolbar->EnableCustomizeButton(TRUE, ID_VIEW_CUSTOMIZE, strCustomize);
	return lres;
}

void CMainFrame::OnApplicationLook(UINT id)
{
	CWaitCursor wait;

	theApp.m_nAppLook = id;

	switch (theApp.m_nAppLook)
	{
	case ID_VIEW_APPLOOK_WIN_2000:
		CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManager));
		break;

	case ID_VIEW_APPLOOK_OFF_XP:
		CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerOfficeXP));
		break;

	case ID_VIEW_APPLOOK_WIN_XP:
		CMFCVisualManagerWindows::m_b3DTabsXPTheme = TRUE;
		CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerWindows));
		break;

	case ID_VIEW_APPLOOK_OFF_2003:
		CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerOffice2003));
		CDockingManager::SetDockingMode(DT_SMART);
		break;

	case ID_VIEW_APPLOOK_VS_2005:
		CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerVS2005));
		CDockingManager::SetDockingMode(DT_SMART);
		break;

	case ID_VIEW_APPLOOK_VS_2008:
		CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerVS2008));
		CDockingManager::SetDockingMode(DT_SMART);
		break;

	case ID_VIEW_APPLOOK_WINDOWS_7:
		CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerWindows7));
		CDockingManager::SetDockingMode(DT_SMART);
		break;

	default:
		switch (theApp.m_nAppLook)
		{
		case ID_VIEW_APPLOOK_OFF_2007_BLUE:
			CMFCVisualManagerOffice2007::SetStyle(CMFCVisualManagerOffice2007::Office2007_LunaBlue);
			break;

		case ID_VIEW_APPLOOK_OFF_2007_BLACK:
			CMFCVisualManagerOffice2007::SetStyle(CMFCVisualManagerOffice2007::Office2007_ObsidianBlack);
			break;

		case ID_VIEW_APPLOOK_OFF_2007_SILVER:
			CMFCVisualManagerOffice2007::SetStyle(CMFCVisualManagerOffice2007::Office2007_Silver);
			break;

		case ID_VIEW_APPLOOK_OFF_2007_AQUA:
			CMFCVisualManagerOffice2007::SetStyle(CMFCVisualManagerOffice2007::Office2007_Aqua);
			break;
		}

		CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerOffice2007));
		CDockingManager::SetDockingMode(DT_SMART);
	}

	RedrawWindow(NULL, NULL, RDW_ALLCHILDREN | RDW_INVALIDATE | RDW_UPDATENOW | RDW_FRAME | RDW_ERASE);

	theApp.WriteInt(_T("ApplicationLook"), theApp.m_nAppLook);
}

void CMainFrame::OnUpdateApplicationLook(CCmdUI* pCmdUI)
{
	pCmdUI->SetRadio(theApp.m_nAppLook == pCmdUI->m_nID);
}


BOOL CMainFrame::LoadFrame(UINT nIDResource, DWORD dwDefaultStyle, CWnd* pParentWnd, CCreateContext* pContext) 
{
	// base class does the real work

	if (!CMDIFrameWndEx::LoadFrame(nIDResource, dwDefaultStyle, pParentWnd, pContext))
	{
		return FALSE;
	}


	// enable customization button for all user toolbars
	BOOL bNameValid;
	CString strCustomize;
	bNameValid = strCustomize.LoadString(IDS_TOOLBAR_CUSTOMIZE);
	ASSERT(bNameValid);

	for (int i = 0; i < iMaxUserToolbars; i ++)
	{
		CMFCToolBar* pUserToolbar = GetUserToolBarByIndex(i);
		if (pUserToolbar != NULL)
		{
			pUserToolbar->EnableCustomizeButton(TRUE, ID_VIEW_CUSTOMIZE, strCustomize);
		}
	}

	if (1) {	// if resetting UI to its default state
#if _MFC_VER < 0xb00
		m_wndMenuBar.RestoreOriginalstate();
		m_wndToolBar.RestoreOriginalstate();
#else	// MS fixed typo
		m_wndMenuBar.RestoreOriginalState();
		m_wndToolBar.RestoreOriginalState();
#endif
		theApp.GetKeyboardManager()->ResetAll();
		theApp.GetContextMenuManager()->ResetState();
	}

	return TRUE;
}

void CMainFrame::OnActivateView(CView *pView)
{
	// dynamic cast because other view types are possible, e.g. print preview
	CFauveDoc	*pDoc;
	if (pView != NULL)
		pDoc = DYNAMIC_DOWNCAST(CFauveDoc, pView->GetDocument());
	else
		pDoc = NULL;
	if (pDoc != m_pActiveDoc) {	// if active document changed
		m_pActiveDoc = pDoc;
		OnUpdate(NULL);
	}
}

LRESULT	CMainFrame::OnDelayedCreate(WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(wParam);
	UNREFERENCED_PARAMETER(lParam);
	// The main window has been initialized, so show and update it
	ShowWindow(theApp.m_nCmdShow);
	UpdateWindow();
	return(0);
}

void CMainFrame::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint)
{
//	printf("CMainFrame::OnUpdate pSender=%Ix lHint=%Id pHint=%Ix\n", pSender, lHint, pHint);
	if (pSender != STATIC_DOWNCAST(CWnd, &m_wndCropBar)) {
		m_wndCropBar.OnUpdate(pSender, lHint, pHint);
	}	
	if (pSender != STATIC_DOWNCAST(CWnd, &m_wndLevelsBar)) {
		m_wndLevelsBar.OnUpdate(pSender, lHint, pHint);
	}
	if (pSender != STATIC_DOWNCAST(CWnd, &m_wndHueBar)) {
		m_wndHueBar.OnUpdate(pSender, lHint, pHint);
	}
}

void CMainFrame::UpdateOptions(const COptions& optsPrev)
{
	CAllDocIter	iter;	// iterate all documents
	CFauveDoc	*pDoc;
	while ((pDoc = STATIC_DOWNCAST(CFauveDoc, iter.GetNextDoc())) != NULL) {
		pDoc->ApplyOptions(&optsPrev);
	}
	// if thread count or multithreading type changed
	if (theApp.m_options.m_nThreadCount != optsPrev.m_nThreadCount
	|| theApp.m_options.m_nMultithreadingType != optsPrev.m_nMultithreadingType) {
		if (!theApp.m_LoopTiler.Destroy()) {
			AfxMessageBox(IDS_APP_ERR_CANT_DESTROY_THREADS);
			return;
		}
		if (theApp.m_options.UseThreads()) {
			if (!theApp.m_LoopTiler.Create(theApp.m_options.m_nThreadCount)) {
				AfxMessageBox(IDS_APP_ERR_CANT_CREATE_THREADS);
				return;
			}
		}
	}
}

void CMainFrame::OnToolsOptions()
{
	COptionsDlg	dlg;
	COptions	optsPrev = theApp.m_options;
	COptions&	optsDlg = dlg;
	optsDlg = theApp.m_options;
	if (dlg.DoModal() == IDOK) {
		theApp.m_options = dlg;
		UpdateOptions(optsPrev);
	}
}

bool CMainFrame::SetFullScreenMode(bool bEnable)
{
	if (bEnable == m_bIsFullScreenMode)	// if already in requested mode
		return true;	// nothing to do
	CFauveDoc	*pDoc = GetActiveMDIDoc();
	if (pDoc == NULL)
		return false;
	POSITION	nPos = pDoc->GetFirstViewPosition();
	CView	*pView = pDoc->GetNextView(nPos);
	if (pView == NULL)
		return false;
	if (bEnable) {	// if entering full screen
		m_pPreFullScreenViewParent = pView->GetParent();
		// get monitor size; this code cribbed from CFullScreenImpl
		CRect rectFrame, rcScreen;
		GetWindowRect(&rectFrame);
		MONITORINFO mi;
		mi.cbSize = sizeof(MONITORINFO);
		if (GetMonitorInfo(MonitorFromPoint(rectFrame.TopLeft(), MONITOR_DEFAULTTONEAREST), &mi)) {
			rcScreen = mi.rcMonitor;
		} else {
			::SystemParametersInfo(SPI_GETWORKAREA, 0, &rcScreen, 0);
		}
		pView->ModifyStyleEx(WS_EX_CLIENTEDGE, 0);	// remove view's client edge
		pView->SetParent(GetDesktopWindow());
		pView->SetWindowPos(&wndTopMost, rcScreen.left, rcScreen.top,
			rcScreen.Width(), rcScreen.Height(), SWP_SHOWWINDOW | SWP_FRAMECHANGED | SWP_NOCOPYBITS);
	} else {	// exiting full screen
		pView->SetParent(m_pPreFullScreenViewParent);
		pView->ModifyStyleEx(0, WS_EX_CLIENTEDGE);	// restore view's client edge
        RecalcLayout();
		CFrameWnd	*pViewFrame = pView->GetParentFrame();
		pViewFrame->RecalcLayout();	// recalc view's parent frame too
		m_pPreFullScreenViewParent = NULL;
	}
	m_bIsFullScreenMode = bEnable;
	return true;
}

void CMainFrame::OnWindowFullScreen()
{
	SetFullScreenMode(!m_bIsFullScreenMode);
}
