/*!
 *@brief	�X�L�����f���֌W�֌W�̃V�F�[�_�[�p�����[�^�B
 */

#pragma once

/*!
 *@brief	�萔�o�b�t�@�̃��W�X�^�ԍ�
 */
enum EnSkinModelCBReg {
	enSkinModelCBReg_VSPS,		//!<���_�V�F�[�_�[�ƃs�N�Z���V�F�[�_�[���ʂ̒萔�o�b�t�@�B
	enSkinModelCBReg_Material,	//!<�}�e���A���p�����[�^�B

	enSkinModelCBReg_Free,		//��������g���ĂȂ�
};

/*!
 *@brief	SRV�̃��W�X�^�ԍ��B
 */
enum EnSkinModelSRVReg{
	enSkinModelSRVReg_AlbedoTexture,			//!<�A���x�h�e�N�X�`���B
	enSkinModelSRVReg_NormalTexture,			//!<�m�[�}���}�b�v�B
	enSkinModelSRVReg_LightngTexture,			//!<���C�e�B���O�p�����[�^�}�b�v�B
	
	enSkinModelSRVReg_BoneMatrixArray,			//!<�{�[���s��̔z��B
	enSkinModelSRVReg_BoneMatrixArrayOld,
	enSkinModelSRVReg_InstancingWorldMatrix,	//�C���X�^���V���O�p���[���h�s��
	enSkinModelSRVReg_InstancingWorldMatrixOld, 
	
	enSkinModelSRVReg_Free = 7,								//��������g���ĂȂ�
	enSkinModelSRVReg_InstancingImposterTextureIndex = 7,	//�������A�C���|�X�^�[�̃C���X�^���V���O�`��Ŏg�p����ꍇ�A��

	enSkinModelSRVReg_Free2,								//��������}�W�Ŏg���ĂȂ�
};
