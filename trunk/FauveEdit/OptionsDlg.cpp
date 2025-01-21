// Copyleft 2022 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      27dec22	initial version

*/

// OptionsDlg.cpp : implementation file
//

#include "stdafx.h"
#include "FauveEdit.h"
#include "OptionsDlg.h"
#include "afxdialogex.h"
#include "Windowsx.h"

// COptionsDlg dialog

IMPLEMENT_DYNAMIC(COptionsDlg, CDialog)

COptionsDlg::COptionsDlg(CWnd* pParent /*=NULL*/)
	: CDialog(COptionsDlg::IDD, pParent)
{
}

COptionsDlg::~COptionsDlg()
{
}

BOOL COptionsDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	m_wndColorButton.SetColor(m_clrBackground);

	return TRUE;  // return TRUE unless you set the focus to a control
}

void COptionsDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_CBIndex(pDX, IDC_OPTIONS_MULTITHREADING_COMBO, m_nMultithreadingType);
	DDX_Text(pDX, IDC_OPTIONS_CUSTOM_THREADS_EDIT, m_nThreadCount);
	DDV_MinMaxInt(pDX, m_nThreadCount, 1, BYTE_MAX);
	DDX_Control(pDX, IDC_OPTIONS_BACKGROUND_COLOR, m_wndColorButton);
	DDX_Text(pDX, IDC_OPTIONS_ANIM_FRAME_RATE_EDIT, m_fAnimationFrameRate);
	DDX_Text(pDX, IDC_OPTIONS_ANIM_HUE_RATE_EDIT1, m_fAnimationHueRate[0]);
	DDX_Text(pDX, IDC_OPTIONS_ANIM_HUE_RATE_EDIT2, m_fAnimationHueRate[1]);
	DDX_Text(pDX, IDC_OPTIONS_ANIM_HUE_RATE_EDIT3, m_fAnimationHueRate[2]);
}

BEGIN_MESSAGE_MAP(COptionsDlg, CDialog)
	ON_MESSAGE(WM_KICKIDLE, OnKickIdle)
	ON_UPDATE_COMMAND_UI(IDC_OPTIONS_CUSTOM_THREADS_EDIT, OnUpdateCustomThreads)
END_MESSAGE_MAP()

// COptionsDlg message handlers

void COptionsDlg::OnOK()
{
	CDialog::OnOK();

	m_clrBackground = m_wndColorButton.GetColor();
}

LRESULT COptionsDlg::OnKickIdle(WPARAM, LPARAM)
{
	UpdateDialogControls(this, FALSE); 
	return 0;
}

void COptionsDlg::OnUpdateCustomThreads(CCmdUI *pCmdUI)
{
	CWnd	*pWnd = GetDlgItem(IDC_OPTIONS_MULTITHREADING_COMBO);
	if (::GetFocus() == pWnd->m_hWnd) {
		int	nSel = ComboBox_GetCurSel(pWnd->m_hWnd);
		COptions	options;
		options.m_nMultithreadingType = nSel;
		options.m_nThreadCount = m_nThreadCount;
		int	nThreads = options.GetThreadCount();
		CString	sVal;
		sVal.Format(_T("%d"), nThreads);
		pCmdUI->SetText(sVal);
		pCmdUI->Enable(nSel == MT_CUSTOM_THREADS);
	}
}