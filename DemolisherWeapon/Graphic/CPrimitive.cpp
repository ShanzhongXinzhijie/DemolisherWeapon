#include "DWstdafx.h"
#include "CPrimitive.h"

namespace DemolisherWeapon {

CPrimitive::CPrimitive()
{
}
CPrimitive::~CPrimitive()
{
	Release();
}

void CPrimitive::Release() {
	m_vertexBuffer.reset();
	m_indexBuffer.reset();
	m_isInit = false;
}

void CPrimitive::Init(D3D_PRIMITIVE_TOPOLOGY topology, int numVertex, SVertex* vertex, int numIndex, unsigned long* index) {
	Release();

	//トポロジーの設定
	m_topology = topology;

	//頂点バッファの作成
	if (GetGraphicsEngine().GetUseAPI() == enDirectX11) {
		m_vertexBuffer = std::make_unique<VertexBufferDX11>();
	}
	if (GetGraphicsEngine().GetUseAPI() == enDirectX12) {
		m_vertexBuffer = std::make_unique<VertexBufferDX12>();
	}
	m_vertexBuffer->Init(numVertex, sizeof(SVertex), vertex);

	//インデックスバッファの作成
	if (GetGraphicsEngine().GetUseAPI() == enDirectX11) {
		m_indexBuffer = std::make_unique<IndexBufferDX11>();
	}
	if (GetGraphicsEngine().GetUseAPI() == enDirectX12) {
		m_indexBuffer = std::make_unique<IndexBufferDX12>();
	}		
	if (numIndex > 0) {
		m_indexBuffer->Init(numIndex, index);
	}

	m_isInit = true;
}
void CPrimitive::Init(D3D_PRIMITIVE_TOPOLOGY topology, int numVertex, SVertex* vertex) {
	Init(topology, numVertex, vertex, 0, nullptr);
}

void CPrimitive::Draw(int numVertex) {
	if (m_isInit) {
		//頂点バッファを設定
		m_vertexBuffer->Attach();
		//インデックスバッファを設定
		if (m_indexBuffer->GetIndexNum() > 0) {
			m_indexBuffer->Attach();
		}

		if (GetGraphicsEngine().GetUseAPI() == enDirectX11) {
			//トポロジーを設定
			GetEngine().GetGraphicsEngine().GetD3DDeviceContext()->IASetPrimitiveTopology(m_topology);

			//描画
			if (m_indexBuffer->GetIndexNum() > 0) {
				//インデックス使用
				GetEngine().GetGraphicsEngine().GetD3DDeviceContext()->DrawIndexed(
					m_indexBuffer->GetIndexNum(),
					0,
					0
				);
			}
			else {
				//通常
				GetEngine().GetGraphicsEngine().GetD3DDeviceContext()->Draw(numVertex, 0);
			}
		}
		if (GetGraphicsEngine().GetUseAPI() == enDirectX12) {
			//トポロジーを設定
			GetGraphicsEngine().GetCommandList()->IASetPrimitiveTopology(m_topology);
			
			//描画
			if (m_indexBuffer->GetIndexNum() > 0) {
				GetGraphicsEngine().GetCommandList()->DrawIndexedInstanced(m_indexBuffer->GetIndexNum(), 1, 0, 0, 0);
			}
			else {
				GetGraphicsEngine().GetCommandList()->DrawInstanced(numVertex, 1, 0, 0);
			}
		}
	}
}
void CPrimitive::DrawIndexed() {
	if (m_indexBuffer->GetIndexNum() <= 0) {
#ifndef DW_MASTER
		char message[256];
		strcpy_s(message, "DrawIndexedに失敗(Draw()を使うべきでは?)\n");
		OutputDebugStringA(message);
#endif
		return;
	}

	Draw(-1);	
}

void VertexBufferDX11::Init(int numVertex, unsigned int vertexStride, void* vertex) {
	m_vertexSize = vertexStride;

	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = numVertex * vertexStride;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;

	D3D11_SUBRESOURCE_DATA InitData;
	ZeroMemory(&InitData, sizeof(InitData));
	InitData.pSysMem = vertex;

	GetGraphicsEngine().GetD3DDevice()->CreateBuffer(&bd, &InitData, &m_vertexBuffer);
}
void VertexBufferDX11::Attach() {
	//頂点バッファを設定
	unsigned int offset = 0;
	GetGraphicsEngine().GetD3DDeviceContext()->IASetVertexBuffers(
		0,
		1,
		m_vertexBuffer.GetAddressOf(),
		&m_vertexSize,
		&offset
	);
}
void VertexBufferDX11::Update(void* vertex) {
	GetGraphicsEngine().GetD3DDeviceContext()->UpdateSubresource(m_vertexBuffer.Get(), 0, nullptr, vertex, 0, 0);
}

void VertexBufferDX12::Init(int numVertex, unsigned int vertexStride, void* vertex) {
	const auto fullVertsSize = numVertex * vertexStride;
	m_fullVertsSize = fullVertsSize;

	//バッファ
	if (FAILED(GetGraphicsEngine().GetD3D12Device()->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(fullVertsSize),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&m_vertexBuffer)))) {
		DW_ERRORBOX(true, "VertexBufferDX12:頂点バッファの作成に失敗しました")
			return;
	}
	m_vertexBuffer->SetName(L"VertexBuffer");

	//ビュー
	m_vertexBufferView.BufferLocation = m_vertexBuffer->GetGPUVirtualAddress();
	m_vertexBufferView.StrideInBytes = vertexStride;
	m_vertexBufferView.SizeInBytes = (UINT)fullVertsSize;

	//中身のコピー
	void* pVertexDataBegin;
	const D3D12_RANGE readRange = { 0, 0 };
	if (FAILED(m_vertexBuffer->Map(0, &readRange, &pVertexDataBegin))) {
		DW_ERRORBOX(true, "VertexBufferDX12:頂点のコピーに失敗しました")
			return;
	}
	memcpy(pVertexDataBegin, vertex, fullVertsSize);
	m_vertexBuffer->Unmap(0, nullptr);
}
void VertexBufferDX12::Attach() {
	GetGraphicsEngine().GetCommandList()->IASetVertexBuffers(0, 1, &m_vertexBufferView);
}
void VertexBufferDX12::Update(void* vertex) {
	//中身のコピー
	void* pVertexDataBegin;
	const D3D12_RANGE readRange = { 0, 0 };
	if (FAILED(m_vertexBuffer->Map(0, &readRange, &pVertexDataBegin))) {
		DW_ERRORBOX(true, "VertexBufferDX12:頂点のコピーに失敗しました")
			return;
	}
	memcpy(pVertexDataBegin, vertex, m_fullVertsSize);
	m_vertexBuffer->Unmap(0, nullptr);
}

