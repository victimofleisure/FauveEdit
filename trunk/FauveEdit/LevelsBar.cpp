// Copyleft 2020 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00		29apr20	initial version

*/

#include "stdafx.h"

#include "LevelsBar.h"
#include "Resource.h"
#include "MainFrm.h"
#include "FauveEdit.h"
#include "FauveDoc.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

const COLORREF CLevelsBar::m_clrMargin = RGB(192, 192, 192);
const COLORREF CLevelsBar::m_clrBkgnd = RGB(255, 255, 255);
const COLORREF CLevelsBar::m_clrLevel = RGB(0, 0, 0);

CLevelsBar::CLevelsBar()
{
	m_iDragMark = MARK_NONE;
	m_iDragOldLevel = 0;
	m_nDragPos = 0;
}

CLevelsBar::~CLevelsBar()
{
}

void CLevelsBar::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint)
{
	UNREFERENCED_PARAMETER(pSender);
	UNREFERENCED_PARAMETER(pHint);
	switch (lHint) {
	case CFauveDoc::HINT_NONE:
	case CFauveDoc::HINT_LEVELS:
		Invalidate();
		break;
	}
}

void CLevelsBar::GetMarkRect(int iMark, int iLevel, CRect& rMark) const
{
	CSize	szLevels(m_rLevels.Size());
	int	nLevels = CFauveDoc::HISTOGRAM_LEVELS;
	double	fWidth = double(szLevels.cx) / nLevels;
	int	x;
	if (!iMark)
		x = Round((iLevel * fWidth) + m_rLevels.left);
	else
		x = Round(((iLevel + 1) * fWidth) + m_rLevels.left - 1);
	rMark = CRect(CPoint(x - MARK_SIZE / 2, m_rLevels.bottom), CSize(MARK_SIZE + 1, MARK_SIZE + 1));
}

int CLevelsBar::HitTest(CPoint point, int& iNearestMark)
{
	iNearestMark = -1;
	CFauveDoc	*pDoc = theApp.GetMainFrame()->GetActiveMDIDoc();
	if (pDoc == NULL || pDoc->m_dibIn.IsEmpty())
		return -1;
	CRect	rMark[MARKS];
	for (int iMark = MARKS - 1; iMark >= 0; iMark--) {
		int	iLevel = pDoc->m_arrLuma[iMark];
		GetMarkRect(iMark, iLevel, rMark[iMark]);
		if (rMark[iMark].PtInRect(point))
			return iMark;
	}
	int	x1 = rMark[MARK_BLACK].CenterPoint().x;
	int	x2 = rMark[MARK_WHITE].CenterPoint().x;
	int	deltaX1 = abs(point.x - x1);
	int	deltaX2 = abs(point.x - x2);
	iNearestMark = deltaX1 > deltaX2;
	return -1;
}

int CLevelsBar::GetMarkValue(int iMark, int x) const
{
	CFauveDoc	*pDoc = theApp.GetMainFrame()->GetActiveMDIDoc();
	ASSERT(pDoc != NULL);
	CSize	szLevels(m_rLevels.Size());
	int	nLevels = CFauveDoc::HISTOGRAM_LEVELS;
	double	fWidth = double(szLevels.cx) / nLevels;
	int	iLevel;
	if (iMark == MARK_BLACK) {
		iLevel = Round((x - m_rLevels.left) / fWidth);
		iLevel = min(iLevel, int(pDoc->m_arrLuma[MARK_WHITE]) - 1);
	} else {
		iLevel = Round((x - m_rLevels.left + 1) / fWidth - 1);
		iLevel = max(iLevel, int(pDoc->m_arrLuma[MARK_BLACK]) + 1);
	}
	iLevel = CLAMP(iLevel, 0, BYTE_MAX);
	return iLevel;
}

void CLevelsBar::EraseMark(CDC& dc, int iMark, int iLevel)
{
	CRect	rMark;
	GetMarkRect(iMark, iLevel, rMark);
	dc.FillSolidRect(rMark, m_clrMargin);
}

void CLevelsBar::DrawMark(CDC& dc, int iMark, int iLevel)
{
	int	nObj;
	if (!iMark)
		nObj = BLACK_BRUSH;
	else
		nObj = WHITE_BRUSH;
	dc.SelectStockObject(nObj);
	CRect	rMark;
	GetMarkRect(iMark, iLevel, rMark);
	DrawMark(dc, CPoint(rMark.CenterPoint().x, rMark.top));
}

