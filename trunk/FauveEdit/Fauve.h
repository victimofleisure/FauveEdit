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

*/

#include "DibEx.h"

class CFauve {
public:
// Constants
	enum {	// color channels
		C_R,	// red
		C_G,	// green
		C_B,	// blue
		COLOR_CHANNELS
	};
	enum {	// luma limits
		L_BLACK,	// black
		L_WHITE,	// white
		LUMA_LIMITS
	};
	enum {
		COLOR_VALUES = 256,	// number of values per color channel
		HISTOGRAM_BINS = COLOR_CHANNELS * COLOR_VALUES,
	};

// Public data
//
// Both bitmaps must exist. The output bitmap's dimensions must match the 
// cropping rectangle, and can't exceed the input bitmap's dimensions. The 
// cropping rectangle must be normalized, and must select a subset of the 
// input bitmap. The luma parameters range from 0 to 255, and the minimum 
// must be less than the maximum. The hue shifts also range from 0 to 255.
// For the fast version, both bitmaps must be bottom-up and use 32 bits 
// per pixel in XRGB format (blue channel in the least significant byte).
//
	CDibEx	m_dibIn;		// input device-independent bitmap
	CDibEx	m_dibOut;		// output device-independent bitmap
	CRect	m_rCrop;		// cropping rectangle
	BYTE	m_arrLuma[LUMA_LIMITS];	// luma minimum and maximum levels
	BYTE	m_arrHue[COLOR_CHANNELS];	// array of per-channel color hue shifts
	UINT	m_arrBin[COLOR_CHANNELS][COLOR_VALUES];	// histogram of each color channel

// Operations
	void	FauveRef(bool bReuseHistogram = false);
	void	FauveFast(bool bReuseHistogram = false);
};

// macros for XRGB format, with blue in least significant byte
#define GET_XRGB_R(clr) BYTE((clr) >> 16)
#define GET_XRGB_G(clr) BYTE((clr) >> 8)
#define GET_XRGB_B(clr) BYTE((clr) & 0xff)
#define MAKE_XRGB(r, g, b) (((r) << 16) | ((g) << 8) | (b))
