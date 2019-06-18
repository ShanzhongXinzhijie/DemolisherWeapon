/*!
 * @brief	アニメーション再生コントローラ。
 */

#include "DWstdafx.h"
#include "AnimationPlayController.h"
#include "Animation.h"
#include "AnimationClip.h"
#include "Graphic/Model/skeleton.h"


namespace DemolisherWeapon {
	
void AnimationPlayController::Init(Skeleton* skeleton)
{
	int numBones = skeleton->GetNumBones();
	//ボーン行列をバシッと確保。
	m_boneMatrix.resize(numBones);
}
	
void AnimationPlayController::InvokeAnimationEvent(Animation* animation, bool isPlayEvent)
{
	auto& animEventArray = m_animationClip->GetAnimationEvent();
	for (auto i = 0; i < m_animationClip->GetNumAnimationEvent(); i++) {
		//アニメーションの起動時間を過ぎている且つ、まだイベント起動していない。
		if (m_time > animEventArray[i].GetInvokeTime() && m_animEventedCnt <= i){//animEventArray[i].IsInvoked() == false) {
			m_animEventedCnt++;	//animEventArray[i].SetInvokedFlag(true);
			if (isPlayEvent) {
				animation->NotifyAnimationEventToListener(
					m_animationClip->GetName(), animEventArray[i].GetEventName()
				);
			}
		}
	}
}
	
void AnimationPlayController::StartLoop()
{
	m_currentKeyFrameNo = 0;
	m_time = 0.0f;

	m_animEventedCnt = 0;
	//アニメーションイベントをすべて未発生にする。
	/*auto& animEventArray = m_animationClip->GetAnimationEvent();
	for (auto i = 0; i < m_animationClip->GetNumAnimationEvent(); i++) {
		animEventArray[i].SetInvokedFlag(false);
	}*/
}
void AnimationPlayController::Update(float deltaTime, Animation* animation, bool isPlayAnimEvent)
{
	if(m_animationClip == nullptr){		
		return ;
	}
	const auto& topBoneKeyFrameList = m_animationClip->GetTopBoneKeyFrameList();
	m_time += deltaTime;

	//アニメーションイベントの発生。
	InvokeAnimationEvent(animation, isPlayAnimEvent);

	//補完時間も進めていく。
	m_interpolateTime = min(m_interpolateEndTime, m_interpolateTime + deltaTime);
	while (true) {
		if (m_currentKeyFrameNo >= (int)topBoneKeyFrameList.size()) {
			//終端まで行った。
			if (m_animationClip->IsLoop()) {
				float amariTime = m_time - topBoneKeyFrameList.back()->time;

				//ループ。
				StartLoop();

				m_time = amariTime;
				InvokeAnimationEvent(animation, isPlayAnimEvent);
			}
			else {
				//ワンショット再生。
				m_currentKeyFrameNo--;
				m_isPlaying = false;	//再生終わり。
				break;
			}
		}
		if (topBoneKeyFrameList.at(m_currentKeyFrameNo)->time >= m_time) {
			//終わり。
			break;
		}
		//次へ。
		m_currentKeyFrameNo++;
	}

	if (!isPlayAnimEvent) { return; }

	//ボーン行列を計算していく。
	const auto& keyFramePtrListArray = m_animationClip->GetKeyFramePtrListArray();
	for (const auto& keyFrameList : keyFramePtrListArray) {
		if (keyFrameList.size() == 0) {
			continue;
		}
		//現在再生中のキーフレームを取ってくる。
		Keyframe* keyframe = keyFrameList.at(m_currentKeyFrameNo);
		if (keyframe->boneIndex < m_boneMatrix.size()) {
			m_boneMatrix[keyframe->boneIndex] = keyframe->transform;

			//前のキーフレームとの補完
			if (m_currentKeyFrameNo > 0 && m_time < keyframe->time) {
				Keyframe* prevKeyframe = keyFrameList.at(m_currentKeyFrameNo - 1);
				CMatrix prevKeyFrameBoneMatrix;
				prevKeyFrameBoneMatrix = prevKeyframe->transform;

				//ブレンド
				float blend = (m_time - prevKeyframe->time) / (keyframe->time - prevKeyframe->time);

				CMatrix& m = m_boneMatrix[keyframe->boneIndex];
				//平行移動の補完
				CVector3 move;
				move.Lerp(
					blend,
					*(CVector3*)prevKeyFrameBoneMatrix.m[3],
					*(CVector3*)m.m[3]
				);
				//平行移動成分を削除。
				m.m[3][0] = 0.0f;
				m.m[3][1] = 0.0f;
				m.m[3][2] = 0.0f;
				prevKeyFrameBoneMatrix.m[3][0] = 0.0f;
				prevKeyFrameBoneMatrix.m[3][1] = 0.0f;
				prevKeyFrameBoneMatrix.m[3][2] = 0.0f;

				//拡大成分の補間。
				CVector3 vBoneScale, vBoneScalePrev;
				vBoneScale.x = (*(CVector3*)m.m[0]).Length();
				vBoneScale.y = (*(CVector3*)m.m[1]).Length();
				vBoneScale.z = (*(CVector3*)m.m[2]).Length();
				vBoneScalePrev.x = (*(CVector3*)prevKeyFrameBoneMatrix.m[0]).Length();
				vBoneScalePrev.y = (*(CVector3*)prevKeyFrameBoneMatrix.m[1]).Length();
				vBoneScalePrev.z = (*(CVector3*)prevKeyFrameBoneMatrix.m[2]).Length();
				CVector3 scale;
				scale.Lerp(
					blend,
					vBoneScalePrev,
					vBoneScale
				);
				//拡大成分を除去。
				m.m[0][0] /= vBoneScale.x;
				m.m[0][1] /= vBoneScale.x;
				m.m[0][2] /= vBoneScale.x;
				m.m[1][0] /= vBoneScale.y;
				m.m[1][1] /= vBoneScale.y;
				m.m[1][2] /= vBoneScale.y;
				m.m[2][0] /= vBoneScale.z;
				m.m[2][1] /= vBoneScale.z;
				m.m[2][2] /= vBoneScale.z;
				prevKeyFrameBoneMatrix.m[0][0] /= vBoneScalePrev.x;
				prevKeyFrameBoneMatrix.m[0][1] /= vBoneScalePrev.x;
				prevKeyFrameBoneMatrix.m[0][2] /= vBoneScalePrev.x;
				prevKeyFrameBoneMatrix.m[1][0] /= vBoneScalePrev.y;
				prevKeyFrameBoneMatrix.m[1][1] /= vBoneScalePrev.y;
				prevKeyFrameBoneMatrix.m[1][2] /= vBoneScalePrev.y;
				prevKeyFrameBoneMatrix.m[2][0] /= vBoneScalePrev.z;
				prevKeyFrameBoneMatrix.m[2][1] /= vBoneScalePrev.z;
				prevKeyFrameBoneMatrix.m[2][2] /= vBoneScalePrev.z;

				//回転の補完
				CQuaternion qBone, qBonePrev;
				qBone.SetRotation(m);
				qBonePrev.SetRotation(prevKeyFrameBoneMatrix);
				CQuaternion rot;
				rot.Slerp(blend, qBonePrev, qBone);

				//拡大行列を作成。
				CMatrix scaleMatrix;
				scaleMatrix.MakeScaling(scale);
				//回転行列を作成。
				CMatrix rotMatrix;
				rotMatrix.MakeRotationFromQuaternion(rot);
				//全部を合成して、ボーン行列を作成。
				m_boneMatrix[keyframe->boneIndex].Mul(scaleMatrix, rotMatrix);
				m_boneMatrix[keyframe->boneIndex].SetTranslation(move);
			}
		}
#ifndef DW_MASTER	
		else {		
			MessageBox(NULL, "AnimationPlayController::Update : 存在しないボーンに値を書き込もうとしています。次のような原因が考えられます。\n"
				"① tkaファイルを出力する時に、選択したルートボーンがスケルトンのルートボーンと異なっている。\n"
				"② 異なるスケルトンのアニメーションクリップを使っている。\n"
				"もう一度tkaファイルを出力しなおしてください。", "error", MB_OK);
			std::abort();
		}
#endif
	}
}

}