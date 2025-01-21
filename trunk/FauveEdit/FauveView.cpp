// Copyleft 2020 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00		29apr20	initial version
		01		17jan25	port to Direct2D

*/

// FauveView.cpp : implementation of the CFauveView class
//

#include "stdafx.h"
// SHARED_HANDLERS can be defined in an ATL project implementing preview, thumbnail
// and search filter handlers and allows sharing of document code with that project.
#ifndef SHARED_HANDLERS
#include "FauveEdit.h"
#endif

#include "FauveDoc.h"
#include "FauveView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CFauveView

IMPLEMENT_DYNCREATE(CFauveView, CView)

BEGIN_MESSAGE_MAP(CFauveView, CView)
	// Standard printing commands
	ON_COMMAND(ID_FILE_PRINT, &CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, &CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, &CFauveView::OnFilePrintPreview)
	ON_WM_CONTEXTMENU()
	ON_WM_RBUTTONUP()
	ON_WM_SIZE()
	ON_WM_CREATE()
	ON_WM_TIMER()
	ON_COMMAND(ID_EDIT_ANIMATE, OnEditAnimate)
	ON_UPDATE_COMMAND_UI(ID_EDIT_ANIMATE, OnUpdateEditAnimate)
	ON_MESSAGE(UWM_DELAYED_RESIZE, OnDelayedResize)
END_MESSAGE_MAP()

// CFauveView construction/destruction

CFauveView::CFauveView()
{
	m_szClient = CSize(0, 0);
	m_bIsAnimating = false;
	m_bIsResizePending = false;
	m_iFrame = 0;
}

CFauveView::~CFauveView()
{
}

BOOL CFauveView::PreCreateWindow(CREATESTRUCT& cs)
{
	// override default window class styles CS_HREDRAW and CS_VREDRAW
	// otherwise resizing frame redraws entire view, causing flicker
	cs.lpszClass = AfxRegisterWndClass(	// create our own window class
		CS_DBLCLKS,						// request double-clicks
		theApp.LoadStandardCursor(IDC_ARROW),	// standard cursor
		NULL,									// no background brush
		theApp.LoadIcon(IDR_MAINFRAME));		// app's icon
	return CView::PreCreateWindow(cs);
}

// CFauveView drawing

void CFauveView::OnDraw(CDC* pDC)
{
	UNREFERENCED_PARAMETER(pDC);
	if (!m_bIsResizePending) {
		m_dc.Draw();
	}
}

void CFauveView::OnInitialUpdate()
{
	CView::OnInitialUpdate();
}

void CFauveView::OnBackgroundColorChange()
{
	COLORREF	clr = theApp.m_options.m_clrBackground;
	// convert color from GDI to Direct2D channel order
	clr = MAKE_XRGB(GET_XRGB_B(clr), GET_XRGB_G(clr), GET_XRGB_R(clr));
	m_dc.m_clrBackground = D2D1::ColorF(clr);
}

// CFauveView printing


void CFauveView::OnFilePrintPreview()
{
#ifndef SHARED_HANDLERS
	AFXPrintPreview(this);
#endif
}

BOOL CFauveView::OnPreparePrinting(CPrintInfo* pInfo)
{
	// default preparation
	return DoPreparePrinting(pInfo);
}

void CFauveView::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add extra initialization before printing
}

void CFauveView::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add cleanup after printing
}

void CFauveView::OnRButtonUp(UINT /* nFlags */, CPoint point)
{
	ClientToScreen(&point);
	OnContextMenu(this, point);
}

void CFauveView::OnContextMenu(CWnd* /* pWnd */, CPoint point)
{
#ifndef SHARED_HANDLERS
	theApp.GetContextMenuManager()->ShowPopupMenu(IDR_POPUP_EDIT, point.x, point.y, this, TRUE);
#endif
}


// CFauveView diagnostics

#ifdef _DEBUG
void CFauveView::AssertValid() const
{
	CView::AssertValid();
}

