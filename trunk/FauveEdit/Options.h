// Copyleft 2022 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      27dec22	initial version
        01      18jan25	add use threads accessor

*/

#pragma once

class COptions {
public:
	COptions();
	enum {	// multithreading types
		MT_NO_THREADS,			// render in UI thread; no worker threads
		MT_ONE_THREAD_PER_CORE,	// one worker thread per logical processor
		MT_CUSTOM_THREADS,		// user-specified number of worker threads
		MULTITHREADING_TYPES
	};
	int		m_nMultithreadingType;	// see enum above
	int		m_nThreadCount;		// custom thread count
	COLORREF	m_clrBackground;	// background color
	float	m_fAnimationFrameRate;	// in frames per second
	float	m_fAnimationHueRate[3];	// hue rotation rates for RGB, in color steps per second
	void	Store();
	void	Load();
	int		GetThreadCount() const;
	bool	UseThreads() const;
	static	int		GetCoreCount();
};

inline bool COptions::UseThreads() const
{
	return m_nMultithreadingType != MT_NO_THREADS;
}
