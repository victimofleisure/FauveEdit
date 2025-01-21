// Copyleft 2025 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
		chris korda

		rev		date		comments
		00		15jan25	initial version

*/

#include "stdafx.h"
#include "D2DDevCtx.h"
#include "Dib.h"

#define CHECK(x) { HRESULT hr = x; if (FAILED(hr)) { OnError(hr); return false; }}

CComPtr<ID3D11Device> CD2DDevCtx::m_pD3DDevice;

void CD2DDevCtx::OnError(HRESULT hr)
{
	printf("CD2DDevCtx::OnError %x\n", hr); 
}

bool CD2DDevCtx::Create(HWND hWnd, int nWidth, int nHeight)
{
	// creating a Direct3D device is slow, so create only one and share it
	if (!m_pD3DDevice) {	// if Direct3D device not yet created
		CHECK(D3D11CreateDevice(
			nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, D3D11_CREATE_DEVICE_BGRA_SUPPORT,
			nullptr, 0, D3D11_SDK_VERSION, &m_pD3DDevice, nullptr, nullptr));
	}
	CComPtr<ID2D1Factory1>	pD2DFactory;
	CHECK(D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &pD2DFactory));
	CComPtr<IDXGIDevice1> pDXGIDevice;
	CHECK(m_pD3DDevice->QueryInterface(IID_PPV_ARGS(&pDXGIDevice)));
	CHECK(pD2DFactory->CreateDevice(pDXGIDevice, &m_pD2DDevice));
	CHECK(m_pD2DDevice->CreateDeviceContext(D2D1_DEVICE_CONTEXT_OPTIONS_NONE, &m_pD2DDeviceContext));
	CComPtr<IDXGIAdapter> pDXGIAdapter;
	CHECK(pDXGIDevice->GetAdapter(&pDXGIAdapter));
	CComPtr<IDXGIFactory2> pDXGIFactory;
	CHECK(pDXGIAdapter->GetParent(IID_PPV_ARGS(&pDXGIFactory)));
	DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {0};
    swapChainDesc.Width = nWidth;
    swapChainDesc.Height = nHeight;
	swapChainDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
    swapChainDesc.SampleDesc.Count = 1;
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc.BufferCount = 2;
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
	CHECK(pDXGIFactory->CreateSwapChainForHwnd(m_pD3DDevice, hWnd, &swapChainDesc, nullptr, nullptr, &m_pSwapChain));
	CComPtr<IDXGISurface> pBackBuffer;
	CHECK(m_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer)));
	CHECK(m_pD2DDeviceContext->CreateBitmapFromDxgiSurface(pBackBuffer, nullptr, &m_pTargetBitmap));
	m_pD2DDeviceContext->SetTarget(m_pTargetBitmap);
	return true;
}

bool CD2DDevCtx::Resize(int nWidth, int nHeight)
{
	ASSERT(m_pSwapChain);
	m_pD2DDeviceContext->SetTarget(nullptr); // release the target
	m_pTargetBitmap.Release();
	CHECK(m_pSwapChain->ResizeBuffers(0, nWidth, nHeight, DXGI_FORMAT_UNKNOWN, 0));
	CComPtr<IDXGISurface> pBackBuffer;
	CHECK(m_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer)));
	CHECK(m_pD2DDeviceContext->CreateBitmapFromDxgiSurface(pBackBuffer, nullptr, &m_pTargetBitmap));
	m_pD2DDeviceContext->SetTarget(m_pTargetBitmap);	// reset the target
	return true;
}

CD2DBitmapDevCtx::CD2DBitmapDevCtx() : m_rDest(0, 0, 0, 0), m_clrBackground(0)
{
}

bool CD2DBitmapDevCtx::CreateResources(int nWidth, int nHeight)
{
	D2D1_BITMAP_PROPERTIES1	propsBitmap = {
		{DXGI_FORMAT_B8G8R8X8_UNORM, D2D1_ALPHA_MODE_IGNORE},
	};
	m_pSourceBitmap.Release();
	CD2DSizeU	szBitmap(nWidth, nHeight);
	CHECK(m_pD2DDeviceContext->CreateBitmap(szBitmap, nullptr, 0, &propsBitmap, &m_pSourceBitmap));
	return true;
}

void CD2DBitmapDevCtx::UpdateLetterbox()
{
	CRect	rWnd(CPoint(0, 0), CD2DSizeU(GetFrameSize()));
	CDib::Letterbox(rWnd, GetSourceBitmapSize(), rWnd);
	m_rDest = rWnd;
}

void CD2DBitmapDevCtx::Resize(int nWidth, int nHeight)
{
	CD2DDevCtx::Resize(nWidth, nHeight);
	CRect	rWnd(CPoint(0, 0), CD2DSizeU(nWidth, nHeight));
	CDib::Letterbox(rWnd, GetSourceBitmapSize(), rWnd);
	m_rDest = rWnd;
	D2D1_MATRIX_3X2_F	matVertFlip = {1, 0, 0, -1, 0, FLOAT(nHeight)};
	m_pD2DDeviceContext->SetTransform(&matVertFlip);
}

void CD2DBitmapDevCtx::Draw()
{
	m_pD2DDeviceContext->BeginDraw();
	D2D1_INTERPOLATION_MODE	modeInterp = static_cast<D2D1_INTERPOLATION_MODE>(D2D1_INTERPOLATION_MODE_DEFINITION_HIGH_QUALITY_CUBIC);
	m_pD2DDeviceContext->Clear(m_clrBackground);	// clear to specified color
	m_pD2DDeviceContext->DrawBitmap(m_pSourceBitmap, m_rDest, 1, modeInterp);
	m_pD2DDeviceContext->EndDraw();
	static const DXGI_PRESENT_PARAMETERS	parmsPresent = {0};
	m_pSwapChain->Present1(1, 0, &parmsPresent);
}

void CD2DBitmapDevCtx::OnError(HRESULT hr)
{
	_com_error	error(hr);
	LPCTSTR pszMsgText = error.ErrorMessage();
	AfxMessageBox(pszMsgText);
}

