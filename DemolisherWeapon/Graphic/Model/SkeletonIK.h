#pragma once

namespace DemolisherWeapon {
	class Bone;

	class SkeletonIK
	{
	public:
		struct IKSetting {
			Bone* tipBone = nullptr; //��[�{�[��
			Bone* rootBone = nullptr;//���{�{�[��
			bool isEnable = true;
			CVector3 targetPos;//IK�̖ڕW�n�_
			float targetSize = 1.0f;//�ڕW�ʒu���̑傫��(�덷���e�l)
			int iteration = 5;//������

			/// <summary>
			/// FootIK��������
			/// ��tipBone��rootBone��ݒ肵�Ă�������
			/// </summary>
			void InitFootIK();
			/// <summary>
			/// FootIK������
			/// </summary>
			void ReleaseFootIK();
			
			bool GetIsFootIK() const { return isFootIK; }
			const std::list<Bone*>& GetIsFootIKBoneList() const { return footIKBoneList; }

		private:
			bool isFootIK = false;
			std::list<Bone*> footIKBoneList;//FootIK�Ɏg���{�[���̃��X�g(�ܐ悩��n�܂�)
		public:
			CVector3 footIKRayEndOffset;//FootIK:�n�ʂƂ̔��背�C�I�_(tipBone�̈ʒu)�ɑ΂���I�t�Z�b�g
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
		/// <returns>IK�̐ݒ�ւ̃|�C���^</returns>
		IKSetting* CreateIK() {
			m_ikList.emplace_back();
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
		/// <returns>�ڐG���Ă��邩?</returns>
		bool CalcFootIKTarget(IKSetting& ik);

	private:
		bool m_isEnable = true;
		std::list<IKSetting> m_ikList;//���s����IK�̃��X�g
	};
}
