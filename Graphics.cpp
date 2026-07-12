#pragma once

#include "core.h"


namespace Graphics{
    static ID3D11Device* pDevice = nullptr;
    static IDXGISwapChain* pSwap = nullptr;
    static ID3D11DeviceContext* pContext = nullptr;
    static ID3D11RenderTargetView* pRenderTargetView = nullptr;

    bool Initialize(HWND hWnd) {
        DXGI_SWAP_CHAIN_DESC sd = {};
        sd.BufferDesc.Width = 0;   // 0 means pick up window width automatically
        sd.BufferDesc.Height = 0;  // 0 means pick up window height automatically
        sd.BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM; // Standard 32-bit color channel
        sd.BufferDesc.RefreshRate.Numerator = 0;
        sd.BufferDesc.RefreshRate.Denominator = 0;
        sd.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
        sd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
        
        // No anti-aliasing multi-sampling for now
        sd.SampleDesc.Count = 1;
        sd.SampleDesc.Quality = 0;
        
        sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        sd.BufferCount = 1;         // 1 back buffer + 1 front buffer
        sd.OutputWindow = hWnd;     // Hook it directly to our flat window handle
        sd.Windowed = TRUE;
        sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
        sd.Flags = 0;

        // Create the device, the context, and the swap chain canvas in one shot
        HRESULT hr = D3D11CreateDeviceAndSwapChain(
            nullptr,
            D3D_DRIVER_TYPE_HARDWARE, // Force hardware GPU rendering
            nullptr,
            D3D11_CREATE_DEVICE_DEBUG,                
            nullptr,
            0,
            D3D11_SDK_VERSION,
            &sd,
            &pSwap,
            &pDevice,
            nullptr,
            &pContext
        );

        ID3D11Texture2D* pBackBuffer = nullptr;
        pSwap->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)(&pBackBuffer));
        pDevice->CreateRenderTargetView(pBackBuffer, nullptr, &pRenderTargetView);
        pBackBuffer->Release();

        // Return true if the GPU bound successfully
        return SUCCEEDED(hr);
    }

    void EndFrame() {
        if (pSwap) {
            // Present the frame! 1u enables VSync so your GPU frames sync with your monitor
            pSwap->Present(1u, 0u); 
        }
    }

    void ClearBuffer(float red, float green, float blue){
        const float color[] = {red, green, blue, 1.0f};
        pContext->ClearRenderTargetView(pRenderTargetView, color);
    }

    void Shutdown() {
        if (pRenderTargetView)  { pRenderTargetView->Release();  pRenderTargetView = nullptr; }
        if (pContext) { pContext->Release(); pContext = nullptr; }
        if (pSwap)    { pSwap->Release();    pSwap = nullptr; }
        if (pDevice)  { pDevice->Release();  pDevice = nullptr; }
        
    }
} // namespace Graphics
