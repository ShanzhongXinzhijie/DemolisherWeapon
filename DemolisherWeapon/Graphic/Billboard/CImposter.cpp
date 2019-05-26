#include "DWstdafx.h"
#include "CImposter.h"

namespace DemolisherWeapon {
namespace GameObj {

	void CImposter::Init(const wchar_t* filepath, const CVector2& resolution, const CVector2& partNum) {
		SkinModel model;
		model.Init(filepath);
		model.UpdateWorldMatrix(0.0f,CQuaternion::Identity(),1.0f);
		Init(model, resolution, partNum);
	}

	void CImposter::Init(SkinModel& model, const CVector2& resolution, const CVector2& partNum) {

		m_gbufferSizeX = (UINT)resolution.x; m_gbufferSizeY = (UINT)resolution.y;
		m_partNumX = (UINT)partNum.x; m_partNumY = (UINT)partNum.y;

		//配列確保
		m_fronts.clear();
		m_ups.clear();
		m_fronts.resize(m_partNumY);
		m_ups.resize(m_partNumY);
		m_fronts.shrink_to_fit();
		m_ups.shrink_to_fit();

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
		texDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
		ge.GetD3DDevice()->CreateTexture2D(&texDesc, NULL, m_GBufferTex[enGBufferLightParam].ReleaseAndGetAddressOf());
		ge.GetD3DDevice()->CreateRenderTargetView(m_GBufferTex[enGBufferLightParam].Get(), nullptr, m_GBufferView[enGBufferLightParam].ReleaseAndGetAddressOf());//レンダーターゲット
		ge.GetD3DDevice()->CreateShaderResourceView(m_GBufferTex[enGBufferLightParam].Get(), nullptr, m_GBufferSRV[enGBufferLightParam].ReleaseAndGetAddressOf());//シェーダーリソースビュー

		//法線
		texDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
		ge.GetD3DDevice()->CreateTexture2D(&texDesc, NULL, m_GBufferTex[enGBufferNormal].ReleaseAndGetAddressOf());
		ge.GetD3DDevice()->CreateRenderTargetView(m_GBufferTex[enGBufferNormal].Get(), nullptr, m_GBufferView[enGBufferNormal].ReleaseAndGetAddressOf());//レンダーターゲット
		ge.GetD3DDevice()->CreateShaderResourceView(m_GBufferTex[enGBufferNormal].Get(), nullptr, m_GBufferSRV[enGBufferNormal].ReleaseAndGetAddressOf());//シェーダーリソースビュー

		//デプス
		texDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
		texDesc.Format = DXGI_FORMAT_R32_TYPELESS;
		ge.GetD3DDevice()->CreateTexture2D(&texDesc, NULL, m_depthStencilTex.ReleaseAndGetAddressOf());
		//デプスステンシル
		D3D11_DEPTH_STENCIL_VIEW_DESC dsv_desc;
		ZeroMemory(&dsv_desc, sizeof(dsv_desc));
		dsv_desc.Format = DXGI_FORMAT_D32_FLOAT;
		dsv_desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
		ge.GetD3DDevice()->CreateDepthStencilView(m_depthStencilTex.Get(), &dsv_desc, m_depthStencilView.ReleaseAndGetAddressOf());
	
		//Gバッファ出力ピクセルシェーダ
		m_imposterPS.Load("Preset/shader/Imposter.fx", "PSMain_RenderImposter", Shader::EnType::PS);

		//ビルボード
		m_billboard.Init(m_GBufferSRV[enGBufferAlbedo].Get());
		m_billboardPS.Load("Preset/shader/Imposter.fx", "PSMain_ImposterRenderGBuffer", Shader::EnType::PS);
		m_billboard.GetModel().GetSkinModel().FindMaterialSetting(
			[&](MaterialSetting* mat) {
				mat->SetNormalTexture(m_GBufferSRV[enGBufferNormal].Get());
				mat->SetLightingTexture(m_GBufferSRV[enGBufferLightParam].Get());
				mat->SetPS(&m_billboardPS);
			}
		);

		//バウンディングボックスからモデルのサイズを求める
		m_imposterMaxSize = 0.0f;
		model.FindMeshes(
			[&](const std::shared_ptr<DirectX::ModelMesh>& meshes) {
				CVector3 size, extents;
				extents = meshes->boundingBox.Extents;

				size = meshes->boundingBox.Center;
				size += extents;
				size.Abs();
				m_imposterMaxSize = max(m_imposterMaxSize, max(size.y,max(size.x, size.z)));

				size = meshes->boundingBox.Center;
				size -= extents;
				size.Abs();
				m_imposterMaxSize = max(m_imposterMaxSize, max(size.y,max(size.x, size.z)));
			}
		);

		//カメラのセットアップ		
		GameObj::NoRegisterOrthoCamera imposterCam;
		imposterCam.SetWidth(m_imposterMaxSize*2.0f);
		imposterCam.SetHeight(m_imposterMaxSize*2.0f);
		imposterCam.SetPos({ 0.0f,0.0f,CVector3(m_imposterMaxSize).Length() + 100.0f });
		imposterCam.SetTarget(CVector3::Zero());
		imposterCam.SetUp(CVector3::Up());
		//※このカメラは手動で更新する...
		imposterCam.UpdateMatrix();

		//カメラ保存
		GameObj::ICamera* beforeCam = GetMainCamera();
		//カメラ変更
		SetMainCamera(&imposterCam);

		//インポスタの作成
		RenderImposter(model);

		//カメラ戻す
		SetMainCamera(beforeCam);

		//スケール初期化
		SetScale(1.0f);
	}

