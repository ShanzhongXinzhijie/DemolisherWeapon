#include "DWstdafx.h"
#include "PrimitiveRender.h"

namespace DemolisherWeapon {
	
	void PrimitiveRender::Init() {
#ifndef DW_DX12_TEMPORARY
		ID3D11Device* d3dDevice = GetEngine().GetGraphicsEngine().GetD3DDevice();		

		m_effect = std::make_unique<DirectX::BasicEffect>(d3dDevice);
		m_effect->SetVertexColorEnabled(true);

		void const* shaderByteCode;
		size_t byteCodeLength;

		m_effect->GetVertexShaderBytecode(&shaderByteCode, &byteCodeLength);

		DirectX::ThrowIfFailed(
			d3dDevice->CreateInputLayout(	DirectX::VertexPositionColor::InputElements,
											DirectX::VertexPositionColor::InputElementCount,
											shaderByteCode, byteCodeLength,
											m_inputLayout.ReleaseAndGetAddressOf()
			)
		);

		m_batch = std::make_unique<DirectX::PrimitiveBatch<DirectX::VertexPositionColor>>(GetEngine().GetGraphicsEngine().GetD3DDeviceContext());
#endif
		//2Dカメラ初期化
		m_2dCamera.SetPos({ GetGraphicsEngine().GetFrameBuffer_W() / 2.0f,-GetGraphicsEngine().GetFrameBuffer_H() / 2.0f,-(m_2dCamera.GetNear() + m_2dCamera.GetFar()) / 2.0f });
		m_2dCamera.SetTarget({ GetGraphicsEngine().GetFrameBuffer_W() / 2.0f,-GetGraphicsEngine().GetFrameBuffer_H() / 2.0f,0.0f });
		m_2dCamera.SetWidth(GetGraphicsEngine().GetFrameBuffer_W());
		m_2dCamera.SetHeight(GetGraphicsEngine().GetFrameBuffer_H());
		m_2dCamera.UpdateMatrix();
	}

	void PrimitiveRender::Resize() {
		//2Dカメラ再初期化
		m_2dCamera.SetPos({ GetGraphicsEngine().GetFrameBuffer_W() / 2.0f,-GetGraphicsEngine().GetFrameBuffer_H() / 2.0f,-(m_2dCamera.GetNear() + m_2dCamera.GetFar()) / 2.0f });
		m_2dCamera.SetTarget({ GetGraphicsEngine().GetFrameBuffer_W() / 2.0f,-GetGraphicsEngine().GetFrameBuffer_H() / 2.0f,0.0f });
		m_2dCamera.SetWidth(GetGraphicsEngine().GetFrameBuffer_W());
		m_2dCamera.SetHeight(GetGraphicsEngine().GetFrameBuffer_H());
		m_2dCamera.UpdateMatrix();
	}

	void PrimitiveRender::Render() {
#ifndef DW_DX12_TEMPORARY
		if (!m_isDraw3D) { return; }
		
		bool isDraw = false;
		for (auto& L : m_ilneList3D) {
			if (L.isHUD < 0 || L.isHUD == GetGraphicsEngine().GetTargetScreenNum()) {
				isDraw = true; break;
			}
		}
		if (!isDraw) { return; }

		//GPUイベントの開始
		GetGraphicsEngine().BeginGPUEvent(L"PrimitiveRender");

		ID3D11DeviceContext* d3dContext = GetEngine().GetGraphicsEngine().GetD3DDeviceContext();

		//レンダーターゲットとか設定		
		d3dContext->OMSetRenderTargets(
			1,
			&GetEngine().GetGraphicsEngine().GetFRT().GetRTV(),
			GetEngine().GetGraphicsEngine().GetGBufferRender().GetDSV()
		);

		//d3dContext->OMSetBlendState(m_states->Opaque(), nullptr, 0xFFFFFFFF);
		//d3dContext->OMSetDepthStencilState(m_states->DepthNone(), 0);
		d3dContext->RSSetState(GetGraphicsEngine().GetCommonStates().CullNone());

		//行列設定
		m_effect->SetView(GetMainCamera()->GetViewMatrix());
		m_effect->SetProjection(GetMainCamera()->GetProjMatrix());

		m_effect->Apply(d3dContext);
		d3dContext->IASetInputLayout(m_inputLayout.Get());
		
		//描画開始
		m_batch->Begin();

		//線分描画
		for (auto& L : m_ilneList3D) {
			if (L.isHUD < 0 || L.isHUD == GetGraphicsEngine().GetTargetScreenNum()) {
				m_batch->DrawLine(DirectX::VertexPositionColor(L.line.start, L.line.color), DirectX::VertexPositionColor(L.line.end, L.line.color));
			}
		}

		//描画終了
		m_batch->End();

		//レンダーターゲット解除
		d3dContext->OMSetRenderTargets(0, NULL, NULL);

		//GPUイベントの終了
		GetGraphicsEngine().EndGPUEvent();
#endif
	}
	void PrimitiveRender::PostRender() {
		if (!m_isDraw3D) { return; }
		//描画リストのクリア
		m_ilneList3D.clear();
		m_isDraw3D = false;
	}

