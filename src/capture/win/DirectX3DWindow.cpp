#include "DirectX3DWindow.h"

#include <iostream>
#include <iomanip>

#include "Buffer.h"

extern "C" {
#include "TimeUtil.h"
#include "png.h"
}

using namespace winrt;
using namespace Windows;
using namespace Windows::Foundation;
using namespace Windows::System;
using namespace Windows::Graphics;
using namespace Windows::Graphics::Capture;
using namespace Windows::Graphics::DirectX;
using namespace Windows::Graphics::DirectX::Direct3D11;
using namespace Windows::Foundation::Numerics;
using namespace Windows::UI;
using namespace Windows::UI::Composition;

extern "C"
{
    HRESULT __stdcall CreateDirect3D11DeviceFromDXGIDevice(::IDXGIDevice* dxgiDevice,
        ::IInspectable** graphicsDevice);

    HRESULT __stdcall CreateDirect3D11SurfaceFromDXGISurface(::IDXGISurface* dgxiSurface,
        ::IInspectable** graphicsSurface);
}

inline auto CreateDirect3DDevice(IDXGIDevice* dxgi_device)
{
    winrt::com_ptr<::IInspectable> d3d_device;
    winrt::check_hresult(CreateDirect3D11DeviceFromDXGIDevice(dxgi_device, d3d_device.put()));
    return d3d_device.as<winrt::Windows::Graphics::DirectX::Direct3D11::IDirect3DDevice>();
}

template <typename T>
auto GetDXGIInterfaceFromObject(winrt::Windows::Foundation::IInspectable const& object)
{
    auto access = object.as<IDirect3DDxgiInterfaceAccess>();
    winrt::com_ptr<T> result;
    winrt::check_hresult(access->GetInterface(winrt::guid_of<T>(), result.put_void()));
    return result;
}

inline auto
CreateD3DDevice(D3D_DRIVER_TYPE const type, winrt::com_ptr<ID3D11Device>& device)
{
    WINRT_ASSERT(!device);

    UINT flags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;

    return D3D11CreateDevice(
        nullptr,
        type,
        nullptr,
        flags,
        nullptr, 0,
        D3D11_SDK_VERSION,
        device.put(),
        nullptr,
        nullptr);
}

inline auto
CreateD3DDevice()
{
    winrt::com_ptr<ID3D11Device> device;
    HRESULT hr = CreateD3DDevice(D3D_DRIVER_TYPE_HARDWARE, device);

    if (DXGI_ERROR_UNSUPPORTED == hr)
    {
        hr = CreateD3DDevice(D3D_DRIVER_TYPE_WARP, device);
    }

    winrt::check_hresult(hr);
    return device;
}

inline auto
CreateDXGISwapChain(
    winrt::com_ptr<ID3D11Device> const& device,
    const DXGI_SWAP_CHAIN_DESC1* desc)
{
    auto dxgiDevice = device.as<IDXGIDevice2>();
    winrt::com_ptr<IDXGIAdapter> adapter;
    winrt::check_hresult(dxgiDevice->GetParent(winrt::guid_of<IDXGIAdapter>(), adapter.put_void()));
    winrt::com_ptr<IDXGIFactory2> factory;
    winrt::check_hresult(adapter->GetParent(winrt::guid_of<IDXGIFactory2>(), factory.put_void()));

    winrt::com_ptr<IDXGISwapChain1> swapchain;
    winrt::check_hresult(factory->CreateSwapChainForComposition(
        device.get(),
        desc,
        nullptr,
        swapchain.put()));

    return swapchain;
}

inline auto
CreateDXGISwapChain(
    winrt::com_ptr<ID3D11Device> const& device,
    uint32_t width,
    uint32_t height,
    DXGI_FORMAT format,
    uint32_t bufferCount)
{
    DXGI_SWAP_CHAIN_DESC1 desc = {};
    desc.Width = width;
    desc.Height = height;
    desc.Format = format;
    desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    desc.SampleDesc.Count = 1;
    desc.SampleDesc.Quality = 0;
    desc.BufferCount = bufferCount;
    desc.Scaling = DXGI_SCALING_STRETCH;
    desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
    desc.AlphaMode = DXGI_ALPHA_MODE_PREMULTIPLIED;

    return CreateDXGISwapChain(device, &desc);
}

