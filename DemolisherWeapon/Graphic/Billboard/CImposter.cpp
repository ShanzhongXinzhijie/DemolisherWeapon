#include "DWstdafx.h"
#include "CImposter.h"
#include "Graphic\Model\SkinModelShaderConst.h"

namespace DemolisherWeapon {
	ImposterTexBank* ImposterTexBank::instance = nullptr;

//インポスターのインスタンシング描画における拡大率を扱うクラス
	void InstancingImposterParamManager::Reset(int instancingMaxNum) {
		m_instanceMax = instancingMaxNum;

		//インデックス配列の確保
		m_params = std::make_unique<CVector2[]>(instancingMaxNum);
		m_paramsCache = std::make_unique<CVector2[]>(instancingMaxNum);

		//TODO
		int stride = sizeof(CVector2);
		
		//StructuredBufferの確保
		D3D11_BUFFER_DESC desc;
		ZeroMemory(&desc, sizeof(desc));
		desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		desc.ByteWidth = static_cast<UINT>(stride * instancingMaxNum);
		desc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
		desc.StructureByteStride = stride;
		GetGraphicsEngine().GetD3DDevice()->CreateBuffer(&desc, NULL, m_paramsSB.ReleaseAndGetAddressOf());

		//ShaderResourceViewの確保
		D3D11_SHADER_RESOURCE_VIEW_DESC descSRV;
		ZeroMemory(&descSRV, sizeof(descSRV));
		descSRV.ViewDimension = D3D11_SRV_DIMENSION_BUFFEREX;
		descSRV.BufferEx.FirstElement = 0;
		descSRV.Format = DXGI_FORMAT_UNKNOWN;
		descSRV.BufferEx.NumElements = desc.ByteWidth / desc.StructureByteStride;
		GetGraphicsEngine().GetD3DDevice()->CreateShaderResourceView(m_paramsSB.Get(), &descSRV, m_paramsSRV.ReleaseAndGetAddressOf());
	}
	InstancingImposterParamManager::InstancingImposterParamManager(int instancingMaxNum, ImposterTexRender* tex) : m_texture(tex) {
		Reset(instancingMaxNum);
	}
	void InstancingImposterParamManager::PreDraw(int instanceNum, int drawInstanceNum, const std::unique_ptr<bool[]>& drawInstanceMask) {
		//カリングされてないもののみコピー
		int drawNum = 0;
		for (int i = 0; i < instanceNum; i++) {
			if (drawInstanceMask[i]) {
				m_params[drawNum] = m_paramsCache[i];
				drawNum++;
			}
		}
		//StructuredBufferを更新
		GetGraphicsEngine().GetD3DDeviceContext()->UpdateSubresource(
			m_paramsSB.Get(), 0, NULL, m_params.get(), 0, 0
		);
		//シェーダーリソースにセット
		GetGraphicsEngine().GetD3DDeviceContext()->VSSetShaderResources(
			enSkinModelSRVReg_InstancingImposterScale, 1, m_paramsSRV.GetAddressOf()
		);
	}
	void InstancingImposterParamManager::AddDrawInstance(int instanceIndex, const CMatrix& SRTMatrix, const CVector3& scale, void *param) {
		m_paramsCache[instanceIndex].x = scale.x / (m_texture->GetModelSize()*2.0f);
		AddRotY(instanceIndex, *(float*)param);
	}
	void InstancingImposterParamManager::AddRotY(int instanceIndex, float rad) {
		m_paramsCache[instanceIndex].y = rad;
	}
	void InstancingImposterParamManager::SetInstanceMax(int instanceMax) {
		if (instanceMax > m_instanceMax) {
			Reset(instanceMax);
		}
	}
	
//インポスターテクスチャ
	void ImposterTexRender::Init(const wchar_t* filepath, const CVector2& resolution, const CVector2& partNum) {
		//解像度・分割数設定
		m_gbufferSizeX = (UINT)resolution.x; m_gbufferSizeY = (UINT)resolution.y;
		m_partNumX = (UINT)partNum.x; m_partNumY = (UINT)partNum.y;
		
		GraphicsEngine& ge = GetEngine().GetGraphicsEngine();

		//テクスチャ作成
		D3D11_TEXTURE2D_DESC texDesc;
		ZeroMemory(&texDesc, sizeof(texDesc));
		texDesc.Width = (UINT)resolution.x;
		texDesc.Height = (UINT)resolution.y;
		texDesc.MipLevels = 1;
		texDesc.ArraySize = 1;
		texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		texDesc.SampleDesc.Count = 1;
		texDesc.SampleDesc.Quality = 0;
		texDesc.Usage = D3D11_USAGE_DEFAULT;
		texDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
		texDesc.CPUAccessFlags = 0;
		texDesc.MiscFlags = 0;

		//アルベド
		ge.GetD3DDevice()->CreateTexture2D(&texDesc, NULL, m_GBufferTex[enGBufferAlbedo].ReleaseAndGetAddressOf());
		ge.GetD3DDevice()->CreateRenderTargetView(m_GBufferTex[enGBufferAlbedo].Get(), nullptr, m_GBufferView[enGBufferAlbedo].ReleaseAndGetAddressOf());//レンダーターゲット
		ge.GetD3DDevice()->CreateShaderResourceView(m_GBufferTex[enGBufferAlbedo].Get(), nullptr, m_GBufferSRV[enGBufferAlbedo].ReleaseAndGetAddressOf());//シェーダーリソースビュー

		//ライトパラメーター
		texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		ge.GetD3DDevice()->CreateTexture2D(&texDesc, NULL, m_GBufferTex[enGBufferLightParam].ReleaseAndGetAddressOf());
		ge.GetD3DDevice()->CreateRenderTargetView(m_GBufferTex[enGBufferLightParam].Get(), nullptr, m_GBufferView[enGBufferLightParam].ReleaseAndGetAddressOf());//レンダーターゲット
		ge.GetD3DDevice()->CreateShaderResourceView(m_GBufferTex[enGBufferLightParam].Get(), nullptr, m_GBufferSRV[enGBufferLightParam].ReleaseAndGetAddressOf());//シェーダーリソースビュー

		//法線
		texDesc.Format = DXGI_FORMAT_R11G11B10_FLOAT;
		ge.GetD3DDevice()->CreateTexture2D(&texDesc, NULL, m_GBufferTex[enGBufferNormal].ReleaseAndGetAddressOf());
		ge.GetD3DDevice()->CreateRenderTargetView(m_GBufferTex[enGBufferNormal].Get(), nullptr, m_GBufferView[enGBufferNormal].ReleaseAndGetAddressOf());//レンダーターゲット
		ge.GetD3DDevice()->CreateShaderResourceView(m_GBufferTex[enGBufferNormal].Get(), nullptr, m_GBufferSRV[enGBufferNormal].ReleaseAndGetAddressOf());//シェーダーリソースビュー

		//トランスルーセント
		texDesc.Format = DXGI_FORMAT_R8_UNORM;
		ge.GetD3DDevice()->CreateTexture2D(&texDesc, NULL, m_GBufferTex[enGBufferTranslucent].ReleaseAndGetAddressOf());
		ge.GetD3DDevice()->CreateRenderTargetView(m_GBufferTex[enGBufferTranslucent].Get(), nullptr, m_GBufferView[enGBufferTranslucent].ReleaseAndGetAddressOf());//レンダーターゲット
		ge.GetD3DDevice()->CreateShaderResourceView(m_GBufferTex[enGBufferTranslucent].Get(), nullptr, m_GBufferSRV[enGBufferTranslucent].ReleaseAndGetAddressOf());//シェーダーリソースビュー

		//モデル読み込み
		SkinModel model;
		model.Init(filepath);
		model.UpdateWorldMatrix(0.0f, CQuaternion::Identity(), 1.0f);
		model.SetIsFrustumCulling(false);//視錐台カリングの無効化//なぜかこれがいる

		//バイアス行列取得
		CMatrix mBias, mBiasScr;
		CoordinateSystemBias::GetBias(mBias, mBiasScr, enFbxUpAxisZ, enFbxRightHanded);
		mBias.Mul(mBiasScr, mBias);

		//バウンディングボックスからモデルのサイズを求める
		m_imposterMaxSize = 0.0f;
		m_boundingBoxMaxSize = { 0.0f };
		m_boundingBoxMinSize = { 0.0f };
		bool isFirst = true;
		model.FindMeshes(
			[&](const std::shared_ptr<DirectX::ModelMesh>& meshes) {
				CVector3 size, extents;
				extents = meshes->boundingBox.Extents;

				//最大値
				size = meshes->boundingBox.Center;
				size += extents;
				mBias.Mul3x3(size);//バイアスの適応
				if (isFirst) {
					m_boundingBoxMaxSize = size;
					m_boundingBoxMinSize = size;
				}
				else {
					m_boundingBoxMaxSize.x = max(m_boundingBoxMaxSize.x, size.x);
					m_boundingBoxMaxSize.y = max(m_boundingBoxMaxSize.y, size.y);
					m_boundingBoxMaxSize.z = max(m_boundingBoxMaxSize.z, size.z);
					m_boundingBoxMinSize.x = min(m_boundingBoxMinSize.x, size.x);
					m_boundingBoxMinSize.y = min(m_boundingBoxMinSize.y, size.y);
					m_boundingBoxMinSize.z = min(m_boundingBoxMinSize.z, size.z);
				}
				size.Abs();
				m_imposterMaxSize = max(m_imposterMaxSize, max(size.y, max(size.x, size.z)));

				//最小値
				size = meshes->boundingBox.Center;
				size -= extents;
				mBias.Mul3x3(size);//バイアスの適応
				if (isFirst) {
					m_boundingBoxMaxSize = size;
					m_boundingBoxMinSize = size;
				}
				else {
					m_boundingBoxMaxSize.x = max(m_boundingBoxMaxSize.x, size.x);
					m_boundingBoxMaxSize.y = max(m_boundingBoxMaxSize.y, size.y);
					m_boundingBoxMaxSize.z = max(m_boundingBoxMaxSize.z, size.z);
					m_boundingBoxMinSize.x = min(m_boundingBoxMinSize.x, size.x);
					m_boundingBoxMinSize.y = min(m_boundingBoxMinSize.y, size.y);
					m_boundingBoxMinSize.z = min(m_boundingBoxMinSize.z, size.z);
				}
				size.Abs();
				m_imposterMaxSize = max(m_imposterMaxSize, max(size.y, max(size.x, size.z)));

				isFirst = false;
			}
		);
	
		//モデルのカメラ方向の大きさ
		m_toCamDirSize = std::make_unique<float[]>(m_partNumX*m_partNumY);
		//StructuredBufferの確保
		D3D11_BUFFER_DESC desc;
		ZeroMemory(&desc, sizeof(desc));
		int stride = sizeof(float);
		desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		desc.ByteWidth = static_cast<UINT>(stride * (m_partNumX*m_partNumY));
		desc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
		desc.StructureByteStride = stride;
		GetGraphicsEngine().GetD3DDevice()->CreateBuffer(&desc, NULL, m_toCamDirSizeSB.ReleaseAndGetAddressOf());
		//ShaderResourceViewの確保
		D3D11_SHADER_RESOURCE_VIEW_DESC descSRV;
		ZeroMemory(&descSRV, sizeof(descSRV));
		descSRV.ViewDimension = D3D11_SRV_DIMENSION_BUFFEREX;
		descSRV.BufferEx.FirstElement = 0;
		descSRV.Format = DXGI_FORMAT_UNKNOWN;
		descSRV.BufferEx.NumElements = desc.ByteWidth / desc.StructureByteStride;
		GetGraphicsEngine().GetD3DDevice()->CreateShaderResourceView(m_toCamDirSizeSB.Get(), &descSRV, m_toCamDirSizeSRV.ReleaseAndGetAddressOf());

		//インポスタテクスチャの作成
		Render(model);

		//StructuredBufferを更新
		GetGraphicsEngine().GetD3DDeviceContext()->UpdateSubresource(
			m_toCamDirSizeSB.Get(), 0, NULL, m_toCamDirSize.get(), 0, 0
		);		
	}

