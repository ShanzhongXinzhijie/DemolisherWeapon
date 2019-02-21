/*!
 *@brief	アニメーションクリップ。
 */

#include "DWstdafx.h"
#include "AnimationClip.h"
#include "Graphic/Model/skeleton.h"

#include <filesystem>

namespace DemolisherWeapon {

bool AnimationClipDataManager::Load(AnimationClipData*& returnACD, const wchar_t* filePath, EnChangeAnimationClipUpAxis changeUpAxis) {
	//キーを作成
	int filenameHash, paramHash;
	filenameHash = Util::MakeHash(filePath);

	int param = changeUpAxis;
	wchar_t str[6]; swprintf_s(str, L"%d", param);
	paramHash = Util::MakeHash(str);

	std::pair<int, int> key = std::make_pair(filenameHash, paramHash);

	//既に登録されてないか?
	if (m_animationClipDataMap.count(key) > 0) {
		//登録されてたらマップから取得
		returnACD = m_animationClipDataMap[key];
		return false;
	}
	else {
		//新規読み込み
		m_animationClipDataMap[key] = new AnimationClipData();
		returnACD = m_animationClipDataMap[key];
		return true;
	}
}

AnimationClipDataManager AnimationClip::m_s_animationClipDataManager;

AnimationClip::~AnimationClip()
{
	//キーフレームを破棄。
	/*for (auto& keyFrame : m_keyframes) {
		delete keyFrame;
	}*/
}

void AnimationClip::Load(const wchar_t* filePath, bool loop, EnChangeAnimationClipUpAxis changeUpAxis)
{	
	//マネージャーからデータ読み込み
	if (m_s_animationClipDataManager.Load(m_animationClipData, filePath, changeUpAxis) == false) {
		//新規作成でない

		//ループ設定
		SetLoopFlag(loop);

		return;
	}

	//新規作成なら初期化する
	m_animationClipData->m_changeUpAxis = changeUpAxis;

	FILE* fp;
	if (_wfopen_s(&fp, filePath, L"rb") != 0) {
#ifndef DW_MASTER
		//ファイルが開けなかったときの処理。
		char message[256];
		sprintf_s(message, "AnimationClipのオープンに失敗。\nファイルパスあってますか?\n%ls\n", filePath);
		MessageBox(NULL, message, "Error", MB_OK);
		//止める。
		std::abort();
#endif
		m_animationClipData = nullptr;
		return;
	}

	//クリップ名記録
	std::experimental::filesystem::path ps = filePath;
	m_animationClipData->m_clipName = ps.stem();

	//ファイルパス記録
	m_animationClipData->m_clipPass = filePath;
	
	//アニメーションクリップのヘッダーをロード。
	AnimClipHeader header;
	fread(&header, sizeof(header), 1, fp);
		
	//アニメーションイベント
	if (header.numAnimationEvent > 0) {
		//ソート用
		std::vector<std::pair<float, AnimationEvent>> sortEvent;

		m_animationClipData->m_animationEvent = std::make_unique<AnimationEvent[]>(header.numAnimationEvent);
		//アニメーションイベントがあるなら、イベント情報をロードする。
		for (auto i = 0; i < (int)header.numAnimationEvent; i++) {
			AnimationEventData animEvent;
			fread(&animEvent, sizeof(animEvent), 1, fp);
			//イベント名をロードする。
			static char eventName[256];
			static wchar_t wEventName[256];
			fread(eventName, animEvent.eventNameLength + 1, 1, fp);
			
			//mbstowcs(wEventName, eventName, 255);
			//マルチバイト文字列をワイド文字列に変換する。
			size_t rval = 0;
			mbstowcs_s(&rval, wEventName, 256, eventName, animEvent.eventNameLength + 1);

			m_animationClipData->m_animationEvent[i].SetInvokeTime(animEvent.invokeTime);
			m_animationClipData->m_animationEvent[i].SetEventName(wEventName);

			//ソート用配列作る
			sortEvent.push_back(std::make_pair(animEvent.invokeTime, m_animationClipData->m_animationEvent[i]));
		}

		//一応、昇順にソート
		std::sort(sortEvent.begin(), sortEvent.end(), [](const std::pair<float, AnimationEvent> &a, const std::pair<float, AnimationEvent> &b) {
			return a.first < b.first;
		});
		for (auto i = 0; i < (int)header.numAnimationEvent; i++) {
			m_animationClipData->m_animationEvent[i] = sortEvent[i].second;
		}
	}
	m_animationClipData->m_numAnimationEvent = header.numAnimationEvent;

	//中身コピーするためのメモリをドカッと確保。
	KeyframeRow* keyframes = new KeyframeRow[header.numKey];
	//キーフレームをドカッと読み込み。
	fread(keyframes, sizeof(KeyframeRow), header.numKey, fp);
	//もうデータのロードはすべて終わったので、ファイルは閉じる。
	fclose(fp);
	//tkaファイルのキーフレームのローカル業レは4x3行列なので
	//ゲームで使用しやすいように、4x4行列に変換していく。
	for (int i = 0; i < (int)header.numKey; i++) {
		//ゲームで使用するKeyframeのインスタンスを生成。
		Keyframe* keyframe = new Keyframe;
		//ボーン番号とか再生時間とかをコピーしていく。
		keyframe->boneIndex = keyframes[i].boneIndex;
		keyframe->transform = CMatrix::Identity();
		keyframe->time = keyframes[i].time;
		//行列はコピー。
		for (int j = 0; j < 4; j++) {
			keyframe->transform.m[j][0] = keyframes[i].transform[j].x;
			keyframe->transform.m[j][1] = keyframes[i].transform[j].y;
			keyframe->transform.m[j][2] = keyframes[i].transform[j].z;
		}

		//ルートボーンに軸バイアスを掛ける
		if (m_animationClipData->m_changeUpAxis != enNonChange && keyframe->boneIndex == 0) {
			CMatrix mBias = CMatrix::Identity();
			if (m_animationClipData->m_changeUpAxis == enZtoY) {
				//Z to Y-up
				mBias.MakeRotationX(CMath::PI * -0.5f);
			}
			if (m_animationClipData->m_changeUpAxis == enYtoZ) {
				//Y to Z-up
				mBias.MakeRotationX(CMath::PI * 0.5f);
			}
			keyframe->transform.Mul(mBias, keyframe->transform);
		}

		//新しく作ったキーフレームを可変長配列に追加。
		m_animationClipData->m_keyframes.push_back(keyframe);
	}

	//キーフレームは全部コピー終わったので、ファイルから読み込んだ分は破棄する。
	delete[] keyframes;

	//ボーン番号ごとにキーフレームを振り分けていく。
	m_animationClipData->m_keyFramePtrListArray.resize(Skeleton::MAX_BONE);
	for (Keyframe* keyframe : m_animationClipData->m_keyframes) {
		m_animationClipData->m_keyFramePtrListArray[keyframe->boneIndex].push_back(keyframe);
		if (m_animationClipData->m_topBoneKeyFramList == nullptr) {
			m_animationClipData->m_topBoneKeyFramList = &m_animationClipData->m_keyFramePtrListArray[keyframe->boneIndex];
		}
	}

	//ループ設定
	SetLoopFlag(loop);
}

}