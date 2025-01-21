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

#pragma once

// CLevelsDlg dialog

class CLevelsDlg : public CDialog
{
	DECLARE_DYNAMIC(CLevelsDlg)

public:
	CLevelsDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CLevelsDlg();

// Dialog Data
	enum { IDD = IDD_LEVELS };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	int m_nMin;
	int m_nMax;
};