	void ImposterTexRender::VSSetSizeToCameraSRV() {
		//シェーダーリソースにセット
		GetGraphicsEngine().GetD3DDeviceContext()->VSSetShaderResources(
			enSkinModelSRVReg_ImposterSizeToCamera, 1, m_toCamDirSizeSRV.GetAddressOf()
		);
	}

	void ImposterTexRender::Render(SkinModel& model) {
		//GPUイベントの開始
		GetGraphicsEngine().BeginGPUEvent(L"RenderImposter");

		ID3D11DeviceContext* DC = GetGraphicsEngine().GetD3DDeviceContext();

		//カメラのセットアップ		
		GameObj::NoRegisterOrthoCamera imposterCam;
		imposterCam.SetWidth(m_imposterMaxSize*2.0f);
		imposterCam.SetHeight(m_imposterMaxSize*2.0f);
		imposterCam.SetPos({ 0.0f,0.0f,CVector3(m_imposterMaxSize).Length() + 100.0f });
		imposterCam.SetTarget(CVector3::Zero());
		imposterCam.SetUp(CVector3::Up());
		imposterCam.SetFar((CVector3(m_imposterMaxSize).Length() + 100.0f)*2.0f);
		//※このカメラは手動で更新する...
		imposterCam.UpdateMatrix();

		//カメラ保存
		GameObj::ICamera* beforeCam = GetMainCamera();
		//カメラ変更
		SetMainCamera(&imposterCam);

		//ビューポート記録
		D3D11_VIEWPORT beforeViewport; UINT kaz = 1;
		DC->RSGetViewports(&kaz, &beforeViewport);


		//デプス
		Microsoft::WRL::ComPtr<ID3D11Texture2D>			depthStencilTex; //デプスステンシルテクスチャ
		Microsoft::WRL::ComPtr<ID3D11DepthStencilView>	depthStencilView;//デプスステンシルビュー
		//デプステクスチャ
		D3D11_TEXTURE2D_DESC texDesc;
		m_GBufferTex[0]->GetDesc(&texDesc);
		texDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
		texDesc.Format = DXGI_FORMAT_R32_TYPELESS;
		GetGraphicsEngine().GetD3DDevice()->CreateTexture2D(&texDesc, NULL, depthStencilTex.ReleaseAndGetAddressOf());		
		//デプスステンシル
		D3D11_DEPTH_STENCIL_VIEW_DESC dsv_desc;
		ZeroMemory(&dsv_desc, sizeof(dsv_desc));
		dsv_desc.Format = DXGI_FORMAT_D32_FLOAT;
		dsv_desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
		GetGraphicsEngine().GetD3DDevice()->CreateDepthStencilView(depthStencilTex.Get(), &dsv_desc, depthStencilView.ReleaseAndGetAddressOf());
		
		//Gバッファ出力ピクセルシェーダ
		SkinModelEffectShader imposterPS;
		imposterPS.Load("Preset/shader/Imposter.fx", "PSMain_RenderImposter", Shader::EnType::PS);

		//Gバッファをクリア
		float clearColor[enGBufferNum][4] = {
			{ 0.0f, 0.0f, 0.0f, 0.0f }, //enGBufferAlbedo
			{ 0.5f, 0.5f, 1.0f, 1.0f }, //enGBufferNormal
			{ 0.0f, 0.0f, 0.0f, 1.0f }, //enGBufferLightParam
			{ 1.0f, 1.0f, 1.0f, 1.0f }, //enGBufferTranslucent
		};
		for (int i = 0; i < enGBufferNum; i++) {
			DC->ClearRenderTargetView(m_GBufferView[i].Get(), clearColor[i]);
		}
		//デプスステンシルをクリア
		DC->ClearDepthStencilView(depthStencilView.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);

		// RenderTarget設定
		ID3D11RenderTargetView* renderTargetViews[enGBufferNum] = { nullptr };
		for (unsigned int i = 0; i < enGBufferNum; i++) {
			renderTargetViews[i] = m_GBufferView[i].Get();
		}
		DC->OMSetRenderTargets(enGBufferNum, renderTargetViews, depthStencilView.Get());

		//シェーダー
		std::list<SKEShaderPtr> beforeShaders;
		model.FindMaterialSetting(
			[&](MaterialSetting* mat) {
				//シェーダ保存
				beforeShaders.emplace_back(mat->GetPS());
				//シェーダ変更
				mat->SetPS(&imposterPS);
			}
		);

		//ビューポート
		D3D11_VIEWPORT viewport;
		viewport.Width = (float)m_gbufferSizeX / m_partNumX;
		viewport.Height = (float)m_gbufferSizeY / m_partNumY;
		viewport.TopLeftY = 0.0f;
		viewport.MinDepth = 0.0f;
		viewport.MaxDepth = 1.0f;

		//モデル描画
		int indY = 0;
		CQuaternion rotY, rotX, rotM;
		for (int i = 0; i < (int)(m_partNumX*m_partNumY); i++) {
			//横端まで行った
			if (i%m_partNumX == 0) {
				//ビューポート縦にずらす
				viewport.TopLeftX = 0;
				if (i != 0) { viewport.TopLeftY += viewport.Height; }

				//横回転リセット
				rotY = CQuaternion::Identity();
				rotY.SetRotation(CVector3::AxisY(), CMath::PI*-1.0f);

				//縦回転進める
				float angle = CMath::PI / (m_partNumY - 1) * (i / m_partNumX);
				rotX.SetRotation(CVector3::AxisX(), CMath::PI*-0.5f + angle);

				//インデックス進める
				if (i != 0) { indY++; }
			}

			//モデルの回転	
			rotM.Concatenate(rotY, rotX);
			model.UpdateWorldMatrix(0.0f, rotM, 1.0f);
				
			//ビューポート設定
			DC->RSSetViewports(1, &viewport);
			//モデル描画
			model.Draw();

			//モデルのカメラ方向の大きさを記録
			float toCamDirMaxSize = 0.0f;
			CVector3 toCamDir;
			
			toCamDir = m_boundingBoxMinSize;
			rotM.Multiply(toCamDir);
			toCamDirMaxSize = CVector3::AxisZ().Dot(toCamDir);

			toCamDir = m_boundingBoxMaxSize;
			rotM.Multiply(toCamDir);
			toCamDirMaxSize = max(toCamDirMaxSize, CVector3::AxisZ().Dot(toCamDir));

			m_toCamDirSize[indY*m_partNumX + i%m_partNumX] = toCamDirMaxSize;
			
			//ビューポート横にずらす
			viewport.TopLeftX += viewport.Width;

			//横回転進める
			rotY.Concatenate(CQuaternion(CVector3::AxisY(), CMath::PI2 / (m_partNumX - 1)));
		}

		//シェーダー戻す
		model.FindMaterialSetting(
			[&](MaterialSetting* mat) {
				//シェーダ変更
				mat->SetPS(beforeShaders.front());
				//先頭削除
				beforeShaders.pop_front();
			}
		);

		//ラスタライザーステート戻す
		GetGraphicsEngine().ResetRasterizerState();

		//レンダーターゲット解除
		GetGraphicsEngine().GetD3DDeviceContext()->OMSetRenderTargets(0, NULL, NULL);

		//ビューポート戻す
		DC->RSSetViewports(1, &beforeViewport);

		//カメラ戻す
		SetMainCamera(beforeCam);

		//GPUイベントの終了
		GetGraphicsEngine().EndGPUEvent();
	}

//インポスターテクスチャバンク
	ImposterTexRender* ImposterTexBank::Load(const wchar_t* filepath, const CVector2& resolution, const CVector2& partNum) {
		//ファイルパスからハッシュ作成
		int index = Util::MakeHash(filepath);
		//設定を文字列化
		std::wstring string = std::to_wstring((int)resolution.x);
		string += L"X";
		string += std::to_wstring((int)resolution.y);
		string += L"::";
		string += std::to_wstring((int)partNum.x);
		string += L"X";
		string += std::to_wstring((int)partNum.y);
		//文字列からハッシュ作成→さっきのハッシュと合成
		index = (int)Util::HashCombine(index, Util::MakeHash(string.c_str()));

		//ハッシュをmapのkeyとして使用
		if (m_impTexMap.count(index) > 0) {
			//もうある
			return m_impTexMap[index];
		}
		else {
			//つくる
			ImposterTexRender* ptr = new ImposterTexRender;
			ptr->Init(filepath, resolution, partNum);
			m_impTexMap.emplace(index, ptr);
			return ptr;
		}
	}

