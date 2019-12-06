#pragma once

namespace DemolisherWeapon {

class CFpsCounter
{
public:
	CFpsCounter();
	~CFpsCounter();

	//������
	void Init(int maxfps, int stdfps) {
		SetUseFpsLimiter(m_useFpsLimiter,maxfps);
		SetStandardFrameRate(stdfps);
	}

	//�t���[�����[�g�v��
	void Count();

	//�t���[�����[�g�`��
	void Draw();

	//�O��t���[���̎��s�ɂ�����������
	float GetFrameTimeSec() const { return m_frametimeSec; };
	//���s��������t���[�������擾
	float GetRunFrameCnt() const { return m_runframecnt; };

	//fps�����ݒ�
	void SetUseFpsLimiter(bool use, int maxfps = -1) { 
		m_useFpsLimiter = use;
		if (maxfps >= 1) {
			m_maxfps = maxfps; 
			m_maxFrameTimeSec = 1.0f / m_maxfps; 
		} 
	}
	//����FPS��ݒ�
	void SetStandardFrameRate(int fps) {
		m_standardfps = fps; 
		m_standardFrameTimeSec = 1.0f / m_standardfps;
	}

	//����FPS���擾
	int GetStandardFrameRate()const { return m_standardfps; }

	//�f�o�b�O����`�悷�邩�ݒ�
	void SetIsDebugDraw(bool enable) { m_isDebugDraw = enable; }
	bool GetIsDebugDraw()const { return m_isDebugDraw; }

private:
	LARGE_INTEGER nFreq, nBefore, nAfter;
	float m_frametimeSec = 0.0f, m_fps = 0.0f, m_runframecnt = 0.0f;

	bool m_useFpsLimiter = true;
	int m_maxfps = 60;
	float m_maxFrameTimeSec = 1.0f / m_maxfps;

	int m_standardfps = 60;
	float m_standardFrameTimeSec = 1.0f / m_standardfps;

#ifndef DW_MASTER
	bool m_isDebugDraw = true;
#else
	bool m_isDebugDraw = false;
#endif

	//fps�\���p
	float m_max = 0.0f, m_min = 100000.0f, m_avg = 0.0f, m_add = 0.0f, m_addTime = 0.0f;
	int m_avgcnt = 0;
};

}