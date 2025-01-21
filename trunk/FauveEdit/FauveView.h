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

// FauveView.h : interface of the CFauveView class
//

#pragma once

#include "D2DDevCtx.h"

class CFauveView : public CView
{
protected: // create from serialization only
	CFauveView();
	DECLARE_DYNCREATE(CFauveView)

// Attributes
public:
	CFauveDoc* GetDocument() const;

// Operations
public:

// Overrides
public:
	virtual void OnDraw(CDC* pDC);  // overridden to draw this view
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual void OnInitialUpdate(); // called first time after construct
	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
	virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);

// Implementation
public:
	virtual ~CFauveView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
	enum {
		ANIMATION_TIMER = 123
	};
	CD2DBitmapDevCtx	m_dc;
	CSize	m_szClient;			// size of our window in client coords
	bool	m_bIsAnimating;		// true if animating
	bool	m_bIsResizePending;	// true if resize is pending
	int		m_iFrame;			// frame index while animating
	BYTE	m_arrPreAnimHue[3];	// pre-animation hue rotation state

	void	Update();
	void	OnBackgroundColorChange();

// Generated message map functions
protected:
	DECLARE_MESSAGE_MAP()
	afx_msg void OnFilePrintPreview();
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	virtual void OnUpdate(CView* /*pSender*/, LPARAM /*lHint*/, CObject* /*pHint*/);
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnEditAnimate();
	afx_msg void OnUpdateEditAnimate(CCmdUI *pCmdUI);
	afx_msg LRESULT	OnDelayedResize(WPARAM wParam, LPARAM lParam);
};

#ifndef _DEBUG  // debug version in FauveView.cpp
inline CFauveDoc* CFauveView::GetDocument() const
   { return reinterpret_cast<CFauveDoc*>(m_pDocument); }
#endif

