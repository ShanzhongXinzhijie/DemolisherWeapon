#pragma once

namespace DemolisherWeapon {

	template<class T>
	class StructuredBuffer
	{
	public:
		StructuredBuffer() = default;
		StructuredBuffer(int num) { Init(num); }

		void Init(int num = 1) {
			//配列の確保
			m_t = std::make_unique<T[]>(num);
			int stride = sizeof(T);

			//StructuredBufferの確保
			D3D11_BUFFER_DESC desc;
			ZeroMemory(&desc, sizeof(desc));
			desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
			desc.ByteWidth = static_cast<UINT>(stride * num);
			desc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
			desc.StructureByteStride = stride;
			GetGraphicsEngine().GetD3DDevice()->CreateBuffer(&desc, NULL, m_SB.ReleaseAndGetAddressOf());

			//ShaderResourceViewの確保
			D3D11_SHADER_RESOURCE_VIEW_DESC descSRV;
			ZeroMemory(&descSRV, sizeof(descSRV));
			descSRV.ViewDimension = D3D11_SRV_DIMENSION_BUFFEREX;
			descSRV.BufferEx.FirstElement = 0;
			descSRV.Format = DXGI_FORMAT_UNKNOWN;
			descSRV.BufferEx.NumElements = desc.ByteWidth / desc.StructureByteStride;
			GetGraphicsEngine().GetD3DDevice()->CreateShaderResourceView(m_SB.Get(), &descSRV, m_SRV.ReleaseAndGetAddressOf());
		}

		//データを取得
		std::unique_ptr<T[]>& GetData() { return m_t; }

		//StructuredBufferを更新
		void UpdateSubresource() {
			GetGraphicsEngine().GetD3DDeviceContext()->UpdateSubresource(
				m_SB.Get(), 0, NULL, m_t.get(), 0, 0
			);
		}

		//SRVの取得
		ID3D11ShaderResourceView* GetSRV()const {
			return m_SRV.Get();
		}
		ID3D11ShaderResourceView*const* GetAddressOfSRV()const {
			return m_SRV.GetAddressOf();
		}

	private:
		std::unique_ptr<T[]>								m_t;
		Microsoft::WRL::ComPtr<ID3D11Buffer>				m_SB;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>	m_SRV;
	};

}