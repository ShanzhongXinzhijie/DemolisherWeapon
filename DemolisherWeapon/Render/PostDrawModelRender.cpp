#include "DWstdafx.h"
#include "PostDrawModelRender.h"

namespace DemolisherWeapon {

	void PostDrawModelRender::Init() {
		D3D11_BLEND_DESC blendDesc;

		//アルファブレンドステート(乗算済み)
		ZeroMemory(&blendDesc, sizeof(blendDesc));
		blendDesc.RenderTarget[0].BlendEnable = true;
		blendDesc.RenderTarget[0].SrcBlend = blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
		blendDesc.RenderTarget[0].DestBlend = blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_INV_SRC_ALPHA;
		blendDesc.RenderTarget[0].BlendOp = blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
		blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
		GetGraphicsEngine().GetD3DDevice()->CreateBlendState(&blendDesc, m_alphaBlendState.ReleaseAndGetAddressOf());

		//加算ブレンドステート
		ZeroMemory(&blendDesc, sizeof(blendDesc));
		blendDesc.RenderTarget[0].BlendEnable = true;
		blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_ONE;
		blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_ONE;
		blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
		blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
		blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
		blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
		blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_RED | D3D11_COLOR_WRITE_ENABLE_BLUE | D3D11_COLOR_WRITE_ENABLE_GREEN;
		GetGraphicsEngine().GetD3DDevice()->CreateBlendState(&blendDesc, m_addBlendState.ReleaseAndGetAddressOf());
	}

	void PostDrawModelRender::Render() {
#ifndef DW_MASTER
		if (!GetMainCamera()) {
			MessageBox(NULL, "カメラが設定されていません!!", "Error", MB_OK);
			std::abort();
		}
#endif
		//ブレンドステート変更
		ID3D11BlendState* oldBlendState = nullptr; FLOAT oldf[4]; UINT olduint;
		GetGraphicsEngine().GetD3DDeviceContext()->OMGetBlendState(&oldBlendState, oldf, &olduint);
		
		//描画先を最終レンダーターゲットにする
		GetGraphicsEngine().GetD3DDeviceContext()->OMSetRenderTargets(
			1,
			&GetGraphicsEngine().GetFRT().GetRTV(),
			GetGraphicsEngine().GetGBufferRender().GetDSV()
		);
		
		//モデル描画
		for (int i = 0; i < DRAW_PRIORITY_MAX; i++) {
			//モデル描画(加算ブレンド)		
			for (auto& model : m_drawModelList_Add[i]) {
				model->Draw(false, 1, m_addBlendState.Get());
			}
			//モデル描画(アルファブレンド)
			for (auto& model : m_drawModelList_Alpha[i]) {
				model->Draw(false, 1, m_alphaBlendState.Get());
			}
		}

		//ラスタライザーステート戻す
		GetGraphicsEngine().ResetRasterizerState();

		//レンダーターゲット解除
		GetGraphicsEngine().GetD3DDeviceContext()->OMSetRenderTargets(0, NULL, NULL);

		//ブレンドステート戻す
		if (oldBlendState) {
			GetGraphicsEngine().GetD3DDeviceContext()->OMSetBlendState(oldBlendState, oldf, olduint);
			oldBlendState->Release();
		}		
	}

	void PostDrawModelRender::PostRender() {
		//モデル登録の解除
		for (auto& list : m_drawModelList_Alpha) {
			list.clear();
		}
		for (auto& list : m_drawModelList_Add) {
			list.clear();
		}
	}
}