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

// FauveDoc.cpp : implementation of the CFauveDoc class
//

#include "stdafx.h"
// SHARED_HANDLERS can be defined in an ATL project implementing preview, thumbnail
// and search filter handlers and allows sharing of document code with that project.
#ifndef SHARED_HANDLERS
#include "FauveEdit.h"
#endif

#include "FauveDoc.h"
#include <propkey.h>
#include "PathStr.h"
#include "MainFrm.h"
#include "PngReader.h"
#include "CropDlg.h"
#include "LevelsDlg.h"
#include "HueDlg.h"
#include "ProgressDlg.h"
#include "FolderDialog.h"
#include "Persist.h"
#include "RecordDlg.h"
#include "PathDlg.h"

#define RK_EXPORT_VIDEO_FOLDER _T("ExportVideoFolder")

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

class CFauvePng {
public:
// Types
	#pragma pack(push)
	#pragma pack(1)
	struct FAUVE_DATA {	// Fauve persistent data
		struct FIXED {		// fixed-length portion of data
			UINT	nSignature;	// data signature
			UINT	nVersion;	// data version
			RECT	rCrop;		// cropping rectangle
			BYTE	nLumaMin;	// minimum luma value
			BYTE	nLumaMax;	// maximum luma value
			BYTE	nHueR;		// red hue shift
			BYTE	nHueG;		// green hue shift
			BYTE	nHueB;		// blue hue shift
			BYTE	nUnused;	// reserved, must be zero
			BYTE	nDibRot;	// bitmap rotation in quadrants
			BYTE	nPad;		// reserved, must be zero
		};
		FIXED	fixed;			// fixed-length portion of Fauve data
		WCHAR	szOrigPath[1];	// Unicode path of original image file
	};
	struct FAUVE_CHUNK {	// define Fauve PNG chunk 
		UINT	nSize;		// chunk data size in bytes
		UINT	nType;		// chunk type (faUV)
		FAUVE_DATA	data;	// variable-length chunk data
	};
	#pragma pack(pop)

// Constants
	enum {
		FAUVE_DATA_SIGNATURE = 0xFAE92022,
		FAUVE_DATA_VERSION = 0,
	};
	static const UINT	FAUVE_CHUNK_TYPE;

// Operations
	static	bool	AppendToPngFile(LPCTSTR pszPngPath, const FAUVE_DATA::FIXED& dataIn, LPCTSTR pszOrigPath);
	static	bool	ReadFromPngFile(LPCTSTR pszPngPath, FAUVE_DATA::FIXED& dataOut, CString& sOrigPath);

protected:
	class CMyPngReader : public CPngReader {
	public:
		CString	m_sOrigPath;
		FAUVE_DATA::FIXED	m_dataOut;
		virtual	bool OnChunk();
	};
};

const UINT CFauvePng::FAUVE_CHUNK_TYPE = CPngReader::MakeChunkType('f', 'a', 'U', 'V');

bool CFauvePng::AppendToPngFile(LPCTSTR pszPngPath, const FAUVE_DATA::FIXED& dataIn, LPCTSTR pszOrigPath)
{
	CFile	f(pszPngPath, CFile::modeReadWrite);
	static const UINT	arrEndChunkData[3] = {0, 0x444e4549, 0x826042ae};
	UINT	arrEndChunkBuf[3];
	const int END_CHUNK_SIZE = sizeof(arrEndChunkData);
	f.Seek(-END_CHUNK_SIZE, CFile::end);
	f.Read(arrEndChunkBuf, END_CHUNK_SIZE);
	if (memcmp(arrEndChunkBuf, arrEndChunkData, END_CHUNK_SIZE)) {	// if end chunk not found
		ASSERT(0);	// unexpected file format
		return false;
	}
	FAUVE_CHUNK	chunk;
	ZeroMemory(&chunk, sizeof(chunk));
	CStringW	sOutPath(pszOrigPath);	// convert path to Unicode if necessary
	LPCWSTR	pszOutPath = sOutPath;
	int	nOutPathSize = (sOutPath.GetLength() + 1) * sizeof(WCHAR);	// include null terminator
	chunk.nSize = _byteswap_ulong(sizeof(FAUVE_DATA::FIXED) + nOutPathSize);
	chunk.nType = FAUVE_CHUNK_TYPE;
	chunk.data.fixed = dataIn;
	chunk.data.fixed.nSignature = static_cast<UINT>(FAUVE_DATA_SIGNATURE);
	chunk.data.fixed.nVersion = FAUVE_DATA_VERSION;
	const BYTE	*pCRCData = reinterpret_cast<const BYTE *>(&chunk.nType);
	UINT	nCRC = 0xffffffffL;	// per PNG specification
	nCRC = CPngReader::update_crc(nCRC, pCRCData, sizeof(FAUVE_DATA::FIXED) + sizeof(UINT));
	nCRC = CPngReader::update_crc(nCRC, reinterpret_cast<const BYTE *>(pszOutPath), nOutPathSize);
	nCRC = _byteswap_ulong(nCRC ^ 0xffffffffL);	// per PNG specification
	f.Seek(-END_CHUNK_SIZE, CFile::end);
	f.Write(&chunk, offsetof(FAUVE_CHUNK, data.szOrigPath));
	f.Write(pszOutPath, nOutPathSize);
	f.Write(&nCRC, sizeof(UINT));
	f.Write(arrEndChunkData, END_CHUNK_SIZE);
	return true;
}

