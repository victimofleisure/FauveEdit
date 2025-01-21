// Copyleft 2022 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00		26nov22	initial version
		01		10jan25	add option to reuse histogram

*/

#include "stdafx.h"
#include "FauveEdit.h"
#include "FauveTiled.h"

CFauveTiled::CFauveTiled()
{
	m_nPrevHeight = 0;
	m_nCropWidth = 0;
	m_nInStride = 0;
	m_nOutStride = 0;
	m_nRenderStage = RST_HISTOGRAM;
	m_bReuseHistogram = false;
}

void CFauveTiled::FauveMulti(bool bReuseHistogram)
{
	ASSERT(!m_dibIn.IsEmpty());
	ASSERT(!m_dibOut.IsEmpty());
	ASSERT(m_rCrop.Size() == m_dibOut.GetSize());
	ASSERT(m_arrLuma[L_BLACK] < m_arrLuma[L_WHITE]);
	ASSERT(m_dibIn.GetPixelFormat() == CDibEx::PF_BPP32);	// 32-bit color depth only
	ASSERT(m_dibOut.GetPixelFormat() == CDibEx::PF_BPP32);	// 32-bit color depth only
	int	nThreads = theApp.m_LoopTiler.GetThreadCount();
	int	nHeight = m_rCrop.Height();
	m_bReuseHistogram = bReuseHistogram;
	if (nThreads != m_arrTile.GetSize() || nHeight != m_nPrevHeight) {
		ASSERT(!bReuseHistogram);
		m_arrTile.SetSize(nThreads);
		m_nPrevHeight = nHeight;
		int	nRowsPerTile = nHeight / nThreads;
		int	nRemainder = nHeight % nThreads;
		int	nStart = 0;
		for (int iThread = 0; iThread < nThreads; iThread++) {
			TILE&	tile = m_arrTile[iThread];
			int	nRows = nRowsPerTile;
			if (nRemainder > 0) {
				nRows++;
				nRemainder--;
			}
			tile.nStart = nStart;
			tile.nRows = nRows;
			nStart += nRows;
		}
	}
	UINT	*pBin, *pBinEnd;	// declare here to avoid compiler warnings
	if (!bReuseHistogram) {
		m_nCropWidth = m_rCrop.Width();
		m_nInStride = m_dibIn.GetStride() / sizeof(UINT);
		m_nOutStride = m_dibOut.GetStride() / sizeof(UINT);
		m_nRenderStage = RST_HISTOGRAM;
		theApp.m_LoopTiler.Run();	// compute histogram for each tile in parallel
		// sum per-tile histograms
		pBin = m_arrBin[0];
		pBinEnd = pBin + HISTOGRAM_BINS;
		while (pBin < pBinEnd) {
			*pBin++ = 0;
		}
		for (int iThread = 0; iThread < nThreads; iThread++) {
			UINT	*pBinIn = m_arrTile[iThread].arrBin[0];
			UINT	*pBinOut = m_arrBin[0];
			pBinEnd = pBinIn + HISTOGRAM_BINS;
			while (pBinIn < pBinEnd) {
				*pBinOut++ += *pBinIn++;
			}
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
	m_nRenderStage = RST_TRANSLATE;
	theApp.m_LoopTiler.Run();	// compute output pixels for each tile in parallel
}

void CFauveTiled::FauveCallback(int iThread)
{
	TILE&	tile = m_arrTile[iThread];
	int	nCols = m_nCropWidth;
	int	nInStride = m_nInStride;
	const UINT	*pInRow = static_cast<UINT *>(m_dibIn.GetBits());
	pInRow += (m_dibIn.GetSize().cy - m_rCrop.bottom + tile.nStart) * nInStride;	// bottom-up
	tile.pInRow = pInRow;	// save for second pass
	if (m_nRenderStage == RST_HISTOGRAM) {	// if render stage is histogram
		UINT	*pBin = tile.arrBin[0];
		UINT	*pBinEnd = pBin + HISTOGRAM_BINS;
		while (pBin < pBinEnd) {
			*pBin++ = 0;
		}
		int	nLumaMin = m_arrLuma[L_BLACK] * 3 - 1;	// matches Round((r + g + b) / 3.0)
		int	nLumaMax = m_arrLuma[L_WHITE] * 3 + 1;
		for (int y = 0; y < tile.nRows; y++) {	// for each input row
			const UINT	*pInPixel = pInRow + m_rCrop.left;
			const UINT	*pInPixelEnd = pInPixel + nCols;
			while (pInPixel < pInPixelEnd) {	// for each input column
				UINT	clr = *pInPixel++;	// get input pixel
				BYTE	r = GET_XRGB_R(clr);
				BYTE	g = GET_XRGB_G(clr);
				BYTE	b = GET_XRGB_B(clr);
				int	nLuma = r + g + b;	// luma range scaled to avoid divide here
				if (nLuma >= nLumaMin && nLuma <= nLumaMax) {	// if luma in range
					tile.arrBin[C_R][r]++;	// update histogram bins
					tile.arrBin[C_G][g]++;
					tile.arrBin[C_B][b]++;
				}
			}
			pInRow += nInStride;
		}
	} else {	// render stage is translate
		pInRow = tile.pInRow;	// saved in first pass
		UINT	*pOutPixel = static_cast<UINT *>(m_dibOut.GetBits()) + tile.nStart * m_nOutStride;
		for (int y = 0; y < tile.nRows; y++) {	// for each output row
			const UINT	*pInPixel = pInRow + m_rCrop.left;
			const UINT	*pInPixelEnd = pInPixel + nCols;
			while (pInPixel < pInPixelEnd) {	// for each input column
				UINT	clr = *pInPixel++;	// get input pixel
				BYTE	r = GET_XRGB_R(clr) + m_arrHue[C_R];
				BYTE	g = GET_XRGB_G(clr) + m_arrHue[C_G];
				BYTE	b = GET_XRGB_B(clr) + m_arrHue[C_B];
				clr = MAKE_XRGB(m_arrBin[C_R][r], m_arrBin[C_G][g], m_arrBin[C_B][b]);
				*pOutPixel++ = clr;	// set output pixel
			}
			pInRow += nInStride;
		}
	}
}
