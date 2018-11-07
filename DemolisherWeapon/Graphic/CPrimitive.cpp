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
	if (m_vertexBuffer) { m_vertexBuffer->Release(); m_vertexBuffer = nullptr; }
	if (m_indexBuffer) { m_indexBuffer->Release(); m_indexBuffer = nullptr; }

	m_isInit = false;
}

void CPrimitive::Init(D3D_PRIMITIVE_TOPOLOGY topology, int numVertex, SVertex* vertex, int numIndex, int* index) {

	m_topology = topology;

	//頂点バッファの作成
	{
		D3D11_BUFFER_DESC bd;
		ZeroMemory(&bd, sizeof(bd));
		bd.Usage = D3D11_USAGE_DEFAULT;
		bd.ByteWidth = numVertex * sizeof(SVertex);
		bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;

		D3D11_SUBRESOURCE_DATA InitData;
		ZeroMemory(&InitData, sizeof(InitData));
		InitData.pSysMem = vertex;

		GetEngine().GetGraphicsEngine().GetD3DDevice()->CreateBuffer(&bd, &InitData, &m_vertexBuffer);
	}	

	//インデックスバッファの作成
	if(numIndex > 0){
		D3D11_BUFFER_DESC bd;
		ZeroMemory(&bd, sizeof(bd));
		bd.Usage = D3D11_USAGE_DEFAULT;
		bd.ByteWidth = 4 * numIndex;//DXGI_FORMAT_R32_UINT
		bd.BindFlags = D3D11_BIND_INDEX_BUFFER;

		D3D11_SUBRESOURCE_DATA InitData;
		ZeroMemory(&InitData, sizeof(InitData));
		InitData.pSysMem = index;

		GetEngine().GetGraphicsEngine().GetD3DDevice()->CreateBuffer(&bd, &InitData, &m_indexBuffer);

		m_numIndex = numIndex;
	}

	m_isInit = true;
}
void CPrimitive::Init(D3D_PRIMITIVE_TOPOLOGY topology, int numVertex, SVertex* vertex) {
	Init(topology, numVertex, vertex, 0, nullptr);
}

void CPrimitive::Draw(int numVertex) {
	if (m_isInit) {

		//頂点バッファを設定
		unsigned int vertexSize = sizeof(SVertex);
		unsigned int offset = 0;
		GetEngine().GetGraphicsEngine().GetD3DDeviceContext()->IASetVertexBuffers(
			0,
			1,
			&m_vertexBuffer,
			&vertexSize,
			&offset
		);
		//インデックスバッファを設定
		if (m_numIndex > 0) {
			GetEngine().GetGraphicsEngine().GetD3DDeviceContext()->IASetIndexBuffer(
				m_indexBuffer,
				DXGI_FORMAT_R32_UINT,
				0
			);
		}
		//トポロジーを設定
		GetEngine().GetGraphicsEngine().GetD3DDeviceContext()->IASetPrimitiveTopology(m_topology);

		//描画
		if (m_numIndex > 0) {
			//インデックス使用
			GetEngine().GetGraphicsEngine().GetD3DDeviceContext()->DrawIndexed(
				m_numIndex,
				0,
				0
			);
		}
		else {
			//通常
			GetEngine().GetGraphicsEngine().GetD3DDeviceContext()->Draw(numVertex,0);
		}
	}
}
void CPrimitive::DrawIndexed() {
	if (m_numIndex <= 0) {
#ifdef _DEBUG
		char message[256];
		strcpy_s(message, "DrawIndexedに失敗(Draw()を使うべきでは?)\n");
		OutputDebugStringA(message);
#endif
		return;
	}

	Draw(-1);	
}

}