bool CFauvePng::CMyPngReader::OnChunk()
{
	if (m_hdr.nType == FAUVE_CHUNK_TYPE) {	// if chunk type matches
		const BYTE	*pChunkData = ReadChunk();
		if (pChunkData == NULL)	// if chunk CRC error
			return false;
		const FAUVE_DATA	*pFauve = (const FAUVE_DATA *)pChunkData;
		if (pFauve->fixed.nSignature != FAUVE_DATA_SIGNATURE)	// if invalid signature
			return false;
		m_dataOut = pFauve->fixed;	// success
		m_sOrigPath = pFauve->szOrigPath;
	} else {	// some other chunk
		SkipChunk();
	}
	return true;
}

bool CFauvePng::ReadFromPngFile(LPCTSTR pszPngPath, FAUVE_DATA::FIXED& dataOut, CString& sOrigPath)
{
	CMyPngReader	reader;
	if (!reader.Read(pszPngPath))
		return false;
	if (reader.m_sOrigPath.IsEmpty())
		return false;
	dataOut = reader.m_dataOut;
	sOrigPath = reader.m_sOrigPath;
	return true;
}

// CFauveDoc

const int CFauveDoc::m_arrUndoTitleId[UNDO_CODES] = {
	#define UCODE_DEF(name) IDS_EDIT_##name,
	#include "UndoCodeData.h"	
};

CString	CFauveDoc::CMyUndoManager::m_sUndoPrefix;
CString	CFauveDoc::CMyUndoManager::m_sRedoPrefix;

IMPLEMENT_DYNCREATE(CFauveDoc, CDocument)

// CFauveDoc construction/destruction

CFauveDoc::CFauveDoc()
{
	m_rCrop.SetRectEmpty();
	m_arrLuma[L_BLACK] = 0;
	m_arrLuma[L_WHITE] = BYTE_MAX;
	ZeroMemory(m_arrHue, sizeof(m_arrHue));
	m_bGotFauveChunk = false;
	m_nDibRot = 0;
	SetUndoManager(&m_undoMgr);
	m_undoMgr.SetRoot(this);
}

CFauveDoc::~CFauveDoc()
{
}

void CFauveDoc::ApplyOptions(const COptions *pPrevOptions)
{
	if (pPrevOptions != NULL) {
		COptionsHint	hint;
		hint.m_pPrevOptions = pPrevOptions;
		UpdateAllViews(NULL, HINT_OPTIONS, &hint);
	}
}

BOOL CFauveDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	// TODO: add reinitialization code here
	// (SDI documents will reuse this document)

	return TRUE;
}

BOOL CFauveDoc::OnOpenDocument(LPCTSTR lpszPathName)
{
	CFauvePng::FAUVE_DATA::FIXED	data;
	if (!CDocument::OnOpenDocument(lpszPathName))
		return false;
	if (CFauvePng::ReadFromPngFile(lpszPathName, data, m_sOrigPath)) {
		m_rCrop = data.rCrop;
		m_arrLuma[L_BLACK] = data.nLumaMin;
		m_arrLuma[L_WHITE] = data.nLumaMax;
		m_arrHue[C_R] = data.nHueR;
		m_arrHue[C_G] = data.nHueG;
		m_arrHue[C_B] = data.nHueB;
		m_nDibRot = data.nDibRot;
		m_bGotFauveChunk = true;
	} else {
		m_sOrigPath = lpszPathName;
		m_bGotFauveChunk = false;
	}
	while (!PathFileExists(m_sOrigPath)) {	// loop until source image found or user cancels
		int	nRetc = AfxMessageBox(IDS_DOC_ERR_SOURCE_NOT_FOUND, MB_YESNO);
		if (nRetc != IDYES)
			return false;
		if (!PromptSourceImagePath(m_sOrigPath))
			return false;
		SetModifiedFlag();	// updating link is a modification
	}
	return ReadSourceImage(m_sOrigPath);
}

bool CFauveDoc::ReadSourceImage(CString sImgPath)
{
	CImage image;
	if (FAILED(image.Load(sImgPath))) {
		AfxMessageBox(IDS_DOC_ERR_CANT_LOAD_IMAGE);
		return false;
	}
	int	nBitsPerPixel = image.GetBPP();
	CBitmap	bitmap;
	bitmap.Attach(image.Detach());
	if (nBitsPerPixel == BPP_32) {	// if 32 bits per pixel
		// already in desired format
		if (!m_dibIn.Create(bitmap)) {
			AfxMessageBox(IDS_DOC_ERR_CANT_CREATE_BITMAP);
			return false;
		}
	} else if (nBitsPerPixel == BPP_24) {	// if 24 bits per pixel
		// convert to 32 bits per pixel
		CDibEx	dibTmp;
		if (!dibTmp.Create(bitmap)) {
			AfxMessageBox(IDS_DOC_ERR_CANT_CREATE_BITMAP);
			return false;
		}
		CSize	sz(dibTmp.GetSize());
		if (!m_dibIn.Create(sz.cx, sz.cy, BPP_32)) {
			AfxMessageBox(IDS_DOC_ERR_CANT_CREATE_DIB);
			return false;
		}
		for (int y = 0; y < sz.cy; y++) {
			for (int x = 0; x < sz.cx; x++) {
				m_dibIn.SetPixel(x, y, dibTmp.GetPixel(x, y));
			}
		}
	} else {
		AfxMessageBox(IDS_DOC_ERR_BAD_IMAGE_TYPE);
		return false;
	}
	if (m_nDibRot)
		RotateDibFast(m_dibIn, m_nDibRot);
	if (!m_rCrop.IsRectEmpty()) {	// if cropping specified
		// clip cropping in case input bitmap dimensions changed
		CRect	rImage(CPoint(0, 0), m_dibIn.GetSize());
		if (!rImage.IntersectRect(m_rCrop, rImage))
			rImage.SetRectEmpty();
		m_rCrop = rImage;
	}
	UpdateFauve();
	UpdateHistogram();
	return true;
}

