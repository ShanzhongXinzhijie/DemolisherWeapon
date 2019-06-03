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
		if (!m_isEnable) { return; }//実行しない

		//登録されているIKどもの実行
		for (auto& IK : m_ikList) {
			if (!IK.isEnable) { continue; }//実行しない

			//FootIKなら足の接触点求める
			if (IK.GetIsFootIK()) {
				if (!CalcFootIKTarget(IK)) {
					//接触してなかったら実行しない
					return;
				}
			}

			//CCD法でIK実行
			CalcCCD(IK);
		}
	}

	void SkeletonIK::CalcCCD(const IKSetting& ik) {
		//反復
		for (int i = 0; i < ik.iteration; i++) {
			//先端のボーンから実行
			for (Bone* joint = ik.tipBone->GetParentBone(); joint != ik.rootBone && joint != nullptr; joint = joint->GetParentBone()) {
				//先端ボーンの位置取得
				CVector3 effectorPos = ik.tipBone->GetPosition();
				//ボーンの位置取得
				CVector3 jointPos = joint->GetPosition();

				// (1) 基準関節→先端ボーン位置への方向ベクトル
				CVector3 basis2Effector = effectorPos - jointPos; basis2Effector.Normalize();
				// (2) 基準関節→目標位置への方向ベクトル
				CVector3 basis2Target = ik.targetPos - jointPos; basis2Target.Normalize();

				//回転角
				float rotationAngle = acos(CMath::ClampFromNegOneToPosOne(basis2Effector.Dot(basis2Target)));
				
				//回転角が一定以上で実行
				if (rotationAngle > 1.0e-5f) {
					//回転軸
					CVector3 rotationAxis;
					rotationAxis.Cross(basis2Effector, basis2Target);
					rotationAxis.Normalize();

					//回転クォータニオン
					CQuaternion qRot;
					qRot.SetRotation(rotationAxis, rotationAngle);
					//行列にする
					CMatrix mRot;
					mRot.MakeRotationFromQuaternion(qRot);

					//回転行列ができたので、ワールド行列に反映。
					CMatrix m = joint->GetWorldMatrix();
					CVector3 jointPos = { m.m[3][0], m.m[3][1], m.m[3][2] };
					m.m[3][0] = 0.0f;
					m.m[3][1] = 0.0f;
					m.m[3][2] = 0.0f;
					//追加の回転を加える。
					m = m * mRot;
					m.m[3][0] = jointPos.x;
					m.m[3][1] = jointPos.y;
					m.m[3][2] = jointPos.z;

					//ローカル行列を求める。
					if (joint->GetParentId() != -1) {
						//親あり
						CMatrix localMat;
						//親の逆行列
						CMatrix toParentSpaceMatrix = joint->GetParentBone()->GetWorldMatrix();
						toParentSpaceMatrix.Inverse();
						//親の逆行列を乗算して、親の座標系での行列を求める。
						localMat = m * toParentSpaceMatrix;
						joint->SetLocalMatrix(localMat);
					}
					else {
						//親なし
						joint->SetLocalMatrix(m);
					}
					//ワールド行列の再計算を行う(子供ボーンのも)
					Skeleton::UpdateBoneWorldMatrix(*joint, joint->GetParentBone()->GetWorldMatrix());
				}
			}

			//ルートボーンから下の骨のワールド行列の再計算を行う。
			//Skeleton::UpdateBoneWorldMatrix(*currentBone, m_skeleton->GetBone(m_rootBoneId)->GetWorldMatrix());

			//先端ボーンと目標位置の距離が一定以下だと抜ける
			if ((ik.tipBone->GetPosition() - ik.targetPos).LengthSq() < ik.targetSize*ik.targetSize) {
				break;
			}
		}
	}

	bool SkeletonIK::CalcFootIKTarget(IKSetting& ik) {
		//根本からつま先までレイで判定
		//Bone* preBone = nullptr;
		//for(auto bone = ik.GetIsFootIKBoneList().rbegin(), end = ik.GetIsFootIKBoneList().rend(); bone != end; ++bone){
		//	if (preBone) {
		//		btVector3 rayStart = preBone->GetPosition();
		//		btVector3 rayEnd = (*bone)->GetPosition();
		//		btCollisionWorld::ClosestRayResultCallback gnd_ray(rayStart, rayEnd);
		//		GetEngine().GetPhysicsWorld().RayTest(rayStart, rayEnd, gnd_ray);
		//		if (gnd_ray.hasHit()) {
		//			//接触点を目標座標にする
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
			//接触点を目標座標にする
			ik.targetPos.Set(gnd_ray.m_hitPointWorld);
			ik.targetPos -= ik.footIKRayEndOffset;
			return true;
		}

		//接触点なし
		return false;
	}

	void SkeletonIK::IKSetting::InitFootIK() {
		ReleaseFootIK();

		//つま先からリストに登録していく
		//Bone* joint = tipBone;
		//while (1) {
		//	if (joint == nullptr) { 
		//		//nullが出たらエラー
		//		DW_ERRORBOX(true, "SkeletonIK::IKSetting::InitFootIK() :nullptrの出現\nボーンの設定に問題がある");
		//		return;
		//	}
		//	//登録
		//	footIKBoneList.emplace_back(joint);
		//	//根本まで登録したら終わり
		//	if (joint == rootBone) { break; }
		//	//親ボーンへ
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