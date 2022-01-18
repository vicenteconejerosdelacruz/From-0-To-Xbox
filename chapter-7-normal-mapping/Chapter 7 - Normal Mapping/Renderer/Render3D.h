#pragma once

template<typename T>
void Render3DPrimitive(ComPtr<ID3D12GraphicsCommandList2> commandList, UINT backBufferIndex, T& obj3D) {
  if (!obj3D.loadingComplete) return;

  commandList->SetGraphicsRootSignature(obj3D.rootSignature.Get());
  ID3D12DescriptorHeap* ppHeaps[] = { obj3D.cbvData.cbvsrvHeap.Get() };
  commandList->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);
  commandList->SetPipelineState(obj3D.pipelineState.Get());

  CD3DX12_GPU_DESCRIPTOR_HANDLE cbvGpuHandle(obj3D.cbvData.cbvsrvHeap->GetGPUDescriptorHandleForHeapStart(), backBufferIndex, obj3D.cbvData.cbvDescriptorSize);
  CD3DX12_GPU_DESCRIPTOR_HANDLE texGpuHandle(obj3D.cbvData.cbvsrvHeap->GetGPUDescriptorHandleForHeapStart(), 3, obj3D.cbvData.cbvDescriptorSize);
  commandList->SetGraphicsRootDescriptorTable(0, cbvGpuHandle);
  commandList->SetGraphicsRootDescriptorTable(1, texGpuHandle);

  commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
  commandList->IASetVertexBuffers(0, 1, &obj3D.vbvData.vertexBufferView);
  commandList->IASetIndexBuffer(&obj3D.ibvData.indexBufferView);
  commandList->DrawIndexedInstanced(_countof(T::indices), 1, 0, 0, 0);
}

template<typename T>
void Render3DPrimitiveShadowMap(ComPtr<ID3D12GraphicsCommandList2> commandList, UINT backBufferIndex, ConstantsBufferViewData<XMMATRIX>& shadowMapCbvData, T& obj3D) {
  if (!obj3D.loadingComplete) return;

  commandList->SetGraphicsRootSignature(obj3D.shadowMapRootSignature.Get());
  ID3D12DescriptorHeap* ppHeaps[] = { shadowMapCbvData.cbvsrvHeap.Get() };
  commandList->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);
  commandList->SetPipelineState(obj3D.shadowMapPipelineState.Get());

  CD3DX12_GPU_DESCRIPTOR_HANDLE cbvGpuHandle(shadowMapCbvData.cbvsrvHeap->GetGPUDescriptorHandleForHeapStart(), backBufferIndex, shadowMapCbvData.cbvDescriptorSize);
  commandList->SetGraphicsRootDescriptorTable(0, cbvGpuHandle);

  commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
  commandList->IASetVertexBuffers(0, 1, &obj3D.vbvData.vertexBufferView);
  commandList->IASetIndexBuffer(&obj3D.ibvData.indexBufferView);
  commandList->DrawIndexedInstanced(_countof(T::indices), 1, 0, 0, 0);
}