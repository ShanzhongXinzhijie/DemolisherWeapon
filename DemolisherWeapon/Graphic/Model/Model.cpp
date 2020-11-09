#include "DWstdafx.h"
#include "Model.h"
#include "Graphic/Model/SkinModelShaderConst.h"

namespace DemolisherWeapon {

	namespace {
		/// <summary>
		/// パスをwhar_tへ変換...
		/// </summary>
		/// <param name="c"></param>
		/// <param name="wchar_nonMake"></param>
		void ConvertWchar(const char* c, std::unique_ptr<wchar_t[]>& wchar_nonMake) {
			size_t iReturnValue;
			size_t size = strlen(c) + 1;
			wchar_nonMake = std::make_unique<wchar_t[]>(size);
			errno_t err = mbstowcs_s(
				&iReturnValue,
				wchar_nonMake.get(),
				size, //上のサイズ
				c,
				size - 1 //コピーする最大文字数
			);
			if (err != 0) {
				DW_ERRORBOX(true, "mbstowcs_s errno:%d", err)
			}
		}
	}

	/*void VertexPositionNormalTangentColorTextureSkinning::SetBlendIndices(DirectX::XMUINT4 const& iindices)
	{
		this->indices = ((iindices.w & 0xff) << 24) | ((iindices.z & 0xff) << 16) | ((iindices.y & 0xff) << 8) | (iindices.x & 0xff);
	}
	void XM_CALLCONV VertexPositionNormalTangentColorTextureSkinning::SetBlendWeights(DirectX::FXMVECTOR iweights)
	{
		DirectX::PackedVector::XMUBYTEN4 packed;
		XMStoreUByteN4(&packed, iweights);
		this->weights = packed.v;
	}*/

	IMaterial::IMaterial(bool isSkinModel, const tkEngine::CTkmFile::SMaterial& tkmMat,int number){
		//名前設定
		std::wstring name = L"TKM_Material_" + std::to_wstring(number);
		//テクスチャ初期化
		std::unique_ptr<wchar_t[]> path;
		if (tkmMat.albedoMapFileName.size() > 0) {
			ConvertWchar(tkmMat.albedoMapFileName.c_str(), path);
			m_materialData.InitAlbedoTexture(path.get());
		}
		if (tkmMat.normalMapFileName.size() > 0) {
			ConvertWchar(tkmMat.normalMapFileName.c_str(), path);
			m_materialData.InitNormalTexture(path.get());
		}
		if (tkmMat.specularMapFileName.size() > 0) {
			ConvertWchar(tkmMat.specularMapFileName.c_str(), path);
			m_materialData.InitLightingTexture(path.get());
		}
		/*
		//リフレクションマップのファイル名をロード。
		tkmMat.reflectionMapFileName = LoadTextureFileName(fp, plusPath.c_str());
		//屈折マップのファイル名をロード。
		tkmMat.refractionMapFileName = LoadTextureFileName(fp, plusPath.c_str());
		*/

		//初期化
		m_materialData.Init(isSkinModel, name);
	}