void IndexBufferDX11::Init(int numIndex, unsigned long* index) {
	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(unsigned long) * numIndex;
	bd.BindFlags = D3D11_BIND_INDEX_BUFFER;

	D3D11_SUBRESOURCE_DATA InitData;
	ZeroMemory(&InitData, sizeof(InitData));
	InitData.pSysMem = index;

	GetEngine().GetGraphicsEngine().GetD3DDevice()->CreateBuffer(&bd, &InitData, &m_indexBuffer);

	m_numIndex = numIndex;
}
void IndexBufferDX11::Attach() {
	GetEngine().GetGraphicsEngine().GetD3DDeviceContext()->IASetIndexBuffer(
		m_indexBuffer.Get(),
		DXGI_FORMAT_R32_UINT,
		0
	);
}

void IndexBufferDX12::Init(int numIndex, unsigned long* index) {
	const auto fullsize = sizeof(unsigned long) * numIndex;

	//バッファ
	if (FAILED(GetGraphicsEngine().GetD3D12Device()->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(fullsize),
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&m_indexBuffer)))) {
		DW_ERRORBOX(true, "IndexBufferDX12:作成に失敗しました")
			return;
	}
	m_indexBuffer->SetName(L"IndexBuffer");

	//ビュー
	m_indexBufferView.BufferLocation = m_indexBuffer->GetGPUVirtualAddress();
	m_indexBufferView.Format = DXGI_FORMAT_R32_UINT;
	m_indexBufferView.SizeInBytes = (UINT)fullsize;

	//コピー
	void* pIndexDataBegin;
	const D3D12_RANGE readRange = { 0, 0 };
	if (FAILED(m_indexBuffer->Map(0, &readRange, &pIndexDataBegin))) {
		DW_ERRORBOX(true, "IndexBufferDX12:コピーに失敗しました")
			return;
	}
	memcpy(pIndexDataBegin, index, fullsize);
	m_indexBuffer->Unmap(0, nullptr);

	//数
	m_numIndex = numIndex;
}
void IndexBufferDX12::Attach() {
	GetGraphicsEngine().GetCommandList()->IASetIndexBuffer(&m_indexBufferView);
}

}