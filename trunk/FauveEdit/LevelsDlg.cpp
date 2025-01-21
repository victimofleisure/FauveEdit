// Copyleft 2022 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00		21nov22	initial version

*/

// LevelsDlg.cpp : implementation file
//

#include "stdafx.h"
#include "FauveEdit.h"
#include "LevelsDlg.h"

// CLevelsDlg dialog

IMPLEMENT_DYNAMIC(CLevelsDlg, CDialog)

CLevelsDlg::CLevelsDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CLevelsDlg::IDD, pParent)
	, m_nMin(0)
	, m_nMax(0)
{

}

CLevelsDlg::~CLevelsDlg()
{
}

void CLevelsDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_LEVELS_EDIT_MIN, m_nMin);
	DDV_MinMaxInt(pDX, m_nMin, 0, BYTE_MAX);
	DDX_Text(pDX, IDC_LEVELS_EDIT_MAX, m_nMax);
	DDV_MinMaxInt(pDX, m_nMax, 0, BYTE_MAX);
	if (pDX->m_bSaveAndValidate && m_nMin >= m_nMax) {
		AfxMessageBox(IDS_LEVELS_RANGE_ERROR);
		DDV_Fail(pDX, IDC_LEVELS_EDIT_MIN);
	}
}


BEGIN_MESSAGE_MAP(CLevelsDlg, CDialog)
END_MESSAGE_MAP()


// CLevelsDlg message handlers