BOOL CFauveDoc::OnSaveDocument(LPCTSTR lpszPathName)
{
	if (!CDocument::OnSaveDocument(lpszPathName))
		return false;
	// convert from 32 to 24 bits per pixel
	CDibEx	dibTmp;
	CSize	sz(m_dibOut.GetSize());
	if (!dibTmp.Create(sz.cx, sz.cy, BPP_24)) {
		AfxMessageBox(IDS_DOC_ERR_CANT_CREATE_DIB);
		return false;
	}
	for (int y = 0; y < sz.cy; y++) {
		for (int x = 0; x < sz.cx; x++) {
			dibTmp.SetPixel(x, y, m_dibOut.GetPixel(x, y));
		}
	}
	CImage	image;
	image.Attach(dibTmp);
	HRESULT	hr = image.Save(lpszPathName, Gdiplus::ImageFormatPNG);
	image.Detach();	// detach before returning
	if (FAILED(hr)) {
		AfxMessageBox(IDS_DOC_ERR_CANT_SAVE_PNG);
		return false;
	}
	// append fauve data to PNG file
	CFauvePng::FAUVE_DATA::FIXED	data;
	ZeroMemory(&data, sizeof(data));
	data.rCrop = m_rCrop;
	data.nLumaMin = m_arrLuma[L_BLACK];
	data.nLumaMax = m_arrLuma[L_WHITE];
	data.nHueR = m_arrHue[C_R];
	data.nHueG = m_arrHue[C_G];
	data.nHueB = m_arrHue[C_B];
	data.nDibRot = m_nDibRot;
	if (!CFauvePng::AppendToPngFile(lpszPathName, data, m_sOrigPath)) {
		AfxMessageBox(IDS_DOC_ERR_CANT_APPEND_PNG);
		return false;
	}
	return true;
}

BOOL CFauveDoc::DoFileSave()
{
	if (m_bGotFauveChunk) {
		return CDocument::DoFileSave();
	}
	if (!DoSave(NULL))
	{
		TRACE(traceAppMsg, 0, "Warning: File save with new name failed.\n");
		return FALSE;
	}
	return TRUE;
}

BOOL CFauveDoc::DoSave(LPCTSTR lpszPathName, BOOL bReplace)
{
	if (m_bGotFauveChunk) {
		return CDocument::DoSave(lpszPathName, bReplace);
	}
	CPathStr	sNewPath(lpszPathName);
	if (sNewPath.IsEmpty())
		sNewPath = m_strPathName;
	sNewPath.RemoveExtension();
	CString	s(m_strPathName);
	s.MakeLower();
	bool	bSourceIsPng = s.Find(_T(".png")) >= 0;
	if (bSourceIsPng)
		sNewPath += _T("_fauve");
	sNewPath += ".png";
	LPCTSTR	pszFilter = _T("PNG Files (*.png)|*.png|All Files (*.*)|*.*||");
	DWORD	nFlags = OFN_HIDEREADONLY | OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT;
	CFileDialog	fd(FALSE, _T(".png"), sNewPath, nFlags, pszFilter, NULL, NULL, TRUE);
	if (fd.DoModal() != IDOK)
		return FALSE;
	sNewPath = fd.GetPathName();
	CWaitCursor wait;
	if (!OnSaveDocument(sNewPath))
	{
		return FALSE;
	}
	SetPathName(sNewPath);	// order matters
	return TRUE;
}

void CFauveDoc::UpdateAllViews(CView* pSender, LPARAM lHint, CObject* pHint)
{
	CDocument::UpdateAllViews(pSender, lHint, pHint);
	theApp.GetMainFrame()->OnUpdate(pSender, lHint, pHint);	// notify main frame
}

void CFauveDoc::GetScaledCropRect(const CRect& rBox, CRect& rDest) const
{
	if (m_rCrop.IsRectNull()) {
		rDest.SetRectEmpty();
	} else {
		CSize	szIn(m_dibIn.GetSize());
		double	fLeft = double(m_rCrop.left) / szIn.cx;
		double	fTop = double(m_rCrop.top) / szIn.cy;
		double	fRight = double(m_rCrop.right) / szIn.cx;
		double	fBottom = double(m_rCrop.bottom) / szIn.cy;
		CSize	szBox(rBox.Size());
		rDest = CRect(
			Round(fLeft * szBox.cx) + rBox.left, 
			Round(fTop * szBox.cy) + rBox.top, 
			Round(fRight * szBox.cx) + rBox.left, 
			Round(fBottom * szBox.cy) + rBox.top);
	}
}

