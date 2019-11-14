#pragma once

namespace DemolisherWeapon {

class CPrimitive
{
public:
	//頂点構造体
	struct SVertex {
		float position[4] = { 0.0f, 0.0f ,0.0f ,0.0f };
		float uv[2] = { 0.0f,0.0f };
	};

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

	//頂点バッファ取得
	ID3D11Buffer* GetVertexBuffer() const{
		return m_vertexBuffer;
	}

private:
	bool m_isInit = false;	

	ID3D11Buffer* m_vertexBuffer = nullptr;
	ID3D11Buffer* m_indexBuffer = nullptr;
	D3D_PRIMITIVE_TOPOLOGY	m_topology;

	int m_numIndex = 0;
};

}