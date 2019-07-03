#include "DWstdafx.h"
#include "TextureFactory.h"

namespace DemolisherWeapon {
	TextureFactory* TextureFactory::instance = nullptr;

	void TextureFactory::Release() {
		for (auto& tex : m_textureMap) {
			tex.second.texture->Release();
			tex.second.textureView->Release();
		}
		m_textureMap.clear();
	}

	void TextureFactory::Load(std::experimental::filesystem::path filepath, ID3D11Resource** return_texture, ID3D11ShaderResourceView** return_textureView) {
		int index = Util::MakeHash(filepath.c_str());
		if (m_textureMap.count(index) > 0) {
			//もうある
			if (return_texture) { *return_texture = m_textureMap[index].texture; (*return_texture)->AddRef(); }
			if (return_textureView) { *return_textureView = m_textureMap[index].textureView; (*return_textureView)->AddRef(); }
		}
		else {
			//つくる
			TextueData texdata;

			//テクスチャ読み込み
			HRESULT hr;
			if (wcscmp(filepath.extension().c_str(), L".dds") == 0) {
				hr = DirectX::CreateDDSTextureFromFile(GetGraphicsEngine().GetD3DDevice(), filepath.c_str(), &texdata.texture, &texdata.textureView);
			}
			else {
				hr = DirectX::CreateWICTextureFromFile(GetGraphicsEngine().GetD3DDevice(), filepath.c_str(), &texdata.texture, &texdata.textureView);
			}
			if (FAILED(hr)) {
#ifndef DW_MASTER
				char message[256];
				sprintf_s(message, "TextureFactory::Load()の画像読み込みに失敗。\nファイルパスあってますか？\n%ls\n", filepath.c_str());
				DemolisherWeapon::Error::Box(message);
#endif
				return;
			}

			//登録
			m_textureMap.emplace(index, texdata);

			//返す
			if (return_texture) { *return_texture = texdata.texture; (*return_texture)->AddRef(); }
			if (return_textureView) { *return_textureView = texdata.textureView; (*return_textureView)->AddRef(); }
		}
	}
}