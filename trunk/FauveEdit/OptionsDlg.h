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

#pragma once

#include "Options.h"
#include "afxcolorbutton.h"

// COptionsDlg dialog

class COptionsDlg : public CDialog, public COptions
{
	DECLARE_DYNAMIC(COptionsDlg)

public:
	COptionsDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~COptionsDlg();

// Dialog Data
	enum { IDD = IDD_OPTIONS };

protected:
	virtual BOOL OnInitDialog();
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual void OnOK();

	CMFCColorButton m_wndColorButton;

	DECLARE_MESSAGE_MAP()
	LRESULT OnKickIdle(WPARAM, LPARAM);
	afx_msg void OnUpdateCustomThreads(CCmdUI *pCmdUI);
};