	void MaterialDX11::Apply() {
		auto deviceContext = GetGraphicsEngine().GetD3DDeviceContext();
		auto m_ptrUseMaterialSetting = &m_materialData.GetUsingMaterialSetting();

		//シェーダーモードにおうじたシェーダをセット
		switch (GetGraphicsEngine().GetModelDrawMode().GetShaderMode()) {
		case ModelDrawMode::enZShader://Z値の描画

			//頂点シェーダ
			deviceContext->VSSetShader((ID3D11VertexShader*)m_ptrUseMaterialSetting->GetVSZ()->GetBody(), NULL, 0);

			//ピクセルシェーダ
			if (m_ptrUseMaterialSetting->GetPSZ() == m_materialData.GetDefaultPSZ(false)) {
				deviceContext->PSSetShader((ID3D11PixelShader*)m_materialData.GetDefaultPSZ(m_ptrUseMaterialSetting->GetIsUseTexZShader())->GetBody(), NULL, 0);
			}
			else {
				deviceContext->PSSetShader((ID3D11PixelShader*)m_ptrUseMaterialSetting->GetPSZ()->GetBody(), NULL, 0);
			}

			break;

		default://通常描画

			//デフォルトシェーダのマクロを切り替える
			int macroind = 0;
			if (m_ptrUseMaterialSetting->GetIsMotionBlur()) { macroind |= SkinModelEffectShader::enMotionBlur; }
			if (m_ptrUseMaterialSetting->GetNormalTexture()) { macroind |= SkinModelEffectShader::enNormalMap; }
			if (m_ptrUseMaterialSetting->GetAlbedoTexture()) { macroind |= SkinModelEffectShader::enAlbedoMap; }
			if (m_ptrUseMaterialSetting->GetLightingTexture()) { macroind |= SkinModelEffectShader::enLightingMap; }
			if (m_ptrUseMaterialSetting->GetTranslucentTexture()) { macroind |= SkinModelEffectShader::enTranslucentMap; }

			//頂点シェーダ
			deviceContext->VSSetShader((ID3D11VertexShader*)m_ptrUseMaterialSetting->GetVS().Get(macroind)->GetBody(), NULL, 0);

			//ピクセルシェーダ
			deviceContext->PSSetShader((ID3D11PixelShader*)m_ptrUseMaterialSetting->GetPS().Get(macroind)->GetBody(), NULL, 0);

			break;
		}

		//テクスチャ
		if (m_ptrUseMaterialSetting->GetAlbedoTexture()) {
			deviceContext->PSSetShaderResources(enSkinModelSRVReg_AlbedoTexture, 1, m_ptrUseMaterialSetting->GetAddressOfAlbedoTexture());
		}
		if (m_ptrUseMaterialSetting->GetNormalTexture()) {
			deviceContext->PSSetShaderResources(enSkinModelSRVReg_NormalTexture, 1, m_ptrUseMaterialSetting->GetAddressOfNormalTexture());
		}
		if (m_ptrUseMaterialSetting->GetLightingTexture()) {
			deviceContext->PSSetShaderResources(enSkinModelSRVReg_LightngTexture, 1, m_ptrUseMaterialSetting->GetAddressOfLightingTexture());
		}
		if (m_ptrUseMaterialSetting->GetTranslucentTexture()) {
			deviceContext->PSSetShaderResources(enSkinModelSRVReg_TranslucentTexture, 1, m_ptrUseMaterialSetting->GetAddressOfTranslucentTexture());
		}

		//定数バッファ
		deviceContext->UpdateSubresource(m_materialData.GetConstantBufferDX11().Get(), 0, NULL, &m_ptrUseMaterialSetting->GetMaterialParam(), 0, 0);
		deviceContext->PSSetConstantBuffers(enSkinModelCBReg_Material, 1, m_materialData.GetConstantBufferDX11().GetAddressOf());
	}

	void MaterialDX12::Apply() {
		//ここでマテリアルの処理
		//ディスクリプタ設定?
	}

	void PipelineStateDX11::Init(MaterialData& mat) {
		void const* shaderByteCode = mat.GetDefaultVS()->GetShader(SkinModelEffectShader::enALL).GetByteCode();
		size_t byteCodeLength = mat.GetDefaultVS()->GetShader(SkinModelEffectShader::enALL).GetByteCodeSize();

		if (mat.GetIsSkining())
		{
			auto hr = GetGraphicsEngine().GetD3DDevice()->CreateInputLayout(VertexPositionNormalTangentColorTextureSkinning::InputElements,
				VertexPositionNormalTangentColorTextureSkinning::InputElementCount,
				shaderByteCode, byteCodeLength,
				m_inputLayout.ReleaseAndGetAddressOf());

			DW_ERRORBOX(FAILED(hr),"PipelineStateDX11 入力レイアウトの作成に失敗")
		}
		else
		{
			auto hr = GetGraphicsEngine().GetD3DDevice()->CreateInputLayout(VertexPositionNormalTangentColorTexture::InputElements,
				VertexPositionNormalTangentColorTexture::InputElementCount,
				shaderByteCode, byteCodeLength,
				m_inputLayout.ReleaseAndGetAddressOf());
			
			DW_ERRORBOX(FAILED(hr), "PipelineStateDX11 入力レイアウトの作成に失敗")
		}
	}
	void PipelineStateDX11::Apply() {
		//VertexBufferの前
		GetGraphicsEngine().GetD3DDeviceContext()->IASetInputLayout(m_inputLayout.Get());
	}

	//TODO
	void PipelineStateDX12::Init(MaterialData& mat) {
	}
	void PipelineStateDX12::Apply() {
	}

