// Copyleft 2025 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00		20jan25	initial version

*/

// PathDlg.cpp : implementation file
//

#include "stdafx.h"
#include "FauveEdit.h"
#include "PathDlg.h"

// CPathDlg dialog

IMPLEMENT_DYNAMIC(CPathDlg, CDialog)

CPathDlg::CPathDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CPathDlg::IDD, pParent)
{
}

CPathDlg::~CPathDlg()
{
}

void CPathDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_PATH_EDIT, m_sPath);
}


BEGIN_MESSAGE_MAP(CPathDlg, CDialog)
	ON_BN_CLICKED(IDC_PATH_BROWSE, &CPathDlg::OnPathBrowse)
END_MESSAGE_MAP()


// CPathDlg message handlers


BOOL CPathDlg::OnInitDialog()
{
	CDialog::OnInitDialog();
	if (!m_sCaption.IsEmpty())
		SetWindowText(m_sCaption);
	return TRUE;  // return TRUE unless you set the focus to a control
}

void CPathDlg::OnPathBrowse()
{
	CFileDialog	dlg(true);
	if (dlg.DoModal() == IDOK) {
		m_sPath = dlg.GetPathName();
		UpdateData(false);
	}
}