void CLevelsBar::DrawMark(CDC& dc, CPoint point)
{
	int	sz = MARK_SIZE;
	int	hsz = sz / 2;
	int	x = point.x;
	int	y = point.y;
	POINT	arrPt[3] = {{x, y}, {x + hsz, y + sz}, {x - hsz, y + sz}};
	dc.Polygon(arrPt, 3);
}

void CLevelsBar::SetLevelFromPos(int iMark, int x, bool bRedrawMark)
{
	CFauveDoc	*pDoc = theApp.GetMainFrame()->GetActiveMDIDoc();
	if (pDoc != NULL) {
		int	iNewLevel = GetMarkValue(iMark, x);
		if (bRedrawMark) {
			RedrawMark(iMark, pDoc->m_arrLuma[iMark], iNewLevel);
		}
		pDoc->SetLevel(iMark, static_cast<BYTE>(iNewLevel), reinterpret_cast<CView *>(this));
	}
}

void CLevelsBar::RedrawMark(int iMark, int iOldLevel, int iNewLevel)
{
	CFauveDoc	*pDoc = theApp.GetMainFrame()->GetActiveMDIDoc();
	ASSERT(pDoc != NULL);
	int	iOtherMark = !iMark;
	int	iOtherLevel = pDoc->m_arrLuma[iOtherMark];
	CRect	rMark[MARKS];
	for (int iMk = 0; iMk < MARKS; iMk++) {
		int	iLevel = iMk == iOtherMark ? iOtherLevel : iOldLevel;
		GetMarkRect(iMk, iLevel, rMark[iMk]);
		rMark[iMk].InflateRect(MARK_SIZE / 2, 0);	// be generous
	}
	CClientDC	dc(this);
	CRect	rTemp;
	if (rTemp.IntersectRect(rMark[MARK_BLACK], rMark[MARK_WHITE])) {	// if marks overlap
		// erase both marks
		for (int iMk = 0; iMk < MARKS; iMk++) {
			int	iLevel = iMk == iOtherMark ? iOtherLevel : iOldLevel;
			EraseMark(dc, iMk, iLevel);
		}
		// draw both marks
		for (int iMk = 0; iMk < MARKS; iMk++) {
			int	iLevel = iMk == iOtherMark ? iOtherLevel : iNewLevel;
			DrawMark(dc, iMk, iLevel);
		}
	} else {	// marks don't overlap
		EraseMark(dc, iMark, iOldLevel);
		DrawMark(dc, iMark, iNewLevel);
	}
}

BEGIN_MESSAGE_MAP(CLevelsBar, CDockablePane)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_PAINT()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_SETCURSOR()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_CAPTURECHANGED()
END_MESSAGE_MAP()

int CLevelsBar::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CDockablePane::OnCreate(lpCreateStruct) == -1)
		return -1;

	return 0;
}

void CLevelsBar::OnSize(UINT nType, int cx, int cy)
{
	CDockablePane::OnSize(nType, cx, cy);
	m_szClient = CSize(cx, cy);
	m_rLevels = CRect(CPoint(0, 0), m_szClient);
	m_rLevels.DeflateRect(LEVELS_MARGIN, LEVELS_MARGIN);
	m_rLevels.bottom -= LEVELS_MARGIN;
	Invalidate();
}

