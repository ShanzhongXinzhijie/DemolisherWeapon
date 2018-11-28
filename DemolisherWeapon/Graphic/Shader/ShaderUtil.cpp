#include "DWstdafx.h"
#include "ShaderUtil.h"

namespace DemolisherWeapon {
	//定数バッファの作成
	void ShaderUtil::CreateConstantBuffer(int size, ID3D11Buffer** buffer) {
		int bufferSize = size;
		D3D11_BUFFER_DESC bufferDesc;
		ZeroMemory(&bufferDesc, sizeof(bufferDesc));
		bufferDesc.Usage = D3D11_USAGE_DEFAULT;
		bufferDesc.ByteWidth = (((bufferSize - 1) / 16) + 1) * 16;
		bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		GetEngine().GetGraphicsEngine().GetD3DDevice()->CreateBuffer(&bufferDesc, NULL, buffer);
	}
}