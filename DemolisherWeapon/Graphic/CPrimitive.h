#pragma once

#include"model/tktkmfile.h"

namespace DemolisherWeapon {

//���_�\����
struct SVertex {
	float position[4] = { 0.0f, 0.0f ,0.0f ,0.0f };
	float uv[2] = { 0.0f,0.0f };

	//���_���C�A�E�g
	static inline D3D12_INPUT_ELEMENT_DESC vertexLayout[] =
	{
		{ "SV_Position", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 16, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
	};
};

//���_�o�b�t�@
class IVertexBuffer {
public:
	virtual ~IVertexBuffer() {}
	virtual void Init(int numVertex, unsigned int vertexStride, void* vertex) = 0;
	virtual void Attach() = 0;
	virtual void Update(void* vertex) = 0;
};
//directx11
class VertexBufferDX11 : public IVertexBuffer {
public:
	void Init(int numVertex, unsigned int vertexStride, void* vertex)override;
	void Attach()override;
	void Update(void* vertex)override;
private:
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_vertexBuffer;
	unsigned int m_vertexSize = 0;
};
//directx12
class VertexBufferDX12 : public IVertexBuffer {
public:
	void Init(int numVertex, unsigned int vertexStride, void* vertexoverride);
	void Attach()override;
	void Update(void* vertex)override;
private:
	Microsoft::WRL::ComPtr<ID3D12Resource> m_vertexBuffer;
	D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView;
	unsigned long long m_fullVertsSize = 0;
};

//�C���f�b�N�X�o�b�t�@
class IIndexBuffer {
public:
	virtual ~IIndexBuffer() {}
	virtual void Init(int numIndex, unsigned long* index) = 0;
	virtual void Attach() = 0;

	//virtual void* OpenReadPointer() = 0;
	//virtual void CloseReadPointer() = 0;

	int GetIndexNum()const { return m_numIndex; }

protected:
	int m_numIndex = 0;
};
//directx11
class IndexBufferDX11 : public IIndexBuffer {
	void Init(int numIndex, unsigned long* index)override;
	void Attach()override;

	//void* OpenReadPointer() override;
	//void CloseReadPointer() override;
private:
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_indexBuffer;
};
//directx12
class IndexBufferDX12 : public IIndexBuffer {
	void Init(int numIndex, unsigned long* index)override;
	void Attach()override;

	//void* OpenReadPointer() override;
	//void CloseReadPointer() override;
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

	//�ʏ�
	void Init(D3D_PRIMITIVE_TOPOLOGY topology, int numVertex, SVertex* vertex);
	void Draw(int numVertex);
	//�C���f�b�N�X�g�p
	void Init(D3D_PRIMITIVE_TOPOLOGY topology, int numVertex, SVertex* vertex, int numIndex, unsigned long* index);
	void DrawIndexed();

	//���_���X�V
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