void CLevelsBar::OnPaint()
{
	CPaintDC dc(this); // device context for painting
	CRect	rc;
	GetClientRect(rc);
	CMainFrame	*pFrame = theApp.GetMainFrame();
	bool	bLevelsDrawn = false;
	if (pFrame != NULL) {
		CFauveDoc	*pDoc = pFrame->GetActiveMDIDoc();
		if (pDoc != NULL && !pDoc->m_dibIn.IsEmpty()) {
			CSize	szLevels(m_rLevels.Size());
			int	nLevels = CFauveDoc::HISTOGRAM_LEVELS;
			double	fWidth = double(szLevels.cx) / nLevels;
			int	x1 = m_rLevels.left;
			DWORD	nLumaMax = pDoc->m_histogram.m_nLumaMax;
			for (int iLevel = 0; iLevel < nLevels; iLevel++) {
				int	x2 = Round(fWidth * (iLevel + 1)) + m_rLevels.left;
				DWORD	nSamp = pDoc->m_histogram.m_arrLuma[iLevel];
				int	y = m_rLevels.bottom - Round(double(nSamp) / nLumaMax * szLevels.cy);
				dc.FillSolidRect(x1, m_rLevels.top, x2, y, m_clrBkgnd);
				dc.FillSolidRect(x1, y, x2, m_rLevels.bottom, m_clrLevel);
				x1 = x2;
			}
			dc.ExcludeClipRect(m_rLevels);
			dc.FillSolidRect(rc, m_clrMargin);
			bLevelsDrawn = true;
			if (m_iDragMark == MARK_BLACK) {
				DrawMark(dc, m_iDragMark, m_iDragOldLevel);
			} else {
				dc.SelectStockObject(BLACK_BRUSH);
				int	x = Round(fWidth * pDoc->m_arrLuma[MARK_BLACK] + m_rLevels.left);
				DrawMark(dc, CPoint(x, m_rLevels.bottom));
			}
			if (m_iDragMark == MARK_WHITE) {
				DrawMark(dc, m_iDragMark, m_iDragOldLevel);
			} else {
				dc.SelectStockObject(WHITE_BRUSH);
				int	x = Round(fWidth * (pDoc->m_arrLuma[MARK_WHITE] + 1) + m_rLevels.left - 1);
				DrawMark(dc, CPoint(x, m_rLevels.bottom));
			}
		}
	}
	if (!bLevelsDrawn)
		dc.FillSolidRect(rc, m_clrBkgnd);
}

void CLevelsBar::OnLButtonDown(UINT nFlags, CPoint point)
{
	CRect	rc;
	GetClientRect(rc);
	if (rc.PtInRect(point)) {
		SetFocus();
		int	iNearestMark;
		int	iHitMark = HitTest(point, iNearestMark);
		if (iHitMark < 0 && iNearestMark >= 0) {	// if missed mark but nearest is valid
			SetLevelFromPos(iNearestMark, point.x);	// redraw mark immediately
			iHitMark = iNearestMark;
		}
		if (iHitMark >= 0) {
			CFauveDoc	*pDoc = theApp.GetMainFrame()->GetActiveMDIDoc();
			ASSERT(pDoc != NULL);
			SetCapture();
			m_iDragMark = iHitMark;
			m_iDragOldLevel = pDoc->m_arrLuma[iHitMark];
			m_nDragPos = point.x;
		}
	} else {
		CDockablePane::OnLButtonDown(nFlags, point);
	}
}

void CLevelsBar::OnLButtonUp(UINT nFlags, CPoint point)
{
	if (m_iDragMark >= 0) {
		m_nDragPos = point.x;
		ReleaseCapture();
	} else {
		CDockablePane::OnLButtonUp(nFlags, point);
	}
}

void CLevelsBar::OnMouseMove(UINT nFlags, CPoint point)
{
	if (m_iDragMark >= 0) {
		int	iDragNewLevel = GetMarkValue(m_iDragMark, point.x);
		if (iDragNewLevel != m_iDragOldLevel) {
			RedrawMark(m_iDragMark, m_iDragOldLevel, iDragNewLevel);
			m_iDragOldLevel = iDragNewLevel;
			m_nDragPos = point.x;
			if (theApp.m_bDragPreview) {
				CFauveDoc	*pDoc = theApp.GetMainFrame()->GetActiveMDIDoc();
				if (pDoc != NULL) {
					pDoc->SetLevel(m_iDragMark, static_cast<BYTE>(iDragNewLevel), reinterpret_cast<CView *>(this));
				}
			}
		}
	} else {
		CDockablePane::OnMouseMove(nFlags, point);
	}
}

void CLevelsBar::OnLButtonDblClk(UINT nFlags, CPoint point)
{
	CRect	rc;
	GetClientRect(rc);
	if (rc.PtInRect(point)) {
	} else {
		CDockablePane::OnLButtonDblClk(nFlags, point);
	}
}

BOOL CLevelsBar::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{
	return CDockablePane::OnSetCursor(pWnd, nHitTest, message);
}

void CLevelsBar::OnCaptureChanged(CWnd *pWnd)
{
	if (m_iDragMark >= 0) {
		SetLevelFromPos(m_iDragMark, m_nDragPos, false);
		m_iDragMark = MARK_NONE;
	}
	CDockablePane::OnCaptureChanged(pWnd);
}
