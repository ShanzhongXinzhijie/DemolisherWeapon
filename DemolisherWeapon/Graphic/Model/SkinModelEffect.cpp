#include "DWstdafx.h"
#include "Graphic/Model/SkinModelEffect.h"
#include "Graphic/Model/SkinModelShaderConst.h"

namespace DemolisherWeapon {

ModelEffect::enShaderMode ModelEffect::m_s_shadermode = enNormalShader;

void __cdecl ModelEffect::Apply(ID3D11DeviceContext* deviceContext)
{
	//シェーダーモードにおうじたシェーダをセット
	switch (m_s_shadermode) {
	case enZShader:
		deviceContext->VSSetShader((ID3D11VertexShader*)m_vsZShader.GetBody(), NULL, 0);
		deviceContext->PSSetShader((ID3D11PixelShader*)m_psZShader.GetBody(), NULL, 0);
		break;
	default:
		//モーションブラーの有効で切り替える
		if (m_pVSShader == &m_vsDefaultShader[enALL] && !m_enableMotionBlur) {
			deviceContext->VSSetShader((ID3D11VertexShader*)m_vsDefaultShader[enNoMotionBlur].GetBody(), NULL, 0);
		}
		else {
			deviceContext->VSSetShader((ID3D11VertexShader*)m_pVSShader->GetBody(), NULL, 0);
		}
		if (m_pPSShader == &m_psDefaultShader[enALL] && !m_enableMotionBlur) {
			deviceContext->PSSetShader((ID3D11PixelShader*)m_psDefaultShader[enNoMotionBlur].GetBody(), NULL, 0);
		}
		else {
			deviceContext->PSSetShader((ID3D11PixelShader*)m_pPSShader->GetBody(), NULL, 0);
		}
		break;
	}

	//テクスチャ
	deviceContext->PSSetShaderResources(enSkinModelSRVReg_AlbedoTexture, 1, &m_pAlbedoTex);

	//定数バッファ
	deviceContext->UpdateSubresource(m_materialParamCB, 0, NULL, &m_materialParam, 0, 0);
	deviceContext->PSSetConstantBuffers(enSkinModelCBReg_Material, 1, &m_materialParamCB);
}

}