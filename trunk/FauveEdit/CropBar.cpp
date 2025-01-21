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

#include "CropBar.h"
#include "Resource.h"
#include "MainFrm.h"
#include "FauveEdit.h"
#include "FauveDoc.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

CCropBar::CCropBar()
{
	m_rBox.SetRectEmpty();
	m_trk.m_nStyle = CRectTracker::dottedLine | CRectTracker::resizeOutside;
}

CCropBar::~CCropBar()
{
}

void CCropBar::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint)
{
	UNREFERENCED_PARAMETER(pSender);
	UNREFERENCED_PARAMETER(pHint);
	switch (lHint) {
	case CFauveDoc::HINT_NONE:
	case CFauveDoc::HINT_ROTATE:
		UpdateSize();
		break;
	case CFauveDoc::HINT_CROP:
		{
			CFauveDoc	*pDoc = theApp.GetMainFrame()->GetActiveMDIDoc();
			if (pDoc != NULL) {
				pDoc->GetScaledCropRect(m_rBox, m_trk.m_rect);
				Invalidate();
			}
		}
		break;
	}
}

void CCropBar::UpdateSize()
{
	CFauveDoc	*pDoc = theApp.GetMainFrame()->GetActiveMDIDoc();
	if (pDoc != NULL && !pDoc->m_dibIn.IsEmpty()) {
		CRect	rc;
		GetClientRect(rc);
		CSize	szClient = rc.Size();
		CSize	szIn(pDoc->m_dibIn.GetSize());
		CDib::Letterbox(rc, szIn, m_rBox);
		CSize	szOut(m_rBox.Size());
		m_bmBack.DeleteObject();
		CClientDC	dc(this);
		if (!m_bmBack.CreateCompatibleBitmap(&dc, szOut.cx, szOut.cy))
			AfxThrowInvalidArgException();
		m_dcMem.SelectObject(pDoc->m_dibIn);
		m_dcBack.SetStretchBltMode(HALFTONE);
		m_dcBack.SelectObject(m_bmBack);
		VERIFY(m_dcBack.StretchBlt(0, 0, szOut.cx, szOut.cy, &m_dcMem, 0, 0, szIn.cx, szIn.cy, SRCCOPY));
		pDoc->GetScaledCropRect(m_rBox, m_trk.m_rect);
	} else {
		m_rBox.SetRectEmpty();
		m_bmBack.DeleteObject();
	}
	Invalidate();
}

void CCropBar::UpdateCrop()
{
	m_trk.m_rect.NormalizeRect();
	if (m_trk.m_rect.IsRectEmpty()) {	// if zero width or height
		m_trk.m_rect.SetRectEmpty();	// set all null
	} else {	// valid rectangle
		m_trk.m_rect.IntersectRect(m_trk.m_rect, m_rBox);	// clip to letterbox
	}
	Invalidate();
	CFauveDoc	*pDoc = theApp.GetMainFrame()->GetActiveMDIDoc();
	if (pDoc != NULL) {
		pDoc->SetScaledCropRect(m_rBox, m_trk.m_rect, reinterpret_cast<CView *>(this));
	}
}

BEGIN_MESSAGE_MAP(CCropBar, CDockablePane)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_PAINT()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_SETCURSOR()
END_MESSAGE_MAP()

int CCropBar::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CDockablePane::OnCreate(lpCreateStruct) == -1)
		return -1;

	CClientDC	dc(this);
	if (!m_dcMem.CreateCompatibleDC(&dc))
		return -1;
	if (!m_dcBack.CreateCompatibleDC(&dc))
		return -1;

	return 0;
}

void CCropBar::OnSize(UINT nType, int cx, int cy)
{
	CDockablePane::OnSize(nType, cx, cy);
	if (theApp.GetMainFrame() != NULL)
		UpdateSize();
}

void CCropBar::OnPaint()
{
	CPaintDC dc(this); // device context for painting
	CRect	rc;
	GetClientRect(rc);
	COLORREF	clrBkgnd = GetSysColor(COLOR_WINDOW);
	if (m_bmBack.m_hObject) {
		VERIFY(dc.BitBlt(m_rBox.left, m_rBox.top, m_rBox.Width(), m_rBox.Height(), &m_dcBack, 0, 0, SRCCOPY));
		CRgn	rgnBkgnd, rgnBox;
		rgnBkgnd.CreateRectRgnIndirect(rc);
		rgnBox.CreateRectRgnIndirect(m_rBox);
		rgnBkgnd.CombineRgn(&rgnBkgnd, &rgnBox, RGN_DIFF);
		CBrush	brBkgnd(clrBkgnd);
		dc.FillRgn(&rgnBkgnd, &brBkgnd);
		if (!m_trk.m_rect.IsRectNull())
			m_trk.Draw(&dc);
	} else {
		dc.FillSolidRect(rc, clrBkgnd);
	}
}

void CCropBar::OnLButtonDown(UINT nFlags, CPoint point)
{
	CRect	rc;
	GetClientRect(rc);
	if (rc.PtInRect(point)) {
		SetFocus();
		CFauveDoc	*pDoc = theApp.GetMainFrame()->GetActiveMDIDoc();
		if (pDoc != NULL) {
			int	nHitTest = m_trk.HitTest(point);
			if (nHitTest < 0) {	// if click outside tracking rectangle
				if (m_trk.TrackRubberBand(this, point)) {	// do tracking
					UpdateCrop();
				} else {	// tracking failed
					if (pDoc->m_rCrop.IsRectEmpty()) {	// if crop doesn't exist
						CSize	sz(m_rBox.Size().cx / 2, m_rBox.Size().cy / 2);
						if (sz.cx && sz.cy) {	// if valid size, create default crop
							CSize	szOffset(sz.cx / 2, sz.cy / 2);
							m_trk.m_rect = CRect(m_rBox.CenterPoint() - szOffset, sz);
						}
					} else {	// crop exists
						m_trk.m_rect.SetRectEmpty();	// remove crop
					}
					UpdateCrop();
				}
			} else {	// click within tracking rectangle
				if (m_trk.Track(this, point, true)) {	// do tracking
					UpdateCrop();
				}
			}
		}
	} else {
		CDockablePane::OnLButtonDown(nFlags, point);
	}
}

void CCropBar::OnLButtonDblClk(UINT nFlags, CPoint point)
{
	CRect	rc;
	GetClientRect(rc);
	if (rc.PtInRect(point)) {
	} else {
		CDockablePane::OnLButtonDblClk(nFlags, point);
	}
}

BOOL CCropBar::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{
	// If the tracker cursors disappear when MFC is used in a static library,
	// it's because the default resource script disables them to save space.
	// They're defined in afxres.rc, and loaded in CRectTracker::Construct.
	// Edit the Resource Includes to remove this line, ignoring the warning:
	// #define _AFX_NO_TRACKER_RESOURCES
	if (m_trk.SetCursor(pWnd, nHitTest))
		return true;
	return CDockablePane::OnSetCursor(pWnd, nHitTest, message);
}