inline auto CreateCaptureItemForWindow(HWND hwnd)
{
	auto activation_factory = winrt::get_activation_factory<winrt::Windows::Graphics::Capture::GraphicsCaptureItem>();
	auto interop_factory = activation_factory.as<IGraphicsCaptureItemInterop>();
	winrt::Windows::Graphics::Capture::GraphicsCaptureItem item = { nullptr };
	interop_factory->CreateForWindow(hwnd, winrt::guid_of<ABI::Windows::Graphics::Capture::IGraphicsCaptureItem>(), reinterpret_cast<void**>(winrt::put_abi(item)));
	return item;
}

Direct3DCapture::Direct3DCapture(HWND hwnd, CircleWRBuf<FrameBuffer> * f) : _frameOut(f)
{
    m_item = CreateCaptureItemForWindow(hwnd);;
    m_device = CreateDirect3DDevice(CreateD3DDevice().as<IDXGIDevice>().get());
    auto d3dDevice = GetDXGIInterfaceFromObject<ID3D11Device>(m_device);
    d3dDevice->GetImmediateContext(m_d3dContext.put());

	auto size = m_item.Size();
    m_swapChain = CreateDXGISwapChain(
                        d3dDevice, 
                        static_cast<uint32_t>(size.Width),
                        static_cast<uint32_t>(size.Height),
                        static_cast<DXGI_FORMAT>(DirectXPixelFormat::R8G8B8A8UIntNormalized),
                        2);

	// Create framepool, define pixel format (DXGI_FORMAT_B8G8R8A8_UNORM), and frame size. 
    m_framePool = Direct3D11CaptureFramePool::Create(
                        m_device,
                        DirectXPixelFormat::R8G8B8A8UIntNormalized,
                        2,
                        size);
    m_session = m_framePool.CreateCaptureSession(m_item);
    m_lastSize = size;
	m_frameArrived = m_framePool.FrameArrived(auto_revoke, { this, &Direct3DCapture::OnFrameArrived });
}

void Direct3DCapture::StartCapture()
{
    CheckClosed();
    m_session.StartCapture();
}

void Direct3DCapture::CheckClosed()
{
    if (m_closed.load() == true)
    {
        throw winrt::hresult_error(RO_E_CLOSED);
    }
}

void Direct3DCapture::Close()
{
    auto expected = false;
    if (m_closed.compare_exchange_strong(expected, true))
    {
		m_frameArrived.revoke();
		m_framePool.Close();
        m_session.Close();

        m_swapChain = nullptr;
        m_framePool = nullptr;
        m_session = nullptr;
        m_item = nullptr;
    }
}


void write_RGB32_image(const char * path, unsigned char *buffer, size_t w, size_t h)
{
    FILE *fp = fopen(path, "wb");
    if(!fp) return ;

    png_structp png = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!png) abort();

    png_infop info = png_create_info_struct(png);
    if (!info) abort();

    if (setjmp(png_jmpbuf(png))) abort();

    png_init_io(png, fp);

    png_set_IHDR(png,
                 info,
                 w, h,
                 8,
                 PNG_COLOR_TYPE_RGBA,
                 PNG_INTERLACE_NONE,
                 PNG_COMPRESSION_TYPE_DEFAULT,
                 PNG_FILTER_TYPE_DEFAULT
    );
    png_write_info(png, info);

    for ( int i=0 ; i<h ; i++) {
        png_write_row(png, (png_bytep)(buffer + i*w*4));
    }

    png_write_end(png, NULL);
    fclose(fp);

    png_destroy_write_struct(&png, &info);

}

static int count = 0;

