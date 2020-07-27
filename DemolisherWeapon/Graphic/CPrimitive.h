#pragma once

namespace DemolisherWeapon {

//頂点構造体
struct SVertex {
	float position[4] = { 0.0f, 0.0f ,0.0f ,0.0f };
	float uv[2] = { 0.0f,0.0f };

	//頂点レイアウト
	static inline D3D12_INPUT_ELEMENT_DESC vertexLayout[] =
	{
		{ "SV_Position", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 16, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
	};
};

//頂点バッファ
class IVertexBuffer {
public:
	virtual ~IVertexBuffer() {}
	virtual void Init(int numVertex, SVertex* vertex) = 0;
	virtual void Attach() = 0;
	virtual void Update(SVertex* vertex) = 0;
};
//directx11
class VertexBufferDX11 : public IVertexBuffer {
public:
	void Init(int numVertex, SVertex* vertex)override;
	void Attach()override;
	void Update(SVertex* vertex)override;
private:
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_vertexBuffer;
};
//directx12
class VertexBufferDX12 : public IVertexBuffer {
public:
	void Init(int numVertex, SVertex* vertex);
	void Attach()override;
	void Update(SVertex* vertex)override;
private:
	Microsoft::WRL::ComPtr<ID3D12Resource> m_vertexBuffer;
	D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView;
	unsigned long long m_fullVertsSize = 0;
};

//インデックスバッファ
class IIndexBuffer {
public:
	virtual ~IIndexBuffer() {}
	virtual void Init(int numIndex, unsigned long* index) = 0;
	virtual void Attach() = 0;

	int GetIndexNum()const { return m_numIndex; }

protected:
	int m_numIndex = 0;
};
//directx11
class IndexBufferDX11 : public IIndexBuffer {
	void Init(int numIndex, unsigned long* index)override;
	void Attach()override;
private:
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_indexBuffer;
};
//directx12
class IndexBufferDX12 : public IIndexBuffer {
	void Init(int numIndex, unsigned long* index)override;
	void Attach()override;
private:
	Microsoft::WRL::ComPtr<ID3D12Resource> m_indexBuffer;
	D3D12_INDEX_BUFFER_VIEW m_indexBufferView;
};

class CPrimitive
{
public:
	CPrimitive();
	~CPrimitive();

	void Release();

	//通常
	void Init(D3D_PRIMITIVE_TOPOLOGY topology, int numVertex, SVertex* vertex);
	void Draw(int numVertex);
	//インデックス使用
	void Init(D3D_PRIMITIVE_TOPOLOGY topology, int numVertex, SVertex* vertex, int numIndex, unsigned long* index);
	void DrawIndexed();

	//頂点を更新
	void UpdateVertex(SVertex* vertex) {
		if (!m_isInit) { return; }
		m_vertexBuffer->Update(vertex);
	}

private:
	bool m_isInit = false;	

	D3D_PRIMITIVE_TOPOLOGY m_topology;
	std::unique_ptr<IVertexBuffer> m_vertexBuffer;
	std::unique_ptr<IIndexBuffer> m_indexBuffer;
};

}