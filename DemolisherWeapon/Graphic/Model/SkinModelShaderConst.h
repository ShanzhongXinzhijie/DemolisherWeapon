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
};

/*!
 *@brief	SRV�̃��W�X�^�ԍ��B
 */
enum EnSkinModelSRVReg{
	enSkinModelSRVReg_AlbedoTexture,		//!<�A���x�h�e�N�X�`���B
	enSkinModelSRVReg_BoneMatrixArray,		//!<�{�[���s��̔z��B
	enSkinModelSRVReg_BoneMatrixArrayOld,
	enRenderZViewProjMatrix,				//Z�l�������ݗp�r���[�v���W�F�N�V�����s��
};