	void PrimitiveRender::Render2D() {
#ifndef DW_DX12_TEMPORARY
		if (!m_isDraw2D) { return; }

		ID3D11DeviceContext* d3dContext = GetEngine().GetGraphicsEngine().GetD3DDeviceContext();		

		d3dContext->OMSetBlendState(GetGraphicsEngine().GetCommonStates().AlphaBlend(), nullptr, 0xFFFFFFFF);
		d3dContext->OMSetDepthStencilState(GetGraphicsEngine().GetCommonStates().DepthNone(), 0);
		d3dContext->RSSetState(GetGraphicsEngine().GetCommonStates().CullNone());

		//行列設定
		m_effect->SetView(m_2dCamera.GetViewMatrix());
		m_effect->SetProjection(m_2dCamera.GetProjMatrix());

		m_effect->Apply(d3dContext);
		d3dContext->IASetInputLayout(m_inputLayout.Get());

		//描画開始
		m_batch->Begin();

		//四角形描画
		for (auto& Q : m_quadList2D) {
			m_batch->DrawQuad(
				DirectX::VertexPositionColor(CVector3(Q.start.x, Q.start.y, 0.f), Q.color),
				DirectX::VertexPositionColor(CVector3(Q.end.x, Q.start.y, 0.f), Q.color),
				DirectX::VertexPositionColor(CVector3(Q.end.x, Q.end.y, 0.f), Q.color),
				DirectX::VertexPositionColor(CVector3(Q.start.x, Q.end.y, 0.f), Q.color)
			);
		}
		//線分描画
		for (auto& L : m_ilneList2D) {
			m_batch->DrawLine(DirectX::VertexPositionColor(L.start, L.color), DirectX::VertexPositionColor(L.end, L.color));
		}

		//描画終了
		m_batch->End();
#endif
	}
	void PrimitiveRender::PostRender2D() {
		if (!m_isDraw2D) { return; }
		//描画リストのクリア
		m_ilneList2D.clear();
		m_quadList2D.clear();
		m_isDraw2D = false;
	}

