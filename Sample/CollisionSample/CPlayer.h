#pragma once

#include "CEnemy.h"

class CPlayer : public IGameObject {
public:
	bool Start()override {
		m_animationClips[0].Load(L"Resource/animData/run.tka", true, enYtoZ);
		m_animationClips[1].Load(L"Resource/animData/violence.tka",false);

		m_model.Init(L"Resource/modelData/unityChan.cmo", m_animationClips, 2);		
		m_rot.SetRotationDeg(CVector3::AxisY(), 90.0f);

		return true;
	};

	void Update()override {

		//���X�e�B�b�N�ňړ�
		m_pos.x += Pad(0).GetStick(L).x*40.0f;
		m_pos.z += Pad(0).GetStick(L).y*40.0f;

		//A�{�^���ōU��
		if (Pad(0).GetButton(enButtonA)) {		
			if (!m_trigger) {
				//�U������̔���
				GameObj::CCollisionObj* attackCol = NewGO<GameObj::CCollisionObj>();
				//�`��̍쐬
				attackCol->CreateSphere(m_pos + CVector3::AxisX()*50.0f, CQuaternion::Identity(), 100.0f);
				//������ݒ�
				attackCol->SetTimer(10);//10�t���[����폜�����
				//�Փˎ��Ɏ��s���鏈����ݒ�
				attackCol->SetCallback([&](GameObj::CCollisionObj::SCallbackParam& param) {
					//�Փ˂�������̖��O��"CEnemy"�Ȃ�5�_���[�W�^����
					if (param.EqualName(L"CEnemy")) {
						CEnemy* enemy = param.GetClass<CEnemy>();//����̔���ɐݒ肳��Ă���CEnemy�̃|�C���^���擾
						enemy->Damege(5);
					}
				}
				);
			}
			m_trigger = true;
		}
		else {
			m_trigger = false;
		}

		m_model.SetPRS(m_pos, m_rot, m_model.GetScale());
		
	};

private:
	GameObj::CSkinModelRender m_model;
	AnimationClip m_animationClips[2];

	CVector3 m_pos = { 0.0f, 0.0f, 0.0f };
	CQuaternion m_rot;

	bool m_trigger = false;
};