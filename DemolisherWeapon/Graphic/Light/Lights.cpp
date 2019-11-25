#include "DWstdafx.h"
#include "Lights.h"

namespace DemolisherWeapon {

namespace GameObj {

void CDirectionLight::PostLoopUpdate(){
	//登録
	if (GetActiveFlag()) {
		GetEngine().GetGraphicsEngine().GetLightManager().AddDirectionLight(this);
	}
};

void CPointLight::PostLoopUpdate(){

	m_struct.position = m_pos;
	//GetMainCamera()->GetViewMatrix().Mul(m_struct.position);

	//座標をビュー座標にする
	//m_struct.positionInView = m_pos;
	//GetMainCamera()->GetViewMatrix().Mul(m_struct.positionInView);

	//登録
	if (GetActiveFlag()) {
		GetEngine().GetGraphicsEngine().GetLightManager().AddPointLight(this);
	}
};

}

LightManager::~LightManager() {
	Release();
}

void LightManager::Init() {

	//ライトパラメーターの定数バッファ
	{	
		int bufferSize = sizeof(SLightParam);
		D3D11_BUFFER_DESC bufferDesc;
		ZeroMemory(&bufferDesc, sizeof(bufferDesc));
		bufferDesc.Usage = D3D11_USAGE_DEFAULT;
		bufferDesc.ByteWidth = (((bufferSize - 1) / 16) + 1) * 16;
		bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		GetEngine().GetGraphicsEngine().GetD3DDevice()->CreateBuffer(&bufferDesc, NULL, &m_lightParamCB);
	}

	//ディレクションライトのストラクチャーバッファ
	{
		D3D11_BUFFER_DESC desc;
		ZeroMemory(&desc, sizeof(desc));
		int stride = sizeof(SDirectionLight);

		desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;					//SRVにバインド可能。
		desc.ByteWidth = static_cast<UINT>(stride * DIRLIGHT_NUM);	//バッファのサイズ。
		desc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
		desc.StructureByteStride = stride;

		//StructuredBufferを作成。
		GetEngine().GetGraphicsEngine().GetD3DDevice()->CreateBuffer(&desc, NULL, &m_directionLightSB);

		//SRVを作成
		D3D11_SHADER_RESOURCE_VIEW_DESC srvdesc;
		ZeroMemory(&srvdesc, sizeof(srvdesc));
		srvdesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFEREX;
		srvdesc.BufferEx.FirstElement = 0;
		srvdesc.Format = DXGI_FORMAT_UNKNOWN;
		srvdesc.BufferEx.NumElements = desc.ByteWidth / desc.StructureByteStride;

		GetEngine().GetGraphicsEngine().GetD3DDevice()->CreateShaderResourceView(m_directionLightSB, &srvdesc, &m_directionLightSRV);
	}

	//ポイントライトのストラクチャーバッファ
	{
		D3D11_BUFFER_DESC desc;
		ZeroMemory(&desc, sizeof(desc));
		int stride = sizeof(SPointLight);

		desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;					//SRVにバインド可能。
		desc.ByteWidth = static_cast<UINT>(stride * POILIGHT_NUM);	//バッファのサイズ。
		desc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
		desc.StructureByteStride = stride;

		//StructuredBufferを作成。
		GetEngine().GetGraphicsEngine().GetD3DDevice()->CreateBuffer(&desc, NULL, &m_pointLightsSB);

		//SRVを作成
		D3D11_SHADER_RESOURCE_VIEW_DESC srvdesc;
		ZeroMemory(&srvdesc, sizeof(srvdesc));
		srvdesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFEREX;
		srvdesc.BufferEx.FirstElement = 0;
		srvdesc.Format = DXGI_FORMAT_UNKNOWN;
		srvdesc.BufferEx.NumElements = desc.ByteWidth / desc.StructureByteStride;

		GetEngine().GetGraphicsEngine().GetD3DDevice()->CreateShaderResourceView(m_pointLightsSB, &srvdesc, &m_pointLightsSRV);

	}
}

void LightManager::Release() {
	m_lightParamCB->Release();
	m_directionLightSB->Release();
	m_directionLightSRV->Release();
	m_pointLightsSB->Release();
	m_pointLightsSRV->Release();
}

void LightManager::UpdateStructuredBuffers() {
	//ディレクションライトの更新
	{
		int i = 0;
		for (auto& DL : m_directionLights) {
			if (DL->GetActiveFlag()) {
				m_rawDirectionLights[i] = DL->GetStruct();
				m_rawDirectionLights[i].color *= CMath::PI;
				i++;
				if (i >= DIRLIGHT_NUM) {
#ifndef DW_MASTER
					char message[256];
					strcpy_s(message, "【多い】ディレクションライトが多い【やめろ】\n");
					OutputDebugStringA(message);
#endif
					break;
				}
			}
		}
		m_directionLights.clear();

		//ライトの数更新
		m_lightParam.numDirectionLight = min(i, DIRLIGHT_NUM);
	}

	//ポイントライトの更新
	{
		int i = 0;
		for (auto& PL : m_pointLights) {
			if (PL->GetActiveFlag()) {
				m_rawPointLights[i] = PL->GetStruct();
				m_rawPointLights[i].color *= CMath::PI;
				i++;
				if (i >= POILIGHT_NUM) {
#ifndef DW_MASTER
					char message[256];
					strcpy_s(message, "【多い】ポイントライトが多い【やめろ】\n");
					OutputDebugStringA(message);
#endif
					break;
				}
			}
		}
		m_pointLights.clear();

		//ライトの数更新
		m_lightParam.numPointLight = min(i, POILIGHT_NUM);
	}

	//StructuredBufferを更新する。
	GetEngine().GetGraphicsEngine().GetD3DDeviceContext()->UpdateSubresource(m_directionLightSB, 0, NULL, m_rawDirectionLights, 0, 0);
	GetEngine().GetGraphicsEngine().GetD3DDeviceContext()->UpdateSubresource(m_pointLightsSB, 0, NULL, m_rawPointLights, 0, 0);
}

void LightManager::UpdateConstantBuffer() {
	//カメラ位置更新
	if (GetMainCamera()) {
		m_lightParam.eyePos = GetMainCamera()->GetPos();

		//フォグ情報更新
		//m_lightParam.fogFar = GetMainCamera()->GetFar();
		if (m_lightParam.numDirectionLight > 0) {
			m_lightParam.fogLightDir = m_rawDirectionLights[0].direction * -1.0f;//TODO -1 いる?
			m_lightParam.fogLightColor = m_rawDirectionLights[0].color;
		}
	}

	//定数バッファ更新
	GetEngine().GetGraphicsEngine().GetD3DDeviceContext()->UpdateSubresource(m_lightParamCB, 0, NULL, &m_lightParam, 0, 0);
}

}