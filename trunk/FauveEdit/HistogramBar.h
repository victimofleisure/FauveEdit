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

#pragma once

class CHistogramBar : public CDockablePane
{
// Construction
public:
	CHistogramBar();
	virtual ~CHistogramBar();

// Attributes
	void	OnUpdate(CView* pSender, LPARAM lHint, CObject *pHint);

protected:
// Constants

// Implementation
	CArrayEx<CD2DPointF, CD2DPointF>	m_aLinePt;	// array of line points

// Message map
	DECLARE_MESSAGE_MAP()
	afx_msg LRESULT CHistogramBar::OnDrawD2D(WPARAM wParam, LPARAM lParam);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnPaint();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
};

