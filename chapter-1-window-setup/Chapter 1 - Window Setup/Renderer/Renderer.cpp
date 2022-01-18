#include "pch.h"
#include "Renderer.h"
#include "../Common/DirectXHelper.h"
#include "DeviceUtils.h"

using namespace Windows::Graphics::Display;

void Renderer::Initialize(CoreWindow^ coreWindow) {
	window = coreWindow;

#if defined(_DEBUG)
	{
		ComPtr<ID3D12Debug1> debugController;
		if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
		{
			debugController->EnableDebugLayer();
		}
	}
#endif

	ComPtr<IDXGIAdapter4> dxgiAdapter4 = GetAdapter();
	d3dDevice = CreateDevice(dxgiAdapter4);
	commandQueue = CreateCommandQueue(d3dDevice);
	swapChain = CreateSwapChain(window.Get(), commandQueue, frameCount);
	backBufferIndex = swapChain->GetCurrentBackBufferIndex();
	rtvDescriptorHeap = CreateDescriptorHeap(d3dDevice, frameCount);
	rtvDescriptorSize = d3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	dsvDescriptorHeap = CreateDescriptorHeap(d3dDevice, 1, D3D12_DESCRIPTOR_HEAP_TYPE_DSV);

	NAME_D3D12_OBJECT(d3dDevice);
	NAME_D3D12_OBJECT(commandQueue);
	NAME_D3D12_OBJECT(rtvDescriptorHeap);
	NAME_D3D12_OBJECT(dsvDescriptorHeap);

	UpdateRenderTargetViews(d3dDevice, swapChain, rtvDescriptorHeap, renderTargets, frameCount);
	UpdateDepthStencilView(d3dDevice, dsvDescriptorHeap, depthStencil, static_cast<int>(window->Bounds.Width), static_cast<int>(window->Bounds.Height));

	for (int i = 0; i < frameCount; ++i) {
		commandAllocators[i] = CreateCommandAllocator(d3dDevice);
		commandAllocators[i]->SetName((L"commandAllocator[" + std::to_wstring(i) + L"]").c_str());
	}

	commandList = CreateCommandList(d3dDevice, commandAllocators[backBufferIndex]);
	NAME_D3D12_OBJECT(commandList);

	fence = CreateFence(d3dDevice);
	fenceEvent = CreateEventHandle();

	UpdateViewportPerspective();
}

void Renderer::Destroy() {
	fence->Release();
	commandList->Release();
	for (auto commandAllocator : commandAllocators) {
		commandAllocator->Release();
	}
	depthStencil->Release();
	for (auto renderTarget : renderTargets) {
		renderTarget->Release();
	}
	dsvDescriptorHeap->Release();
	rtvDescriptorHeap->Release();
	swapChain->Release();
	commandQueue->Release();
	d3dDevice->Release();
}

void Renderer::UpdateViewportPerspective() {
	float aspectRatio = window->Bounds.Width / window->Bounds.Height;
	scissorRect = { 0, 0, static_cast<LONG>(window->Bounds.Width), static_cast<LONG>(window->Bounds.Height) };
	screenViewport = { 0.0f, 0.0f, window->Bounds.Width, window->Bounds.Height, 0.0f, 1.0f };
}

void Renderer::Resize(UINT width, UINT height) {
	Flush(commandQueue, fence, fenceValue, fenceEvent);

	for (UINT i = 0; i < frameCount; ++i)
	{
		renderTargets[i].Reset();
		frameFenceValues[i] = frameFenceValues[backBufferIndex];
	}

	DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
	DX::ThrowIfFailed(swapChain->GetDesc(&swapChainDesc));
	DX::ThrowIfFailed(swapChain->ResizeBuffers(frameCount, width, height,
		swapChainDesc.BufferDesc.Format, swapChainDesc.Flags));

	backBufferIndex = swapChain->GetCurrentBackBufferIndex();

	UpdateRenderTargetViews(d3dDevice, swapChain, rtvDescriptorHeap, renderTargets, frameCount);

	depthStencil.Reset();
	UpdateDepthStencilView(d3dDevice, dsvDescriptorHeap, depthStencil, width, height);

	UpdateViewportPerspective();
}

void Renderer::ResetCommands() {
	auto commandAllocator = commandAllocators[backBufferIndex];
	commandAllocator->Reset();
	commandList->Reset(commandAllocator.Get(), nullptr);
}

void Renderer::SetRenderTargets() {
	auto backBuffer = renderTargets[backBufferIndex];

	CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(backBuffer.Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
	commandList->ResourceBarrier(1, &barrier);

	commandList->RSSetViewports(1, &screenViewport);
	commandList->RSSetScissorRects(1, &scissorRect);

	CD3DX12_CPU_DESCRIPTOR_HANDLE rtv(rtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart(), backBufferIndex, rtvDescriptorSize);
	commandList->ClearRenderTargetView(rtv, DirectX::Colors::CornflowerBlue, 0, nullptr);

	CD3DX12_CPU_DESCRIPTOR_HANDLE dsv(dsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart());
	commandList->ClearDepthStencilView(dsv, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

	commandList->OMSetRenderTargets(1, &rtv, false, &dsv);
}

void Renderer::Present() {
	auto backBuffer = renderTargets[backBufferIndex];

	CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(backBuffer.Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
	commandList->ResourceBarrier(1, &barrier);

	DX::ThrowIfFailed(commandList->Close());
	ID3D12CommandList* const commandLists[] = { commandList.Get() };
	commandQueue->ExecuteCommandLists(_countof(commandLists), commandLists);

	DX::ThrowIfFailed(swapChain->Present(1, 0));
	frameFenceValues[backBufferIndex] = Signal(commandQueue, fence, fenceValue);
	backBufferIndex = swapChain->GetCurrentBackBufferIndex();

	//make the CPU to wait for the GPU to finish the current processing
	//hacer que la CPU espere a que el procesamiento de la GPU termine
	WaitForFenceValue(fence, frameFenceValues[backBufferIndex], fenceEvent);
}

void Renderer::CloseCommandsAndFlush() {
	commandList->Close();
	ID3D12CommandList* const commandLists[] = { commandList.Get() };
	commandQueue->ExecuteCommandLists(_countof(commandLists), commandLists);
	Flush(commandQueue, fence, fenceValue, fenceEvent);
}