void CFauveDoc::SetScaledCropRect(const CRect& rBox, const CRect& rSrc, CView *pSender)
{
	NotifyUndoableEdit(0, UCODE_CROP);
	if (rSrc.IsRectNull()) {
		m_rCrop.SetRectEmpty();
	} else {
		CSize	szBox = rBox.Size();
		double	fLeft = double(rSrc.left - rBox.left) / szBox.cx;
		double	fTop = double(rSrc.top - rBox.top) / szBox.cy;
		double	fRight = double(rSrc.right - rBox.left) / szBox.cx;
		double	fBottom = double(rSrc.bottom - rBox.top) / szBox.cy;
		CSize	szIn = m_dibIn.GetSize();
		m_rCrop = CRect(
			Round(fLeft * szIn.cx), 
			Round(fTop * szIn.cy), 
			Round(fRight * szIn.cx), 
			Round(fBottom * szIn.cy));
	}
	SetModifiedFlag();
	UpdateFauve();
	UpdateAllViews(pSender, HINT_CROP);
}

bool CFauveDoc::SetCrop(const CRect& rCrop, CView *pSender)
{
	CRect	rImage(CPoint(0, 0), m_dibIn.GetSize());
	CRect	rNew;
	if (rCrop.IsRectEmpty()) {	// if no cropping
		rNew.SetRectEmpty();
	} else {	// cropping specified
		if (!rNew.IntersectRect(rCrop, rImage))	// if cropping doesn't intersect image
			return false;
		if (rNew == rImage)	// if cropping equals image
			rNew.SetRectEmpty();	// same as no cropping
	}
	if (rNew == m_rCrop)	// if cropping unchanged
		return true;	// nothing to do
	NotifyUndoableEdit(0, UCODE_CROP);
	m_rCrop = rNew;
	SetModifiedFlag();
	UpdateFauve();
	UpdateAllViews(pSender, HINT_CROP);
	return true;
}

void CFauveDoc::SetLevel(int iMark, BYTE nLevel, CView *pSender)
{
	ASSERT(iMark >= 0 && iMark < LUMA_LIMITS);
	if (nLevel == m_arrLuma[iMark])
		return;
	NotifyUndoableEdit(iMark, UCODE_LEVELS, UE_COALESCE);
	m_arrLuma[iMark] = nLevel;
	UpdateFauve();
	SetModifiedFlag();
	UpdateAllViews(pSender, HINT_LEVELS);
}

void CFauveDoc::SetLevels(BYTE nLumaMin, BYTE nLumaMax, CView *pSender)
{
	ASSERT(nLumaMin < nLumaMax);
	NotifyUndoableEdit(-1, UCODE_LEVELS);
	m_arrLuma[L_BLACK] = nLumaMin;
	m_arrLuma[L_WHITE] = nLumaMax;
	UpdateFauve();
	SetModifiedFlag();
	UpdateAllViews(pSender, HINT_LEVELS);
}

void CFauveDoc::SetHue(int iChannel, BYTE nHueShift, CView *pSender)
{
	ASSERT(iChannel >= 0 && iChannel < COLOR_CHANNELS);
	if (nHueShift == m_arrHue[iChannel])
		return;
	NotifyUndoableEdit(iChannel, UCODE_HUE, UE_COALESCE);
	m_arrHue[iChannel] = nHueShift;
	UpdateFauve(true);	// reuse histogram
	SetModifiedFlag();
	UpdateAllViews(pSender, CFauveDoc::HINT_HUE);
}

void CFauveDoc::SetHues(BYTE nRed, BYTE nGreen, BYTE nBlue, CView *pSender)
{
	NotifyUndoableEdit(-1, UCODE_HUE);
	m_arrHue[C_R] = nRed;
	m_arrHue[C_G] = nGreen;
	m_arrHue[C_B] = nBlue;
	UpdateFauve(true);	// reuse histogram
	SetModifiedFlag();
	UpdateAllViews(pSender, CFauveDoc::HINT_HUE);
}

bool CFauveDoc::SetSourceImage(CString& sPath)
{
	if (!ReadSourceImage(sPath))	// if can't read source image
		return false;
	NotifyUndoableEdit(-1, UCODE_SOURCE_IMAGE);
	SetModifiedFlag();
	UpdateAllViews(NULL);
	m_sOrigPath = sPath;	// update data member
	return true;
}

void CFauveDoc::Rotate(int nQuadrants)
{
	NotifyUndoableEdit(0, UCODE_ROTATE);
	RotateDibFast(m_dibIn, nQuadrants);
	RotateSubrect(m_rCrop, m_dibIn.GetSize(), nQuadrants);
	m_nDibRot = (m_nDibRot + nQuadrants) & 3;
	SetModifiedFlag();
	UpdateFauve();
	UpdateAllViews(NULL, HINT_ROTATE);
}

void CFauveDoc::UpdateFauve(bool bReuseHistogram)
{
	CRect	rCrop;
	if (m_rCrop.IsRectNull())
		rCrop = CRect(CPoint(0, 0), m_dibIn.GetSize());
	else
		rCrop = m_rCrop;
	CSize	szCrop(rCrop.Size());
	// create output DIB if it's empty or crop size changed
	if (m_dibOut.IsEmpty() || m_dibOut.GetSize() != szCrop) {
		if (!m_dibOut.Create(szCrop.cx, szCrop.cy, BPP_32))
			AfxThrowInvalidArgException();
	}
	CFauve::m_rCrop = rCrop;	// copy to base class rectangle
//	BENCH_START
	if (theApp.m_options.UseThreads()) {	// if multithreading
		theApp.m_LoopTiler.m_pTargetDoc = this;
		FauveMulti(bReuseHistogram);
	} else {	// single threaded
		FauveFast(bReuseHistogram);
	}
//	BENCH_STOP
}