void CFauveView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CFauveDoc* CFauveView::GetDocument() const // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CFauveDoc)));
	return (CFauveDoc*)m_pDocument;
}
#endif //_DEBUG


// CFauveView message handlers


int CFauveView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CView::OnCreate(lpCreateStruct) == -1)
		return -1;
	if (!m_dc.Create(m_hWnd, lpCreateStruct->cx, lpCreateStruct->cy))
		return -1;
	OnBackgroundColorChange();
	return 0;
}

void CFauveView::OnSize(UINT nType, int cx, int cy)
{
	CView::OnSize(nType, cx, cy);
	m_szClient = CSize(cx, cy);
	if (!m_bIsResizePending) {
		m_bIsResizePending = true;
		PostMessage(UWM_DELAYED_RESIZE);
	}
}

void CFauveView::Update()
{
	CFauveDoc	*pDoc = GetDocument();
	ASSERT(pDoc != NULL);
	if (!pDoc->m_dibIn.IsEmpty()) {
		CSize	szNew(pDoc->m_dibOut.GetSize());
		CSize	szOld(m_dc.GetSourceBitmapSize());
		if (szNew != szOld) {
			m_dc.CreateResources(szNew.cx, szNew.cy);
			m_dc.UpdateLetterbox();
		}
		UINT	nPitch = szNew.cx * sizeof(UINT);
		m_dc.m_pSourceBitmap->CopyFromMemory(nullptr, pDoc->m_dibOut.GetBits(), nPitch);
	}
}

void CFauveView::OnUpdate(CView* /*pSender*/, LPARAM lHint, CObject* pHint)
{
	switch (lHint) {
	case CFauveDoc::HINT_OPTIONS:
		{
			CFauveDoc::COptionsHint	*pOptionsHint = static_cast<CFauveDoc::COptionsHint *>(pHint);
			if (theApp.m_options.m_clrBackground != pOptionsHint->m_pPrevOptions->m_clrBackground) {
				OnBackgroundColorChange();
				RedrawWindow();
			}
		}
		break;
	default:
		Update();
		RedrawWindow();
	}
}

void CFauveView::OnTimer(UINT_PTR nIDEvent)
{
	if (nIDEvent == ANIMATION_TIMER) {
		CFauveDoc	*pDoc = GetDocument();
		double	fScale = m_iFrame / theApp.m_options.m_fAnimationFrameRate;
		for (int iChan = 0; iChan < 3; iChan++) {
			pDoc->m_arrHue[iChan] = m_arrPreAnimHue[iChan] + BYTE(Round(theApp.m_options.m_fAnimationHueRate[iChan] * fScale));
		}
		pDoc->UpdateFauve(true);	// reuse histogram
		pDoc->UpdateAllViews(NULL, CFauveDoc::HINT_HUE);
		m_iFrame++;
	} else {
		CView::OnTimer(nIDEvent);
	}
}

void CFauveView::OnEditAnimate()
{
	m_bIsAnimating ^= 1;
	if (m_bIsAnimating) {
		CFauveDoc	*pDoc = GetDocument();
		ASSERT(pDoc != NULL);
		pDoc->NotifyUndoableEdit(-1, UCODE_HUE);
		memcpy(m_arrPreAnimHue, pDoc->m_arrHue, sizeof(m_arrPreAnimHue));
		m_iFrame = 0;
		int	nPeriod = Round(1000.0 / theApp.m_options.m_fAnimationFrameRate);
		SetTimer(ANIMATION_TIMER, nPeriod, NULL);
	} else {
		KillTimer(ANIMATION_TIMER);
	}
}

void CFauveView::OnUpdateEditAnimate(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(m_bIsAnimating);
}

LRESULT	CFauveView::OnDelayedResize(WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(wParam);
	UNREFERENCED_PARAMETER(lParam);
	m_bIsResizePending = false;
	m_dc.Resize(m_szClient.cx, m_szClient.cy);
	Invalidate();
	return 0;
}
