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

#include "Fauve.h"
#include "ArrayEx.h"

class CFauveTiled : public CFauve {
public:
	CFauveTiled();
	void	FauveMulti(bool bReuseHistogram = false);
	void	FauveCallback(int iThread);

protected:
	struct TILE {
		int		nStart;
		int		nRows;
		const UINT	*pInRow;
		UINT	arrBin[COLOR_CHANNELS][COLOR_VALUES];	// histogram of each color channel
	};
	enum {
		RST_HISTOGRAM,
		RST_TRANSLATE,
		RENDER_STAGES
	};
	CArrayEx<TILE, TILE&>	m_arrTile;
	int		m_nPrevHeight;
	int		m_nCropWidth;
	UINT	m_nInStride;
	UINT	m_nOutStride;
	bool	m_nRenderStage;
	bool	m_bReuseHistogram;
};

