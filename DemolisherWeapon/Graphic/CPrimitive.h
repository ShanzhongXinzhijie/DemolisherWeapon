#pragma once

namespace DemolisherWeapon {

class CPrimitive
{
public:
	//í∏ì_ç\ë¢ëÃ
	struct SVertex {
		float position[4];
		float uv[2];
	};

public:
	CPrimitive();
	~CPrimitive();

	void Release();

	void Init(D3D_PRIMITIVE_TOPOLOGY topology, int numVertex, SVertex* vertex, int numIndex, int* index);
	void Draw();

private:
	bool m_isInit = false;	

	ID3D11Buffer* m_vertexBuffer = nullptr;
	ID3D11Buffer* m_indexBuffer = nullptr;
	D3D_PRIMITIVE_TOPOLOGY	m_topology;

	int m_numIndex;
};

}