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

// CHueDlg dialog

class CHueDlg : public CDialog
{
	DECLARE_DYNAMIC(CHueDlg)

public:
	CHueDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CHueDlg();

// Dialog Data
	enum { IDD = IDD_HUE };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	int m_nRed;
	int m_nGreen;
	int m_nBlue;
};
