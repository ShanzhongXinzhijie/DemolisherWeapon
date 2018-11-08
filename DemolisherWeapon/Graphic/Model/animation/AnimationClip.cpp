/*!
 *@brief	�A�j���[�V�����N���b�v�B
 */

#include "DWstdafx.h"
#include "AnimationClip.h"
#include "Graphic/Model/skeleton.h"


namespace DemolisherWeapon {

AnimationClip::~AnimationClip()
{
	//�L�[�t���[����j���B
	for (auto& keyFrame : m_keyframes) {
		delete keyFrame;
	}
}

void AnimationClip::Load(const wchar_t* filePath, bool loop, EnChangeAnimationClipUpAxis changeUpAxis)
{
	m_changeUpAxis = changeUpAxis;

	FILE* fp;
	if (_wfopen_s(&fp, filePath, L"rb") != 0) {
#ifdef _DEBUG
		//�t�@�C�����J���Ȃ������Ƃ��̏����B
		char message[256];
		sprintf_s(message, "AnimationClip::Load, �t�@�C���̃I�[�v���Ɏ��s���܂����B%ls\n", filePath);
		MessageBox(NULL, message, "Error", MB_OK);
		//�~�߂�B
		std::abort();
#endif
		return;
	}
	
	//�A�j���[�V�����N���b�v�̃w�b�_�[�����[�h�B
	AnimClipHeader header;
	fread(&header, sizeof(header), 1, fp);
		
	if (header.numAnimationEvent > 0) {
		//�A�j���[�V�����C�x���g�͖��Ή��B
		//�A�E��i�Ń`�������W���Ă݂悤�B
		std::abort();
	}


	//���g�R�s�[���邽�߂̃��������h�J�b�Ɗm�ہB
	KeyframeRow* keyframes = new KeyframeRow[header.numKey];
	//�L�[�t���[�����h�J�b�Ɠǂݍ��݁B
	fread(keyframes, sizeof(KeyframeRow), header.numKey, fp);
	//�����f�[�^�̃��[�h�͂��ׂďI������̂ŁA�t�@�C���͕���B
	fclose(fp);
	//tka�t�@�C���̃L�[�t���[���̃��[�J���ƃ���4x3�s��Ȃ̂�
	//�Q�[���Ŏg�p���₷���悤�ɁA4x4�s��ɕϊ����Ă����B
	for (int i = 0; i < (int)header.numKey; i++) {
		//�Q�[���Ŏg�p����Keyframe�̃C���X�^���X�𐶐��B
		Keyframe* keyframe = new Keyframe;
		//�{�[���ԍ��Ƃ��Đ����ԂƂ����R�s�[���Ă����B
		keyframe->boneIndex = keyframes[i].boneIndex;
		keyframe->transform = CMatrix::Identity();
		keyframe->time = keyframes[i].time;
		//�s��̓R�s�[�B
		for (int j = 0; j < 4; j++) {
			keyframe->transform.m[j][0] = keyframes[i].transform[j].x;
			keyframe->transform.m[j][1] = keyframes[i].transform[j].y;
			keyframe->transform.m[j][2] = keyframes[i].transform[j].z;
		}

		//���[�g�{�[���Ɏ��o�C�A�X���|����
		if (m_changeUpAxis != enNonChange && keyframe->boneIndex == 0) {
			CMatrix mBias = CMatrix::Identity();
			if (m_changeUpAxis == enZtoY) {
				//Z to Y-up
				mBias.MakeRotationX(CMath::PI * 0.5f);
			}
			if (m_changeUpAxis == enYtoZ) {
				//Y to Z-up
				mBias.MakeRotationX(CMath::PI * -0.5f);
			}
			keyframe->transform.Mul(mBias, keyframe->transform);
		}

		//�V����������L�[�t���[�����ϒ��z��ɒǉ��B
		m_keyframes.push_back(keyframe);
	}

	//�L�[�t���[���͑S���R�s�[�I������̂ŁA�t�@�C������ǂݍ��񂾕��͔j������B
	delete[] keyframes;

	//�{�[���ԍ����ƂɃL�[�t���[����U�蕪���Ă����B
	m_keyFramePtrListArray.resize(Skeleton::MAX_BONE);
	for (Keyframe* keyframe : m_keyframes) {
		m_keyFramePtrListArray[keyframe->boneIndex].push_back(keyframe);
		if (m_topBoneKeyFramList == nullptr) {
			m_topBoneKeyFramList = &m_keyFramePtrListArray[keyframe->boneIndex];
		}
	}

	//���[�v�ݒ�
	SetLoopFlag(loop);
}

}