	void CModelMeshParts::InitFromTkmFile(const tkEngine::CTkmFile& tkmFile) {
		m_meshs.resize(tkmFile.GetNumMesh());
		int meshNo = 0;
		tkmFile.QueryMeshParts([&](const tkEngine::CTkmFile::SMesh& mesh) {
			CreateMeshFromTkmMesh(mesh, meshNo);
			meshNo++;
		});
	}

	void CModelMeshParts::CreateMeshFromTkmMesh(const tkEngine::CTkmFile::SMesh& tkmMesh, int meshNo, bool isRayTrace)
	{
		//DXR実行不可
		if (GetGraphicsEngine().GetUseAPI() == enDirectX11) {
			isRayTrace = false;
		}

		int numVertex = (int)tkmMesh.vertexBuffer.size();
		int vertexStride = sizeof(tkEngine::CTkmFile::SVertex);
		int vertexStrideRTX = sizeof(tkEngine::CTkmFile::SVertex);

		//メッシュ作成
		auto mesh = std::make_unique<SModelMesh>();
		mesh->m_skinFlags.reserve(tkmMesh.materials.size());

		//頂点データ作成
		int idx = 0;
		VertexPositionNormalTangentColorTextureSkinning* vds = nullptr;
		VertexPositionNormalTangentColorTexture* vd = nullptr;
		VertexPositionNormalTangentColorTexture* vdRTX = nullptr;
		for (auto& tkmvd : tkmMesh.vertexBuffer) {
			if (tkmvd.skinWeights.x > 0.0f) {
				//スキンあり
				DW_ERRORBOX(vd != nullptr,"CModelMeshParts::CreateMeshFromTkmMesh\n頂点データ作成エラー\n※ご連絡ください")
				if (!vds) {
					vds = new VertexPositionNormalTangentColorTextureSkinning[numVertex];
					mesh->m_vertexData = dynamic_cast<VertexPositionNormalTangentColorTexture*>(vds);
					vertexStride = sizeof(VertexPositionNormalTangentColorTextureSkinning);
				}

				//コピー
				vds[idx].position = tkmvd.pos;
				vds[idx].normal = tkmvd.normal;
				vds[idx].tangent = tkmvd.tangent;
				vds[idx].textureCoordinate = tkmvd.uv;
				for (int i = 0; i < 4; i++) {
					vds[idx].indices[i] = tkmvd.indices[i];
				}
				vds[idx].weights = tkmvd.skinWeights;
			}
			else {
				//すきんなし
				DW_ERRORBOX(vds != nullptr, "CModelMeshParts::CreateMeshFromTkmMesh\n頂点データ作成エラー\n※ご連絡ください")
				if (!vd) {
					vd = new VertexPositionNormalTangentColorTexture[numVertex];
					mesh->m_vertexData = vd;
					vertexStride = sizeof(VertexPositionNormalTangentColorTexture);
				}

				//コピー
				vd[idx].position = tkmvd.pos;
				vd[idx].normal = tkmvd.normal;
				vd[idx].tangent = tkmvd.tangent;
				vd[idx].textureCoordinate = tkmvd.uv;
			}

			//レイトレ用頂点バッファ
			if (isRayTrace) {
				if (!vdRTX) {
					vdRTX = new VertexPositionNormalTangentColorTexture[numVertex];
					mesh->m_vertexDataDXR = vdRTX;
					vertexStrideRTX = sizeof(VertexPositionNormalTangentColorTexture);
				}
				//コピー
				vdRTX[idx].position = tkmvd.pos;
				vdRTX[idx].normal = tkmvd.normal;
				vdRTX[idx].tangent = tkmvd.tangent;
				vdRTX[idx].textureCoordinate = tkmvd.uv;
			}

			idx++;
		}

		//頂点バッファ作成
		if (GetGraphicsEngine().GetUseAPI() == enDirectX11) {
			mesh->m_vertexBuffer = std::make_unique<VertexBufferDX11>();
		}
		if (GetGraphicsEngine().GetUseAPI() == enDirectX12) {
			mesh->m_vertexBuffer = std::make_unique<VertexBufferDX12>();
		}
		//mesh->m_vertexBuffer->Init(numVertex, vertexStride, (void*)&tkmMesh.vertexBuffer[0]);
		mesh->m_vertexBuffer->Init(numVertex, vertexStride, (void*)&mesh->m_vertexData[0]);
		mesh->m_vertexNum = numVertex;

		//レイトレ用頂点バッファ作成
		if (isRayTrace) {
			if (GetGraphicsEngine().GetUseAPI() == enDirectX12) {
				mesh->m_vertexBufferDXR = std::make_unique<VertexBufferDX12>();
			}
			mesh->m_vertexBufferDXR->Init(numVertex, vertexStrideRTX, (void*)&mesh->m_vertexDataDXR[0]);
		}

		//スキンがあるか?
		auto SetSkinFlag = [&](int index) {
			//if (false){//
			if(tkmMesh.vertexBuffer[index].skinWeights.x > 0.0f) {
				//スキンがある。
				mesh->m_skinFlags.push_back(1);
			}
			else {
				//スキンなし。
				mesh->m_skinFlags.push_back(0);
			}
		};

		//インデックスバッファを作成。
		if (!tkmMesh.indexBuffer16Array.empty()) {
			//インデックスのサイズが2byte
			mesh->m_indexBufferArray.reserve(tkmMesh.indexBuffer16Array.size());
			for (auto& tkIb : tkmMesh.indexBuffer16Array) {
				//インデックスデータ作成
				mesh->m_indexDataArray.emplace_back();
				for (auto& srcib : tkIb.indices) {
					mesh->m_indexDataArray.back().emplace_back(srcib);
				}

				//インデックスバッファの初期化
				std::unique_ptr<IIndexBuffer> ib;
				if (GetGraphicsEngine().GetUseAPI() == enDirectX11) {
					ib = std::make_unique<IndexBufferDX11>();
				}
				if (GetGraphicsEngine().GetUseAPI() == enDirectX12) {
					ib = std::make_unique<IndexBufferDX12>();
				}
				ib->Init((int)tkIb.indices.size(), mesh->m_indexDataArray.back().data());

				//スキンがあるかどうかを設定する。
				SetSkinFlag(tkIb.indices[0]);

				//配列にインデックスバッファを収める
				mesh->m_indexBufferArray.push_back(std::move(ib));
			}
		}
		else {
			//インデックスのサイズが4byte
			mesh->m_indexBufferArray.reserve(tkmMesh.indexBuffer32Array.size());
			for (auto& tkIb : tkmMesh.indexBuffer32Array) {
				//インデックスデータ作成
				mesh->m_indexDataArray.emplace_back();
				for (auto& srcib : tkIb.indices) {
					mesh->m_indexDataArray.back().emplace_back(srcib);
				}

				//インデックスバッファの初期化
				std::unique_ptr<IIndexBuffer> ib;
				if (GetGraphicsEngine().GetUseAPI() == enDirectX11) {
					ib = std::make_unique<IndexBufferDX11>();
				}
				if (GetGraphicsEngine().GetUseAPI() == enDirectX12) {
					ib = std::make_unique<IndexBufferDX12>();
				}
				ib->Init((int)tkIb.indices.size(), mesh->m_indexDataArray.back().data());

				//スキンがあるかどうかを設定する。
				SetSkinFlag(tkIb.indices[0]);

				//配列にインデックスバッファを収める
				mesh->m_indexBufferArray.push_back(std::move(ib));
			}
		}
		//マテリアルを作成。
		//パイプラインステートも
		int ind = 0;
		mesh->m_materials.reserve(tkmMesh.materials.size());
		mesh->m_pipelineState.reserve(tkmMesh.materials.size());
		for (auto& tkmMat : tkmMesh.materials) {
			//作成
			std::unique_ptr<IMaterial> mat;
			std::unique_ptr<IPipelineState> pso;

			if (GetGraphicsEngine().GetUseAPI() == enDirectX11) {
				 mat = std::make_unique<MaterialDX11>(mesh->m_skinFlags[ind], tkmMat, ind);
				 pso = std::make_unique<PipelineStateDX11>();
				 pso->Init(mat->GetMaterialData());
			}
			if (GetGraphicsEngine().GetUseAPI() == enDirectX12) {
				mat = std::make_unique<MaterialDX12>(mesh->m_skinFlags[ind], tkmMat, ind);
				pso = std::make_unique<PipelineStateDX12>();
				pso->Init(mat->GetMaterialData());
			}

			//保存
			mesh->m_materials.push_back(std::move(mat));
			mesh->m_pipelineState.push_back(std::move(pso));

			ind++;
		}

		m_meshs[meshNo] = std::move(mesh);
	}