	void PrimitiveRender::RenderHUD(int HUDNum) {
#ifndef DW_DX12_TEMPORARY
		m_clearedHUD = false;

		if (m_isDrawHUD.size() <= HUDNum || !m_isDrawHUD[HUDNum]) { return; }

		ID3D11DeviceContext* d3dContext = GetEngine().GetGraphicsEngine().GetD3DDeviceContext();

		d3dContext->OMSetBlendState(GetGraphicsEngine().GetCommonStates().AlphaBlend(), nullptr, 0xFFFFFFFF);
		d3dContext->OMSetDepthStencilState(GetGraphicsEngine().GetCommonStates().DepthNone(), 0);
		d3dContext->RSSetState(GetGraphicsEngine().GetCommonStates().CullNone());

		//行列設定
		m_effect->SetView(m_2dCamera.GetViewMatrix());
		m_effect->SetProjection(m_2dCamera.GetProjMatrix());

		m_effect->Apply(d3dContext);
		d3dContext->IASetInputLayout(m_inputLayout.Get());

		//描画開始
		m_batch->Begin();

		//四角形描画
		if (m_quadListHUD.size() > HUDNum) {
			for (auto& Q : m_quadListHUD[HUDNum]) {
				m_batch->DrawQuad(
					DirectX::VertexPositionColor(CVector3(Q.start.x, Q.start.y, 0.f), Q.color),
					DirectX::VertexPositionColor(CVector3(Q.end.x, Q.start.y, 0.f), Q.color),
					DirectX::VertexPositionColor(CVector3(Q.end.x, Q.end.y, 0.f), Q.color),
					DirectX::VertexPositionColor(CVector3(Q.start.x, Q.end.y, 0.f), Q.color)
				);
			}
		}
		//線分描画
		if (m_ilneListHUD.size() > HUDNum) {
			for (auto& L : m_ilneListHUD[HUDNum]) {
				m_batch->DrawLine(DirectX::VertexPositionColor(L.start, L.color), DirectX::VertexPositionColor(L.end, L.color));
			}
		}

		//描画終了
		m_batch->End();
#endif
	}
	void PrimitiveRender::PostRenderHUD() {
		if (m_clearedHUD) { return; }//消去済み
		m_clearedHUD = true;

		//描画リストのクリア
		for (auto& line : m_ilneListHUD) {
			line.clear();
		}
		for (auto& quad : m_quadListHUD) {
			quad.clear();
		}
		for (auto it = m_isDrawHUD.begin(), e = m_isDrawHUD.end(); it != e; ++it) {
			*it = false;
		}
	}

	void PrimitiveRender::DrawLine(const CVector3& start, const CVector3& end, const CVector4& color, bool is3D) {
		BeginDraw(is3D);
		
		//線分描画
		m_batch->DrawLine(DirectX::VertexPositionColor(start, color), DirectX::VertexPositionColor(end, color));

		EndDraw();
	}
	void PrimitiveRender::DrawQuad(const CVector3& min, const CVector3& max, const CVector4& color, bool is3D) {
		BeginDraw(is3D);

		//四角形描画
		m_batch->DrawQuad(
			DirectX::VertexPositionColor(CVector3(min.x, min.y, 0.f), color),
			DirectX::VertexPositionColor(CVector3(max.x, min.y, 0.f), color),
			DirectX::VertexPositionColor(CVector3(max.x, max.y, 0.f), color),
			DirectX::VertexPositionColor(CVector3(min.x, max.y, 0.f), color)
		);

		EndDraw();
	}

	void PrimitiveRender::BeginDraw(bool is3D) {
#ifndef DW_DX12_TEMPORARY
		ID3D11DeviceContext* d3dContext = GetEngine().GetGraphicsEngine().GetD3DDeviceContext();
		
		if (is3D) {
			//行列設定
			m_effect->SetView(GetMainCamera()->GetViewMatrix());
			m_effect->SetProjection(GetMainCamera()->GetProjMatrix());

			d3dContext->RSSetState(GetGraphicsEngine().GetCommonStates().CullNone());
		}
		else {
			//行列設定
			m_effect->SetView(m_2dCamera.GetViewMatrix());
			m_effect->SetProjection(m_2dCamera.GetProjMatrix());

			d3dContext->OMSetBlendState(GetGraphicsEngine().GetCommonStates().AlphaBlend(), nullptr, 0xFFFFFFFF);
			d3dContext->OMSetDepthStencilState(GetGraphicsEngine().GetCommonStates().DepthNone(), 0);
			d3dContext->RSSetState(GetGraphicsEngine().GetCommonStates().CullNone());
		}

		m_effect->Apply(d3dContext);
		d3dContext->IASetInputLayout(m_inputLayout.Get());

		//描画開始
		m_batch->Begin();
#endif
	}
	void PrimitiveRender::EndDraw() {
		//描画終了
		m_batch->End();
	}

	void PrimitiveRender2D::Render() {
		//2Dプリミティブの描画
		GetGraphicsEngine().GetPrimitiveRender().Render2D();//描画
		GetGraphicsEngine().GetPrimitiveRender().PostRender2D();//後始末
	}
}