#pragma once

namespace DemolisherWeapon {

	class ShaderUtil
	{
	public:
		//定数バッファの作成
		static void CreateConstantBuffer(int size, ID3D11Buffer** buffer);
	};

}