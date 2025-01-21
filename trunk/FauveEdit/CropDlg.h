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

#pragma once

// CCropDlg dialog

class CCropDlg : public CDialog
{
	DECLARE_DYNAMIC(CCropDlg)

public:
	CCropDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CCropDlg();

// Dialog Data
	enum { IDD = IDD_CROP };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	CPoint	m_point;
	CSize	m_size;
};