	void CImposter::RenderImposter(SkinModel& model) {
		//GPUイベントの開始
		GetGraphicsEngine().BeginGPUEvent(L"RenderImposter");

		//Gバッファをクリア
		float clearColor[enGBufferNum][4] = {
			{ 0.5f, 0.5f, 0.5f, 0.0f }, //enGBufferAlbedo
			{ 0.0f, 1.0f, 0.0f, 1.0f }, //enGBufferNormal
			{ 0.0f, 0.0f, 0.0f, 1.0f }, //enGBufferLightParam
		};
		for (int i = 0; i < enGBufferNum; i++) {
			GetEngine().GetGraphicsEngine().GetD3DDeviceContext()->ClearRenderTargetView(m_GBufferView[i].Get(), clearColor[i]);
		}
		//デプスステンシルをクリア
		GetEngine().GetGraphicsEngine().GetD3DDeviceContext()->ClearDepthStencilView(m_depthStencilView.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);
		//D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL

		// RenderTarget設定
		ID3D11RenderTargetView* renderTargetViews[enGBufferNum] = { nullptr };
		for (unsigned int i = 0; i < enGBufferNum; i++) {
			renderTargetViews[i] = m_GBufferView[i].Get();
		}
		GetEngine().GetGraphicsEngine().GetD3DDeviceContext()->OMSetRenderTargets(enGBufferNum, renderTargetViews, m_depthStencilView.Get());

		//シェーダー
		std::list<SKEShaderPtr> beforeShaders;
		model.FindMaterialSetting(
			[&](MaterialSetting* mat) {
				//シェーダ保存
				beforeShaders.emplace_back(mat->GetPS());
				//シェーダ変更
				mat->SetPS(&m_imposterPS);
			}
		);
		
		//ビューポート
		D3D11_VIEWPORT viewport;
		viewport.MinDepth = 0.0f;
		viewport.MaxDepth = 1.0f;

		//ビューポート記録
		D3D11_VIEWPORT beforeViewport; UINT kaz = 1;
		GetEngine().GetGraphicsEngine().GetD3DDeviceContext()->RSGetViewports(&kaz, &beforeViewport);

		//モデル描画
		int indY = 0;
		CQuaternion rotY, rotX, rotM;
		viewport.Width = (float)m_gbufferSizeX / m_partNumX;
		viewport.Height = (float)m_gbufferSizeY / m_partNumY;
		viewport.TopLeftY = 0;
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
			GetEngine().GetGraphicsEngine().GetD3DDeviceContext()->RSSetViewports(1, &viewport);			

			//モデル描画
			model.Draw();

			//上方向と前方向保存
			CVector3 impFront = CVector3::AxisZ(), impUp = CVector3::AxisY();//インポスタの前方向と上方向
			rotM.Multiply(impFront);
			rotM.Multiply(impUp);
			//impFront.x *= -1.0f;
			//impUp.x *= -1.0f;
			m_fronts[indY].emplace_back(impFront);
			m_ups[indY].emplace_back(impUp);

			//ビューポート横にずらす
			viewport.TopLeftX += viewport.Width;				
			
			//横回転進める
			rotY.Concatenate(CQuaternion(CVector3::AxisY(), CMath::PI2 / (m_partNumX - 1)));
		}

