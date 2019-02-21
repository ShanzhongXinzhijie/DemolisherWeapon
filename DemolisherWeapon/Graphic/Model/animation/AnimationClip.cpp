/*!
 *@brief	�A�j���[�V�����N���b�v�B
 */

#include "DWstdafx.h"
#include "AnimationClip.h"
#include "Graphic/Model/skeleton.h"

#include <filesystem>

namespace DemolisherWeapon {

bool AnimationClipDataManager::Load(AnimationClipData*& returnACD, const wchar_t* filePath, EnChangeAnimationClipUpAxis changeUpAxis) {
	//�L�[���쐬
	int filenameHash, paramHash;
	filenameHash = Util::MakeHash(filePath);

	int param = changeUpAxis;
	wchar_t str[6]; swprintf_s(str, L"%d", param);
	paramHash = Util::MakeHash(str);

	std::pair<int, int> key = std::make_pair(filenameHash, paramHash);

	//���ɓo�^����ĂȂ���?
	if (m_animationClipDataMap.count(key) > 0) {
		//�o�^����Ă���}�b�v����擾
		returnACD = m_animationClipDataMap[key];
		return false;
	}
	else {
		//�V�K�ǂݍ���
		m_animationClipDataMap[key] = new AnimationClipData();
		returnACD = m_animationClipDataMap[key];
		return true;
	}
}

AnimationClipDataManager AnimationClip::m_s_animationClipDataManager;

AnimationClip::~AnimationClip()
{
	//�L�[�t���[����j���B
	/*for (auto& keyFrame : m_keyframes) {
		delete keyFrame;
	}*/
}

void AnimationClip::Load(const wchar_t* filePath, bool loop, EnChangeAnimationClipUpAxis changeUpAxis)
{	
	//�}�l�[�W���[����f�[�^�ǂݍ���
	if (m_s_animationClipDataManager.Load(m_animationClipData, filePath, changeUpAxis) == false) {
		//�V�K�쐬�łȂ�

		//���[�v�ݒ�
		SetLoopFlag(loop);

		return;
	}

	//�V�K�쐬�Ȃ珉��������
	m_animationClipData->m_changeUpAxis = changeUpAxis;

	FILE* fp;
	if (_wfopen_s(&fp, filePath, L"rb") != 0) {
#ifndef DW_MASTER
		//�t�@�C�����J���Ȃ������Ƃ��̏����B
		char message[256];
		sprintf_s(message, "AnimationClip�̃I�[�v���Ɏ��s�B\n�t�@�C���p�X�����Ă܂���?\n%ls\n", filePath);
		MessageBox(NULL, message, "Error", MB_OK);
		//�~�߂�B
		std::abort();
#endif
		m_animationClipData = nullptr;
		return;
	}

	//�N���b�v���L�^
	std::experimental::filesystem::path ps = filePath;
	m_animationClipData->m_clipName = ps.stem();
	
	//�A�j���[�V�����N���b�v�̃w�b�_�[�����[�h�B
	AnimClipHeader header;
	fread(&header, sizeof(header), 1, fp);
		
	//�A�j���[�V�����C�x���g
	if (header.numAnimationEvent > 0) {
		//�\�[�g�p
		std::vector<std::pair<float, AnimationEvent>> sortEvent;

		m_animationClipData->m_animationEvent = std::make_unique<AnimationEvent[]>(header.numAnimationEvent);
		//�A�j���[�V�����C�x���g������Ȃ�A�C�x���g�������[�h����B
		for (auto i = 0; i < (int)header.numAnimationEvent; i++) {
			AnimationEventData animEvent;
			fread(&animEvent, sizeof(animEvent), 1, fp);
			//�C�x���g�������[�h����B
			static char eventName[256];
			static wchar_t wEventName[256];
			fread(eventName, animEvent.eventNameLength + 1, 1, fp);
			
			//mbstowcs(wEventName, eventName, 255);
			//�}���`�o�C�g����������C�h������ɕϊ�����B
			size_t rval = 0;
			mbstowcs_s(&rval, wEventName, 256, eventName, animEvent.eventNameLength + 1);

			m_animationClipData->m_animationEvent[i].SetInvokeTime(animEvent.invokeTime);
			m_animationClipData->m_animationEvent[i].SetEventName(wEventName);

			//�\�[�g�p�z����
			sortEvent.push_back(std::make_pair(animEvent.invokeTime, m_animationClipData->m_animationEvent[i]));
		}

		//�ꉞ�A�����Ƀ\�[�g
		std::sort(sortEvent.begin(), sortEvent.end(), [](const std::pair<float, AnimationEvent> &a, const std::pair<float, AnimationEvent> &b) {
			return a.first < b.first;
		});
		for (auto i = 0; i < (int)header.numAnimationEvent; i++) {
			m_animationClipData->m_animationEvent[i] = sortEvent[i].second;
		}
	}
	m_animationClipData->m_numAnimationEvent = header.numAnimationEvent;

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
		if (m_animationClipData->m_changeUpAxis != enNonChange && keyframe->boneIndex == 0) {
			CMatrix mBias = CMatrix::Identity();
			if (m_animationClipData->m_changeUpAxis == enZtoY) {
				//Z to Y-up
				mBias.MakeRotationX(CMath::PI * -0.5f);
			}
			if (m_animationClipData->m_changeUpAxis == enYtoZ) {
				//Y to Z-up
				mBias.MakeRotationX(CMath::PI * 0.5f);
			}
			keyframe->transform.Mul(mBias, keyframe->transform);
		}

		//�V����������L�[�t���[�����ϒ��z��ɒǉ��B
		m_animationClipData->m_keyframes.push_back(keyframe);
	}

	//�L�[�t���[���͑S���R�s�[�I������̂ŁA�t�@�C������ǂݍ��񂾕��͔j������B
	delete[] keyframes;

	//�{�[���ԍ����ƂɃL�[�t���[����U�蕪���Ă����B
	m_animationClipData->m_keyFramePtrListArray.resize(Skeleton::MAX_BONE);
	for (Keyframe* keyframe : m_animationClipData->m_keyframes) {
		m_animationClipData->m_keyFramePtrListArray[keyframe->boneIndex].push_back(keyframe);
		if (m_animationClipData->m_topBoneKeyFramList == nullptr) {
			m_animationClipData->m_topBoneKeyFramList = &m_animationClipData->m_keyFramePtrListArray[keyframe->boneIndex];
		}
	}

	//���[�v�ݒ�
	SetLoopFlag(loop);
}

}