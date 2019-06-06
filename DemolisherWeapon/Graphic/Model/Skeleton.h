/*!
 *@brief	スケルトン
 */
#pragma once

#include "CoordinateSystemBias.h"
#include "SkeletonIK.h"

namespace DemolisherWeapon {

/*!
*@brief	ボーン。
*/
class Bone {
public:
	/*!
	*@brief	コンストラクタ。
	*@param[in]	boneName		ボーンの名前。
	*@param[in]	bindPose		バインドポーズの行列。
	*@param[in] invBindPose		バインドポーズの逆行列。
	*@param[in]	parentId		親のボーン番号。
	*@param[in] boneId			ボーン番号。
	*/
	Bone(
		const wchar_t* boneName,
		const CMatrix& bindPose,
		const CMatrix& invBindPose,
		int parentId,
		int boneId
	) :
		m_boneName(boneName),
		m_bindPose(bindPose),
		m_invBindPose(invBindPose),
		m_parentId(parentId),
		m_worldMatrix(bindPose),
		m_boneId(boneId)
	{
	}
	/*!
	 *@brief	ローカル行列を設定
	 */
	void SetLocalMatrix(const CMatrix& m)
	{
		//ImNonCalc();
		m_localMatrix = m;
	}
	/*!
	 *@brief	ローカル行列を取得。
	 */
	const CMatrix& GetLocalMatrix() const
	{
		return m_localMatrix;
	}
	/*!
	*@brief	ワールド行列を設定。
	*/
	void SetWorldMatrix(const CMatrix& m)
	{
		ImNonCalc();
		m_worldMatrix = m;
	}
	/*!
	 *@brief	ワールド行列を取得。
	 */
	const CMatrix& GetWorldMatrix() const
	{
		return m_worldMatrix;
	}
	/*!
	*@brief	バインドポーズの行列を取得。
	*/
	const CMatrix& GetBindPoseMatrix() const
	{
		return m_bindPose;
	}
	/*!
	 *@brief	バインドポーズの逆行列を取得。
	 */
	const CMatrix& GetInvBindPoseMatrix() const
	{
		return m_invBindPose;
	}
	/*!
	 *@brief	親のボーン行列を取得。
	 */
	int GetParentId() const
	{
		return m_parentId;
	}
	/*!
	 *@brief	ボーン番号の取得。
	 */
	int GetNo() const
	{
		return m_boneId;
	}

	/*!
	*@brief	子供を追加。
	*/
	void AddChild(Bone* bone) 
	{
		m_children.push_back(bone);
	}
	/*!
	*@brief	子供を取得。
	*/
	std::vector<Bone*>& GetChildren()
	{
		return m_children;
	}

	/// <summary>
	/// 親ボーンのポインタを設定
	/// </summary>
	/// <param name="bone">親ボーンのポインタ</param>
	void SetParentBone(Bone* bone) {
		m_parent = bone;
	}
	/// <summary>
	/// 親ボーンのポインタを取得
	/// </summary>
	/// <returns></returns>
	Bone* GetParentBone() {
		return m_parent;
	}
	
	/*!
	 *@brief	名前の取得。
	 */
	const wchar_t* GetName() const
	{
		return m_boneName.c_str();
	}
	/*!
	*@brief	このボーンのワールド空間での位置と回転とスケールを計算する。
	*@param[out]	trans		平行移動量の格納先。
	*@param[out]	rot			回転量の格納先。
	*@param[out]	scale		拡大率の格納先。
	*/
	void CalcWorldTRS();
	void CalcWorldTRS(CVector3& trans, CQuaternion& rot, CVector3& scale) {
		CalcWorldTRS();
		trans = m_positoin;
		rot = m_rotation;
		scale = m_scale;
	};
	//ワールド空間での位置を取得
	const CVector3& GetPosition(){
		CalcWorldTRS();
		return m_positoin;
	}
	//ワールド空間での回転を取得
	const CQuaternion& GetRotation() {
		CalcWorldTRS();
		return m_rotation;
	}
	//ワールド空間での拡大を取得
	const CVector3& GetScale() {
		CalcWorldTRS();
		return m_scale;
	}

	//座標系をセット
	void SetCoordinateSystem(EnFbxUpAxis fbxUpAxis,	EnFbxCoordinateSystem fbxCoordinate) {
		m_isUseBias = true;
		m_enFbxUpAxis = fbxUpAxis;
		m_enFbxCoordinate = fbxCoordinate;
		CoordinateSystemBias::GetBias(m_reverseBiasRot, m_reverseBiasScr, m_enFbxUpAxis, m_enFbxCoordinate, true);
	}

private:

	void ImNonCalc() { m_isCalced = false; }
	
private:

