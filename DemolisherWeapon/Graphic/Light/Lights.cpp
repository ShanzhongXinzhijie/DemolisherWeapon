#include "DWstdafx.h"
#include "Lights.h"

namespace DemolisherWeapon {

namespace GameObj {

void CDirectionLight::PostLoopUpdate(){
	//�o�^
	if (GetActiveFlag()) {
		GetEngine().GetGraphicsEngine().GetLightManager().AddDirectionLight(this);
	}
};

void CPointLight::PostLoopUpdate(){

	m_struct.position = m_pos;
	//GetMainCamera()->GetViewMatrix().Mul(m_struct.position);

	//���W���r���[���W�ɂ���
	//m_struct.positionInView = m_pos;
	//GetMainCamera()->GetViewMatrix().Mul(m_struct.positionInView);

	//�o�^
	if (GetActiveFlag()) {
		GetEngine().GetGraphicsEngine().GetLightManager().AddPointLight(this);
	}
};

}

LightManager::~LightManager() {
	Release();
}

void LightManager::Init() {

	//���C�g�p�����[�^�[�̒萔�o�b�t�@
	{	
		int bufferSize = sizeof(SLightParam);
		D3D11_BUFFER_DESC bufferDesc;
		ZeroMemory(&bufferDesc, sizeof(bufferDesc));
		bufferDesc.Usage = D3D11_USAGE_DEFAULT;
		bufferDesc.ByteWidth = (((bufferSize - 1) / 16) + 1) * 16;
		bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		GetEngine().GetGraphicsEngine().GetD3DDevice()->CreateBuffer(&bufferDesc, NULL, &m_lightParamCB);
	}

	//�f�B���N�V�������C�g�̃X�g���N�`���[�o�b�t�@
	{
		D3D11_BUFFER_DESC desc;
		ZeroMemory(&desc, sizeof(desc));
		int stride = sizeof(SDirectionLight);

		desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;					//SRV�Ƀo�C���h�\�B
		desc.ByteWidth = static_cast<UINT>(stride * DIRLIGHT_NUM);	//�o�b�t�@�̃T�C�Y�B
		desc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
		desc.StructureByteStride = stride;

		//StructuredBuffer���쐬�B
		GetEngine().GetGraphicsEngine().GetD3DDevice()->CreateBuffer(&desc, NULL, &m_directionLightSB);

		//SRV���쐬
		D3D11_SHADER_RESOURCE_VIEW_DESC srvdesc;
		ZeroMemory(&srvdesc, sizeof(srvdesc));
		srvdesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFEREX;
		srvdesc.BufferEx.FirstElement = 0;
		srvdesc.Format = DXGI_FORMAT_UNKNOWN;
		srvdesc.BufferEx.NumElements = desc.ByteWidth / desc.StructureByteStride;

		GetEngine().GetGraphicsEngine().GetD3DDevice()->CreateShaderResourceView(m_directionLightSB, &srvdesc, &m_directionLightSRV);
	}

	//�|�C���g���C�g�̃X�g���N�`���[�o�b�t�@
	{
		D3D11_BUFFER_DESC desc;
		ZeroMemory(&desc, sizeof(desc));
		int stride = sizeof(SPointLight);

		desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;					//SRV�Ƀo�C���h�\�B
		desc.ByteWidth = static_cast<UINT>(stride * POILIGHT_NUM);	//�o�b�t�@�̃T�C�Y�B
		desc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
		desc.StructureByteStride = stride;

		//StructuredBuffer���쐬�B
		GetEngine().GetGraphicsEngine().GetD3DDevice()->CreateBuffer(&desc, NULL, &m_pointLightsSB);

		//SRV���쐬
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
	//�f�B���N�V�������C�g�̍X�V
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
					strcpy_s(message, "�y�����z�f�B���N�V�������C�g�������y��߂�z\n");
					OutputDebugStringA(message);
#endif
					break;
				}
			}
		}
		m_directionLights.clear();

		//���C�g�̐��X�V
		m_lightParam.numDirectionLight = min(i, DIRLIGHT_NUM);
	}

	//�|�C���g���C�g�̍X�V
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
					strcpy_s(message, "�y�����z�|�C���g���C�g�������y��߂�z\n");
					OutputDebugStringA(message);
#endif
					break;
				}
			}
		}
		m_pointLights.clear();

		//���C�g�̐��X�V
		m_lightParam.numPointLight = min(i, POILIGHT_NUM);
	}

	//StructuredBuffer���X�V����B
	GetEngine().GetGraphicsEngine().GetD3DDeviceContext()->UpdateSubresource(m_directionLightSB, 0, NULL, m_rawDirectionLights, 0, 0);
	GetEngine().GetGraphicsEngine().GetD3DDeviceContext()->UpdateSubresource(m_pointLightsSB, 0, NULL, m_rawPointLights, 0, 0);
}

void LightManager::UpdateConstantBuffer() {
	//�J�����ʒu�X�V
	if (GetMainCamera()) {
		m_lightParam.eyePos = GetMainCamera()->GetPos();

		//�t�H�O���X�V
		//m_lightParam.fogFar = GetMainCamera()->GetFar();
		if (m_lightParam.numDirectionLight > 0) {
			m_lightParam.fogLightDir = m_rawDirectionLights[0].direction ;//TODO -1 ����? //.direction�͂��ł�*-1����Ă�
			m_lightParam.fogLightColor = m_rawDirectionLights[0].color;
		}
	}

	//�萔�o�b�t�@�X�V
	GetEngine().GetGraphicsEngine().GetD3DDeviceContext()->UpdateSubresource(m_lightParamCB, 0, NULL, &m_lightParam, 0, 0);
}

}