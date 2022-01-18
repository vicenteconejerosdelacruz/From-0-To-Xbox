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

template <typename T>
void Render3DMesh(ComPtr<ID3D12GraphicsCommandList2> commandList, UINT backBufferIndex, T& obj, UINT meshIndex) {
  auto &vbv = obj.vbvData[meshIndex];
  auto &ibv = obj.ibvData[meshIndex];
  auto &cbv = obj.cbvData[meshIndex];

  ID3D12DescriptorHeap* ppHeaps[] = { cbv.cbvsrvHeap.Get() };
  commandList->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);

  CD3DX12_GPU_DESCRIPTOR_HANDLE cbvGpuHandle(cbv.cbvsrvHeap->GetGPUDescriptorHandleForHeapStart(), backBufferIndex, cbv.cbvDescriptorSize);
  CD3DX12_GPU_DESCRIPTOR_HANDLE texGpuHandle(cbv.cbvsrvHeap->GetGPUDescriptorHandleForHeapStart(), 3, cbv.cbvDescriptorSize);
  commandList->SetGraphicsRootDescriptorTable(0, cbvGpuHandle);
  commandList->SetGraphicsRootDescriptorTable(1, texGpuHandle);

  commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

  commandList->IASetVertexBuffers(0, 1, &vbv.vertexBufferView);
  commandList->IASetIndexBuffer(&ibv.indexBufferView);
  commandList->DrawIndexedInstanced(ibv.indexBufferView.SizeInBytes / sizeof(UINT16), 1, 0, 0, 0);
}

template <typename T,typename S>
void Render3DMeshShadowMap(ComPtr<ID3D12GraphicsCommandList2> commandList, UINT backBufferIndex, ConstantsBufferViewData<S>& shadowMapCbvData, T& obj, UINT meshIndex, BOOL hasAlphaCut = FALSE) {
  auto &vbv = obj.vbvData[meshIndex];
  auto &ibv = obj.ibvData[meshIndex];

  ID3D12DescriptorHeap* ppHeaps[] = { shadowMapCbvData.cbvsrvHeap.Get() };
  commandList->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);

  CD3DX12_GPU_DESCRIPTOR_HANDLE cbvGpuHandle(shadowMapCbvData.cbvsrvHeap->GetGPUDescriptorHandleForHeapStart(), backBufferIndex, shadowMapCbvData.cbvDescriptorSize);
  commandList->SetGraphicsRootDescriptorTable(0, cbvGpuHandle);
  if (hasAlphaCut) {
    CD3DX12_GPU_DESCRIPTOR_HANDLE texGpuHandle(shadowMapCbvData.cbvsrvHeap->GetGPUDescriptorHandleForHeapStart(), 3, shadowMapCbvData.cbvDescriptorSize);
    commandList->SetGraphicsRootDescriptorTable(1, texGpuHandle);
  }

  commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
  commandList->IASetVertexBuffers(0, 1, &vbv.vertexBufferView);
  commandList->IASetIndexBuffer(&ibv.indexBufferView);
  commandList->DrawIndexedInstanced(ibv.indexBufferView.SizeInBytes / sizeof(UINT16), 1, 0, 0, 0);
}