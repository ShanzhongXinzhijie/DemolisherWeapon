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
			if (IK.GetIsFootIK()) {
				if (!CalcFootIKTarget(IK)) {
					//�ڐG���ĂȂ���������s���Ȃ�
					continue;
				}
			}

			//CCD�@��IK���s
			CalcCCD(IK);
		}
	}

	void SkeletonIK::CalcCCD(const IKSetting& ik) {
		//����
		for (int i = 0; i < ik.iteration; i++) {
			//��[�̃{�[��������s
			for (Bone* joint = ik.tipBone->GetParentBone(); joint != ik.rootBone->GetParentBone() && joint != nullptr; joint = joint->GetParentBone()) {
				//��[�{�[���̈ʒu�擾
				CVector3 effectorPos = ik.tipBone->GetPosition();
				//�{�[���̈ʒu�擾
				CVector3 jointPos = joint->GetPosition();

				// (1) ��֐߁���[�{�[���ʒu�ւ̕����x�N�g��
				CVector3 basis2Effector = effectorPos - jointPos; basis2Effector.Normalize();
				// (2) ��֐߁��ڕW�ʒu�ւ̕����x�N�g��
				CVector3 basis2Target = ik.targetPos - jointPos; basis2Target.Normalize();

				//��]�p
				float rotationAngle = acos(CMath::ClampFromNegOneToPosOne(basis2Effector.Dot(basis2Target)));
				
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

	bool SkeletonIK::CalcFootIKTarget(IKSetting& ik) {
		//���{����ܐ�܂Ń��C�Ŕ���
		//Bone* preBone = nullptr;
		//for(auto bone = ik.GetIsFootIKBoneList().rbegin(), end = ik.GetIsFootIKBoneList().rend(); bone != end; ++bone){
		//	if (preBone) {
		//		btVector3 rayStart = preBone->GetPosition();
		//		btVector3 rayEnd = (*bone)->GetPosition();
		//		btCollisionWorld::ClosestRayResultCallback gnd_ray(rayStart, rayEnd);
		//		GetEngine().GetPhysicsWorld().RayTest(rayStart, rayEnd, gnd_ray);
		//		if (gnd_ray.hasHit()) {
		//			//�ڐG�_��ڕW���W�ɂ���
		//			ik.targetPos.Set(gnd_ray.m_hitPointWorld);
		//			return true;
		//		}
		//	}
		//	preBone = *bone;
		//}

		btVector3 rayStart = ik.rootBone->GetPosition(); 
		btVector3 rayEnd = ik.tipBone->GetPosition() + ik.footIKRayEndOffset;
		btCollisionWorld::ClosestRayResultCallback gnd_ray(rayStart, rayEnd);
		GetEngine().GetPhysicsWorld().RayTest(rayStart, rayEnd, gnd_ray);
		if (gnd_ray.hasHit()) {
			//�ڐG�_��ڕW���W�ɂ���
			ik.targetPos.Set(gnd_ray.m_hitPointWorld);
			ik.targetPos -= ik.footIKRayEndOffset;
			return true;
		}

		//�ڐG�_�Ȃ�
		return false;
	}

	void SkeletonIK::IKSetting::InitFootIK() {
		ReleaseFootIK();

		//�ܐ悩�烊�X�g�ɓo�^���Ă���
		//Bone* joint = tipBone;
		//while (1) {
		//	if (joint == nullptr) { 
		//		//null���o����G���[
		//		DW_ERRORBOX(true, "SkeletonIK::IKSetting::InitFootIK() :nullptr�̏o��\n�{�[���̐ݒ�ɖ�肪����");
		//		return;
		//	}
		//	//�o�^
		//	footIKBoneList.emplace_back(joint);
		//	//���{�܂œo�^������I���
		//	if (joint == rootBone) { break; }
		//	//�e�{�[����
		//	joint = joint->GetParentBone();
		//}

		isFootIK = true;
	}
	void SkeletonIK::IKSetting::ReleaseFootIK() {
		if (!isFootIK) { return; }
		//footIKBoneList.clear();
		isFootIK = false;
	}
}