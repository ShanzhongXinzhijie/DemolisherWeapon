#include "DWstdafx.h"
#include "SkeletonIK.h"

namespace DemolisherWeapon {

	SkeletonIK::SkeletonIK()
	{
	}


	SkeletonIK::~SkeletonIK()
	{
	}

	void SkeletonIK::Update() {
		if (!m_isEnable) { return; }//���s���Ȃ�

		//�o�^����Ă���IK�ǂ��̎��s
		for (auto& IK : m_ikList) {
			if (!IK.isEnable) { continue; }//���s���Ȃ�

			//FootIK�Ȃ瑫�̐ڐG�_���߂�
			if (IK.isFootIK) {
				CalcFootIKTarget(IK);
			}

			//CCD�@��IK���s
			CalcCCD(IK);
		}
	}

	void SkeletonIK::CalcCCD(const IKSetting& ik) {
		//����
		for (int i = 0; i < ik.iteration; i++) {
			//��[�̃{�[��������s
			for (Bone* joint = ik.tipBone->GetParentBone(); joint != ik.rootBone && joint != nullptr; joint = joint->GetParentBone()) {
				//��[�{�[���̈ʒu�擾
				CVector3 effectorPos = ik.tipBone->GetPosition();
				//�{�[���̈ʒu�擾
				CVector3 jointPos = joint->GetPosition();

				// (1) ��֐߁���[�{�[���ʒu�ւ̕����x�N�g��
				CVector3 basis2Effector = effectorPos - jointPos; basis2Effector.Normalize();
				// (2) ��֐߁��ڕW�ʒu�ւ̕����x�N�g��
				CVector3 basis2Target = ik.targetPos - jointPos; basis2Target.Normalize();

				//��]�p
				float rotationAngle = acos(basis2Effector.Dot(basis2Target));
				
				//��]�p�����ȏ�Ŏ��s
				if (rotationAngle > 1.0e-5f) {
					//��]��
					CVector3 rotationAxis;
					rotationAxis.Cross(basis2Effector, basis2Target);
					rotationAxis.Normalize();

					//��]�N�H�[�^�j�I��
					CQuaternion qRot;
					qRot.SetRotation(rotationAxis, rotationAngle);
					//�s��ɂ���
					CMatrix mRot;
					mRot.MakeRotationFromQuaternion(qRot);

					//��]�s�񂪂ł����̂ŁA���[���h�s��ɔ��f�B
					CMatrix m = joint->GetWorldMatrix();
					CVector3 jointPos = { m.m[3][0], m.m[3][1], m.m[3][2] };
					m.m[3][0] = 0.0f;
					m.m[3][1] = 0.0f;
					m.m[3][2] = 0.0f;
					//�ǉ��̉�]��������B
					m = m * mRot;
					m.m[3][0] = jointPos.x;
					m.m[3][1] = jointPos.y;
					m.m[3][2] = jointPos.z;

					//���[�J���s������߂�B
					if (joint->GetParentId() != -1) {
						//�e����
						CMatrix localMat;
						//�e�̋t�s��
						CMatrix toParentSpaceMatrix = joint->GetParentBone()->GetWorldMatrix();
						toParentSpaceMatrix.Inverse();
						//�e�̋t�s�����Z���āA�e�̍��W�n�ł̍s������߂�B
						localMat = m * toParentSpaceMatrix;
						joint->SetLocalMatrix(localMat);
					}
					else {
						//�e�Ȃ�
						joint->SetLocalMatrix(m);
					}
					//���[���h�s��̍Čv�Z���s��(�q���{�[���̂�)
					Skeleton::UpdateBoneWorldMatrix(*joint, joint->GetParentBone()->GetWorldMatrix());
				}
			}

			//���[�g�{�[�����牺�̍��̃��[���h�s��̍Čv�Z���s���B
			//Skeleton::UpdateBoneWorldMatrix(*currentBone, m_skeleton->GetBone(m_rootBoneId)->GetWorldMatrix());

			//��[�{�[���ƖڕW�ʒu�̋��������ȉ����Ɣ�����
			if ((ik.tipBone->GetPosition() - ik.targetPos).LengthSq() < ik.targetSize*ik.targetSize) {
				break;
			}
		}
	}

	void SkeletonIK::CalcFootIKTarget(const IKSetting& ik) {

	}

}