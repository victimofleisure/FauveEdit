// Copyleft 2022 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00		24sep22	initial version
		01		10jan25	add option to reuse histogram
		02		23jan25	apply hue shift to bins instead of pixels

*/

#include "stdafx.h"
#include "Fauve.h"

void CFauve::FauveRef(bool bReuseHistogram)
{
	ASSERT(!m_dibIn.IsEmpty());
	ASSERT(!m_dibOut.IsEmpty());
	ASSERT(m_rCrop.Size() == m_dibOut.GetSize());
	ASSERT(m_arrLuma[L_BLACK] < m_arrLuma[L_WHITE]);
	CSize	sz(m_rCrop.Size());
	if (!bReuseHistogram) {
		ZeroMemory(m_arrBin, sizeof(m_arrBin));
		int	nLumaMin = m_arrLuma[L_BLACK] * 3 - 1;	// matches Round((r + g + b) / 3.0)
		int	nLumaMax = m_arrLuma[L_WHITE] * 3 + 1;
		for (int y = m_rCrop.top; y < m_rCrop.bottom; y++) {	// for each input row
			for (int x = m_rCrop.left; x < m_rCrop.right; x++) {	// for each input column
				UINT	clr = m_dibIn.GetPixel(x, y);	// get input pixel
				BYTE	r = GET_XRGB_R(clr);
				BYTE	g = GET_XRGB_G(clr);
				BYTE	b = GET_XRGB_B(clr);
				int	nLuma = r + g + b;	// luma range scaled to avoid divide here
				if (nLuma >= nLumaMin && nLuma <= nLumaMax) {	// if luma in range
					m_arrBin[C_R][r]++;	// update histogram bins
					m_arrBin[C_G][g]++;
					m_arrBin[C_B][b]++;
				}
			}
		}
		for (int iChan = 0; iChan < COLOR_CHANNELS; iChan++) {	// for each color channel
			UINT	*pSample = m_arrBin[iChan];
			UINT	nMaxVal = 0;
			int	iVal;
			for (iVal = 0; iVal < COLOR_VALUES; iVal++) {	// for each color value
				if (pSample[iVal] > nMaxVal)	// if sample exceeds maximum
					nMaxVal = pSample[iVal];	// update maximum
			}
			for (iVal = 0; iVal < COLOR_VALUES; iVal++) {	// for each color value
				pSample[iVal] = Round(double(pSample[iVal]) / nMaxVal * 0xff);	// normalize sample
			}
		}
	}
	for (int y = 0; y < sz.cy; y++) {	// for each output row
		for (int x = 0; x < sz.cx; x++) {	// for each output column
			UINT	clr = m_dibIn.GetPixel(m_rCrop.left + x, m_rCrop.top + y);	// get input pixel
			BYTE	r = GET_XRGB_R(clr) + m_arrHue[C_R];	// apply hue shift
			BYTE	g = GET_XRGB_G(clr) + m_arrHue[C_G];
			BYTE	b = GET_XRGB_B(clr) + m_arrHue[C_B];
			clr = MAKE_XRGB(m_arrBin[C_R][r], m_arrBin[C_G][g], m_arrBin[C_B][b]);
			m_dibOut.SetPixel(x, y, clr);	// set output pixel
		}
	}
}

