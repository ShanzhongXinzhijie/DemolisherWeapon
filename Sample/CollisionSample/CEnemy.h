#pragma once

class CEnemy : public IGameObject {
public:	

	bool Start()override {
		m_model.Init(L"Resource/modelData/enemy_00.cmo");
		m_model.SetRot(CQuaternion::GetRotationDeg(CVector3::AxisY(), -90.0f));
		m_model.SetScale(CVector3::One()*20.0f);
		m_pos = CVector3(300.0f,0.0f,0.0f);

		//���炢����̍쐬
		m_collision = std::make_unique<SuicideObj::CCollisionObj>();
		//�`��̍쐬
		m_collision->CreateSphere(m_pos + CVector3::AxisY()*100.0f, CQuaternion::Identity(), 200.0f);
		//������ݒ�
		m_collision->SetTimer(enNoTimer);//enNoTimer�Ŏ����Ȃ�
		//���O��ݒ�
		m_collision->SetName(L"CEnemy");
		//�N���X�̃|�C���^��ݒ�
		m_collision->SetClass(this);

		return true;
	};

	void Update()override {		
		//�ړ�
		m_cnt++;
		if (m_cnt <= 30) {
			m_pos += CVector3::AxisX()*10.0f;
		}
		else if (m_cnt <= 60) {
			m_pos += CVector3::AxisX()*-10.0f;
		}
		if (m_cnt == 60) { m_cnt = 0; }
		m_model.SetPos(m_pos);

		//�R���W�����̍X�V
		m_collision->SetPosition(m_pos + CVector3::AxisY()*100.0f);
	};

	void PostRender()override {
		wchar_t output[256];
		swprintf_s(output, L"CEnemy��HP: %d", m_hp);
		m_font.Draw(output, { 0.9f,0.9f }, CVector4::White(), CVector2::One(), {1.0f,1.0f});
	}

	//�_���[�W�H�炤
	void Damege(int damege) {
		m_hp -= damege;
		if (m_hp <= 0) {
			DeleteGO(this);
		}
	}

private:
	GameObj::CSkinModelRender m_model;

	int m_hp = 100;
	CFont m_font;

	CVector3 m_pos = { 0.0f, 0.0f, 0.0f };
	int m_cnt = 0;

	std::unique_ptr<SuicideObj::CCollisionObj> m_collision;
};