		//ビューポート戻す
		GetEngine().GetGraphicsEngine().GetD3DDeviceContext()->RSSetViewports(1, &beforeViewport);

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
		GetEngine().GetGraphicsEngine().ResetRasterizerState();

		//レンダーターゲット解除
		GetEngine().GetGraphicsEngine().GetD3DDeviceContext()->OMSetRenderTargets(0, NULL, NULL);

		//GPUイベントの終了
		GetGraphicsEngine().EndGPUEvent();
	}

	void CImposter::Update() {
		//インポスター用インデックス計算
		int x = 0, y = 0;
		float z = 0.0f;

		CVector3 polyDir = { 0.0f,0.0f,-1.0f };
		GameObj::CBillboard::GetBillboardQuaternion().Multiply(polyDir);
		//polyDir = GetMainCamera()->GetPos() - m_billboard.GetPos();
		polyDir.Normalize();

		CVector3 axisDir;

		CVector3 impFront = CVector3::AxisZ(), impUp = CVector3::AxisY();//インポスタの前方向と上方向
		//↑の空間に投影したポリゴン方向(2D
		CVector3 toueiV;
		toueiV.x = impFront.Dot(polyDir);
		toueiV.y = impUp.Dot(polyDir);

		CVector3 toueiV3D;
		toueiV3D += impFront * toueiV.x;
		toueiV3D += impUp * toueiV.y;
		toueiV3D.Normalize();


		toueiV3D = polyDir;
		impFront = polyDir; impFront.y = 0; impFront.Normalize();

		toueiV.x = impFront.Length(); toueiV.y = polyDir.y; toueiV.Normalize();

		//X軸回転
		float XRot = acos(toueiV3D.Dot(impFront));
		if (CVector2(toueiV.x, toueiV.y).GetNorm().Cross(CVector2(1.0f,0.0f)) > 0.0f) {//CVector2(1.0f,0.0f)はimpFront
			y = -XRot / CMath::PI * m_partNumY - (int)(m_partNumY / 2.0f + 0.5f);
		}
		else {
			y = XRot / CMath::PI * m_partNumY - (int)(m_partNumY / 2.0f + 0.5f);
		}

		//Y軸回転		
		axisDir = CVector3(0.0f, 0.0f, 1.0f);
		polyDir.y = 0.0f; polyDir.Normalize();
		float YRot = acos(polyDir.Dot(axisDir));
		if (CVector2(polyDir.x, polyDir.z).Cross(CVector2(axisDir.x, axisDir.z)) > 0.0f) {
			x += -YRot / CMath::PI2 * m_partNumX + (int)(m_partNumX / 2.0f + 0.5f);
		}
		else {
			x += YRot / CMath::PI2 * m_partNumX + (int)(m_partNumX / 2.0f + 0.5f);
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
		m_billboard.GetModel().GetSkinModel().SetImposterIndex(x,y);
		CQuaternion zRot; zRot.SetRotation(CVector3::AxisZ(), z);
		SetRot(zRot);
	}
}
}