void CFauveDoc::UpdateHistogram()
{
	ZeroMemory(m_histogram.m_arrLuma, sizeof(m_histogram.m_arrLuma));
	CSize	szIn = m_dibIn.GetSize();
	const DWORD *pInPixel = static_cast<DWORD *>(m_dibIn.GetBits());
	int	nPixels = szIn.cy * szIn.cx;
	for (int iPixel = 0; iPixel < nPixels; iPixel++) {
		DWORD	clr = *pInPixel++;
		BYTE	r = GET_XRGB_R(clr);
		BYTE	g = GET_XRGB_G(clr);
		BYTE	b = GET_XRGB_B(clr);
		DWORD	nLuma = Round((r + g + b) / 3.0);
		m_histogram.m_arrLuma[nLuma]++;
	}
	const int	nVals = _countof(m_histogram.m_arrLuma);
	DWORD	*pSample = m_histogram.m_arrLuma;
	DWORD	nMaxVal = 0;
	for (int iVal = 0; iVal < nVals; iVal++) {
		if (pSample[iVal] > nMaxVal)
			nMaxVal = pSample[iVal];
	}
	m_histogram.m_nLumaMax = nMaxVal;
}

void CFauveDoc::RotateDibRef(const CDibEx& dibIn, CDibEx& dibOut, int nQuadrants)
{
	nQuadrants &= 3;	// wrap quadrant count; positive is counterclockwise
	if (!nQuadrants)	// if zero quadrants
		return;	// nothing to do
	CSize	sz = dibIn.GetSize();
	CSize	szRot;
	if (nQuadrants & 1) {	// if rotating by odd number of quadrants
		szRot = CSize(sz.cy, sz.cx);	// flip axes
	} else {
		szRot = sz;
	}
	if (!dibOut.Create(szRot.cx, szRot.cy, BPP_32))
		AfxThrowInvalidArgException();
	switch (nQuadrants) {
	case 1:	// rotate 90 degrees CCW
		for (int y = 0; y < sz.cy; y++) {
			for (int x = 0; x < sz.cx; x++) {
				dibOut.SetPixel(y, sz.cx - 1 - x, dibIn.GetPixel(x, y));
			}
		}
		break;
	case 2:	// rotate 180 degrees CCW
		for (int y = 0; y < sz.cy; y++) {
			for (int x = 0; x < sz.cx; x++) {
				dibOut.SetPixel(sz.cx - 1 - x, sz.cy - 1 - y, dibIn.GetPixel(x, y));
			}
		}
		break;
	case 3:	// rotate 270 degrees CCW
		for (int y = 0; y < sz.cy; y++) {
			for (int x = 0; x < sz.cx; x++) {
				dibOut.SetPixel(sz.cy - 1 - y, x, dibIn.GetPixel(x, y));
			}
		}
		break;
	}
}

void CFauveDoc::RotateDibRef(CDibEx& dib, int nQuadrants)
{
	CDibEx	dibRot;
	RotateDibRef(dib, dibRot, nQuadrants);
	dib = dibRot;
}

void CFauveDoc::RotateDibFast(const CDibEx& dibIn, CDibEx& dibOut, int nQuadrants)
{
	ASSERT(dibIn.GetPixelFormat() == CDibEx::PF_BPP32);	// 32-bit color depth only
	nQuadrants &= 3;	// wrap quadrant count; positive is counterclockwise
	if (!nQuadrants)	// if zero quadrants
		return;	// nothing to do
	CSize	sz = dibIn.GetSize();
	CSize	szRot;
	if (nQuadrants & 1) {	// if rotating by odd number of quadrants
		szRot = CSize(sz.cy, sz.cx);	// flip axes
	} else {
		szRot = sz;
	}
	if (!dibOut.Create(szRot.cx, szRot.cy, BPP_32))
		AfxThrowInvalidArgException();
	// input pixels are read sequentially from low to high address
	const DWORD	*pIn = reinterpret_cast<DWORD *>(dibIn.GetBits());
	DWORD *pOut = reinterpret_cast<DWORD *>(dibOut.GetBits());
	UINT	nOutStride = dibOut.GetStride() >> 2;	// convert from bytes to dwords
	switch (nQuadrants) {
	case 1:	// rotate 90 degrees CCW
		pOut += nOutStride;
		for (int y = 0; y < sz.cy; y++) {
			pOut--;
			DWORD	*pOut2 = pOut;
			for (int x = 0; x < sz.cx; x++) {
				*pOut2 = *pIn++;
				pOut2 += nOutStride;
			}
		}
		break;
	case 2:	// rotate 180 degrees CCW
		pOut += sz.cy * nOutStride;
		for (int y = 0; y < sz.cy; y++) {
			for (int x = 0; x < sz.cx; x++) {
				*--pOut = *pIn++;
			}
		}
		break;
	case 3:	// rotate 270 degrees CCW
		pOut += sz.cx * nOutStride;
		for (int y = 0; y < sz.cy; y++) {
			DWORD	*pOut2 = pOut;
			for (int x = 0; x < sz.cx; x++) {
				pOut2 -= nOutStride;
				*pOut2 = *pIn++;
			}
			pOut++;
		}
		break;
	}
}

