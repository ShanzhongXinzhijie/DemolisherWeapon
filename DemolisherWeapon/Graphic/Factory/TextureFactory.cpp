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

	bool TextureFactory::Load(std::experimental::filesystem::path filepath, ID3D11Resource** return_texture, ID3D11ShaderResourceView** return_textureView, const TextueData** return_textureData) {
		int index = Util::MakeHash(filepath.c_str());
		if (m_textureMap.count(index) <= 0) {		
			//����
			TextueData texdata;

			//�e�N�X�`���ǂݍ���
			HRESULT hr;
			if (wcscmp(filepath.extension().c_str(), L".dds") == 0) {
				texdata.isDDS = true;
				hr = DirectX::CreateDDSTextureFromFile(GetGraphicsEngine().GetD3DDevice(), filepath.c_str(), &texdata.texture, &texdata.textureView);
			}
			else {
				texdata.isDDS = false;
				hr = DirectX::CreateWICTextureFromFile(GetGraphicsEngine().GetD3DDevice(), filepath.c_str(), &texdata.texture, &texdata.textureView);
			}
			if (FAILED(hr)) {
#ifndef DW_MASTER
				char message[256];
				sprintf_s(message, "TextureFactory::Load()�̉摜�ǂݍ��݂Ɏ��s�B\n�t�@�C���p�X�����Ă܂����H\n%ls\n", filepath.c_str());
				DemolisherWeapon::Error::Box(message);
#endif
				return false;
			}

			//�摜�T�C�Y�̎擾
			D3D11_RESOURCE_DIMENSION resType = D3D11_RESOURCE_DIMENSION_UNKNOWN;
			texdata.texture->GetType(&resType);
			switch (resType) {
			case D3D11_RESOURCE_DIMENSION_TEXTURE2D:
			{
				ID3D11Texture2D* tex2d = static_cast<ID3D11Texture2D*>(texdata.texture);
				D3D11_TEXTURE2D_DESC desc; tex2d->GetDesc(&desc);
				//�摜�T�C�Y�̎擾
				texdata.width = desc.Width;
				texdata.height = desc.Height;
			}
			break;
			default:
#ifndef DW_MASTER
				char message[256];
				sprintf_s(message, "TextureFactory::Load()�u�Ȃ񂩂������v\n%ls\n", filepath.c_str());
				MessageBox(NULL, message, "Error", MB_OK);
				std::abort();
#endif
				return false;
				break;
			}

			//�o�^
			m_textureMap.emplace(index, texdata);
		}

		//���o���ĕԂ�
		const TextueData& refTexdata = m_textureMap[index];
		if (return_texture) { *return_texture = refTexdata.texture; (*return_texture)->AddRef(); }
		if (return_textureView) { *return_textureView = refTexdata.textureView; (*return_textureView)->AddRef(); }
		if (return_textureData) { *return_textureData = &refTexdata; }

		return true;
	}
}