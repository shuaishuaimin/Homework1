#include "d3dApp.h"
#include <windowsx.h>

using Microsoft::WRL::ComPtr;
using namespace std;
using namespace DirectX;

LRESULT CALLBACK
MainWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	// forward hwnd on because we can get message (e.g: WM_CREATE)
	// before CreateWindow returns, and thus before mhMainWnd is valid.
	return D3DApp::GetApp()->MsgProc(hwnd, msg, wParam, lParam);
}

D3DApp* D3DApp::mApp = nullptr;
D3DApp* D3DApp::GetApp()
{
	return mApp;
}

D3DApp::D3DApp(HINSTANCE hInstance) : mhAppInst(hInstance)
{
	// only one D3DApp can be construct
	assert(mApp == nullptr);
	mApp = this;
}

D3DApp::~D3DApp()
{
	// if device is empty, flush the command list(queue)
	if (md3dDevice != nullptr)
	{
		FlushCommandQueue();
	}
}

HINSTANCE D3DApp::AppInst()const
{
	return mhAppInst;
}
HWND D3DApp::MainWnd()const
{
	return mhMainWnd;
}
float D3DApp::AspectRatio()const
{
	return static_cast<float>(mClientWidth) / mClientHeight;
}

bool D3DApp::Get4xMsaaState()const
{
	return m4xMsaaState;
}

void D3DApp::Set4xMsaaState(bool value)
{
	if (m4xMsaaState != value)
	{
		m4xMsaaState = value;

		// Recreate the swap chain and buffers with new multi sample settings
		CreateSwapChain();
		OnResize();
	}
}

// main thread
int D3DApp::Run()
{
	MSG msg = {0};

	mTimer.Reset();

	// if message is not wm_quit. Refresh the window
	while (msg.message != WM_QUIT)
	{
	// if there are Window messages then process them
		if (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
		{
			TranslateMessage( &msg );
			DispatchMessage( &msg );
		}
	// otherwise, do animation/game stuff
		else
		{
			mTimer.Tick();

			// if game pause sleep for wait
			// else calculate frame states and update timer, draw timer to screen
			if (!mAppPaused)
			{
				CalculateFrameStats();
				Update(mTimer);
				Draw(mTimer);
			}
			else
			{
				Sleep(100);
			}
		}
	}

	return (int)msg.wParam;
}

bool D3DApp::Initialize()
{
	// only mainwindow or directx3D is not inited, return initialize failed;
	if (!InitMainWindow()) return false;

	if (!InitDirect3D()) return false;

	// init resize code
	OnResize();

	return true;
}

// when input heap, we should two heap, that is rtvHeap and dsvHeap
// their type is D3D12_DESCRIPTOR_HEAP_DESC
// we should create description and its own information, that is way to explain heap 
//	data in heap can not be explained by computer without explain
void D3DApp::CreateRtvAndDsvDescriptorHeaps()
{
	D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc;
	rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	rtvHeapDesc.NodeMask = 0;
	rtvHeapDesc.NumDescriptors = SwapChainBufferCount;
	rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;

	// create heap ,if failed, throw exception
	ThrowIfFailed(md3dDevice->CreateDescriptorHeap(
		&rtvHeapDesc, IID_PPV_ARGS(mRtvHeap.GetAddressOf())));

	// create dsvHeapDesc
	D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc;
	dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	dsvHeapDesc.NodeMask = 0;
	dsvHeapDesc.NumDescriptors = 1;
	dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	ThrowIfFailed(md3dDevice->CreateDescriptorHeap(
	&dsvHeapDesc, IID_PPV_ARGS(mDsvHeap.GetAddressOf())));
}

// resize
// we must create device, swapchain and directcmdlist
void D3DApp::OnResize()
{
	assert(md3dDevice);
	assert(mSwapChain);
	assert(mDirectCmdListAlloc);

	// flush before changing any resources
	FlushCommandQueue();

	ThrowIfFailed(mCommandList->Reset(mDirectCmdListAlloc.Get(), nullptr));

	// release the previous resources we will be recreating.
	for (auto SwapChainBufferContent : mSwapChainBuffer)
	{
		SwapChainBufferContent.Reset();
	}
	mDepthStencilBuffer.Reset();

	// resize the swap chain
	ThrowIfFailed(mSwapChain->ResizeBuffers(
		SwapChainBufferCount,
		mClientWidth, mClientHeight,
		mBackBufferFormat,
		DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH));

	mCurrBackBuffer = 0;

	// handle ??
	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHeapHandle(mRtvHeap->GetCPUDescriptorHandleForHeapStart());
	for (UINT i = 0; i < SwapChainBufferCount; i++)
	{
		ThrowIfFailed(mSwapChain->GetBuffer(i, IID_PPV_ARGS(&mSwapChainBuffer[i])));
		md3dDevice->CreateRenderTargetView(mSwapChainBuffer[i].Get(), nullptr, rtvHeapHandle);
		rtvHeapHandle.Offset(1, mRtvDescriptorSize);
	}

	//create the depth stencil buffer and view
	D3D12_RESOURCE_DESC depthStenciDesc;
	depthStenciDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	depthStenciDesc.Alignment = 0;
	depthStenciDesc.Width = mClientWidth;
	depthStenciDesc.Height = mClientHeight;
	depthStenciDesc.DepthOrArraySize = 1;
	depthStenciDesc.MipLevels = 1;

	// correction 11/12/2016: SSAO chapter requires an SRV to the depth buffer to read from
	// the depth. Therefore, because we need to create two views to the same resources:
	// 1. SRV format: DXGI_FORMAT_R24_UNORM_X8_TYPELESS
	// 2. DSV format: DXGI_FORMAT_D24_UNORM_S8_UINT
	// we need to create the depth buffer resources with a typeless format
	depthStenciDesc.SampleDesc.Count = m4xMsaaState ? 4 : 1;
	depthStenciDesc.SampleDesc.Quality = m4xMsaaState ? (m4xMsaaQuality - 1) : 0;
	depthStenciDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	depthStenciDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

	// clear ??
	D3D12_CLEAR_VALUE optClear;
	optClear.Format = mDepthStencilFormat;
	optClear.DepthStencil.Depth = 1.0f;
	optClear.DepthStencil.Stencil = 0;
	ThrowIfFailed(md3dDevice->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
		D3D12_HEAP_FLAG_NONE,
		&depthStenciDesc,
		D3D12_RESOURCE_STATE_COMMON,
		&optClear,
		IID_PPV_ARGS(mDepthStencilBuffer.GetAddressOf())));

	// Create descriptor to mip level 0 of entire resource using the format of resource.
	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc;
	dsvDesc.Flags = D3D12_DSV_FLAG_NONE;
	dsvDesc.Format = mDepthStencilFormat;
	dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	dsvDesc.Texture2D.MipSlice = 0;
	md3dDevice->CreateDepthStencilView(mDepthStencilBuffer.Get(), &dsvDesc, DepthStencilView());

	// Transition the resource from its initial state to be used as a depth buffer
	mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(mDepthStencilBuffer.Get(),D3D12_RESOURCE_STATE_COMMON,
		D3D12_RESOURCE_STATE_DEPTH_WRITE));

	//Execute the resize commands
	ThrowIfFailed(mCommandList->Close());
	ID3D12CommandList* cmdsLists[] = {mCommandList.Get()};
	mCommandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);

	// wait until resize is complete
	FlushCommandQueue();

	// Update the viewport transform to cover the client area
	mScreenViewport.TopLeftX = 0;
	mScreenViewport.TopLeftY = 0;
	mScreenViewport.Width = static_cast<float>(mClientWidth);
	mScreenViewport.Height = static_cast<float>(mClientHeight);
	mScreenViewport.MinDepth = 0.0f;
	mScreenViewport.MaxDepth = 1.0f;

	mScissorRect = { 0, 0, mClientWidth, mClientHeight};
}


