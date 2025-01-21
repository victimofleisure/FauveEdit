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

#pragma once


// CPathDlg dialog

class CPathDlg : public CDialog
{
	DECLARE_DYNAMIC(CPathDlg)

public:
	CPathDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CPathDlg();

// Dialog Data
	enum { IDD = IDD_PATH };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	CString m_sPath;
	CString	m_sCaption;
	virtual BOOL OnInitDialog();
	afx_msg void OnPathBrowse();
};
