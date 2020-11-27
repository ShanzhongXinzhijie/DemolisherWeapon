/*!
 *@brief	スケルトン
 */
#include "DWstdafx.h"
#include "skeleton.h"
#include "SkinModelShaderConst.h"
#include <comdef.h> 

namespace DemolisherWeapon {

void Bone::CalcWorldTRS()
{
	if (!m_isCalced) {
		CMatrix mWorld = m_worldMatrix;		

		//バイアスを消す
		if (m_isUseBias) {
			//左手座標系に変換
			if (m_enFbxCoordinate == enFbxRightHanded) {
				mWorld.m[2][0] *= -1.0f;
				mWorld.m[2][1] *= -1.0f;
				mWorld.m[2][2] *= -1.0f;
				mWorld.m[2][3] *= -1.0f;
			}
		}

		//拡大のバイアス解除
		if (m_isUseBias) {
			//mWorld.Mul(m_reverseBiasScr, mWorld);
		}

		//行列から拡大率を取得する。
		m_scale.x = mWorld.v[0].Length();
		m_scale.y = mWorld.v[1].Length();
		m_scale.z = mWorld.v[2].Length();

		//行列から平行移動量を取得する。
		m_positoin.Set(mWorld.v[3]);

		//行列から拡大率と平行移動量を除去して回転量を取得する。
		mWorld.v[0].Normalize();
		mWorld.v[1].Normalize();
		mWorld.v[2].Normalize();
		mWorld.v[3].Set(0.0f, 0.0f, 0.0f, 1.0f);
		
		//回転のバイアス解除
		if (m_isUseBias) {
			mWorld.Mul(m_reverseBiasRot, mWorld);
		}

		m_rotation.SetRotation(mWorld);
		
		m_isCalced = true;
	}
}
Skeleton::Skeleton()
{
	//リザーブ。
	m_bones.reserve(MAX_BONE);
}
Skeleton::~Skeleton()
{
	//ちゃんとメモリは解放しましょう。
	for (int boneNo = 0; boneNo < m_bones.size(); boneNo++) {
		delete m_bones[boneNo];
	}
}
void Skeleton::UpdateBoneWorldMatrix(Bone& bone, const CMatrix& parentMatrix)
{
	//ワールド行列を計算する。
	CMatrix mBoneWorld;
	CMatrix localMatrix = bone.GetLocalMatrix();
	//親の行列とローカル行列を乗算して、ワールド行列を計算する。
	mBoneWorld.Mul(localMatrix, parentMatrix);
	bone.SetWorldMatrix(mBoneWorld);

	//子供のワールド行列も計算する。
	std::vector<Bone*>& children = bone.GetChildren();
	for (int childNo = 0; childNo < children.size(); childNo++ ) {
		//この骨のワールド行列をUpdateBoneWorldMatrixに渡して、さらに子供のワールド行列を計算する。
		UpdateBoneWorldMatrix(*children[childNo], mBoneWorld);
	}
}
bool Skeleton::Load(const wchar_t* filePath)
{
	//tksファイルをオープン。
	FILE* fp;
	if (_wfopen_s(&fp, filePath, L"rb") != 0) {
		return false;
	}

	//骨の数を取得。
	int numBone = 0;
	//tksファイルの先頭から、4バイト読み込む。
	//先頭４バイトに骨の数のデータが入っている。
	fread(&numBone, sizeof(numBone), 1, fp);
	//骨を一本一本読み込んでいく。
	for (int boneNo = 0; boneNo < numBone; boneNo++) {
		int nameCount = 0;
		//骨の名前の文字数を読み込む。
		fread(&nameCount, 1, 1, fp);
		//骨の名前を読み込めるだけのメモリを確保する。
		char* name = new char[nameCount + 1];
		//骨の名前を読み込み。+1は終端文字列を読み込むため
		fread(name, nameCount+1, 1, fp);
		//親の骨番号を取得。
		int parentNo;
		fread(&parentNo, sizeof(parentNo),  1, fp);
		//バインドポーズを取得。
		CVector3 bindPose[4];
		fread(&bindPose, sizeof(bindPose), 1, fp);
		//バインドポーズの逆行列を取得。
		CVector3 invBindPose[4];
		fread(&invBindPose, sizeof(invBindPose), 1, fp);
		
		//バインドポーズを表す行列を作成する。
		CMatrix bindPoseMatrix;
		memcpy(bindPoseMatrix.m[0], &bindPose[0], sizeof(bindPose[0]));
		memcpy(bindPoseMatrix.m[1], &bindPose[1], sizeof(bindPose[1]));
		memcpy(bindPoseMatrix.m[2], &bindPose[2], sizeof(bindPose[2]));
		memcpy(bindPoseMatrix.m[3], &bindPose[3], sizeof(bindPose[3]));
		bindPoseMatrix.m[0][3] = 0.0f;
		bindPoseMatrix.m[1][3] = 0.0f;
		bindPoseMatrix.m[2][3] = 0.0f;
		bindPoseMatrix.m[3][3] = 1.0f;

		//バインドポーズの逆行列。
		CMatrix invBindPoseMatrix;
		memcpy(invBindPoseMatrix.m[0], &invBindPose[0], sizeof(invBindPose[0]));
		memcpy(invBindPoseMatrix.m[1], &invBindPose[1], sizeof(invBindPose[1]));
		memcpy(invBindPoseMatrix.m[2], &invBindPose[2], sizeof(invBindPose[2]));
		memcpy(invBindPoseMatrix.m[3], &invBindPose[3], sizeof(invBindPose[3]));
		invBindPoseMatrix.m[0][3] = 0.0f;
		invBindPoseMatrix.m[1][3] = 0.0f;
		invBindPoseMatrix.m[2][3] = 0.0f;
		invBindPoseMatrix.m[3][3] = 1.0f;
		
		//マルチバイト文字列をワイド文字列に変換する。
		wchar_t boneName[256]; size_t rval = 0;
		mbstowcs_s(&rval, boneName, 256, name, nameCount + 1);
		//tksファイルからロードしたボーン情報を使って、新しいボーンを作成。
		Bone* bone = new Bone(
			boneName,			//ボーンの名前。
			bindPoseMatrix,		//バインドポーズを表す行列。
			invBindPoseMatrix,	//バインドポーズを表す行列の逆行列。
			parentNo,			//親の番号。
			boneNo				//ボーン番号。
		);
		//ちゃんと不要になったら削除。
		delete[] name;
		//ファイルから読み込んで、作成した骨の情報をリストにプッシュバック。
		m_bones.push_back(bone);
	}

	//ファイルは開いたら、ちゃんと閉じる。。
	fclose(fp);

	//スケルトンの読み込みが完了したので、親の座標系での行列を求める。
	for (int boneNo = 0; boneNo < m_bones.size(); boneNo++)  {
		Bone* bone = m_bones[boneNo];
		if (bone->GetParentId() != -1) {
			//親がいる。
			m_bones.at(bone->GetParentId())->AddChild(bone);
			//親ボーンのポインタを登録
			bone->SetParentBone(m_bones.at(bone->GetParentId()));
			//親の逆行列を取得する。h
			const CMatrix& parentMatrix = m_bones.at(bone->GetParentId())->GetInvBindPoseMatrix();
			//骨のバインドポーズの行列に、親の逆行列を乗算して、親の座標系での行列を求める。
			CMatrix localMatrix;
			localMatrix.Mul(bone->GetBindPoseMatrix(), parentMatrix);
			bone->SetLocalMatrix(localMatrix);
		}
		else {
			//親がいない。
			bone->SetLocalMatrix(bone->GetBindPoseMatrix());
		}
	}

	//ボーン行列のストラクチャードバッファを初期化。
	InitBoneMatrixArrayStructuredBuffer();

	return true;
}

void Skeleton::InitBoneMatrixArrayStructuredBuffer()
{
	//初期化
	m_boneMatrixs.Init((int)m_bones.size());
	m_boneMatrixs_Old.Init((int)m_bones.size());
}
void Skeleton::Update(const CMatrix& mWorld)
{
	if (m_bones.empty()) { return; }

	//ここがワールド行列を計算しているところ！！！
	for (int boneNo = 0; boneNo < m_bones.size(); boneNo++) {
		Bone* bone = m_bones[boneNo];
		if (bone->GetParentId() != -1) {
			continue;
		}
		//ルートが見つかったので、ボーンのワールド行列を計算していく。
		UpdateBoneWorldMatrix(*bone, mWorld);
	}

	//IKを実行
	m_ik.Update();

	//ボーン行列を計算
	std::unique_ptr<CMatrix[]>& boneDatas = m_boneMatrixs.GetData();
	for (int boneNo = 0; boneNo < m_bones.size(); boneNo++) {
		Bone* bone = m_bones[boneNo];
		CMatrix mBone;
		//ワールド行列にバインドポーズの逆行列をかけたものがボーン行列？？？
		mBone.Mul(bone->GetInvBindPoseMatrix(), bone->GetWorldMatrix());
		boneDatas[boneNo] = mBone;
	}
}
void Skeleton::UpdateBoneMatrixOld() 
{
	if (m_bones.empty()) { return; }

	std::unique_ptr<CMatrix[]>& boneDatas = m_boneMatrixs.GetData();
	std::unique_ptr<CMatrix[]>& boneOldDatas = m_boneMatrixs_Old.GetData();
	for (int boneNo = 0; boneNo < m_bones.size(); boneNo++) {
		boneOldDatas[boneNo] = boneDatas[boneNo];
	}
}
/*!
*@brief	ボーン行列の配列をGPUに転送。
*/
void Skeleton::SendBoneMatrixArrayToGPU()
{
	if (m_bones.empty()) { return; }

	//StructuredBufferを更新
	m_boneMatrixs.UpdateSubresource();
	m_boneMatrixs_Old.UpdateSubresource();

	//ボーン行列を頂点シェーダーステージに設定。
	GetGraphicsEngine().GetD3DDeviceContext()->VSSetShaderResources(enSkinModelSRVReg_BoneMatrixArray, 1, m_boneMatrixs.GetAddressOfSRV());
	GetGraphicsEngine().GetD3DDeviceContext()->VSSetShaderResources(enSkinModelSRVReg_BoneMatrixArrayOld, 1, m_boneMatrixs_Old.GetAddressOfSRV());
}

}