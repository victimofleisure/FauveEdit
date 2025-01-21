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

#pragma once

#include "Dib.h"

class CCropBar : public CDockablePane
{
// Construction
public:
	CCropBar();
	virtual ~CCropBar();

// Attributes
	void	OnUpdate(CView* pSender, LPARAM lHint, CObject *pHint);

protected:
// Data members
	CDC	m_dcMem;			// device context for source bitmap
	CDC	m_dcBack;			// device context for back buffer
	CBitmap	m_bmBack;		// bitmap for back buffer
	CRect	m_rBox;			// letterbox
	CRectTracker	m_trk;	// rectangle tracker

// Implementation
	void	UpdateSize();
	void	UpdateCrop();

// Message map
	DECLARE_MESSAGE_MAP()
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnPaint();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
};