// Both bitmaps must be bottom-up and use 32 bits per pixel in XRGB format.
void CFauve::FauveFast(bool bReuseHistogram)
{
	ASSERT(!m_dibIn.IsEmpty());
	ASSERT(!m_dibOut.IsEmpty());
	ASSERT(m_rCrop.Size() == m_dibOut.GetSize());
	ASSERT(m_rCrop.bottom <= m_dibIn.GetSize().cy);
	ASSERT(m_arrLuma[L_BLACK] < m_arrLuma[L_WHITE]);
	ASSERT(m_dibIn.GetPixelFormat() == CDibEx::PF_BPP32);	// 32-bit color depth only
	ASSERT(m_dibOut.GetPixelFormat() == CDibEx::PF_BPP32);	// 32-bit color depth only
	CSize	sz(m_rCrop.Size());
	const UINT	*pInRow = static_cast<UINT *>(m_dibIn.GetBits());
	UINT	nInStride = m_dibIn.GetStride() / sizeof(UINT);
	UINT	iStartPixel = (m_dibIn.GetSize().cy - m_rCrop.bottom) * nInStride;	// bottom-up
	pInRow += iStartPixel;
	const UINT	*pInStartRow = pInRow;
	UINT	*pBin, *pBinEnd;	// declare here to avoid compiler warnings
	if (!bReuseHistogram) {
		pBin = m_arrBin[0];
		pBinEnd = pBin + HISTOGRAM_BINS;
		while (pBin < pBinEnd) {
			*pBin++ = 0;
		}
		int	nLumaMin = m_arrLuma[L_BLACK] * 3 - 1;	// matches Round((r + g + b) / 3.0)
		int	nLumaMax = m_arrLuma[L_WHITE] * 3 + 1;
		int	y;
		for (y = 0; y < sz.cy; y++) {	// for each input row
			const UINT	*pInPixel = pInRow + m_rCrop.left;
			const UINT	*pInPixelEnd = pInPixel + sz.cx;
			while (pInPixel < pInPixelEnd) {	// for each input column
				UINT	clr = *pInPixel++;	// get input pixel
				BYTE	r = GET_XRGB_R(clr);
				BYTE	g = GET_XRGB_G(clr);
				BYTE	b = GET_XRGB_B(clr);
				int	nLuma = r + g + b;	// luma range scaled to avoid divide here
				if (nLuma >= nLumaMin && nLuma <= nLumaMax) {	// if luma in range
					m_arrBin[C_R][r]++;	// update histogram bins
					m_arrBin[C_G][g]++;
					m_arrBin[C_B][b]++;
				}
			}
			pInRow += nInStride;
		}
		for (int iChan = 0; iChan < COLOR_CHANNELS; iChan++) {	// for each color channel
			pBin = m_arrBin[iChan];
			pBinEnd = pBin + COLOR_VALUES;
			UINT	nMaxVal = 0;
			while (pBin < pBinEnd) {	// for each color value
				if (*pBin > nMaxVal)	// if sample exceeds maximum
					nMaxVal = *pBin;	// update maximum
				pBin++;
			}
			pBin = m_arrBin[iChan];
			while (pBin < pBinEnd) {	// for each color value
				*pBin++ = Round(double(*pBin) / nMaxVal * 0xff);	// normalize sample
			}
		}
	}
	// apply hue shift to bins; assume fewer bins than output pixels
	for (int iChan = 0; iChan < COLOR_CHANNELS; iChan++) {	// for each color channel
		const UINT	*pBinFirst = m_arrBin[iChan];
		const UINT	*pBinLast = pBinFirst + COLOR_VALUES;
		const UINT	*pBinTarget = pBinFirst + m_arrHue[iChan];
		const UINT	*pBinCur = pBinTarget;
		UINT	*pIdx = m_arrIdx[iChan];
		while (pBinCur < pBinLast) {	// copy from target to last
			*pIdx++ = *pBinCur++;
		}
		pBinCur = pBinFirst;
		while (pBinCur < pBinTarget) {	// copy from first to target
			*pIdx++ = *pBinCur++;
		}
	}
	pInRow = pInStartRow;
	UINT	*pOutPixel = static_cast<UINT *>(m_dibOut.GetBits());
	for (int y = 0; y < sz.cy; y++) {	// for each output row
		const UINT	*pInPixel = pInRow + m_rCrop.left;
		const UINT	*pInPixelEnd = pInPixel + sz.cx;
		while (pInPixel < pInPixelEnd) {	// for each input column
			UINT	clr = *pInPixel++;	// get input pixel
			BYTE	r = GET_XRGB_R(clr);
			BYTE	g = GET_XRGB_G(clr);
			BYTE	b = GET_XRGB_B(clr);
			clr = MAKE_XRGB(m_arrIdx[C_R][r], m_arrIdx[C_G][g], m_arrIdx[C_B][b]);
			*pOutPixel++ = clr;	// set output pixel
		}
		pInRow += nInStride;
	}
}
