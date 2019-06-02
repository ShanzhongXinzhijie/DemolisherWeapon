#include "DWstdafx.h"
#include "CImposter.h"
#include "Graphic\Model\SkinModelShaderConst.h"

namespace DemolisherWeapon {
	ImposterTexBank* ImposterTexBank::instance = nullptr;

	InstancingImposterIndex::InstancingImposterIndex(int instancingMaxNum) {
		m_instanceMax = instancingMaxNum;
		m_instanceNum = 0;

		//インデックス配列の確保
		m_instancingIndex = std::make_unique<int[][2]>(instancingMaxNum);

		//StructuredBufferの確保
		D3D11_BUFFER_DESC desc;
		ZeroMemory(&desc, sizeof(desc));
		int stride = sizeof(int[2]);
		desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		desc.ByteWidth = static_cast<UINT>(stride * instancingMaxNum);
		desc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
		desc.StructureByteStride = stride;
		GetGraphicsEngine().GetD3DDevice()->CreateBuffer(&desc, NULL, m_indexSB.ReleaseAndGetAddressOf());

		//ShaderResourceViewの確保
		D3D11_SHADER_RESOURCE_VIEW_DESC descSRV;
		ZeroMemory(&descSRV, sizeof(descSRV));
		descSRV.ViewDimension = D3D11_SRV_DIMENSION_BUFFEREX;
		descSRV.BufferEx.FirstElement = 0;
		descSRV.Format = DXGI_FORMAT_UNKNOWN;
		descSRV.BufferEx.NumElements = desc.ByteWidth / desc.StructureByteStride;
		GetGraphicsEngine().GetD3DDevice()->CreateShaderResourceView(m_indexSB.Get(), &descSRV, m_indexSRV.ReleaseAndGetAddressOf());
	}

	void InstancingImposterIndex::PreDrawUpdate() {
		//シェーダーリソースにセット
		GetGraphicsEngine().GetD3DDeviceContext()->PSSetShaderResources(
			enSkinModelSRVReg_InstancingImposterTextureIndex, 1, m_indexSRV.GetAddressOf()
		);
	}
	void InstancingImposterIndex::PostLoopPostUpdate() {
		//StructuredBufferを更新
		GetGraphicsEngine().GetD3DDeviceContext()->UpdateSubresource(
			m_indexSB.Get(), 0, NULL, m_instancingIndex.get(), 0, 0
		);
		m_instanceNum = 0;
	}

	void InstancingImposterIndex::AddDrawInstance(int x, int y) {
		if (m_instanceNum + 1 >= m_instanceMax) {
#ifndef DW_MASTER
			char message[256];
			sprintf_s(message, "【InstancingImposterIndex】インスタンスの最大数に達しています！\nインスタンス最大数:%d\n", m_instanceMax);
			OutputDebugStringA(message);
#endif				
			return;
		}
		m_instancingIndex[m_instanceNum][0] = x;
		m_instancingIndex[m_instanceNum][1] = y;
		m_instanceNum++;
	}
	
