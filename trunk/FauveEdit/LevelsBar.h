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

class CLevelsBar : public CDockablePane
{
// Construction
public:
	CLevelsBar();
	virtual ~CLevelsBar();

// Attributes
	void	OnUpdate(CView* pSender, LPARAM lHint, CObject *pHint);

protected:
// Constants
	enum {
		LEVELS_MARGIN = 8,
		MARK_SIZE = LEVELS_MARGIN + 2,
	};
	enum {
		MARK_BLACK,
		MARK_WHITE,
		MARKS,
		MARK_NONE = -1,
	};
	static const COLORREF	m_clrMargin;
	static const COLORREF	m_clrBkgnd;
	static const COLORREF	m_clrLevel;

// Data members
	CSize	m_szClient;			// size of client area
	CRect	m_rLevels;			// rectangle containing levels histogram
	int		m_iDragMark;		// index of mark being dragged, or -1 if none
	int		m_iDragOldLevel;	// previous level of mark being dragged
	int		m_nDragPos;			// current x-coordinate during dragging

// Implementation
	static	void	DrawMark(CDC& dc, CPoint point);
	void	EraseMark(CDC& dc, int iMark, int iLevel);
	void	DrawMark(CDC& dc, int iMark, int iLevel);
	void	GetMarkRect(int iMark, int iLevel, CRect& rMark) const;
	int		GetMarkValue(int iMark, int x) const;
	int		HitTest(CPoint point, int& iNearestMark);
	void	SetLevelFromPos(int iMark, int x, bool bRedrawMark = true);
	void	RedrawMark(int iMark, int iOldLevel, int iNewLevel);

// Message map
	DECLARE_MESSAGE_MAP()
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnPaint();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnCaptureChanged(CWnd *pWnd);
};

