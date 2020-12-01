#include "DWstdafx.h"
#include "CImposter.h"
#include "Graphic/Model/SkinModelShaderConst.h"

namespace DemolisherWeapon {
	ImposterTexBank* ImposterTexBank::instance = nullptr;

//インポスターのインスタンシング描画における拡大率を扱うクラス
	void InstancingImposterParamManager::Reset(int instancingMaxNum) {
		m_instanceMax = instancingMaxNum;

		//ストラクチャーバッファ初期化
		m_paramsSB.Init(instancingMaxNum);

		//インデックス配列の確保
		m_paramsCache = std::make_unique<CVector2[]>(instancingMaxNum);
	}
	InstancingImposterParamManager::InstancingImposterParamManager(int instancingMaxNum, ImposterTexRender* tex) : m_texture(tex) {
		Reset(instancingMaxNum);
	}
	void InstancingImposterParamManager::PreDraw(int instanceNum, int drawInstanceNum, const std::unique_ptr<bool[]>& drawInstanceMask) {
		//カリングされてないもののみコピー
		int drawNum = 0;
		for (int i = 0; i < instanceNum; i++) {
			if (drawInstanceMask[i]) {
				m_paramsSB.GetData()[drawNum] = m_paramsCache[i];
				drawNum++;
			}
		}
		//StructuredBufferを更新
		m_paramsSB.UpdateSubresource();
		//シェーダーリソースにセット
		GetGraphicsEngine().GetD3DDeviceContext()->VSSetShaderResources(
			enSkinModelSRVReg_InstancingImposterScale, 1, m_paramsSB.GetAddressOfSRV()
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
	void ImposterTexRender::Init(SkinModel& model, const CVector2& resolution, const CVector2& partNum, const CQuaternion& rotOffset, bool isJustFit) {
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
		//TexData初期化
		m_albedoTextureData.isDDS = true;//?
		m_albedoTextureData.width = texDesc.Width;
		m_albedoTextureData.height = texDesc.Height;
		m_GBufferTex[enGBufferAlbedo]->AddRef();
		m_albedoTextureData.texture.Attach(m_GBufferTex[enGBufferAlbedo].Get());
		m_GBufferSRV[enGBufferAlbedo]->AddRef();
		m_albedoTextureData.textureView.Attach(m_GBufferSRV[enGBufferAlbedo].Get());

		//{
		//	//リソース作成
		//	auto texDesc = CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_R8G8B8A8_UNORM, (UINT)resolution.x, (UINT)resolution.y, 1, 1, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET);
		//	auto defaultHeapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
		//	DirectX::ThrowIfFailed(
		//		ge.GetD3D12Device()->CreateCommittedResource(
		//			&defaultHeapProperties,
		//			D3D12_HEAP_FLAG_NONE,
		//			&texDesc,
		//			D3D12_RESOURCE_STATE_RENDER_TARGET,//D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE
		//			nullptr,
		//			IID_PPV_ARGS(&m_raytracingOutput)
		//		)
		//	);
		//	m_raytracingOutput->SetName(L"ImposterGBufferAlbedo");
		//}

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
		
		//ワールド行列の初期化
		CMatrix beforeWorldMatrix = model.GetWorldMatrix();//元を記録しておく
		model.UpdateWorldMatrix(0.0f, CQuaternion::Identity(), 1.0f);//ワールド行列の初期化

		//バイアス行列取得
		CMatrix mBias, mBiasScr;
		CoordinateSystemBias::GetBias(mBias, mBiasScr, model.GetFBXUpAxis(), model.GetFBXCoordinateSystem());
		mBias.Mul(mBiasScr, mBias);

		//バウンディングボックスからモデルのサイズを求める
		m_imposterMaxSize = 0.0f;
		m_imposterSizeZ = 0.0f;
		m_boundingBoxMaxSize = { 0.0f };
		m_boundingBoxMinSize = { 0.0f };		
		CVector3 CenterV, ExpendV;//isJustFit用
		{
			model.GetBoundingBox(m_boundingBoxMinSize, m_boundingBoxMaxSize);
			mBias.Mul3x3(m_boundingBoxMinSize);//バイアスの適応
			mBias.Mul3x3(m_boundingBoxMaxSize);//バイアスの適応

			//一辺の最大長求める
			CQuaternion rotY, rotX, rotM;
			for (int i = 0; i < (int)(m_partNumX * m_partNumY); i++) {
				//横端まで行った
				if (i % m_partNumX == 0) {
					//横回転リセット
					rotY = CQuaternion::Identity();
					rotY.SetRotation(CVector3::AxisY(), CMath::PI * -1.0f);

					//縦回転進める
					if (i != 0 && m_partNumY > 1) {
						float angle = CMath::PI / (m_partNumY - 1) * (i / m_partNumX);
						rotX.SetRotation(CVector3::AxisX(), CMath::PI * -0.5f + angle);
					}
					else {
						rotX.SetRotation(CVector3::AxisX(), CMath::PI * -0.5f);
					}
				}

				//回転行列作成
				rotM.Concatenate(rotY, rotX);
				rotM.Concatenate(rotM, rotOffset);//回転オフセット適用
				model.UpdateWorldMatrix(0.0f, rotM, 1.0f);

				//辺の長さ求める
				CVector3 minV, maxV;
				model.GetUpdatedBoundingBox(minV, maxV);
				m_imposterMaxSize = max(m_imposterMaxSize, abs(minV.x));
				m_imposterMaxSize = max(m_imposterMaxSize, abs(minV.y));
				m_imposterMaxSize = max(m_imposterMaxSize, abs(maxV.x));
				m_imposterMaxSize = max(m_imposterMaxSize, abs(maxV.y));
				m_imposterSizeZ = max(m_imposterSizeZ, abs(minV.z - maxV.z));

				//isJustFit用
				CenterV = (minV + maxV) * 0.5f;
				ExpendV = maxV - CenterV;

				//横回転進める
				rotY.Concatenate(CQuaternion(CVector3::AxisY(), CMath::PI2 / (m_partNumX - 1)));
			}
		}
	
		//モデルのカメラ方向の大きさ
		m_toCamDirSize.Init(m_partNumX*m_partNumY);

		//インポスタテクスチャの作成
		Render(model, rotOffset, isJustFit, CenterV, ExpendV);

		//モデルのワールド行列を戻す
		model.SetWorldMatrix(beforeWorldMatrix, true);

		//StructuredBufferを更新
		m_toCamDirSize.UpdateSubresource();
	}

	void ImposterTexRender::VSSetSizeToCameraSRV() {
		//シェーダーリソースにセット
		GetGraphicsEngine().GetD3DDeviceContext()->VSSetShaderResources(
			enSkinModelSRVReg_ImposterSizeToCamera, 1, m_toCamDirSize.GetAddressOfSRV()
		);
	}

	void ImposterTexRender::Render(SkinModel& model, const CQuaternion& rotOffset, bool isJustFit, const CVector3& justFitCenter, const CVector3& justFitExpand) {
		//GPUイベントの開始
		GetGraphicsEngine().BeginGPUEvent(L"RenderImposter");

		ID3D11DeviceContext* DC = GetGraphicsEngine().GetD3DDeviceContext();

		//カメラのセットアップ		
		GameObj::NoRegisterOrthoCamera imposterCam;
		imposterCam.SetUp(CVector3::Up());
		if (isJustFit) {
			float size = max(justFitExpand.x, justFitExpand.y)*2.0f;
			imposterCam.SetWidth(size);
			imposterCam.SetHeight(size);
			imposterCam.SetPos(justFitCenter + CVector3::AxisZ()*(justFitExpand.z +100.0f));
			imposterCam.SetTarget(justFitCenter);
			imposterCam.SetFar(justFitExpand.z*2.0f + 100.0f);
		}
		else {
			imposterCam.SetWidth(m_imposterMaxSize * 2.0f);
			imposterCam.SetHeight(m_imposterMaxSize * 2.0f);
			imposterCam.SetPos({ 0.0f,0.0f,m_imposterSizeZ*0.5f + 100.0f });
			imposterCam.SetTarget(CVector3::Zero());
			imposterCam.SetFar(m_imposterSizeZ + 100.0f);
		}
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
				if (i != 0 && m_partNumY > 1) {
					float angle = CMath::PI / (m_partNumY - 1) * (i / m_partNumX);
					rotX.SetRotation(CVector3::AxisX(), CMath::PI * -0.5f + angle);
				}
				else {
					rotX.SetRotation(CVector3::AxisX(), CMath::PI * -0.5f);
				}

				//インデックス進める
				if (i != 0) { indY++; }
			}

			//モデルの回転	
			rotM.Concatenate(rotY, rotX);
			rotM.Concatenate(rotM, rotOffset);//回転オフセット適用
			model.UpdateWorldMatrix(0.0f, rotM, 1.0f);
				
			//ビューポート設定
			DC->RSSetViewports(1, &viewport);
			//モデル描画
			model.Draw();

			//バウンディングボックスの表示
			/*{
				CVector3 min, max;
				model.GetUpdatedBoundingBox(min, max);
				ImmediateDrawLine({ min.x,min.y,min.z }, { min.x,max.y,min.z }, { 1.0f,0.0f,1.0f,1.0f });
				ImmediateDrawLine({ max.x,min.y,min.z }, { max.x,max.y,min.z }, { 1.0f,0.0f,1.0f,1.0f });
				ImmediateDrawLine({ min.x,min.y,max.z }, { min.x,max.y,max.z }, { 1.0f,0.0f,1.0f,1.0f });
				ImmediateDrawLine({ max.x,min.y,max.z }, { max.x,max.y,max.z }, { 1.0f,0.0f,1.0f,1.0f });

				ImmediateDrawLine({ min.x,min.y,min.z }, { min.x,min.y,max.z }, { 1.0f,0.0f,1.0f,1.0f });
				ImmediateDrawLine({ min.x,min.y,min.z }, { max.x,min.y,min.z }, { 1.0f,0.0f,1.0f,1.0f });
				ImmediateDrawLine({ max.x,min.y,min.z }, { max.x,min.y,max.z }, { 1.0f,0.0f,1.0f,1.0f });
				ImmediateDrawLine({ min.x,min.y,max.z }, { max.x,min.y,max.z }, { 1.0f,0.0f,1.0f,1.0f });

				ImmediateDrawLine({ min.x,max.y,min.z }, { min.x,max.y,max.z }, { 1.0f,0.0f,1.0f,1.0f });
				ImmediateDrawLine({ min.x,max.y,min.z }, { max.x,max.y,min.z }, { 1.0f,0.0f,1.0f,1.0f });
				ImmediateDrawLine({ max.x,max.y,min.z }, { max.x,max.y,max.z }, { 1.0f,0.0f,1.0f,1.0f });
				ImmediateDrawLine({ min.x,max.y,max.z }, { max.x,max.y,max.z }, { 1.0f,0.0f,1.0f,1.0f });
			}
			{
				CVector3 min = justFitCenter - justFitExpand, max = justFitCenter + justFitExpand; 
				ImmediateDrawLine({ min.x,min.y,min.z }, { min.x,max.y,min.z }, { 1.0f,1.0f,0.0f,1.0f });
				ImmediateDrawLine({ max.x,min.y,min.z }, { max.x,max.y,min.z }, { 1.0f,1.0f,0.0f,1.0f });
				ImmediateDrawLine({ min.x,min.y,max.z }, { min.x,max.y,max.z }, { 1.0f,1.0f,0.0f,1.0f });
				ImmediateDrawLine({ max.x,min.y,max.z }, { max.x,max.y,max.z }, { 1.0f,1.0f,0.0f,1.0f });

				ImmediateDrawLine({ min.x,min.y,min.z }, { min.x,min.y,max.z }, { 1.0f,1.0f,0.0f,1.0f });
				ImmediateDrawLine({ min.x,min.y,min.z }, { max.x,min.y,min.z }, { 1.0f,1.0f,0.0f,1.0f });
				ImmediateDrawLine({ max.x,min.y,min.z }, { max.x,min.y,max.z }, { 1.0f,1.0f,0.0f,1.0f });
				ImmediateDrawLine({ min.x,min.y,max.z }, { max.x,min.y,max.z }, { 1.0f,1.0f,0.0f,1.0f });

				ImmediateDrawLine({ min.x,max.y,min.z }, { min.x,max.y,max.z }, { 1.0f,1.0f,0.0f,1.0f });
				ImmediateDrawLine({ min.x,max.y,min.z }, { max.x,max.y,min.z }, { 1.0f,1.0f,0.0f,1.0f });
				ImmediateDrawLine({ max.x,max.y,min.z }, { max.x,max.y,max.z }, { 1.0f,1.0f,0.0f,1.0f });
				ImmediateDrawLine({ min.x,max.y,max.z }, { max.x,max.y,max.z }, { 1.0f,1.0f,0.0f,1.0f });
			}*/
			//float size = max(justFitExpand.x, justFitExpand.y) * 2.0f;
			//ImmediateDrawQuad({ -size ,-size,size }, { size ,size,size }, { 1.0f,1.0f,0.0f,1.0f });

			//モデルのカメラ方向の大きさを記録
			float toCamDirMaxSize = 0.0f;
			CVector3 toCamDir;
			
			toCamDir = m_boundingBoxMinSize;
			rotM.Multiply(toCamDir);
			toCamDirMaxSize = toCamDir.z;

			toCamDir = m_boundingBoxMaxSize;
			rotM.Multiply(toCamDir);
			toCamDirMaxSize = max(toCamDirMaxSize, toCamDir.z);

			m_toCamDirSize.GetData()[indY*m_partNumX + i%m_partNumX] = toCamDirMaxSize;
			
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

	float ImposterTexRender::GetDirectionOfCameraSize(int x, int y)const {
		return m_toCamDirSize.GetData()[(m_partNumY - 1 + y)*m_partNumX + x];
	}

//インポスターテクスチャバンク
	ImposterTexRender* ImposterTexBank::Load(const wchar_t* identifier, SkinModel& model, const CVector2& resolution, const CVector2& partNum, const CQuaternion& rotOffset, bool isJustFit) {
		//ハッシュ作成
		int index = Util::MakeHash(identifier);

		//ハッシュをmapのkeyとして使用
		if (m_impTexMap.count(index) > 0) {
			//もうある
			return m_impTexMap[index];
		}
		else {
			//つくる
			ImposterTexRender* ptr = new ImposterTexRender;
			ptr->Init(model, resolution, partNum, rotOffset, isJustFit);
			m_impTexMap.emplace(index, ptr);
			return ptr;
		}
	}

	ImposterTexRender* ImposterTexBank::Get(const wchar_t* identifier) {
		//ハッシュ作成
		int index = Util::MakeHash(identifier);

		//ハッシュをmapのkeyとして使用
		if (m_impTexMap.count(index) > 0) {
			//ある
			return m_impTexMap[index];
		}
		else {
			//ない
			return nullptr;
		}
	}

	void ImposterTexBank::Release() {
		for (auto& T : m_impTexMap) {
			delete T.second;
		}
		m_impTexMap.clear();
	}
	
//インポスター
	bool CImposter::Init(const wchar_t* identifier, SkinModel& model, const CVector2& resolution, const CVector2& partNum, int instancingNum) {
		//テクスチャ読み込み
		m_texture = nullptr;
		m_texture = ImposterTexBank::GetInstance().Load(identifier, model, resolution, partNum);
		if (!m_texture) { return false; }
		//初期化
		InnerInit(identifier, instancingNum);
		return true;
	}
	bool CImposter::Init(const wchar_t* identifier, int instancingNum) {
		//テクスチャ読み込み
		m_texture = nullptr;
		m_texture = ImposterTexBank::GetInstance().Get(identifier);
		if (!m_texture) { return false; }
		//初期化
		InnerInit(identifier, instancingNum);
		return true;
	}
	void CImposter::InnerInit(const wchar_t* identifier, int instancingNum){
		std::wstring moji = L"CImposter-";
		moji += identifier; moji += L"-CImposter";

		//ビルボード
		m_billboard.Init(m_texture->GetSRV(ImposterTexRender::enGBufferAlbedo), instancingNum, moji.c_str());

		//シェーダ読み込み
		if (!m_s_isShaderLoaded) {
			{
				//インスタンシング用シェーダ
				D3D_SHADER_MACRO macrosVS[] = { "INSTANCING", "1", "ALL_VS", "1", NULL, NULL };
				m_s_vsShader[enInstancing].Load("Preset/shader/Imposter.fx", "VSMain_Imposter", Shader::EnType::VS, "INSTANCING", macrosVS);
				m_s_vsZShader[enInstancing].Load("Preset/shader/Imposter.fx", "VSMain_RenderZ_Imposter", Shader::EnType::VS, "INSTANCING", macrosVS);

				D3D_SHADER_MACRO macros[] = {
					"INSTANCING", "1",
					NULL, NULL
				};
				m_s_imposterPS[enInstancing].Load("Preset/shader/Imposter.fx", "PSMain_ImposterRenderGBuffer", Shader::EnType::PS, "INSTANCING", macros);

				D3D_SHADER_MACRO macrosZ[] = { "TEXTURE", "1","INSTANCING", "1", NULL, NULL };
				m_s_zShader[enInstancing].Load("Preset/shader/Imposter.fx", "PSMain_ImposterRenderZ", Shader::EnType::PS, "TEXTURE_INSTANCING", macrosZ);
			}
			{
				D3D_SHADER_MACRO macrosVS[] = { "ALL_VS", "1", NULL, NULL };
				m_s_vsShader[enNormal].Load("Preset/shader/Imposter.fx", "VSMain_Imposter", Shader::EnType::VS, "NORMAL", macrosVS);
				m_s_vsZShader[enNormal].Load("Preset/shader/Imposter.fx", "VSMain_RenderZ_Imposter", Shader::EnType::VS, "NORMAL", macrosVS);

				m_s_imposterPS[enNormal].Load("Preset/shader/Imposter.fx", "PSMain_ImposterRenderGBuffer", Shader::EnType::PS);

				D3D_SHADER_MACRO macrosZ[] = { "TEXTURE", "1", NULL, NULL };
				m_s_zShader[enNormal].Load("Preset/shader/Imposter.fx", "PSMain_ImposterRenderZ", Shader::EnType::PS, "TEXTURE", macrosZ);
			}
			m_s_isShaderLoaded = true;
		}

		//いろいろ設定
		m_billboard.GetModel().GetSkinModel().FindMaterialSetting(
			[&](MaterialSetting* mat) {
				//テクスチャ
				mat->SetNormalTexture(m_texture->GetSRV(ImposterTexRender::enGBufferNormal));
				mat->SetLightingTexture(m_texture->GetSRV(ImposterTexRender::enGBufferLightParam));
				mat->SetTranslucentTexture(m_texture->GetSRV(ImposterTexRender::enGBufferTranslucent));
				//シェーダ
				mat->SetVS(&m_s_vsShader[m_billboard.GetIsInstancing() ? enInstancing : enNormal]);
				mat->SetVSZ(&m_s_vsZShader[m_billboard.GetIsInstancing() ? enInstancing : enNormal]);
				mat->SetPS(&m_s_imposterPS[m_billboard.GetIsInstancing() ? enInstancing : enNormal]);
				mat->SetPSZ(&m_s_zShader[m_billboard.GetIsInstancing() ? enInstancing : enNormal]);
			}
		);

		//分割数設定
		m_billboard.GetModel().GetSkinModel().SetImposterPartNum(m_texture->GetPartNumX(), m_texture->GetPartNumY());
		
		//描画前処理の設定
		if (m_billboard.GetIsInstancing()) {
			m_billboard.GetInstancingModel().GetInstancingModel()->SetPreDrawFunction([this]() { m_texture->VSSetSizeToCameraSRV(); });
		}
		else {
			m_billboard.GetModel().GetSkinModel().SetPreDrawFunction(L"DW_SetImposterSizeToCamera",[this](SkinModel*) { m_texture->VSSetSizeToCameraSRV(); });
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