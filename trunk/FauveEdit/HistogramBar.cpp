// Copyleft 2026 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00		02oct26	initial version

*/

#include "stdafx.h"

#include "HistogramBar.h"
#include "Resource.h"
#include "MainFrm.h"
#include "FauveEdit.h"
#include "FauveDoc.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

CHistogramBar::CHistogramBar()
{
}

CHistogramBar::~CHistogramBar()
{
}

void CHistogramBar::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint)
{
	UNREFERENCED_PARAMETER(pSender);
	UNREFERENCED_PARAMETER(lHint);
	UNREFERENCED_PARAMETER(pHint);
	Invalidate();
}

BEGIN_MESSAGE_MAP(CHistogramBar, CDockablePane)
	ON_REGISTERED_MESSAGE(AFX_WM_DRAW2D, OnDrawD2D)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_PAINT()
	ON_WM_ERASEBKGND()
END_MESSAGE_MAP()

LRESULT CHistogramBar::OnDrawD2D(WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(wParam);
	CRenderTarget* pRT = reinterpret_cast<CRenderTarget*>(lParam);
	ASSERT_VALID(pRT);
	static const int	aColor[CFauve::COLOR_CHANNELS] = {
		D2D1::ColorF::Red,
		D2D1::ColorF::Green,
		D2D1::ColorF::Blue,
	};
	if (pRT->IsValid()) {	// if valid render target
		D2D1_SIZE_F szRender = pRT->GetSize();	// get target size in DIPs
		pRT->Clear(D2D1::ColorF(1, 1, 1));
		CFauveDoc	*pDoc = theApp.GetMainFrame()->GetActiveMDIDoc();
		if (pDoc != NULL) {
			const int nChans = CFauve::COLOR_CHANNELS;
			const int nVals = CFauve::COLOR_VALUES;
			double	fBarWidth = szRender.width / nVals;
			double	fBarHeight = szRender.height / nChans;
			m_aLinePt.FastSetSize(nVals * 2 + 1);
			for (int iChan = nChans - 1; iChan >= 0; iChan--) {	// for each color channel
				CD2DPathGeometry	geom(pRT);
				geom.Create(pRT);
				CD2DGeometrySink	sink(geom);
				CD2DSolidColorBrush	br(pRT, D2D1::ColorF(aColor[iChan]));
				double	fX1 = 0;
				double	fY2 =  fBarHeight * (iChan + 1);
				sink.BeginFigure(CD2DPointF(float(fX1), float(fY2)), D2D1_FIGURE_BEGIN_FILLED);
				for (int iVal = 0; iVal < nVals; iVal++) {	// for each color value
					double	fY1 = fY2 - double(pDoc->m_arrIdx[iChan][iVal]) / BYTE_MAX * fBarHeight;
					double	fX2 = fBarWidth * (iVal + 1);
					m_aLinePt[iVal * 2] = CD2DPointF(float(fX1), float(fY1));
					m_aLinePt[iVal * 2 + 1] = CD2DPointF(float(fX2), float(fY1));
					fX1 = fX2;
				}
				m_aLinePt[nVals * 2] = CD2DPointF(float(fX1), float(fY2));	// close
				sink.AddLines(m_aLinePt);
				sink.EndFigure(D2D1_FIGURE_END_CLOSED);
				sink.Close();
				pRT->FillGeometry(&geom, &br);
			}
		}
	}
	return 0;
}

int CHistogramBar::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CDockablePane::OnCreate(lpCreateStruct) == -1)
		return -1;
	EnableD2DSupport();
	return 0;
}

void CHistogramBar::OnSize(UINT nType, int cx, int cy)
{
	CDockablePane::OnSize(nType, cx, cy);
	Invalidate();
}

void CHistogramBar::OnPaint()
{
	ValidateRect(NULL);
}

BOOL CHistogramBar::OnEraseBkgnd(CDC* pDC)
{
	UNREFERENCED_PARAMETER(pDC);
	return TRUE;
}
