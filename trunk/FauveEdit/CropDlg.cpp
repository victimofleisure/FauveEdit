// Copyleft 2022 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00		21nov22	initial version
		01		18jan25	convert to point and size

*/

// CropDlg.cpp : implementation file
//

#include "stdafx.h"
#include "FauveEdit.h"
#include "CropDlg.h"

// CCropDlg dialog

IMPLEMENT_DYNAMIC(CCropDlg, CDialog)

CCropDlg::CCropDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CCropDlg::IDD, pParent)
	, m_point(0, 0)
	, m_size(0, 0)
{

}

CCropDlg::~CCropDlg()
{
}

void CCropDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_CROP_EDIT_LEFT, m_point.x);
	DDV_MinMaxInt(pDX, m_point.x, 0, INT_MAX);
	DDX_Text(pDX, IDC_CROP_EDIT_TOP, m_point.y);
	DDV_MinMaxInt(pDX, m_point.y, 0, INT_MAX);
	DDX_Text(pDX, IDC_CROP_EDIT_WIDTH, m_size.cx);
	DDV_MinMaxInt(pDX, m_size.cx, 0, INT_MAX);
	DDX_Text(pDX, IDC_CROP_EDIT_HEIGHT, m_size.cy);
	DDV_MinMaxInt(pDX, m_size.cy, 0, INT_MAX);
}


BEGIN_MESSAGE_MAP(CCropDlg, CDialog)
END_MESSAGE_MAP()


// CCropDlg message handlers

