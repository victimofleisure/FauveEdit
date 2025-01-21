// Copyleft 2025 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
		chris korda

		rev		date		comments
		00		15jan25	initial version

*/

#pragma once

#include "d2d1_1.h"
#include "d3d11.h"
#include "dxgi1_2.h"

class CD2DDevCtx {
public:
	virtual ~CD2DDevCtx();
	bool	Create(HWND hWnd, int nWidth, int nHeight);
	bool	IsCreated() const;
	bool	Resize(int nWidth, int nHeight);
	CD2DSizeU	GetFrameSize() const;
	virtual void	OnError(HRESULT hr);

protected:
	static CComPtr<ID3D11Device> m_pD3DDevice;	// Direct3D device
	CComPtr<ID2D1Factory1>	m_pD2DFactory;	// Direct2D factory
	CComPtr<ID2D1Device>	m_pD2DDevice;	// Direct2D device
	CComPtr<ID2D1DeviceContext>	m_pD2DDeviceContext;	// Direct2D device context
	CComPtr<IDXGISwapChain1>	m_pSwapChain;	// DXGI swap chain
	CComPtr<ID2D1Bitmap1>	m_pTargetBitmap;	// bitmap to which Direct2D renders
};

inline CD2DDevCtx::~CD2DDevCtx()
{
}

inline bool CD2DDevCtx::IsCreated() const
{
	return m_pTargetBitmap != NULL;
}

inline CD2DSizeU CD2DDevCtx::GetFrameSize() const
{
	if (m_pD2DDeviceContext != NULL)
		return m_pD2DDeviceContext->GetPixelSize();
	else
		return CD2DSizeU(0, 0);
}

class CD2DBitmapDevCtx : public CD2DDevCtx {
public:
	CD2DBitmapDevCtx();
	CComPtr<ID2D1Bitmap1>	m_pSourceBitmap;	// source bitmap to scale
	CD2DRectF	m_rDest;	// destination rectangle; size of target bitmap
	D2D1::ColorF	m_clrBackground;	// background color; default is black
	void	Resize(int nWidth, int nHeight);
	void	UpdateLetterbox();
	bool	CreateResources(int nWidth, int nHeight);
	CD2DSizeU	GetSourceBitmapSize() const;
	void	Draw();
	virtual	void	OnError(HRESULT hr);
};

inline CD2DSizeU CD2DBitmapDevCtx::GetSourceBitmapSize() const
{
	if (m_pSourceBitmap != NULL)
		return m_pSourceBitmap->GetPixelSize();
	else
		return CD2DSizeU(0, 0);
}
