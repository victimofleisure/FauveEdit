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

class CHueBar : public CPaneDialog
{
// Construction
public:
	CHueBar();
	virtual ~CHueBar();

// Attributes
	void	OnUpdate(CView* pSender, LPARAM lHint, CObject *pHint);

protected:
// Data members
	CSliderCtrl	m_wndSlider1;
	CSliderCtrl	m_wndSlider2;
	CSliderCtrl	m_wndSlider3;

// Implementation
	virtual void DoDataExchange(CDataExchange* pDX);

// Message map
	DECLARE_MESSAGE_MAP()
	afx_msg LRESULT HandleInitDialog(WPARAM wParam, LPARAM lParam);
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnUpdateSlider(CCmdUI *pCmdUI);
};