	std::wstring	m_boneName;
	int				m_parentId = -1;	//!<親のボーン番号。
	int				m_boneId = -1;		//!<ボーン番号。
	CMatrix			m_bindPose;		//!<バインドポーズ。
	CMatrix			m_invBindPose;	//!<バインドポーズの逆行列。
	CMatrix			m_localMatrix = CMatrix::Identity();	//!<ローカル行列。
	CMatrix			m_worldMatrix = CMatrix::Identity();	//!<ワールド行列。
	CVector3		m_positoin = CVector3::Zero();			//!<このボーンのワールド空間での位置。最後にCalcWorldTRSを実行したときの結果が格納されている。
	CVector3		m_scale = CVector3::One();				//!<このボーンの拡大率。最後にCalcWorldTRSを実行したときの結果が格納されている。
	CQuaternion		m_rotation = CQuaternion::Identity();	//!<このボーンの回転。最後にCalcWorldTRSを実行したときの結果が格納されている。
	std::vector<Bone*>	m_children;		//!<子供。
	Bone* m_parent = nullptr;			//親

	bool m_isCalced = false;//CalcWorldTRS済みか?
	bool m_isUseBias = false;
	EnFbxUpAxis			  m_enFbxUpAxis = enFbxUpAxisZ;			//FBXの上方向
	EnFbxCoordinateSystem m_enFbxCoordinate = enFbxRightHanded;	//FBXの座標系
	CMatrix m_reverseBiasScr;
	CMatrix m_reverseBiasRot;
};
/*!
 *@brief	スケルトン。
 */
class Skeleton  {
public:
	using OnPostProcessSkeletonUpdate = std::function<void()>;

	Skeleton();
	~Skeleton();
	
	/*!
	 *@brief	ボーンのローカル行列を設定。
	 *@param[in]	boneNo		ボーン番号
	 *@param[in]	m			行列。
	 */
	void SetBoneLocalMatrix(int boneNo, const CMatrix& m)
	{
		if (boneNo > (int)m_bones.size() - 1) {
			return;
		}
		m_bones[boneNo]->SetLocalMatrix(m);
	}
	/*!
	 *@brief	ボーンの数を取得。
	 */
	int GetNumBones() const
	{
		return static_cast<int>(m_bones.size());
	}
	/*!
	*@brief	ロード。
	*@param[in]	filePath	ファイルパス。
	*/
	bool Load(const wchar_t* filePath);
	/*!
	 *@brief	ボーンの名前からボーンIDを検索。
	 */
	int FindBoneID(const wchar_t* boneName) const
	{
		for (int i = 0; i < (int)m_bones.size(); i++) {
			if (wcscmp(m_bones[i]->GetName(), boneName) == 0) {
				return i;
			}
		}
		//見つからなかった。
		return -1;
	}
	/*!
	*@brief	ボーンを取得。
	*/
	Bone* GetBone(int boneNo)
	{
		return m_bones[boneNo];
	}
	/*!
	*@brief	ボーン行列の配列をGPUに転送。
	*/
	void SendBoneMatrixArrayToGPU();

	/// <summary>
	/// IK実行クラスの取得
	/// </summary>
	/// <returns>IK実行クラスのインスタンス</returns>
	SkeletonIK& GetSkeletonIK() {
		return m_ik;
	}

public:
	/*!
	*@brief	ボーン行列のストラクチャードバッファを初期化。
	*/
	void InitBoneMatrixArrayStructuredBuffer();
	/*!
	*@brief	ボーン行列のシェーダーリソースビューを初期化。
	*/
	void InitBoneMatrixArrayShaderResourceView();
	/*!
	 *@brief	更新。
	 */
	void Update(const CMatrix& mWorld);
	void UpdateBoneMatrixOld();
	/*!
	*@brief	ボーンのワールド行列の更新関数。
	*@details
	* 通常はユーザーがこの関数を呼び出す必要はありません。
	*@param[in]	bone		更新するボーン。
	*@param[in]	parentMatrix	親のボーンのワールド行列。
	*/
	static 	void UpdateBoneWorldMatrix(Bone& bone, const CMatrix& parentMatrix);

	static const int MAX_BONE = 512;	//!<ボーンの最大数。

private:
	std::vector<Bone*>			m_bones;					//!<ボーンの配列。

	std::vector<CMatrix>		m_boneMatrixs;				//!<ボーン行列。
	ID3D11Buffer*				m_boneMatrixSB = nullptr;	//!<ボーン行列のストラクチャーバッファ。
	ID3D11ShaderResourceView*	m_boneMatrixSRV = nullptr;	//!<ボーン行列のSRV。

	//前フレームのやつ
	std::vector<CMatrix>		m_boneMatrixs_Old;				//!<ボーン行列。
	ID3D11Buffer*				m_boneMatrixSB_Old = nullptr;	//!<ボーン行列のストラクチャーバッファ。
	ID3D11ShaderResourceView*	m_boneMatrixSRV_Old = nullptr;	//!<ボーン行列のSRV。

	SkeletonIK m_ik;//IK実行クラス
};

}