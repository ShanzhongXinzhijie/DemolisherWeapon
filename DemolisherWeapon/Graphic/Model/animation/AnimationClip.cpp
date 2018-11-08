/*!
 *@brief	アニメーションクリップ。
 */

#include "DWstdafx.h"
#include "AnimationClip.h"
#include "Graphic/Model/skeleton.h"


namespace DemolisherWeapon {

AnimationClip::~AnimationClip()
{
	//キーフレームを破棄。
	for (auto& keyFrame : m_keyframes) {
		delete keyFrame;
	}
}

void AnimationClip::Load(const wchar_t* filePath, bool loop, EnChangeAnimationClipUpAxis changeUpAxis)
{
	m_changeUpAxis = changeUpAxis;

	FILE* fp;
	if (_wfopen_s(&fp, filePath, L"rb") != 0) {
#ifdef _DEBUG
		//ファイルが開けなかったときの処理。
		char message[256];
		sprintf_s(message, "AnimationClip::Load, ファイルのオープンに失敗しました。%ls\n", filePath);
		MessageBox(NULL, message, "Error", MB_OK);
		//止める。
		std::abort();
#endif
		return;
	}
	
	//アニメーションクリップのヘッダーをロード。
	AnimClipHeader header;
	fread(&header, sizeof(header), 1, fp);
		
	if (header.numAnimationEvent > 0) {
		//アニメーションイベントは未対応。
		//就職作品でチャレンジしてみよう。
		std::abort();
	}


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
		if (m_changeUpAxis != enNonChange && keyframe->boneIndex == 0) {
			CMatrix mBias = CMatrix::Identity();
			if (m_changeUpAxis == enZtoY) {
				//Z to Y-up
				mBias.MakeRotationX(CMath::PI * 0.5f);
			}
			if (m_changeUpAxis == enYtoZ) {
				//Y to Z-up
				mBias.MakeRotationX(CMath::PI * -0.5f);
			}
			keyframe->transform.Mul(mBias, keyframe->transform);
		}

		//新しく作ったキーフレームを可変長配列に追加。
		m_keyframes.push_back(keyframe);
	}

	//キーフレームは全部コピー終わったので、ファイルから読み込んだ分は破棄する。
	delete[] keyframes;

	//ボーン番号ごとにキーフレームを振り分けていく。
	m_keyFramePtrListArray.resize(Skeleton::MAX_BONE);
	for (Keyframe* keyframe : m_keyframes) {
		m_keyFramePtrListArray[keyframe->boneIndex].push_back(keyframe);
		if (m_topBoneKeyFramList == nullptr) {
			m_topBoneKeyFramList = &m_keyFramePtrListArray[keyframe->boneIndex];
		}
	}

	//ループ設定
	SetLoopFlag(loop);
}

}