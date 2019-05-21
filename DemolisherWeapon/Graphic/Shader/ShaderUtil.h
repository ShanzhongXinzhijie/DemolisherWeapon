#pragma once

namespace DemolisherWeapon {

	class ShaderUtil
	{
	public:
		/// <summary>
		/// 定数バッファの作成
		/// サイズが16バイトの倍数になるように確保される。
		/// ※なお、データは16バイト境界を跨いではならない。
		/// https://www.3dgep.com/texturing-lighting-directx-11/#Packing_Rules_for_Constant_Buffers
		/// </summary>
		/// <param name="size">定数バッファのサイズ</param>
		/// <param name="buffer">定数バッファのポインタが返ってくる</param>
		static void CreateConstantBuffer(int size, ID3D11Buffer** buffer);
	};

}