LRESULT D3DApp::MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	// WM_ACTIVATE is sent when the window is activate or deactivated
	// we pause the game when the window is deactivated and unpause it
	// when it becomes active.
	case WM_ACTIVATE:
		if (LOWORD(wParam) == WA_INACTIVE)
		{
			mAppPaused = true;
			mTimer.Stop();
		}
		else
		{
			mAppPaused = false;
			mTimer.Start();
		}
		return 0;

	case WM_SIZE:
		// save the new client area dismensions.
		mClientWidth = LOWORD(lParam);
		mClientHeight = HIWORD(lParam);
		if (md3dDevice)
		{
			if (wParam == SIZE_MINIMIZED)
			{
				mAppPaused = true;
				mMinimized = true;
				mMaximized = false;
			}
			else if (wParam == SIZE_MAXIMIZED)
			{
				mAppPaused = false;
				mMinimized = false;
				mMaximized = true;
				OnResize();
			}
			else if (wParam == SIZE_RESTORED)
			{
				// Restoring from minimized state?
				if (mMinimized)
				{
					mAppPaused = false;
					mMinimized = false;
					OnResize();
				}

				// Restoring from maximized state?
				else if (mMaximized)
				{
					mAppPaused = false;
					mMaximized = false;
					OnResize();
				}
				else if (mResizing)
				{
					// If user is dragging the resize bars, we do not resize 
					// the buffers here because as the user continuously 
					// drags the resize bars, a stream of WM_SIZE messages are
					// sent to the window, and it would be pointless (and slow)
					// to resize for each WM_SIZE message received from dragging
					// the resize bars.  So instead, we reset after the user is 
					// done resizing the window and releases the resize bars, which 
					// sends a WM_EXITSIZEMOVE message.
				}
				else // API call such as SetWindowPos or mSwapChain->SetFullscreenState.
				{
					OnResize();
				}
			}
		}
		return 0;

		// WM_EXITSIZEMOVE is sent when the user releases the resize bars.
	// Here we reset everything based on the new window dimensions.
	case WM_EXITSIZEMOVE:
		mAppPaused = false;
		mResizing = false;
		mTimer.Start();
		OnResize();
		return 0;

		// WM_DESTROY is sent when the window is being destroyed.
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;

		// The WM_MENUCHAR message is sent when a menu is active and the user presses 
		// a key that does not correspond to any mnemonic or accelerator key. 
	case WM_MENUCHAR:
		// Don't beep when we alt-enter.
		return MAKELRESULT(0, MNC_CLOSE);

		// Catch this message so to prevent the window from becoming too small.
	case WM_GETMINMAXINFO:
		((MINMAXINFO*)lParam)->ptMinTrackSize.x = 200;
		((MINMAXINFO*)lParam)->ptMinTrackSize.y = 200;
		return 0;

	case WM_LBUTTONDOWN:
	case WM_MBUTTONDOWN:
	case WM_RBUTTONDOWN:
		OnMouseDown(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		return 0;
	case WM_LBUTTONUP:
	case WM_MBUTTONUP:
	case WM_RBUTTONUP:
		OnMouseUp(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		return 0;
	case WM_MOUSEMOVE:
		OnMouseMove(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		return 0;
	case WM_KEYUP:
		if (wParam == VK_ESCAPE)
		{
			PostQuitMessage(0);
		}
		else if ((int)wParam == VK_F2)
			Set4xMsaaState(!m4xMsaaState);

		return 0;
	}

	return DefWindowProc(hwnd, msg, wParam, lParam);
}

bool D3DApp::InitMainWindow()
{
	WNDCLASS wc;
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = MainWndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = mhAppInst;
	wc.hIcon = LoadIcon(0, IDI_APPLICATION);
	wc.hCursor = LoadCursor(0, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)GetStockObject(NULL_BRUSH);
	wc.lpszMenuName = 0;
	wc.lpszClassName = L"MainWnd";

	if (!RegisterClass(&wc))
	{
		MessageBox(0, L"Register failed", 0, 0);
		return false;
	}

	// compute window rectangle dismensions based on requested client area dismensions
	RECT R = {0, 0, mClientWidth, mClientHeight};
	AdjustWindowRect(&R, WS_OVERLAPPEDWINDOW, false);
	int width = R.right - R.left;
	int height = R.bottom - R.top;

	mhMainWnd = CreateWindow(L"MainWnd", mMainWndCaption.c_str(),
		WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, width, height, 0, 0, mhAppInst, 0);

	if (!mhMainWnd)
	{
		MessageBox(0, L"CreateWindow failed.", 0, 0);
		return false;
	}

	// show window must do
	ShowWindow(mhMainWnd, SW_SHOW);
	UpdateWindow(mhMainWnd);

	return true;
}

bool D3DApp::InitDirect3D()
{
#if defined(DEBUG) || defined(_DEBUG)
//enable the d3d12 debug layer
{
	ComPtr<ID3D12Debug> debugController;
	ThrowIfFailed(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)));
	debugController->EnableDebugLayer();
}
#endif

	ThrowIfFailed(CreateDXGIFactory1(IID_PPV_ARGS(&mdxgiFactory)));

	// Try to create hard ware device !!important
	HRESULT hardwareResult = D3D12CreateDevice(
		nullptr, D3D_FEATURE_LEVEL_11_0, 
		IID_PPV_ARGS(&md3dDevice));

	// Fallback to WARP device.
	if (FAILED(hardwareResult))
	{
		ComPtr<IDXGIAdapter> pWarpAdapter;
		ThrowIfFailed(mdxgiFactory->EnumWarpAdapter(IID_PPV_ARGS(&pWarpAdapter)));

		ThrowIfFailed(D3D12CreateDevice)
	}

	return true;
}

void D3DApp::Update(const GameTimer& gt)
{
	
}

void D3DApp::Draw(const GameTimer& gt)
{

}



void D3DApp::CreateCommandObjects()
{
}
void D3DApp::CreateSwapChain()
{}

void D3DApp::FlushCommandQueue()
{}

ID3D12Resource* D3DApp::CurrentBackBuffer()const
{
	ID3D12Resource* Nullptr();
	return Nullptr();
}
D3D12_CPU_DESCRIPTOR_HANDLE D3DApp::CurrentBackBufferView()const
{
}
D3D12_CPU_DESCRIPTOR_HANDLE D3DApp::DepthStencilView()const
{}

void D3DApp::CalculateFrameStats()
{}

void D3DApp::LogAdapters(){}
void D3DApp::LogAdaperOutputs(IDXGIAdapter* adapter){}
void D3DApp::LogOutputDisplayModes(IDXGIOutput* output, DXGI_FORMAT format){}

