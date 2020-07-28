#include "DWstdafx.h"
#include "TextureFactory.h"

namespace DemolisherWeapon {

	TextueData CreateTexture(std::experimental::filesystem::path filepath, bool generateMipmaps) {
		TextueData texdata;

		//テクスチャ読み込み
		HRESULT hr;
		if (wcscmp(filepath.extension().c_str(), L".dds") == 0) {
			//DDS
			texdata.isDDS = true;
#ifdef DW_DX12
			//リソース読み込み
			DirectX::ResourceUploadBatch resourceUpload(GetGraphicsEngine().GetD3D12Device());
			resourceUpload.Begin();
			hr = DirectX::CreateDDSTextureFromFile(GetGraphicsEngine().GetD3D12Device(), resourceUpload, filepath.c_str(), texdata.d3d12texture.ReleaseAndGetAddressOf(), generateMipmaps);
			auto uploadResourcesFinished = resourceUpload.End(GetGraphicsEngine().GetXTK12CommandQueue());
			uploadResourcesFinished.wait();
			//SRV作成
			if (SUCCEEDED(hr)) {
				texdata.descriptorHandle = GetGraphicsEngine().GetDX12().CreateSRV(texdata.d3d12texture.Get());
			}
#else
			if (generateMipmaps) {
				hr = DirectX::CreateDDSTextureFromFile(GetGraphicsEngine().GetD3DDevice(), GetGraphicsEngine().GetD3DDeviceContext(), filepath.c_str(), &texdata.texture, &texdata.textureView);
			}
			else {
				hr = DirectX::CreateDDSTextureFromFile(GetGraphicsEngine().GetD3DDevice(), filepath.c_str(), &texdata.texture, &texdata.textureView);
			}
#endif
		}
		else {
			//DDS以外
			texdata.isDDS = false;
#ifdef DW_DX12
			DirectX::ResourceUploadBatch resourceUpload(GetGraphicsEngine().GetD3D12Device());
			resourceUpload.Begin();
			hr = DirectX::CreateWICTextureFromFile(GetGraphicsEngine().GetD3D12Device(), resourceUpload, filepath.c_str(), texdata.d3d12texture.ReleaseAndGetAddressOf(), generateMipmaps);
			auto uploadResourcesFinished = resourceUpload.End(GetGraphicsEngine().GetXTK12CommandQueue());
			uploadResourcesFinished.wait();
			//SRV作成
			if (SUCCEEDED(hr)) {
				texdata.descriptorHandle = GetGraphicsEngine().GetDX12().CreateSRV(texdata.d3d12texture.Get());
			}
#else
			if (generateMipmaps) {
				hr = DirectX::CreateWICTextureFromFile(GetGraphicsEngine().GetD3DDevice(), GetGraphicsEngine().GetD3DDeviceContext(), filepath.c_str(), &texdata.texture, &texdata.textureView);
			}
			else {
				hr = DirectX::CreateWICTextureFromFile(GetGraphicsEngine().GetD3DDevice(), filepath.c_str(), &texdata.texture, &texdata.textureView);
			}
#endif
		}
		if (FAILED(hr)) {
#ifndef DW_MASTER
			char message[256];
			sprintf_s(message, "CreateTexture()の画像読み込みに失敗。\nファイルパスあってますか？\n%ls\n", filepath.c_str());
			DemolisherWeapon::Error::Box(message);
#endif
			return {};
		}

#ifdef DW_DX12
		//画像サイズの取得
		D3D12_RESOURCE_DIMENSION resType = D3D12_RESOURCE_DIMENSION_UNKNOWN;
		texdata.width = static_cast<UINT>(texdata.d3d12texture->GetDesc().Width);
		texdata.height = static_cast<UINT>(texdata.d3d12texture->GetDesc().Height);
#else
		//画像サイズの取得
		D3D11_RESOURCE_DIMENSION resType = D3D11_RESOURCE_DIMENSION_UNKNOWN;
		texdata.texture->GetType(&resType);
		switch (resType) {
		case D3D11_RESOURCE_DIMENSION_TEXTURE2D:
		{
			ID3D11Texture2D* tex2d = static_cast<ID3D11Texture2D*>(texdata.texture.Get());
			D3D11_TEXTURE2D_DESC desc; tex2d->GetDesc(&desc);
			//画像サイズの取得
			texdata.width = desc.Width;
			texdata.height = desc.Height;
		}
		break;
		default:
#ifndef DW_MASTER
			char message[256];
			sprintf_s(message, "CreateTexture()「なんかちがう」\n%ls\n", filepath.c_str());
			MessageBox(NULL, message, "Error", MB_OK);
			std::abort();
#endif
			return {};
			break;
		}
#endif

		return texdata;
	}

	TextureFactory* TextureFactory::instance = nullptr;

	void TextureFactory::Release() {
		m_textureMap.clear();
	}

	bool TextureFactory::Load(std::experimental::filesystem::path filepath, ID3D11Resource** return_texture, ID3D11ShaderResourceView** return_textureView, const TextueData** return_textureData, bool generateMipmaps) {
		int index = Util::MakeHash(filepath.c_str());
		if (m_textureMap.count(index) <= 0) {		
			//つくる
			TextueData texdata = CreateTexture(filepath, generateMipmaps);
			if (!texdata.isLoaded()) {
				return false;
			}

			//登録
			m_textureMap.emplace(index, texdata);
		}

		//取り出して返す
		const TextueData& refTexdata = m_textureMap[index];
		if (return_texture) { *return_texture = refTexdata.texture.Get(); (*return_texture)->AddRef(); }
		if (return_textureView) { *return_textureView = refTexdata.textureView.Get(); (*return_textureView)->AddRef(); }
		if (return_textureData) { *return_textureData = &refTexdata; }

		return true;
	}
}