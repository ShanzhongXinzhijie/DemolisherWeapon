#pragma once

namespace DemolisherWeapon {
	class Bone;

	class SkeletonIK
	{
	public:
		struct IKSetting {
			Bone* tipBone = nullptr; //��[�{�[��
			Bone* rootBone = nullptr;//���{�{�[��
			bool isFootIK = false;
			bool isEnable = true;
			CVector3 targetPos;//IK�̖ڕW�n�_
			float targetSize = 1.0f;//�ڕW�ʒu���̑傫��(�덷���e�l)
			int iteration = 5;//������
		};

	public:
		SkeletonIK();
		~SkeletonIK();

		/// <summary>
		/// IK�����s
		/// </summary>
		void Update();

		/// <summary>
		/// ���s����IK�̏�����
		/// </summary>
		/// <param name="setting">IK�̐ݒ�</param>
		/// <returns>���s����IK�̐ݒ�\���̂ւ̃|�C���^</returns>
		IKSetting* CreateIK(const IKSetting& setting) {
			m_ikList.emplace_back(setting);
			return &m_ikList.back();
		}

		/// <summary>
		/// ���s���邩�ݒ�
		/// </summary>
		/// <param name="enable">���s���邩</param>
		void SetEnable(bool enable) { m_isEnable = enable; }

	private:
		/// <summary>
		/// CCD�@�ɂ��IK�̎��s
		/// </summary>
		/// <param name="ik">���s����IK</param>
		void CalcCCD(const IKSetting& ik);

		/// <summary>
		/// FootIK�̐ڐG�_�̌v�Z
		/// </summary>
		/// <param name="ik">���s����IK</param>
		void CalcFootIKTarget(const IKSetting& ik);

	private:
		bool m_isEnable = true;
		std::list<IKSetting> m_ikList;//���s����IK�̃��X�g
	};
}
