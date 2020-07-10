#include "DWstdafx.h"
#include "MeshData.h"

namespace DemolisherWeapon {
	namespace Physics2 {

		void MeshData::CreateFromSkinModel(const SkinModel& model, const CMatrix* offsetMatrix) {
#ifndef DW_DX12_TEMPORARY
			//バイアス行列の作成
			CMatrix mBias, mBiasScr;
			CoordinateSystemBias::GetBias(mBias, mBiasScr, model.GetFBXUpAxis(), model.GetFBXCoordinateSystem());
			mBias.Mul(mBiasScr, mBias);
			//オフセット行列と混ぜる
			if (offsetMatrix != nullptr) {
				mBias.Mul(mBias, (*offsetMatrix));
			}

			//メッシュの読み出し
			model.FindMesh([&](const auto& mesh) {
				ID3D11DeviceContext* deviceContext = GetEngine().GetGraphicsEngine().GetD3DDeviceContext();
				//頂点バッファを作成。
				{
					//頂点バッファのポインタを手に入れる
					D3D11_MAPPED_SUBRESOURCE subresource;
					HRESULT hr = deviceContext->Map(mesh->vertexBuffer.Get(), 0, D3D11_MAP_WRITE_NO_OVERWRITE, 0, &subresource);
					if (FAILED(hr)) {
						return;
					}

					//頂点数を求める
					D3D11_BUFFER_DESC bufferDesc;
					mesh->vertexBuffer->GetDesc(&bufferDesc);
					int vertexCount = bufferDesc.ByteWidth / mesh->vertexStride;

					//頂点座標を取得
					char* pData = reinterpret_cast<char*>(subresource.pData);
					CVector3 pos;
					for (int i = 0; i < vertexCount; i++) {
						//座標にキャスト
						pos = *reinterpret_cast<CVector3*>(pData);
						//バイアスをかける。
						mBias.Mul(pos);
						//保存
						m_vertexBuffer.push_back(pos);
						//次の頂点へ。
						pData += mesh->vertexStride;
					}

					//頂点バッファのポインタを破棄(GPUアクセス可能に)
					deviceContext->Unmap(mesh->vertexBuffer.Get(), 0);
				}
				//インデックスバッファを作成。
				{
					//インデックスバッファのポインタを手に入れる
					D3D11_MAPPED_SUBRESOURCE subresource;
					HRESULT hr = deviceContext->Map(mesh->indexBuffer.Get(), 0, D3D11_MAP_WRITE_NO_OVERWRITE, 0, &subresource);
					if (FAILED(hr)) {
						return;
					}

					//インデックス数を求める
					D3D11_BUFFER_DESC bufferDesc;
					mesh->indexBuffer->GetDesc(&bufferDesc);					
					int stride = 2;//@todo cmoファイルはインデックスバッファのサイズは2byte固定。
					int indexCount = bufferDesc.ByteWidth / stride;

					//インデックス取得
					unsigned short* pIndex = reinterpret_cast<unsigned short*>(subresource.pData);
					Index index;
					for (int i = 0; i < indexCount; i++) {
						index.push_back(pIndex[i]);
					}
					m_indexBuffer.push_back(index);

					//インデックスバッファのポインタを破棄(GPUアクセス可能に)
					deviceContext->Unmap(mesh->indexBuffer.Get(), 0);
				}
			});
#endif
		}

	}
}
