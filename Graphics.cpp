#pragma once

#include "core.h"


namespace Graphics{
    static ID3D11Device* pDevice = nullptr;
    static IDXGISwapChain* pSwap = nullptr;
    static ID3D11DeviceContext* pContext = nullptr;
    static ID3D11RenderTargetView* pRenderTargetView = nullptr;

    bool Initialize(HWND hWnd) {
        DXGI_SWAP_CHAIN_DESC sd = {};
        sd.BufferCount = 1;         // 1 back buffer + 1 front buffer
        sd.BufferDesc.Width = 0;   // 0 means pick up window width automatically
        sd.BufferDesc.Height = 0;  // 0 means pick up window height automatically
        sd.BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM; // Standard 32-bit color channel
        // match the desktop refresh rate automatically 
        // it is normally numerator/denominator, so if .num = 60 and .denom = 1, it is 60hz but setting to 0 gives default
        sd.BufferDesc.RefreshRate.Numerator = 0;
        sd.BufferDesc.RefreshRate.Denominator = 0;  
        sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        sd.OutputWindow = hWnd; 
        // No anti-aliasing multi-sampling for now
        sd.SampleDesc.Count = 1;
        sd.SampleDesc.Quality = 0;
  
        sd.Windowed = TRUE;

        // msdn doesnt include the below because of zero-initialization, just doing it to be explicit
        sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
        sd.Flags = 0;

        // msdn explicitly provides a feature level (11_1, 11_0 or 10_1...), but i just use default
        HRESULT hr = D3D11CreateDeviceAndSwapChain(
            nullptr,
            D3D_DRIVER_TYPE_HARDWARE, // Force hardware GPU rendering
            nullptr,    
            D3D11_CREATE_DEVICE_DEBUG, // ! requires windows sdk installed or will crash            
            nullptr,    // default feature level 
            0,
            D3D11_SDK_VERSION,
            &sd,
            &pSwap,
            &pDevice,
            nullptr,
            &pContext
        );
        if (FAILED(hr)) return false;

        ID3D11Texture2D* pBackBuffer = nullptr;
        pSwap->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)(&pBackBuffer));

        pDevice->CreateRenderTargetView(pBackBuffer, nullptr, &pRenderTargetView);
        pBackBuffer->Release();

        pContext->OMSetRenderTargets(1, &pRenderTargetView, nullptr);

        D3D11_VIEWPORT vp = {};
        vp.Width = 640.0f;  // Match your window dimensions
        vp.Height = 480.0f;
        vp.MinDepth = 0.0f;
        vp.MaxDepth = 1.0f;
        vp.TopLeftX = 0.0f;
        vp.TopLeftY = 0.0f;
        pContext->RSSetViewports(1, &vp);

        // Return true if the GPU bound successfully
        return true;
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

    void DrawTestTriangle(){
        // hate my life, respectfully 
        // why the fuck didnt they just make it use a 2 item array in the first place

        // 1 2d triangle at center of screen
        struct Vertex{float x; float y;};
        const Vertex vertices[] = {{0.0f, 0.5f}, {0.5f, -0.5f}, {-0.5f, -0.5f}};

        ID3D11Buffer* pVertexBuffer;
        D3D11_BUFFER_DESC bd = {};
        bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
        bd.Usage = D3D11_USAGE_DEFAULT;
        bd.CPUAccessFlags = 0u;
        bd.MiscFlags = 0u;
        bd.ByteWidth = sizeof(vertices);
        bd.StructureByteStride = sizeof(Vertex);

        D3D11_SUBRESOURCE_DATA sd = {};
        sd.pSysMem = vertices;
    
        CHECK(SUCCEEDED( pDevice->CreateBuffer(&bd, &sd, &pVertexBuffer)), "Failed to create buffer");

        // Bind vertex buffer to pipeline
        const UINT stride = sizeof(Vertex);
        const UINT offset = 0u;
        pContext->IASetVertexBuffers(0u, 1u, &pVertexBuffer, &stride, &offset);

        pContext->Draw(3u, 0u);
        pVertexBuffer->Release();
        // F u 
    }
} // namespace Graphics
