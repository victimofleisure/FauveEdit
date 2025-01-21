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
#include "FauveEdit.h"
#include "HueBar.h"
#include "MainFrm.h"
#include "FauveDoc.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

CHueBar::CHueBar()
{
}

CHueBar::~CHueBar()
{
}

void CHueBar::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint)
{
	UNREFERENCED_PARAMETER(pSender);
	UNREFERENCED_PARAMETER(pHint);
	CFauveDoc	*pDoc = theApp.GetMainFrame()->GetActiveMDIDoc();
	if (pDoc != NULL) {
		switch (lHint) {
		case CFauveDoc::HINT_NONE:
		case CFauveDoc::HINT_HUE:
			m_wndSlider1.SetPos(pDoc->m_arrHue[CFauve::C_R]);
			m_wndSlider2.SetPos(pDoc->m_arrHue[CFauve::C_G]);
			m_wndSlider3.SetPos(pDoc->m_arrHue[CFauve::C_B]);
			break;
		}
	} else {
		m_wndSlider1.SetPos(0);
		m_wndSlider2.SetPos(0);
		m_wndSlider3.SetPos(0);
	}
}

BEGIN_MESSAGE_MAP(CHueBar, CPaneDialog)
	ON_MESSAGE(WM_INITDIALOG, HandleInitDialog)
	ON_WM_HSCROLL()
	ON_UPDATE_COMMAND_UI_RANGE(IDC_HUE_SLIDER_1, IDC_HUE_SLIDER_3, OnUpdateSlider)
END_MESSAGE_MAP()

LRESULT CHueBar::HandleInitDialog(WPARAM wParam, LPARAM lParam)
{
	CPaneDialog::HandleInitDialog(wParam, lParam);
	m_wndSlider1.SetRange(0, 255);
	m_wndSlider2.SetRange(0, 255);
	m_wndSlider3.SetRange(0, 255);
	return true;
}

void CHueBar::DoDataExchange(CDataExchange* pDX)
{
	CPaneDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_HUE_SLIDER_1, m_wndSlider1);
	DDX_Control(pDX, IDC_HUE_SLIDER_2, m_wndSlider2);
	DDX_Control(pDX, IDC_HUE_SLIDER_3, m_wndSlider3);	
}

void CHueBar::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	UNREFERENCED_PARAMETER(nPos);
	UINT	nID = pScrollBar->GetDlgCtrlID();
	int	iChannel = nID - IDC_HUE_SLIDER_1;	// assume resources are sorted
	CFauveDoc	*pDoc = theApp.GetMainFrame()->GetActiveMDIDoc();
	if (pDoc != NULL) {
		if (nSBCode != SB_THUMBTRACK || theApp.m_bDragPreview) {
			CSliderCtrl	*pSlider = (CSliderCtrl	*)pScrollBar;
			int	nSliderPos = pSlider->GetPos();
			ASSERT(nSliderPos >= 0 && nSliderPos <= BYTE_MAX);
			BYTE	nHueShift = static_cast<BYTE>(nSliderPos);
			pDoc->SetHue(iChannel, nHueShift, reinterpret_cast<CView *>(this));
		}
	}
}

void CHueBar::OnUpdateSlider(CCmdUI *pCmdUI)
{
	CFauveDoc	*pDoc = theApp.GetMainFrame()->GetActiveMDIDoc();
	pCmdUI->Enable(pDoc != NULL);
}
