#include "DWstdafx.h"
#include "CSkinModelRender.h"
#include "Graphic/Model/SkinModelShaderConst.h"

namespace DemolisherWeapon {
namespace GameObj {

ID3D11RasterizerState* CSkinModelRender::m_mostDepthRSCw = nullptr;
ID3D11RasterizerState* CSkinModelRender::m_mostDepthRSCCw = nullptr;
ID3D11RasterizerState* CSkinModelRender::m_mostDepthRSNone = nullptr;

CSkinModelRender::CSkinModelRender(bool isRegister) : IQSGameObject(isRegister)
{
	if (GetGraphicsAPI() == enDirectX11) {
		if (!m_mostDepthRSCw) {
			D3D11_RASTERIZER_DESC desc = {};
			desc.CullMode = D3D11_CULL_FRONT;
			desc.FillMode = D3D11_FILL_SOLID;
			desc.DepthClipEnable = true;
			desc.MultisampleEnable = true;
			desc.DepthBias = (INT)DEPTH_BIAS_D32_FLOAT(1.0f);
			GetGraphicsEngine().GetD3DDevice()->CreateRasterizerState(&desc, &m_mostDepthRSCw);
		}
		if (!m_mostDepthRSCCw) {
			D3D11_RASTERIZER_DESC desc = {};
			desc.CullMode = D3D11_CULL_BACK;
			desc.FillMode = D3D11_FILL_SOLID;
			desc.DepthClipEnable = true;
			desc.MultisampleEnable = true;
			desc.DepthBias = (INT)DEPTH_BIAS_D32_FLOAT(1.0f);
			GetGraphicsEngine().GetD3DDevice()->CreateRasterizerState(&desc, &m_mostDepthRSCCw);
		}
		if (!m_mostDepthRSNone) {
			D3D11_RASTERIZER_DESC desc = {};
			desc.CullMode = D3D11_CULL_NONE;
			desc.FillMode = D3D11_FILL_SOLID;
			desc.DepthClipEnable = true;
			desc.MultisampleEnable = true;
			desc.DepthBias = (INT)DEPTH_BIAS_D32_FLOAT(1.0f);
			GetGraphicsEngine().GetD3DDevice()->CreateRasterizerState(&desc, &m_mostDepthRSNone);
		}
	}
}

void CSkinModelRender::PreLoopUpdate() {
	if (!m_isInit) { return; }

	//�����W�̍X�V
	m_model.UpdateOldMatrix();
}

void CSkinModelRender::Update() {
	if (!m_isInit || m_animUpdating || !m_isEnableUpdate) { return; }

	if (!m_isUpdated) {
		m_animUpdating = true;
		m_animCon.Update();
		m_animUpdating = false;
	}
	if(!m_isUpdated || !m_isUpdatedWorldMatrix){
		UpdateWorldMatrix(m_isRefreshMode);
	}

	m_isUpdated = true;
}

void CSkinModelRender::PostUpdate() {
	if (!m_isInit) { return; }

	ImNonUpdate();
}

void CSkinModelRender::PostLoopUpdate() {
	if (!m_isInit) { return; }		
	if (!m_model.GetIsDraw()) { return; }

	if (m_isShadowCaster) {
		//�V���h�E�}�b�v�`��O��Ŏ��s���鏈���𑗂�
		//if (m_shadowMapPrePost) { GetGraphicsEngine().GetShadowMapRender().AddPrePostAction(m_shadowMapPrePost.get()); }
		//�V���h�E�}�b�v�����_�[�Ƀ��f������
		AddDrawModelToShadowMapRender(&m_model, m_priority, (m_isDrawReverse != m_isShadowDrawReverse));
	}
	if (m_isPostDraw) {
		//�|�X�g�h���[�����_�[�Ƀ��f������
		GetGraphicsEngine().AddDrawModelToPostDrawRender(&m_model, m_priority, m_postDrawBlendMode, m_isDrawReverse);
	}
	else {
		//3D���f�������_�[�Ƀ��f������
		AddDrawModelToD3Render(&m_model, m_priority, m_isDrawReverse);
	}

	//�o�E���f�B���O�{�b�N�X�̕\��
	if (m_isDrawBoundingBox) {
		CVector3 min, max;
		m_model.GetUpdatedBoundingBox(min, max);
		DrawLine3D({ min.x,min.y,min.z }, { min.x,max.y,min.z }, { 1.0f,0.0f,1.0f,1.0f });
		DrawLine3D({ max.x,min.y,min.z }, { max.x,max.y,min.z }, { 1.0f,0.0f,1.0f,1.0f });
		DrawLine3D({ min.x,min.y,max.z }, { min.x,max.y,max.z }, { 1.0f,0.0f,1.0f,1.0f });
		DrawLine3D({ max.x,min.y,max.z }, { max.x,max.y,max.z }, { 1.0f,0.0f,1.0f,1.0f });

		DrawLine3D({ min.x,min.y,min.z }, { min.x,min.y,max.z }, { 1.0f,0.0f,1.0f,1.0f });
		DrawLine3D({ min.x,min.y,min.z }, { max.x,min.y,min.z }, { 1.0f,0.0f,1.0f,1.0f });
		DrawLine3D({ max.x,min.y,min.z }, { max.x,min.y,max.z }, { 1.0f,0.0f,1.0f,1.0f });
		DrawLine3D({ min.x,min.y,max.z }, { max.x,min.y,max.z }, { 1.0f,0.0f,1.0f,1.0f });

		DrawLine3D({ min.x,max.y,min.z }, { min.x,max.y,max.z }, { 1.0f,0.0f,1.0f,1.0f });
		DrawLine3D({ min.x,max.y,min.z }, { max.x,max.y,min.z }, { 1.0f,0.0f,1.0f,1.0f });
		DrawLine3D({ max.x,max.y,min.z }, { max.x,max.y,max.z }, { 1.0f,0.0f,1.0f,1.0f });
		DrawLine3D({ min.x,max.y,max.z }, { max.x,max.y,max.z }, { 1.0f,0.0f,1.0f,1.0f });
	}
}

/*void CSkinModelRender::Init(const wchar_t* filePath,
	AnimationClip* animationClips,
	int numAnimationClips 
) {
	EnFbxUpAxis fbxUpAxis = enFbxUpAxisZ;
	EnFbxCoordinateSystem fbxCoordinate = enFbxRightHanded;

	//.tkm�͍���n?
	const wchar_t* extension = wcsrchr(filePath, L'.');
	if (_wcsicmp(extension, L".tkm") == 0) {
		//fbxUpAxis = enFbxUpAxisY;
		//fbxCoordinate = enFbxLeftHanded;
	}

	Init(filePath, animationClips, numAnimationClips, fbxUpAxis, fbxCoordinate);
}*/
void CSkinModelRender::Init(const wchar_t* filePath,
	AnimationClip* animationClips,
	int numAnimationClips,
	EnFbxUpAxis fbxUpAxis,
	EnFbxCoordinateSystem fbxCoordinate,
	bool isUseFlyweightFactory
) {

	if (m_isInit) { return; }

	//���f���ǂݍ���
	m_model.Init(filePath, fbxUpAxis, fbxCoordinate, isUseFlyweightFactory);
	
	//�A�j���[�V�����̏������B
	if (animationClips != nullptr) {
		m_animCon.Init(
			m_model,			//�A�j���[�V�����𗬂��X�L�����f���B
			animationClips,		//�A�j���[�V�����N���b�v�̔z��B
			numAnimationClips	//�A�j���[�V�����N���b�v�̐��B
		);
	}

	m_isInit = true;
}

void CSkinModelRender::InitPostDraw(PostDrawModelRender::enBlendMode blendMode, bool isPMA, enSoftParticleMode isSoftParticle, float softParticleArea) {
	m_isPostDraw = true; m_postDrawBlendMode = blendMode;

	enSoftParticleMode oldIsSoftParticle = m_shaderSNAIsSoftParticle;

	//�V�F�[�_�ǂݍ���
	if (!m_loadedShaderSNA || m_shaderSNAIsConvertPMA != !isPMA || m_shaderSNAIsSoftParticle != isSoftParticle) {
		m_loadedShaderSNA = true; m_shaderSNAIsConvertPMA = !isPMA; m_shaderSNAIsSoftParticle = isSoftParticle;

		//�}�N��
		D3D_SHADER_MACRO macros[3] = { NULL,NULL,NULL,NULL,NULL,NULL }, macrosTex[3] = { "TEXTURE","1",NULL,NULL,NULL,NULL };
		char shaderName[64] = "DEFAULT", shaderNameTex[64] = "TEXTURE";
		//�\�t�g�p�[�e�B�N���ݒ�
		if (m_shaderSNAIsSoftParticle == enSoftParticle) {
			macros[0].Name = "SOFT_PARTICLE";
			macros[0].Definition = "1";
			strcpy_s(shaderName, "SOFT_PARTICLE");
			macrosTex[1].Name = "SOFT_PARTICLE";
			macrosTex[1].Definition = "1";
			strcpy_s(shaderNameTex, "TEXTURE+SOFT_PARTICLE");
		}
		if (m_shaderSNAIsSoftParticle == enRevSoftParticle) {
			macros[0].Name = "REV_SOFT_PARTICLE";
			macros[0].Definition = "1";
			strcpy_s(shaderName, "REV_SOFT_PARTICLE");
			macrosTex[1].Name = "REV_SOFT_PARTICLE";
			macrosTex[1].Definition = "1";
			strcpy_s(shaderNameTex, "TEXTURE+REV_SOFT_PARTICLE");
		}

		if (m_shaderSNAIsConvertPMA) {
			//��Z�ς݃A���t�@�ɕϊ�����
			m_psSozaiNoAzi.Load("Preset/shader/model.fx", "PSMain_SozaiNoAzi_ConvertToPMA", Shader::EnType::PS, shaderName, macros);
			m_psSozaiNoAziTex.Load("Preset/shader/model.fx", "PSMain_SozaiNoAzi_ConvertToPMA", Shader::EnType::PS, shaderNameTex, macrosTex);
		}
		else {
			//�ʏ�
			m_psSozaiNoAzi.Load("Preset/shader/model.fx", "PSMain_SozaiNoAzi", Shader::EnType::PS, shaderName, macros);
			m_psSozaiNoAziTex.Load("Preset/shader/model.fx", "PSMain_SozaiNoAzi", Shader::EnType::PS, shaderNameTex, macrosTex);
		}
	}
	//�V�F�[�_�ݒ�
	GetSkinModel().FindMaterialSetting(
		[&](MaterialSetting* mat) {
			//�s�N�Z���V�F�[�_
			if (mat->GetAlbedoTexture()) {
				mat->SetPS(&m_psSozaiNoAziTex);//�e�N�X�`������
			}
			else {
				mat->SetPS(&m_psSozaiNoAzi);//�e�N�X�`���Ȃ�
			}
		}
	);
	//�\�t�g�p�[�e�B�N���ݒ�
	if (isSoftParticle != enOff && oldIsSoftParticle == enOff || isSoftParticle == enOff && oldIsSoftParticle != enOff) {
		if (isSoftParticle != enOff) {
			//�f�v�X�o�b�t�@��ݒ�
			GetSkinModel().SetPreDrawFunction(
				L"DW_SetViewPosTexture",
				[&](SkinModel*) {
					GetGraphicsEngine().GetD3DDeviceContext()->PSSetShaderResources(enSkinModelSRVReg_ViewPosTexture, 1, &GetGraphicsEngine().GetGBufferRender().GetGBufferSRV(GBufferRender::enGBufferPosition));
				}
			);
			//�f�v�X�o�b�t�@������
			GetSkinModel().SetPostDrawFunction(
				L"DW_RemoveViewPosTexture",
				[&](SkinModel*) {
					ID3D11ShaderResourceView* view[] = { NULL };
					GetGraphicsEngine().GetD3DDeviceContext()->PSSetShaderResources(enSkinModelSRVReg_ViewPosTexture, 1, view);
				}
			);
		}
		else {
			//�ݒ����
			GetSkinModel().ErasePreDrawFunction(L"DW_SetViewPosTexture");
			GetSkinModel().ErasePostDrawFunction(L"DW_RemoveViewPosTexture");
		}
	}
	//�\�t�g�p�[�e�B�N�����������ݒ�
	if (softParticleArea < 0.0f) {
		//�f�t�H���g
		GetSkinModel().SetSoftParticleArea(50.0f*(GetEngine().GetDistanceScale() / 3.5f));		
	}
	else {
		GetSkinModel().SetSoftParticleArea(softParticleArea);
	}
}

}
}