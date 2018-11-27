#include "DWstdafx.h"
#include "Graphic/Model/SkinModelEffect.h"
#include "Graphic/Model/SkinModelShaderConst.h"

namespace DemolisherWeapon {

ModelEffect::enShaderMode ModelEffect::m_s_shadermode = enNormalShader;

void __cdecl ModelEffect::Apply(ID3D11DeviceContext* deviceContext)
{
	switch (m_s_shadermode) {
	case enZShader:
		deviceContext->VSSetShader((ID3D11VertexShader*)m_vsZShader.GetBody(), NULL, 0);
		deviceContext->PSSetShader((ID3D11PixelShader*)m_psZShader.GetBody(), NULL, 0);
		break;
	default:
		deviceContext->VSSetShader((ID3D11VertexShader*)m_pVSShader->GetBody(), NULL, 0);
		deviceContext->PSSetShader((ID3D11PixelShader*)m_pPSShader->GetBody(), NULL, 0);
		break;
	}

	deviceContext->PSSetShaderResources(enSkinModelSRVReg_AlbedoTexture, 1, &m_albedoTex);

	//deviceContext->UpdateSubresource(m_materialParamCB.GetBody(), 0, NULL, &m_materialParam, 0, 0);
	//deviceContext->PSSetConstantBuffers(enSkinModelCBReg_Material, 1, &m_materialParamCB.GetBody());
}

}