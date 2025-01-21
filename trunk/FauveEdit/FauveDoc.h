// Copyleft 2020 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00		29apr20	initial version
		01		10jan25	add option to reuse histogram
		02		20jan25	add edit source image

*/

// FauveDoc.h : interface of the CFauveDoc class
//

#pragma once

#include "FauveTiled.h"
#include "Undoable.h"
#include "UndoManager.h"
#include "UndoCodes.h"

class CFauveDoc : public CDocument, public CFauveTiled, public CUndoable
{
protected: // create from serialization only
	CFauveDoc();
	DECLARE_DYNCREATE(CFauveDoc)

public:
// Constants
	enum {
		HINT_NONE,
		HINT_CROP,
		HINT_ROTATE,
		HINT_LEVELS,
		HINT_HUE,
		HINT_OPTIONS,
	};
	enum {	// bits per pixel
		BPP_24 = 24,
		BPP_32 = 32,
	};
	enum {
		HISTOGRAM_LEVELS = 256,	// number of luma levels in histogram
	};
	static const int m_arrUndoTitleId[UNDO_CODES];

// Types
	struct HISTOGRAM {
		DWORD	m_arrLuma[HISTOGRAM_LEVELS];
		DWORD	m_nLumaMax;
	};
	class CMyUndoManager : public CUndoManager {
	public:
		CMyUndoManager();
		virtual	void	OnUpdateTitles();
		CString	m_sUndoMenuItem;	// undo's edit menu item string; prefixed undo title
		CString	m_sRedoMenuItem;	// redo's edit menu item string; prefixed redo title
		static	CString	m_sUndoPrefix;	// prefix for undo's edit menu item, from resource
		static	CString	m_sRedoPrefix;	// prefix for redo's edit menu item, from resource
	};
	class COptionsHint : public CObject {
	public:
		const COptions	*m_pPrevOptions;	// previous options
	};

// Public data
	CRect	m_rCrop;		// cropping rectangle
	HISTOGRAM	m_histogram;	// histogram of input image
	CString	m_sOrigPath;	// path of original image file
	bool	m_bGotFauveChunk;	// true if we read fauve PNG chunk
	BYTE	m_nDibRot;		// bitmap rotation
	CMyUndoManager	m_undoMgr;	// undo manager

// Attributes
	void	GetScaledCropRect(const CRect& rBox, CRect& rDest) const;
	void	SetScaledCropRect(const CRect& rBox, const CRect& rSrc, CView *pSender = NULL);
	bool	SetCrop(const CRect& rCrop, CView *pSender = NULL);
	void	SetLevel(int iMark, BYTE nLevel, CView *pSender = NULL);
	void	SetLevels(BYTE nLumaMin, BYTE nLumaMax, CView *pSender = NULL);
	void	SetHue(int iChannel, BYTE nHueShift, CView *pSender = NULL);
	void	SetHues(BYTE nRed, BYTE nGreen, BYTE nBlue, CView *pSender = NULL);
	void	Rotate(int nQuadrants = 1);
	bool	SetSourceImage(CString& sPath);

// Operations
	void	UpdateFauve(bool bReuseHistogram = false);
	void	UpdateHistogram();
	void	UpdateAllViews(CView* pSender, LPARAM lHint = 0L, CObject* pHint = NULL);	// not virtual!
	static	void	RotateDibRef(const CDibEx& dibIn, CDibEx& dibOut, int nQuadrants = 1);
	static	void	RotateDibRef(CDibEx& dib, int nQuadrants = 1);
	static	void	RotateDibFast(const CDibEx& dibIn, CDibEx& dibOut, int nQuadrants = 1);
	static	void	RotateDibFast(CDibEx& dib, int nQuadrants = 1);
	static	void	RotateSubrect(CRect& rect, const CSize& szParent, int nQuadrants = 1);
	bool	ExportVideo();
	void	ApplyOptions(const COptions *pPrevOptions);
	bool	PromptSourceImagePath(CString& sImgPath);
	bool	ReadSourceImage(CString sImgPath);

// Overrides
	virtual BOOL OnNewDocument();
	virtual void Serialize(CArchive& ar);
	virtual	BOOL OnOpenDocument(LPCTSTR lpszPathName);
	virtual BOOL OnSaveDocument(LPCTSTR lpszPathName);
	virtual BOOL DoFileSave();
	virtual BOOL DoSave(LPCTSTR lpszPathName, BOOL bReplace = TRUE);
	virtual	void SaveUndoState(CUndoState& State);
	virtual	void RestoreUndoState(const CUndoState& State);
	virtual	CString	GetUndoTitle(const CUndoState& State);

#ifdef SHARED_HANDLERS
	virtual void InitializeSearchContent();
	virtual void OnDrawThumbnail(CDC& dc, LPRECT lprcBounds);
#endif // SHARED_HANDLERS

// Implementation
public:
// Types
	class CUndoCrop : public CRefObj {
	public:
		CRect	m_rCrop;
	};

// Overrides
	virtual ~CFauveDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

// Helpers
	void	SaveCrop(CUndoState& State);
	void	RestoreCrop(const CUndoState& State);
	void	SaveRotate(CUndoState& State);
	void	RestoreRotate(const CUndoState& State);
	void	SaveLevels(CUndoState& State);
	void	RestoreLevels(const CUndoState& State);
	void	SaveHue(CUndoState& State);
	void	RestoreHue(const CUndoState& State);
	void	SaveSourceImage(CUndoState& State);
	void	RestoreSourceImage(const CUndoState& State);

protected:
#ifdef SHARED_HANDLERS
	// Helper function that sets search content for a Search Handler
	void SetSearchContent(const CString& value);
#endif // SHARED_HANDLERS

// Generated message map functions
	DECLARE_MESSAGE_MAP()
	afx_msg void OnEditUndo();
	afx_msg void OnEditRedo();
	afx_msg void OnUpdateEditUndo(CCmdUI *pCmdUI);
	afx_msg void OnUpdateEditRedo(CCmdUI *pCmdUI);
	afx_msg void OnEditRotateCW();
	afx_msg void OnEditCrop();
	afx_msg void OnEditLevels();
	afx_msg void OnEditHue();
	afx_msg void OnEditSourceImage();
	afx_msg void OnFileExportVideo();
};