void CFauveDoc::RotateDibFast(CDibEx& dib, int nQuadrants)
{
	CDibEx	dibRot;
	RotateDibFast(dib, dibRot, nQuadrants);
	dib = dibRot;
}

void CFauveDoc::RotateSubrect(CRect& rect, const CSize& szParent, int nQuadrants)
{
	if (rect.IsRectEmpty())	// if rectangle is empty
		return;	// nothing to do
	CSize	sz(rect.Size());
	CPoint	pt;
	switch (nQuadrants & 3) {
	case 1:	// rotate 90 degrees CCW
		pt = CPoint(rect.top, szParent.cy - rect.left - sz.cx);
		rect = CRect(pt, CSize(sz.cy, sz.cx));	// swap axes of size
		break;
	case 2:	// rotate 180 degrees CCW
		pt = CPoint(szParent.cx - rect.left - sz.cx, szParent.cy - rect.top - sz.cy);
		rect = CRect(pt, sz);
		break;
	case 3:	// rotate 270 degrees CCW
		pt = CPoint(szParent.cx - rect.top - sz.cy, rect.left);
		rect = CRect(pt, CSize(sz.cy, sz.cx));	// swap axes of size
		break;
	}
}

// CFauveDoc undo

void CFauveDoc::SaveCrop(CUndoState& State)
{
	CRefPtr<CUndoCrop>	pInfo;
	pInfo.CreateObj();
	pInfo->m_rCrop = m_rCrop;
	State.SetObj(pInfo);
}

void CFauveDoc::RestoreCrop(const CUndoState& State)
{
	const CUndoCrop	*pInfo = static_cast<CUndoCrop *>(State.GetObj());
	m_rCrop = pInfo->m_rCrop;
	UpdateFauve();
	UpdateAllViews(NULL, HINT_CROP);
}

void CFauveDoc::SaveRotate(CUndoState& State)
{
	State.m_Val.p.x.c.al = m_nDibRot;
}

void CFauveDoc::RestoreRotate(const CUndoState& State)
{
	int nPrevRot = State.m_Val.p.x.c.al;
	int	nDeltaRot = (nPrevRot - m_nDibRot) & 3;
	m_nDibRot = static_cast<BYTE>(nPrevRot);
	RotateDibFast(m_dibIn, nDeltaRot);
	RotateSubrect(m_rCrop, m_dibIn.GetSize(), nDeltaRot);
	UpdateFauve();
	UpdateAllViews(NULL, HINT_ROTATE);
}

void CFauveDoc::SaveLevels(CUndoState& State)
{
	State.m_Val.p.x.c.al = m_arrLuma[L_BLACK];
	State.m_Val.p.x.c.ah = m_arrLuma[L_WHITE];
}

void CFauveDoc::RestoreLevels(const CUndoState& State)
{
	m_arrLuma[L_BLACK] = State.m_Val.p.x.c.al;
	m_arrLuma[L_WHITE] = State.m_Val.p.x.c.ah;
	UpdateFauve();
	UpdateAllViews(NULL, HINT_LEVELS);
}

void CFauveDoc::SaveHue(CUndoState& State)
{
	State.m_Val.p.x.c.al = m_arrHue[C_R];
	State.m_Val.p.x.c.ah = m_arrHue[C_G];
	State.m_Val.p.x.c.bl = m_arrHue[C_B];
}

void CFauveDoc::RestoreHue(const CUndoState& State)
{
	m_arrHue[C_R] = State.m_Val.p.x.c.al;
	m_arrHue[C_G] = State.m_Val.p.x.c.ah;
	m_arrHue[C_B] = State.m_Val.p.x.c.bl;
	UpdateFauve(true);	// reuse histogram
	UpdateAllViews(NULL, HINT_HUE);
}

void CFauveDoc::SaveSourceImage(CUndoState& State)
{
	CRefPtr<CUndoState::CRefString>	pStr;
	pStr.CreateObj();
	pStr->m_str = m_sOrigPath;
	State.SetObj(pStr);
}

void CFauveDoc::RestoreSourceImage(const CUndoState& State)
{
	CUndoState::CRefString	*pStr = static_cast<CUndoState::CRefString*>(State.GetObj());
	SetSourceImage(pStr->m_str);
}

void CFauveDoc::SaveUndoState(CUndoState& State)
{
	switch (State.GetCode()) {
	case UCODE_CROP:
		SaveCrop(State);
		break;
	case UCODE_ROTATE:
		SaveRotate(State);
		break;
	case UCODE_LEVELS:
		SaveLevels(State);
		break;
	case UCODE_HUE:
		SaveHue(State);
		break;
	case UCODE_SOURCE_IMAGE:
		SaveSourceImage(State);
		break;
	}
}

void CFauveDoc::RestoreUndoState(const CUndoState& State)
{
	switch (State.GetCode()) {
	case UCODE_CROP:
		RestoreCrop(State);
		break;
	case UCODE_ROTATE:
		RestoreRotate(State);
		break;
	case UCODE_LEVELS:
		RestoreLevels(State);
		break;
	case UCODE_HUE:
		RestoreHue(State);
		break;
	case UCODE_SOURCE_IMAGE:
		RestoreSourceImage(State);
		break;
	}
}