	void CModelMeshParts::Draw(int instanceNum) {
		for (auto& mesh : m_meshs) {
			//頂点バッファを設定
			mesh->m_vertexBuffer->Attach();
			//マテリアルごとにドロー。
			for (int matNo = 0; matNo < mesh->m_materials.size(); matNo++) {
				//パイプラインステートを設定
				mesh->m_pipelineState[matNo]->Apply();
				//インデックスバッファを設定
				mesh->m_indexBufferArray[matNo]->Attach();
				//マテリアルの設定
				mesh->m_materials[matNo]->Apply();

				if (GetGraphicsEngine().GetUseAPI() == enDirectX11) {
					//トポロジーを設定
					GetGraphicsEngine().GetD3DDeviceContext()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
					//描画
					if (instanceNum == 1) {
						GetGraphicsEngine().GetD3DDeviceContext()->DrawIndexed(mesh->m_indexBufferArray[matNo]->GetIndexNum(), 0, 0);
					}
					else {
						//インスタンシング
						GetGraphicsEngine().GetD3DDeviceContext()->DrawIndexedInstanced(mesh->m_indexBufferArray[matNo]->GetIndexNum(), instanceNum, 0, 0, 0);
					}
				}
				if (GetGraphicsEngine().GetUseAPI() == enDirectX12) {
					//トポロジーを設定
					GetGraphicsEngine().GetCommandList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
					//描画
					GetGraphicsEngine().GetCommandList()->DrawIndexedInstanced(mesh->m_indexBufferArray[matNo]->GetIndexNum(), instanceNum, 0, 0, 0);
				}
			}
		}

		/// <summary>
		/// //
		/// </summary>
		/// 
		/*if (m_isCreateDescriptorHeap == false) {
			//ディスクリプタヒープを作成。
			CreateDescriptorHeaps();
		}

		auto& ge12 = g_graphicsEngine->As<CGraphicsEngineDx12>();
		//レンダリングコンテキストをDx12版にダウンキャスト
		auto& rc12 = rc.As<CRenderContextDx12>();
		auto& lightMgr = ge12.GetLightManager()->As<CLightManagerDx12>();

		//メッシュごとにドロー
		//プリミティブのトポロジーはトライアングルリストのみ。
		rc12.SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		//定数バッファを更新する。
		SConstantBuffer cb;
		cb.mWorld = mWorld;
		cb.mView = mView;
		cb.mProj = mProj;
		m_commonConstantBuffer.Update(&cb);

		if (m_boneMatricesStructureBuffer.IsInited()) {
			//ボーン行列を更新する。
			m_boneMatricesStructureBuffer.Update(m_skeleton->GetBoneMatricesTopAddress());
		}

		int descriptorHeapNo = 0;
		for (auto& mesh : m_meshs) {
			//頂点バッファを設定。
			rc12.SetVertexBuffer(mesh->m_vertexBuffer);
			//マテリアルごとにドロー。
			for (int matNo = 0; matNo < mesh->m_materials.size(); matNo++) {
				//このマテリアルが貼られているメッシュの描画開始。
				mesh->m_materials[matNo]->BeginRender(rc, mesh->skinFlags[matNo]);

				auto& descriptorHeap = m_descriptorHeap.at(descriptorHeapNo);

				rc12.SetDescriptorHeap(descriptorHeap);
				descriptorHeapNo++;
				//インデックスバッファを設定。
				auto& ib = mesh->m_indexBufferArray[matNo];
				rc12.SetIndexBuffer(ib);

				//ドロー。
				rc12.DrawIndexed(ib->GetCount());
			}
		}*/
	}

	void CModel::LoadTkmFileAsync(const char* filePath)
	{
		m_tkmFile.LoadAsync(filePath);
	}
	void CModel::LoadTkmFile(const char* filePath)
	{
		m_tkmFile.Load(filePath);
	}
	bool CModel::IsInited() const
	{
		return m_tkmFile.IsLoaded();
	}
	void CModel::CreateMeshParts()
	{
		if (m_tkmFile.IsLoaded() == false) {
			DW_WARNING_BOX(true, "この関数はtkmファイルのロードが完了してから呼び出してください。");
			return;
		}
		m_meshParts.InitFromTkmFile(m_tkmFile);
	}
	void CModel::Draw(int instanceNum)
	{
		m_meshParts.Draw(instanceNum);
	}

}