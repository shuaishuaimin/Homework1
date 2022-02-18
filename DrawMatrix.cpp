#include "stdafx.h"
#include <DirectXColors.h>
#include "DrawMatrix.h"


using namespace DirectX;
using Microsoft::WRL::ComPtr;
using namespace DirectX::PackedVector;

DrawMatrix::DrawMatrix(HINSTANCE hInstance) : D3DApp(hInstance)
{
	
}
DrawMatrix::~DrawMatrix()
{

}

bool DrawMatrix::Initialize()
{
	if (!D3DApp::Initialize())
		return false;

	// reset
	ConstructProjectionMatrix();
	ThrowIfFailed(mCommandList->Reset(mDirectCmdListAlloc.Get(), nullptr));
	BuildDescriptorHeaps();
	BulidConstantBuffers();
	BuildRootSignature();
	BuildShadersAndInputLayOut();
	BuildBoxGeometry();
	BuildPSO();

	ThrowIfFailed(mCommandList->Close());
	ID3D12CommandList* cmdLists[] = { mCommandList.Get() };
	mCommandQueue->ExecuteCommandLists(_countof(cmdLists), cmdLists);

	FlushCommandQueue();
	return true;
}

void DrawMatrix::OnResize()
{
	D3DApp::OnResize();
}

void DrawMatrix::ConstructProjectionMatrix()
{
	// The windows resized, so update the aspect ratio and recompute the projection matrix.
	XMMATRIX P = XMMatrixPerspectiveFovLH(0.25f * MathHelper::Pi, AspectRatio(), 1.0f, 1000.0f);
	XMStoreFloat4x4(&mProj, P);
}

void DrawMatrix::Update(const GameTimer& gt)
{
	
}

void DrawMatrix::Draw(const GameTimer& gt)
{
	// Reuse the memory associated with command recording.
	// we can only  reset when the associated command lists have finished execution on the GPU.
	ThrowIfFailed(mDirectCmdListAlloc->Reset());

	// A command list can be reset after it has been added to the command queue via ExecuteCommandList.
	// Reusing the command list reuses memory.
	ThrowIfFailed(mCommandList->Reset(mDirectCmdListAlloc.Get(), nullptr));

	// Indicate a state transition on the resource usage.
	auto BarrierTransPresentToRT = CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer(),
		D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
	mCommandList->ResourceBarrier(1, &BarrierTransPresentToRT);

	// Set the viewport and scissor rect. This needs to be reset whenever the command list is reset.
	mCommandList->RSSetViewports(1, &mScreenViewport);
	mCommandList->RSSetScissorRects(1, &mScissorRect);

	// Clear the back buffer and depth buffer.
	mCommandList->ClearRenderTargetView(CurrentBackBufferView(), Colors::LightPink, 0, nullptr);
	mCommandList->ClearDepthStencilView(DepthStencilView(), D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);

	// Specify the buffers we are going to render to
	auto CBV = CurrentBackBufferView();
	auto DSV = DepthStencilView();
	mCommandList->OMSetRenderTargets(1, &CBV, true, &DSV);

	// Indicate a state transition on the resource usage.
	auto BarrierTransRTToPresent = CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer(),
		D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
	mCommandList->ResourceBarrier(1, &BarrierTransRTToPresent);

	// Done recording commands ! important ,otherwise gpu instantce will stop
	ThrowIfFailed(mCommandList->Close());

	// Add the command list to the queue for execution.
	ID3D12CommandList* cmdsLists[] = { mCommandList.Get() };
	mCommandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);

	// swap the back and front buffers
	ThrowIfFailed(mSwapChain->Present(0, 0));
	mCurrBackBuffer = (mCurrBackBuffer + 1) % SwapChainBufferCount;

	// Wait until frame commands are complete, this waiting is inefficient and is done for simplicity
	// Later we will show how to organize our rendering code so we do not have to wait per frame
	FlushCommandQueue();
}

void DrawMatrix::OnMouseDown(WPARAM btnState, int x, int y)
{

}
void DrawMatrix::OnMouseUp(WPARAM btnState, int x, int y)
{

}
void DrawMatrix::OnMouseMove(WPARAM btnState, int x, int y)
{

}

void DrawMatrix::BuildDescriptorHeaps()
{

}
void DrawMatrix::BulidConstantBuffers()
{

}
void DrawMatrix::BuildRootSignature()
{

}
void DrawMatrix::BuildShadersAndInputLayOut()
{

}
void DrawMatrix::BuildBoxGeometry()
{

}
void DrawMatrix::BuildPSO()
{

}

