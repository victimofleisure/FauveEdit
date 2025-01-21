// Copyleft 2022 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      27dec22	initial version

*/

#include "stdafx.h"
#include "Options.h"
#include "RegTempl.h"

#define RK_MULTITHREADING_TYPE	_T("MultithreadingType")
#define RK_THREAD_COUNT			_T("ThreadCount")
#define RK_BACKGROUND_COLOR		_T("BackgroundColor")
#define RK_ANIMATION_FRAME_RATE	_T("AnimationFrameRate")
#define RK_ANIMATION_HUE_RATE	_T("AnimationHueRate")

COptions::COptions()
{
	m_nMultithreadingType = MT_ONE_THREAD_PER_CORE;
	m_nThreadCount = 1;
	m_clrBackground = RGB(255, 255, 255);
	m_fAnimationFrameRate = 25;
	m_fAnimationHueRate[0] = 25;
	m_fAnimationHueRate[1] = 50;
	m_fAnimationHueRate[2] = 75;
}

void COptions::Store()
{
	WrReg(RK_MULTITHREADING_TYPE, m_nMultithreadingType);
	WrReg(RK_THREAD_COUNT, m_nThreadCount);
	WrReg(RK_BACKGROUND_COLOR, m_clrBackground);
	WrReg(RK_ANIMATION_FRAME_RATE, m_fAnimationFrameRate);
	WrReg(RK_ANIMATION_HUE_RATE, m_fAnimationHueRate);
}

void COptions::Load()
{
	RdReg(RK_MULTITHREADING_TYPE, m_nMultithreadingType);
	RdReg(RK_THREAD_COUNT, m_nThreadCount);
	RdReg(RK_BACKGROUND_COLOR, m_clrBackground);
	RdReg(RK_ANIMATION_FRAME_RATE, m_fAnimationFrameRate);
	RdReg(RK_ANIMATION_HUE_RATE, m_fAnimationHueRate);
}

int COptions::GetCoreCount()
{
	SYSTEM_INFO	sysInfo;
	GetSystemInfo(&sysInfo);
	return sysInfo.dwNumberOfProcessors;
}

int COptions::GetThreadCount() const
{
	switch (m_nMultithreadingType) {
	case MT_ONE_THREAD_PER_CORE:
		return GetCoreCount();
	case MT_CUSTOM_THREADS:
		return CLAMP(m_nThreadCount, 1, BYTE_MAX);
	default:
		return 1;
	}
}