	void ImposterTexRender::Init(const wchar_t* filepath, const CVector2& resolution, const CVector2& partNum) {
		//解像度・分割数設定
		m_gbufferSizeX = (UINT)resolution.x; m_gbufferSizeY = (UINT)resolution.y;
		m_partNumX = (UINT)partNum.x; m_partNumY = (UINT)partNum.y;

		//配列確保
		/*m_fronts.clear();
		m_ups.clear();
		m_fronts.resize(m_partNumY);
		m_ups.resize(m_partNumY);
		m_fronts.shrink_to_fit();
		m_ups.shrink_to_fit();*/

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

		//モデル読み込み
		SkinModel model;
		model.Init(filepath);
		model.UpdateWorldMatrix(0.0f, CQuaternion::Identity(), 1.0f);
		
		//バウンディングボックスからモデルのサイズを求める
		m_imposterMaxSize = 0.0f;
		model.FindMeshes(
			[&](const std::shared_ptr<DirectX::ModelMesh>& meshes) {
				CVector3 size, extents;
				extents = meshes->boundingBox.Extents;

				size = meshes->boundingBox.Center;
				size += extents;
				size.Abs();
				m_imposterMaxSize = max(m_imposterMaxSize, max(size.y, max(size.x, size.z)));

				size = meshes->boundingBox.Center;
				size -= extents;
				size.Abs();
				m_imposterMaxSize = max(m_imposterMaxSize, max(size.y, max(size.x, size.z)));
			}
		);

		//インポスタテクスチャの作成
		Render(model);
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

	ImposterTexRender* ImposterTexBank::Load(const wchar_t* filepath, const CVector2& resolution, const CVector2& partNum) {
		int index = Util::MakeHash(filepath);
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

namespace GameObj {

	void CImposter::Init(const wchar_t* filepath, const CVector2& resolution, const CVector2& partNum, int instancingNum) {
		//テクスチャ生成
		m_texture = ImposterTexBank::GetInstance().Load(filepath, resolution, partNum);

		//ビルボード
		m_billboard.Init(m_texture->GetSRV(ImposterTexRender::enGBufferAlbedo), instancingNum, filepath);
		if (m_billboard.GetIsInstancing()) {
			//インスタンシング用シェーダ
			D3D_SHADER_MACRO macros[] = {
				"INSTANCING", "1",
				NULL, NULL
			};
			m_billboardPS.Load("Preset/shader/Imposter.fx", "PSMain_ImposterRenderGBuffer", Shader::EnType::PS, "INSTANCING", macros);
		}
		else {
			m_billboardPS.Load("Preset/shader/Imposter.fx", "PSMain_ImposterRenderGBuffer", Shader::EnType::PS);
		}
		m_billboard.GetModel().GetSkinModel().FindMaterialSetting(
			[&](MaterialSetting* mat) {
				mat->SetNormalTexture(m_texture->GetSRV(ImposterTexRender::enGBufferNormal));
				mat->SetLightingTexture(m_texture->GetSRV(ImposterTexRender::enGBufferLightParam));
				mat->SetPS(&m_billboardPS);
			}
		);
		//ビルボード設定解除
		//(こちら側で回転させる)
		m_billboard.GetModel().GetSkinModel().SetIsBillboard(false);
		//インポスターとして設定
		//m_billboard.GetModel().GetSkinModel().SetIsImposter(true);
		//分割数設定
		m_billboard.GetModel().GetSkinModel().SetImposterPartNum(m_texture->GetPartNumX(), m_texture->GetPartNumY());
		//インスタンシング用のクラス設定
		//TODO インスタンシング順とインデックス順、一致するのか? インスタンシングモデルに紐付ける?
		if (m_billboard.GetIsInstancing()) {
			if (!m_billboard.GetInstancingModel().GetInstancingModel()->GetIInstanceData()) {
				//新規作成
				m_billboard.GetInstancingModel().GetInstancingModel()->SetIInstanceData(std::make_unique<InstancingImposterIndex>(m_billboard.GetInstancingModel().GetInstancingModel()->GetInstanceMax()));
			}
			//既存のもの使う
			m_instancingIndex = dynamic_cast<InstancingImposterIndex*>(m_billboard.GetInstancingModel().GetInstancingModel()->GetIInstanceData());
		}
		else {
			m_instancingIndex = nullptr;
		}
		//ラスタライザーステート
		/*
		D3D11_RASTERIZER_DESC desc = {};
		desc.CullMode = D3D11_CULL_FRONT;
		desc.FillMode = D3D11_FILL_SOLID;
		desc.DepthClipEnable = true;
		desc.MultisampleEnable = true;
		desc.DepthBias = -(INT)DEPTH_BIAS_D32_FLOAT(m_texture->GetModelSize() / (GetMainCamera()->GetFar()-GetMainCamera()->GetNear()));
		GetGraphicsEngine().GetD3DDevice()->CreateRasterizerState(&desc, m_depthRSCw.ReleaseAndGetAddressOf());
		desc.CullMode = D3D11_CULL_BACK;
		desc.FillMode = D3D11_FILL_SOLID;
		GetGraphicsEngine().GetD3DDevice()->CreateRasterizerState(&desc, m_depthRSCCw.ReleaseAndGetAddressOf());
		m_billboard.GetModel().GetSkinModel().SetRasterizerState(m_depthRSCw.Get(), m_depthRSCCw.Get());
		*/

		//スケール初期化
		SetScale(1.0f);

		m_isInit = true;
	}

	void CImposter::PostLoopUpdate() {
		if (!m_isInit) { return; }
		if (!m_billboard.GetModel().GetIsDraw()) { return; }//描画しないなら実行しない
		if (!GetMainCamera()) {
#ifndef DW_MASTER
			OutputDebugStringA("CImposter::PostLoopUpdate() カメラが設定されていません。\n");
#endif
			return;
		}

		//インポスター用インデックス計算
		int x = 0, y = 0;
		
		CVector3 polyDir;
		polyDir += { 0.0f,0.0f,-1.0f };
		GetMainCamera()->GetImposterQuaternion(m_pos).Multiply(polyDir);
		polyDir.Normalize();

		CVector3 axisDir;
		
		//X軸回転
		axisDir = polyDir;
		axisDir.y = 0; axisDir.Normalize();
		float XRot = acos(polyDir.Dot(axisDir));
		if (CVector2(CVector2(polyDir.x, polyDir.z).Length(), polyDir.y).GetNorm().Cross(CVector2(1.0f,0.0f)) > 0.0f) {//CVector2(1.0f,0.0f)はaxisDir
			y = (int)std::round(-XRot / CMath::PI * m_texture->GetPartNumY()) - (int)(m_texture->GetPartNumY() / 2.0f - 0.5f);
		}
		else {
			y = (int)std::round(XRot / CMath::PI * m_texture->GetPartNumY()) - (int)(m_texture->GetPartNumY() / 2.0f - 0.5f);
		}
		
		//Y軸回転		
		axisDir = CVector3(0.0f, 0.0f, 1.0f);
		polyDir.y = 0.0f; polyDir.Normalize();
		float YRot = acos(polyDir.Dot(axisDir));
		if (CVector2(polyDir.x, polyDir.z).Cross(CVector2(axisDir.x, axisDir.z)) > 0.0f) {
			x += (int)std::round(-YRot / CMath::PI2 * m_texture->GetPartNumX()) + (int)(m_texture->GetPartNumX() / 2.0f - 0.5f);
		}
		else {
			x += (int)std::round(YRot / CMath::PI2 * m_texture->GetPartNumX()) + (int)(m_texture->GetPartNumX() / 2.0f - 0.5f);
		}

		/*float distance = -1.0f;
		for (int indy = 0; indy < m_partNumY; indy++) {
			for (int indx = 0; indx < m_partNumX; indx++) {
				if (distance < 0.0f || distance >(polyDir - m_fronts[indy][indx]).Length() + ( - m_ups[indy][indx]).Length()) {
					distance = (polyDir - m_fronts[indy][indx]).Length() + ( - m_ups[indy][indx]).Length();
					x = indx, y = indy;
				}
			}
		}*/

		//モデルに設定
		if (m_billboard.GetIsInstancing()) {
			m_instancingIndex->AddDrawInstance(x, y);
		}
		else {
			m_billboard.GetModel().GetSkinModel().SetImposterIndex(x, y);
		}

		//カメラ方向にモデルサイズ分座標ずらす
		//※埋まり防止
		CVector3 bias = GetMainCamera()->GetPos() - m_pos;
		bias.Normalize();
		bias *= m_scale*m_texture->GetModelSize();
		m_billboard.SetPos(m_pos + bias);		
		//m_billboard.SetPos(m_pos);

		//回転
		CQuaternion rot;
		rot.SetRotation(CVector3::AxisY(), x * -(CMath::PI2 / (m_texture->GetPartNumX()-1)) + CMath::PI + CMath::PI);
		//rot.Multiply(CQuaternion(CVector3::AxisX(), CMath::PI*0.5f));
		CVector3 AxisX = CVector3::AxisX();
		rot.Multiply(AxisX);
		rot.Concatenate(CQuaternion(AxisX, -y * -(CMath::PI / (m_texture->GetPartNumY()-1)) + CMath::PI*0.5f));
		m_billboard.SetRot(rot);

		//行列の更新
		m_billboard.GetModel().GetSkinModel().UpdateWorldMatrix(m_billboard.GetModel().GetPos(), m_billboard.GetModel().GetRot(), m_billboard.GetModel().GetScale());
	}
}
}