void Direct3DCapture::OnFrameArrived(
    Direct3D11CaptureFramePool const& sender,
    winrt::Windows::Foundation::IInspectable const&)
{
    auto frame = sender.TryGetNextFrame();
    bool newSize = false;

std::cout << "Direct3DCapture::OnFrameArrived" << std::endl;
    /* frame size changed */
    if (frame.ContentSize().Width != m_lastSize.Width ||
        frame.ContentSize().Height != m_lastSize.Height)
    {
        newSize = true;
        m_lastSize = frame.ContentSize();
        m_swapChain->ResizeBuffers(
                        2, 
                        static_cast<uint32_t>(m_lastSize.Width),
                        static_cast<uint32_t>(m_lastSize.Height),
                        static_cast<DXGI_FORMAT>(DirectXPixelFormat::B8G8R8A8UIntNormalized), 
                        0);
    }

    winrt::com_ptr<ID3D11Texture2D> backBuffer;
    winrt::check_hresult(m_swapChain->GetBuffer(0, winrt::guid_of<ID3D11Texture2D>(), backBuffer.put_void()));
    auto surfaceTexture = GetDXGIInterfaceFromObject<ID3D11Texture2D>(frame.Surface());

    ID3D11Device* d3dDevice;
    surfaceTexture->GetDevice(&d3dDevice);
    ID3D11DeviceContext* d3dContext;
    d3dDevice->GetImmediateContext(&d3dContext);

    D3D11_MAPPED_SUBRESOURCE mapInfo;
    D3D11_TEXTURE2D_DESC desc;
    surfaceTexture->GetDesc(&desc);
    desc.Usage = D3D11_USAGE_STAGING;
    desc.BindFlags = 0;
    desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
    desc.MiscFlags = 0;

    ID3D11Texture2D* stagingTexture = NULL;
    HRESULT hr = d3dDevice->CreateTexture2D(&desc, nullptr, &stagingTexture);
    if (FAILED(hr)) {
        // throw std::invalid_argument("received negative value");
        std::cout << "Failed to create staging texture";
        return;
    }

    d3dContext->CopyResource(stagingTexture, surfaceTexture.get());
    hr = d3dContext->Map(stagingTexture, 0, D3D11_MAP_READ, 0, &mapInfo);

    if (FAILED(hr)) {
        // throw std::invalid_argument("received negative value");
        std::cout << "Failed to map staging texture";
    }

    std::cout << get_current_time_string() << " h=" << desc.Height << " w=" << desc.Width << " pData=" 
            << mapInfo.pData << " RowPitch=" << mapInfo.RowPitch << " desc.Format=" << desc.Format 
            << " desc.ArraySize=" << desc.ArraySize << " mapInfo.DepthPitch="<< mapInfo.DepthPitch << std::endl;

    auto * f = _frameOut->getToWrite();
std::cout << "before get to write=" << (f==nullptr) <<  std::endl;
    if (f == nullptr) {
        m_d3dContext->Unmap(stagingTexture, 0);
        stagingTexture->Release();
        return;
    }

    f->reSet(desc.Width * desc.Height * 4);
//    byte * bits = (byte *)malloc(desc.Width * desc.Height * 4);
    auto source = reinterpret_cast<byte*>(mapInfo.pData);
    auto dest = f->getData();
    for (int i=0; i<desc.Height; i++) {
        memcpy(dest+i*4*desc.Width, source + i*mapInfo.RowPitch, desc.Width * 4);
    }

    char path[100] = {0};
    fprintf(stderr, "testing_%d.png", count++);
//    write_RGB32_image(path, (unsigned char *) dest, desc.Width, desc.Height);

    m_d3dContext->Unmap(stagingTexture, 0);
    stagingTexture->Release();

    if (newSize)
    {
        m_framePool.Recreate(
                        m_device,
                        DirectXPixelFormat::R8G8B8A8UIntNormalized,
                        2,
                        m_lastSize);
    }
}

