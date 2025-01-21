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

// HueDlg.cpp : implementation file
//

#include "stdafx.h"
#include "FauveEdit.h"
#include "HueDlg.h"

// CHueDlg dialog

IMPLEMENT_DYNAMIC(CHueDlg, CDialog)

CHueDlg::CHueDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CHueDlg::IDD, pParent)
	, m_nRed(0)
	, m_nGreen(0)
	, m_nBlue(0)
{

}

CHueDlg::~CHueDlg()
{
}

void CHueDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_HUE_EDIT_RED, m_nRed);
	DDV_MinMaxInt(pDX, m_nRed, 0, 255);
	DDX_Text(pDX, IDC_HUE_EDIT_GREEN, m_nGreen);
	DDV_MinMaxInt(pDX, m_nGreen, 0, 255);
	DDX_Text(pDX, IDC_HUE_EDIT_BLUE, m_nBlue);
	DDV_MinMaxInt(pDX, m_nBlue, 0, 255);
}


BEGIN_MESSAGE_MAP(CHueDlg, CDialog)
END_MESSAGE_MAP()


// CHueDlg message handlers
