#include "DWstdafx.h"
#include "PrimitiveRender.h"
#include "DirectXTK/Src/PlatformHelpers.h"

namespace DemolisherWeapon {

	PrimitiveRender::PrimitiveRender()
	{
	}
	
	PrimitiveRender::~PrimitiveRender()
	{
	}

	void PrimitiveRender::Init() {
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
	}

	void PrimitiveRender::Render() {
		if (!m_isDraw) { return; }

		ID3D11DeviceContext* d3dContext = GetEngine().GetGraphicsEngine().GetD3DDeviceContext();

		//レンダーターゲットとか設定		
		d3dContext->OMSetRenderTargets(
			1,
			&GetEngine().GetGraphicsEngine().GetFRT().GetRTV(),
			GetEngine().GetGraphicsEngine().GetGBufferRender().GetDSV()
		);

		//d3dContext->OMSetBlendState(m_states->Opaque(), nullptr, 0xFFFFFFFF);
		//d3dContext->OMSetDepthStencilState(m_states->DepthNone(), 0);
		//d3dContext->RSSetState(m_states->CullNone());

		//行列設定
		m_effect->SetView(GetMainCamera()->GetViewMatrix());
		m_effect->SetProjection(GetMainCamera()->GetProjMatrix());

		m_effect->Apply(d3dContext);
		d3dContext->IASetInputLayout(m_inputLayout.Get());
		
		//描画開始
		m_batch->Begin();

		//線分描画
		for (auto& L : m_ilneList) {
			m_batch->DrawLine(DirectX::VertexPositionColor(L.start, L.color), DirectX::VertexPositionColor(L.end, L.color));
		}

		//描画終了
		m_batch->End();

		//レンダーターゲット解除
		GetEngine().GetGraphicsEngine().GetD3DDeviceContext()->OMSetRenderTargets(0, NULL, NULL);
	}
	void PrimitiveRender::PostRender() {
		m_ilneList.clear();
		m_isDraw = false;
	}
}