CString	CFauveDoc::GetUndoTitle(const CUndoState& State)
{
	CString	sTitle;
	int	nUndoCode = State.GetCode();
	if (nUndoCode >= 0 && nUndoCode < UNDO_CODES) {
		sTitle.LoadString(m_arrUndoTitleId[nUndoCode]);
	}
	return sTitle;
}

CFauveDoc::CMyUndoManager::CMyUndoManager()
{
	if (m_sUndoPrefix.IsEmpty()) {	// if prefixes not loaded yet
		m_sUndoPrefix.LoadString(IDS_EDIT_UNDO_PREFIX);
		m_sUndoPrefix += ' ';	// add separator
		m_sRedoPrefix.LoadString(IDS_EDIT_REDO_PREFIX);
		m_sRedoPrefix += ' ';	// add separator
	}
	OnUpdateTitles();
}

void CFauveDoc::CMyUndoManager::OnUpdateTitles()
{
	// append here instead of in undo/redo update command UI handlers,
	// to reduce high-frequency memory reallocation when mouse moves
	m_sUndoMenuItem = m_sUndoPrefix + GetUndoTitle();
	m_sRedoMenuItem = m_sRedoPrefix + GetRedoTitle();
}

// CFauveDoc serialization

void CFauveDoc::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		// TODO: add storing code here
	}
	else
	{
		// TODO: add loading code here
	}
}

#ifdef SHARED_HANDLERS

// Support for thumbnails
void CFauveDoc::OnDrawThumbnail(CDC& dc, LPRECT lprcBounds)
{
	// Modify this code to draw the document's data
	dc.FillSolidRect(lprcBounds, RGB(255, 255, 255));

	CString strText = _T("TODO: implement thumbnail drawing here");
	LOGFONT lf;

	CFont* pDefaultGUIFont = CFont::FromHandle((HFONT) GetStockObject(DEFAULT_GUI_FONT));
	pDefaultGUIFont->GetLogFont(&lf);
	lf.lfHeight = 36;

	CFont fontDraw;
	fontDraw.CreateFontIndirect(&lf);

	CFont* pOldFont = dc.SelectObject(&fontDraw);
	dc.DrawText(strText, lprcBounds, DT_CENTER | DT_WORDBREAK);
	dc.SelectObject(pOldFont);
}

// Support for Search Handlers
void CFauveDoc::InitializeSearchContent()
{
	CString strSearchContent;
	// Set search contents from document's data. 
	// The content parts should be separated by ";"

	// For example:  strSearchContent = _T("point;rectangle;circle;ole object;");
	SetSearchContent(strSearchContent);
}

void CFauveDoc::SetSearchContent(const CString& value)
{
	if (value.IsEmpty())
	{
		RemoveChunk(PKEY_Search_Contents.fmtid, PKEY_Search_Contents.pid);
	}
	else
	{
		CMFCFilterChunkValueImpl *pChunk = NULL;
		ATLTRY(pChunk = new CMFCFilterChunkValueImpl);
		if (pChunk != NULL)
		{
			pChunk->SetTextValue(PKEY_Search_Contents, value, CHUNK_TEXT);
			SetChunkValue(pChunk);
		}
	}
}

#endif // SHARED_HANDLERS

// CFauveDoc diagnostics

#ifdef _DEBUG
void CFauveDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CFauveDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG

BEGIN_MESSAGE_MAP(CFauveDoc, CDocument)
	ON_COMMAND(ID_EDIT_UNDO, OnEditUndo)
	ON_COMMAND(ID_EDIT_REDO, OnEditRedo)
	ON_UPDATE_COMMAND_UI(ID_EDIT_UNDO, OnUpdateEditUndo)
	ON_UPDATE_COMMAND_UI(ID_EDIT_REDO, OnUpdateEditRedo)
	ON_COMMAND(ID_EDIT_ROTATE_CW, OnEditRotateCW)
	ON_COMMAND(ID_EDIT_CROP, OnEditCrop)
	ON_COMMAND(ID_EDIT_LEVELS, OnEditLevels)
	ON_COMMAND(ID_EDIT_HUE, OnEditHue)
	ON_COMMAND(ID_FILE_EXPORT_VIDEO,OnFileExportVideo)
	ON_COMMAND(ID_EDIT_SOURCE_IMAGE, OnEditSourceImage)
END_MESSAGE_MAP()

void CFauveDoc::OnEditRotateCW()
{
	Rotate(-1);
}

void CFauveDoc::OnEditUndo()
{
	m_undoMgr.Undo();
}

void CFauveDoc::OnEditRedo()
{
	m_undoMgr.Redo();
}

void CFauveDoc::OnUpdateEditUndo(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(m_undoMgr.CanUndo());
	pCmdUI->SetText(m_undoMgr.m_sUndoMenuItem);
}

void CFauveDoc::OnUpdateEditRedo(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(m_undoMgr.CanRedo());
	pCmdUI->SetText(m_undoMgr.m_sRedoMenuItem);
}

void CFauveDoc::OnEditCrop()
{
	CCropDlg	dlg;
	if (m_rCrop.IsRectEmpty()) {
		dlg.m_point = CPoint(0, 0);
		dlg.m_size = m_dibIn.GetSize();
	} else {
		dlg.m_point = m_rCrop.TopLeft();
		dlg.m_size = m_rCrop.Size();
	}
	if (dlg.DoModal() == IDOK) {
		SetCrop(CRect(dlg.m_point, dlg.m_size));
	}
}