	void ImposterTexBank::Release() {
		for (auto& T : m_impTexMap) {
			delete T.second;
		}
		m_impTexMap.clear();
	}
	
//インポスター
	void CImposter::Init(const wchar_t* filepath, const CVector2& resolution, const CVector2& partNum, int instancingNum) {
		//テクスチャ生成
		m_texture = ImposterTexBank::GetInstance().Load(filepath, resolution, partNum);

		//ビルボード
		m_billboard.Init(m_texture->GetSRV(ImposterTexRender::enGBufferAlbedo), instancingNum, filepath);

		//シェーダ読み込み
		if (m_billboard.GetIsInstancing()) {
			//インスタンシング用シェーダ
			D3D_SHADER_MACRO macrosVS[] = { "INSTANCING", "1", "ALL_VS", "1", NULL, NULL };
			m_vsShader.Load("Preset/shader/Imposter.fx", "VSMain_Imposter", Shader::EnType::VS, "INSTANCING", macrosVS);
			m_vsZShader.Load("Preset/shader/Imposter.fx", "VSMain_RenderZ_Imposter", Shader::EnType::VS, "INSTANCING", macrosVS);

			D3D_SHADER_MACRO macros[] = {
				"INSTANCING", "1",
				NULL, NULL
			};
			m_imposterPS.Load("Preset/shader/Imposter.fx", "PSMain_ImposterRenderGBuffer", Shader::EnType::PS, "INSTANCING", macros);
			
			D3D_SHADER_MACRO macrosZ[] = { "TEXTURE", "1","INSTANCING", "1", NULL, NULL };
			m_zShader.Load("Preset/shader/Imposter.fx", "PSMain_ImposterRenderZ", Shader::EnType::PS, "TEXTURE_INSTANCING", macrosZ);
		}
		else {
			D3D_SHADER_MACRO macrosVS[] = { "ALL_VS", "1", NULL, NULL };
			m_vsShader.Load("Preset/shader/Imposter.fx", "VSMain_Imposter", Shader::EnType::VS, "NORMAL", macrosVS);
			m_vsZShader.Load("Preset/shader/Imposter.fx", "VSMain_RenderZ_Imposter", Shader::EnType::VS, "NORMAL", macrosVS);

			m_imposterPS.Load("Preset/shader/Imposter.fx", "PSMain_ImposterRenderGBuffer", Shader::EnType::PS);
			
			D3D_SHADER_MACRO macrosZ[] = { "TEXTURE", "1", NULL, NULL };
			m_zShader.Load("Preset/shader/Imposter.fx", "PSMain_ImposterRenderZ", Shader::EnType::PS, "TEXTURE", macrosZ);
		}

		//いろいろ設定
		m_billboard.GetModel().GetSkinModel().FindMaterialSetting(
			[&](MaterialSetting* mat) {
				//テクスチャ
				mat->SetNormalTexture(m_texture->GetSRV(ImposterTexRender::enGBufferNormal));
				mat->SetLightingTexture(m_texture->GetSRV(ImposterTexRender::enGBufferLightParam));
				mat->SetTranslucentTexture(m_texture->GetSRV(ImposterTexRender::enGBufferTranslucent));
				//シェーダ
				mat->SetVS(&m_vsShader);
				mat->SetVSZ(&m_vsZShader);
				mat->SetPS(&m_imposterPS);
				mat->SetPSZ(&m_zShader);
			}
		);

		//分割数設定
		m_billboard.GetModel().GetSkinModel().SetImposterPartNum(m_texture->GetPartNumX(), m_texture->GetPartNumY());
		
		//描画前処理の設定
		if (m_billboard.GetIsInstancing()) {
			m_billboard.GetInstancingModel().GetInstancingModel()->SetPreDrawFunction([this]() { m_texture->VSSetSizeToCameraSRV(); });
		}
		else {
			m_billboard.GetModel().GetSkinModel().SetPreDrawFunction([this](SkinModel*) { m_texture->VSSetSizeToCameraSRV(); });
		}

		//インスタンシング
		if (m_billboard.GetIsInstancing()) {
			//IInstanceDataを設定
			if (!m_billboard.GetInstancingModel().GetInstancingModel()->GetIInstanceData(L"InstancingImposterParamManager")) {
				//新規作成
				m_billboard.GetInstancingModel().GetInstancingModel()->AddIInstanceData(L"InstancingImposterParamManager", std::make_unique<InstancingImposterParamManager>(m_billboard.GetInstancingModel().GetInstancingModel()->GetInstanceMax(), m_texture));
			}
			//Y軸回転値のポインタを設定
			if (m_billboard.GetIsInstancing()) {
				m_billboard.GetInstancingModel().SetParamPtr(&m_rotYrad);
			}
		}

		//スケール初期化
		SetScale(1.0f);

		m_isInit = true;
	}

