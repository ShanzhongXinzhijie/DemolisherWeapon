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
	

	
void AnimationPlayController::StartLoop()
{
	m_currentKeyFrameNo = 0;
	m_time = 0.0f;
}
void AnimationPlayController::Update(float deltaTime, Animation* animation)
{
	if(m_animationClip == nullptr){
		
		return ;
	}
	const auto& topBoneKeyFrameList = m_animationClip->GetTopBoneKeyFrameList();
	m_time += deltaTime;

	//補完時間も進めていく。
	m_interpolateTime = min(m_interpolateEndTime, m_interpolateTime + deltaTime);
	while (true) {
		if (m_currentKeyFrameNo >= (int)topBoneKeyFrameList.size()) {
			//終端まで行った。
			if (m_animationClip->IsLoop()) {
				//ループ。
				StartLoop();
			}
			else {
				//ワンショット再生。
				m_currentKeyFrameNo--;
				m_isPlaying = false;	//再生終わり。
			}
			break;
		}
		if (topBoneKeyFrameList.at(m_currentKeyFrameNo)->time >= m_time) {
			//終わり。
			break;
		}
		//次へ。
		m_currentKeyFrameNo++;
	}
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
		}
		else {
#ifdef _DEBUG			
			MessageBox(NULL, "AnimationPlayController::Update : 存在しないボーンに値を書き込もうとしています。次のような原因が考えられます。\n"
				"① tkaファイルを出力する時に、選択したルートボーンがスケルトンのルートボーンと異なっている。\n"
				"② 異なるスケルトンのアニメーションクリップを使っている。\n"
				"もう一度tkaファイルを出力しなおしてください。", "error", MB_OK);
			std::abort();
#endif
		}
	}
}

}