void CFauveDoc::OnEditLevels()
{
	CLevelsDlg	dlg;
	dlg.m_nMin = m_arrLuma[L_BLACK];
	dlg.m_nMax = m_arrLuma[L_WHITE];
	if (dlg.DoModal() == IDOK) {
		SetLevels(static_cast<BYTE>(dlg.m_nMin), static_cast<BYTE>(dlg.m_nMax));		
	}
}

void CFauveDoc::OnEditHue()
{
	CHueDlg	dlg;	
	dlg.m_nRed = m_arrHue[C_R];
	dlg.m_nGreen = m_arrHue[C_G];
	dlg.m_nBlue = m_arrHue[C_B];
	if (dlg.DoModal() == IDOK) {
		SetHues(static_cast<BYTE>(dlg.m_nRed), static_cast<BYTE>(dlg.m_nGreen), static_cast<BYTE>(dlg.m_nBlue));
	}
}

bool CFauveDoc::PromptSourceImagePath(CString& sImgPath)
{
	CPathDlg	dlg;
	dlg.m_sPath = sImgPath;
	dlg.m_sCaption.LoadString(IDS_SOURCE_DLG_CAPTION);
	if (dlg.DoModal() != IDOK)
		return false;
	sImgPath = dlg.m_sPath;
	return true;
}

void CFauveDoc::OnEditSourceImage()
{
	CString	sNewPath(m_sOrigPath);
	if (PromptSourceImagePath(sNewPath))	// if user doesn't cancel
		SetSourceImage(sNewPath);
}

bool CFauveDoc::ExportVideo()
{
	UINT	nFlags = BIF_USENEWUI | BIF_RETURNONLYFSDIRS;
	CString	sTitle(_T("Export Video"));
	CPathStr	sOutFolder(CPersist::GetString(REG_SETTINGS, RK_EXPORT_VIDEO_FOLDER));
	if (!CFolderDialog::BrowseFolder(sTitle, sOutFolder, NULL, nFlags, sOutFolder))
		return false;
	CPersist::WriteString(REG_SETTINGS, RK_EXPORT_VIDEO_FOLDER, sOutFolder);
	CRecordDlg	dlgRec;
	dlgRec.m_nFrameWidth = m_dibOut.GetSize().cx;
	dlgRec.m_nFrameHeight = m_dibOut.GetSize().cy;
	if (dlgRec.DoModal() != IDOK)
		return false;
	if (dlgRec.m_nFrameWidth != m_dibOut.GetSize().cx || dlgRec.m_nFrameHeight != m_dibOut.GetSize().cy) {
		AfxMessageBox(IDS_DOC_ERR_CANT_RESIZE);
		return false;
	}
	CString	sFileName;
	sFileName.Format(_T("img%05d.png"), 0);
	CPathStr	sFirstFramePath(sOutFolder);
	sFirstFramePath.Append(sFileName);
	if (PathFileExists(sFirstFramePath)) {	// if first frame already exists
		if (AfxMessageBox(IDS_PHASE_EXPORT_OVERWRITE_WARN, MB_YESNO | MB_DEFBUTTON2 | MB_ICONWARNING) != IDYES)
			return false;
	}
	int	nFrames = dlgRec.m_nDurationFrames;
	CProgressDlg	dlgProgress;
	if (!dlgProgress.Create())
		AfxThrowNotSupportedException();
	dlgProgress.SetRange(0, nFrames);
	BYTE	arrHueSave[3];
	memcpy(arrHueSave, m_arrHue, sizeof(m_arrHue));
	int	iFrame;
	for (iFrame = 0; iFrame < nFrames; iFrame++) {
		dlgProgress.SetPos(iFrame);
		if (dlgProgress.Canceled()) {
			break;
		}
		double	fScale = iFrame / theApp.m_options.m_fAnimationFrameRate;
		for (int iChan = 0; iChan < 3; iChan++) {
			m_arrHue[iChan] = arrHueSave[iChan] + BYTE(Round(theApp.m_options.m_fAnimationHueRate[iChan] * fScale));
		}
		UpdateFauve();
		UpdateAllViews(NULL, CFauveDoc::HINT_HUE);
		// convert from 32 to 24 bits per pixel
		CDibEx	dibTmp;
		CSize	sz(m_dibOut.GetSize());
		if (!dibTmp.Create(sz.cx, sz.cy, BPP_24)) {
			AfxMessageBox(IDS_DOC_ERR_CANT_CREATE_DIB);
			break;
		}
		for (int y = 0; y < sz.cy; y++) {
			for (int x = 0; x < sz.cx; x++) {
				dibTmp.SetPixel(x, y, m_dibOut.GetPixel(x, y));
			}
		}
		CImage	image;
		image.Attach(dibTmp);
		sFileName.Format(_T("img%05d.png"), iFrame);
		CPathStr	sOutPath(sOutFolder);
		sOutPath.Append(sFileName);
		HRESULT	hr = image.Save(sOutPath, Gdiplus::ImageFormatPNG);
		image.Detach();	// detach before returning
		if (FAILED(hr)) {
			AfxMessageBox(IDS_DOC_ERR_CANT_SAVE_PNG);
			break;
		}
	}
	memcpy(m_arrHue, arrHueSave, sizeof(m_arrHue));
	UpdateFauve();
	UpdateAllViews(NULL, CFauveDoc::HINT_HUE);
	return iFrame >= nFrames;
}

void CFauveDoc::OnFileExportVideo()
{
	ExportVideo();
}