	/*
	void CImposter::CalcWorldMatrixAndIndex(bool isShadowDrawMode, const SkinModel& model, const ImposterTexRender& texture, const CVector3& pos, float scale, CVector3& position_return, CQuaternion& rotation_return, float& scale_return, int& index_x, int& index_y) {
		
		if (!GetMainCamera()) {
#ifndef DW_MASTER
			OutputDebugStringA("CImposter::CalcWorldMatrixAndIndex() カメラが設定されていません。\n");
#endif
			return;
		}

		//インポスター用インデックス計算
		CVector3 polyDir = GetMainCamera()->GetPos() - pos; polyDir.Normalize();
		
		//X軸回転
		CVector3 axisDir = polyDir; axisDir.x = CVector2(polyDir.x, polyDir.z).Length();
		float XRot = std::atan2(axisDir.y, axisDir.x);
		index_y = (int)std::round(XRot / CMath::PI * texture.GetPartNumY()) - (int)(texture.GetPartNumY() / 2.0f - 0.5f);

		//Y軸回転		
		float YRot = std::atan2(polyDir.x, polyDir.z);
		index_x = (int)std::round(-YRot / CMath::PI2 * texture.GetPartNumX()) + (int)(texture.GetPartNumX() / 2.0f - 0.5f);

		//カメラ方向にモデルサイズ分座標ずらす
		//※埋まり防止
		if (!isShadowDrawMode) {
			polyDir *= scale * texture.GetDirectionOfCameraSize(index_x, index_y);
		}

		//回転
		CQuaternion rot;
		rot.SetRotation(CVector3::AxisY(), index_x * -(CMath::PI2 / (texture.GetPartNumX() - 1)) + CMath::PI2);
		rot.Multiply(CQuaternion(CVector3::AxisX(), -index_y * -(CMath::PI / (texture.GetPartNumY() - 1)) + CMath::PI*0.5f));

		//返す
		position_return = pos;// +polyDir;
		rotation_return = CQuaternion::Identity();// rot;
		scale_return = scale * texture.GetModelSize()*2.0f;
	}
	*/
}