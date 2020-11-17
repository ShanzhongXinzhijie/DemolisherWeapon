#pragma once

#include "Skeleton.h"
#include "SkinModelDataManager.h"
#include "SkinModelEffect.h"
#include "Util/Util.h"
#include "Render/MotionBlurRender.h"

namespace DemolisherWeapon {

class CModel;
struct SModelMesh;

/*!
*@brief	�X�L�����f���N���X�B
*/
class SkinModel
{
public:
	//���b�V�������������Ƃ��̃R�[���o�b�N�֐��B
	using OnFindMesh = std::function<void(const std::unique_ptr<DirectX::ModelMeshPart>&)>;
	using OnFindMaterial = std::function<void(ModelEffect*)>;

	/*!
	*@brief	�f�X�g���N�^�B
	*/
	~SkinModel();
	
	/// <summary>
	/// ������
	/// </summary>
	/// <param name="filePath">���[�h����cmo�܂���tkm�t�@�C���̃t�@�C���p�X</param>
	/// <param name="enFbxUpAxis">fbx�̏㎲�B�f�t�H���g��enFbxUpAxisZ</param>
	/// <param name="enFbxCoordinate">fbx�̍��W�n�B�f�t�H���g��enFbxRightHanded</param>
	/// <param name="isUseFlyweightFactory">���f���̃��[�h��FlyweightFactory���g�p���邩</param>
	void Init(std::filesystem::path filePath, EnFbxUpAxis enFbxUpAxis = enFbxUpAxisZ, EnFbxCoordinateSystem enFbxCoordinate = enFbxRightHanded, bool isUseFlyweightFactory = true);
	
	/*!
	*@brief	���f�������[���h���W�n�ɕϊ����邽�߂̃��[���h�s����X�V����B
			���X�P���g�����X�V�����
	*@param[in]	position		���f���̍��W�B
	*@param[in]	rotation		���f���̉�]�B
	*@param[in]	scale			���f���̊g�嗦�B
	*@param[in]	RefreshOldPos	���s������t���b�V�����邩
	*/
	void UpdateWorldMatrix(const CVector3& position, const CQuaternion& rotation, const CVector3& scale, bool RefreshOldPos = false);
	//���s�ړ������̂ݍX�V
	void UpdateWorldMatrixTranslation(const CVector3& position, bool RefreshOldPos = false);

	/// <summary>
	/// ���[���h�s���ݒ�
	/// �����ĐF�X�X�V�����
	/// </summary>
	/// <param name="worldMatrix">�ݒ肷�郏�[���h�s��</param>
	/// <param name="RefreshOldPos">���s������t���b�V�����邩</param>
	void SetWorldMatrix(const CMatrix& worldMatrix, bool RefreshOldPos = false);

	//���[���h�s����v�Z����
	void CalcWorldMatrix(const CVector3& position, const CQuaternion& rotation, const CVector3& scale, CMatrix& returnWorldMatrix, CMatrix& returnSRTMatrix)const;
	void CalcWorldMatrix(const CVector3& position, const CQuaternion& rotation, const CVector3& scale, CMatrix& returnWorldMatrix)const {
		CalcWorldMatrix(position, rotation, scale, returnWorldMatrix, returnWorldMatrix);
	}

	//(�g��~��]�~���s�ړ�)�s����v�Z����
	void CalcSRTMatrix(const CVector3& position, const CQuaternion& rotation, const CVector3& scale, CMatrix& returnWorldMatrix)const;

	//���s��̋L�^
	void UpdateOldMatrix() {
		//�O��̃��[���h�s����L�^
		m_worldMatrixOld = m_worldMatrix;
		//�{�[��
		m_skeleton.UpdateBoneMatrixOld();
	}

	/// <summary>
	/// ���[���h�s��̎擾
	/// </summary>
	[[nodiscard]]
	const CMatrix& GetWorldMatrix()const {
		return m_worldMatrix;
	}

	/// <summary>
	/// �o�E���f�B���O�{�b�N�X���v�Z(���[���h�s���K��)
	/// </summary>
	/// <param name="worldMatrix">���[���h�s��</param>
	/// <param name="return_aabbMin">�o�E���f�B���O�{�b�N�X�̍ŏ��l�����Ԃ��Ă���</param>
	/// <param name="return_aabbMax">�o�E���f�B���O�{�b�N�X�̍ő�l�����Ԃ��Ă���</param>
	void CalcBoundingBoxWithWorldMatrix(const CMatrix& worldMatrix, CVector3& return_aabbMin, CVector3& return_aabbMax);

	/*!
	*@brief	�{�[���������B
	*@param[in]		boneName	�{�[���̖��O�B
	*@return	���������{�[���B������Ȃ������ꍇ��nullptr��Ԃ��܂��B
	*/
	[[nodiscard]]
	Bone* FindBone(const wchar_t* boneName)
	{
		int boneId = m_skeleton.FindBoneID(boneName);
		return m_skeleton.GetBone(boneId);
	}

	/*!
	*@brief	���f����`��B
	*@param[in]	reverseCull  �ʂ𔽓]���邩
	*@param[in]	instanceNum  �C���X�^���X��
	*/
	void Draw(bool reverseCull = false, int instanceNum = 1, ID3D11BlendState* pBlendState = nullptr, ID3D11DepthStencilState* pDepthStencilState = nullptr);

	//���f���̖��O���擾
	[[nodiscard]]
	const wchar_t* GetModelName()const
	{
		return m_modelName.c_str();
	}
	//���f���̖��O�̈�v�𔻒�
	[[nodiscard]]
	bool EqualModelName(const wchar_t* name) const
	{
		return wcscmp(name, m_modelName.c_str()) == 0;
	}

	/*!
	*@brief	�X�P���g���̎擾�B
	*/
	[[nodiscard]]
	Skeleton& GetSkeleton()
	{
		return m_skeleton;
	}

	/// <summary>
	/// ���b�V������������B(DirectX::Model��)
	/// </summary>
	/// <param name="onFindMesh">���b�V�������������Ƃ��̃R�[���o�b�N�֐�</param>
	void FindMesh(OnFindMesh onFindMesh) const
	{
#ifndef DW_DX12_TEMPORARY
		if (!m_modelDx) {
			DW_WARNING_BOX(true, "FindMesh:m_modelDx��NULL")
			return;
		}
		for (auto& modelMeshs : m_modelDx->meshes) {
			for (std::unique_ptr<DirectX::ModelMeshPart>& mesh : modelMeshs->meshParts) {
				onFindMesh(mesh);
			}
		}
#endif
	}
	/// <summary>
	/// ���b�V������������B(CModel��)
	/// </summary>
	/// <param name="onFindMesh">���b�V�������������Ƃ��̃R�[���o�b�N�֐�</param>
	void FindMeshCModel(std::function<void(const std::unique_ptr<SModelMesh>&)> onFindMesh) const;

	/// <summary>
	/// ���b�V���̏W������������B
	/// </summary>
	/// <param name="onFindMeshes">���b�V���̏W�������������Ƃ��̃R�[���o�b�N�֐�</param>
	void FindMeshes(std::function<void(const std::shared_ptr<DirectX::ModelMesh>&)> onFindMeshes)const 
	{
		if (!m_modelDx) {
			DW_WARNING_BOX(true, "FindMeshes:m_modelDx��NULL")
			return;
		}
		for (auto& modelMeshs : m_modelDx->meshes) {
			onFindMeshes(modelMeshs);
		}
	}

	/// <summary>
	/// �}�e���A������������B(DirectX::Model��)
	/// </summary>
	/// <param name="onFindMaterial">�}�e���A�������������Ƃ��̃R�[���o�b�N�֐�</param>
	void FindMaterial(OnFindMaterial onFindMaterial) const
	{
#ifndef DW_DX12_TEMPORARY
		FindMesh([&](const std::unique_ptr<DirectX::ModelMeshPart>& mesh) {
			ModelEffect* effect = reinterpret_cast<ModelEffect*>(mesh->effect.get());
			onFindMaterial(effect);
		});
#endif
	}
	/// <summary>
	/// �}�e���A���f�[�^����������
	/// </summary>
	/// <param name="onFindMaterial">�}�e���A���f�[�^�����������Ƃ��̃R�[���o�b�N�֐�</param>
	void FindMaterialData(std::function<void(MaterialData&)> onFindMaterial) const;

	//�}�e���A���ݒ�����������ėL����
	void InitMaterialSetting() {
		isMatSetInit = true;
		isMatSetEnable = true;

		bool isAllocated = m_materialSetting.size() > 0;
		int i = 0;
		FindMaterialData(
			[&](MaterialData& mat) {
				if (!isAllocated) { m_materialSetting.emplace_back(); }//�}�e���A���ݒ�̊m��
				m_materialSetting[i].Init(&mat);//������
				i++;
			}
		);
	}
	//�}�e���A���ݒ�̗L���E������ݒ�
	void SetMaterialSettingEnable(bool enable) {
		isMatSetEnable = enable;
	}
	//�}�e���A���ݒ����������
	void FindMaterialSetting(std::function<void(MaterialSetting*)> onFindMaterialSetting) 
	{
		if (!isMatSetInit) { InitMaterialSetting(); }

		for (auto& mat : m_materialSetting) {
			onFindMaterialSetting(&mat);
		}
	}

	//�f�v�X�X�e���V���X�e�[�g���Z�b�g
	void SetDepthStencilState(ID3D11DepthStencilState* state) {
		m_pDepthStencilState = state;
	}
	//���X�^���C�U�[�X�e�[�g���Z�b�g
	void SetRasterizerState(ID3D11RasterizerState* RSCw, ID3D11RasterizerState* RSCCw, ID3D11RasterizerState* RSNone) {
		m_pRasterizerStateCw = RSCw;
		m_pRasterizerStateCCw = RSCCw;
		m_pRasterizerStateNone = RSNone;
	}
	void GetRasterizerState(ID3D11RasterizerState*& RSCw, ID3D11RasterizerState*& RSCCw, ID3D11RasterizerState*& RSNone) const{
		RSCw = m_pRasterizerStateCw;
		RSCCw = m_pRasterizerStateCCw;
		RSNone = m_pRasterizerStateNone;
	}
	//�[�x�o�C�A�X���Z�b�g(�V�F�[�_�p)
	void SetDepthBias(float bias) {
		m_depthBias = bias;
	}
	float GetDepthBias()const {
		return m_depthBias;
	}

	//�ʂ̌�����ݒ�
	void SetCullMode(D3D11_CULL_MODE cullMode) {
		m_cull = cullMode;
	}

	//�V�F�[�_�[�p�ϐ�t��ݒ�
	void Set_t(float t) { m_cb_t = t; }

	/// <summary>
	/// �C���|�X�^�[�̖�����ݒ�
	/// </summary>
	/// <param name="x">���̖���</param>
	/// <param name="y">�c�̖���</param>
	void SetImposterPartNum(int x, int y) {
		m_imposterPartNum[0] = x;
		m_imposterPartNum[1] = y;
	}
	//�C���|�X�^�[�̃p�����[�^��ݒ�
	void SetImposterParameter(float scale, float rotYrad) {
		m_imposterScale = scale;
		m_imposterRotY = rotYrad;
	}

	/// <summary>
	/// �\�t�g�p�[�e�B�N�����L���ɂȂ鋗����ݒ�
	/// </summary>
	void SetSoftParticleArea(float distance) {
		m_softParticleArea = distance;
	}

	//�C���X�^���X����ݒ�
	void SetInstanceNum(int num) {
		m_instanceNum = num;
	}

	//���[���h�s����v�Z���邩�ݒ�
	void SetIsCalcWorldMatrix(bool enable) {
		m_isCalcWorldMatrix = enable;
	}

	//������J�����O���s�����ݒ�
	void SetIsFrustumCulling(bool enable) {
		m_isFrustumCull = enable;
	}
	bool GetIsFrustumCulling()const {
		return m_isFrustumCull;
	}

	/// <summary>
	/// �o�E���f�B���O�{�b�N�X(AABB)��ݒ�
	/// </summary>
	/// <param name="min">�{�b�N�X�̍ŏ����W</param>
	/// <param name="max">�{�b�N�X�̍ő���W</param>
	void SetBoundingBox(const CVector3& min, const CVector3& max) {
		m_minAABB_Origin = min;
		m_maxAABB_Origin = max;
		m_centerAABB = m_minAABB_Origin + m_maxAABB_Origin; m_centerAABB /= 2.0f;
		m_extentsAABB = m_maxAABB_Origin - m_centerAABB;
		//�o�E���f�B���O�{�b�N�X������
		UpdateBoundingBoxWithWorldMatrix();
	}
	/// <summary>
	/// ���[���h�s�񓙂�K�����Ă��Ȃ��o�E���f�B���O�{�b�N�X(AABB)���擾
	/// </summary>
	/// <param name="return_min">(�߂�l)�{�b�N�X�̍ŏ����W</param>
	/// <param name="return_max">(�߂�l)�{�b�N�X�̍ő���W</param>
	void GetBoundingBox(CVector3& return_min, CVector3& return_max)const {
		return_min = m_minAABB_Origin;
		return_max = m_maxAABB_Origin;
	}
	/// <summary>
	/// ���[���h�s�񓙓K�������o�E���f�B���O�{�b�N�X(AABB)���擾
	/// </summary>
	/// <param name="return_min">(�߂�l)�{�b�N�X�̍ŏ����W</param>
	/// <param name="return_max">(�߂�l)�{�b�N�X�̍ő���W</param>
	void GetUpdatedBoundingBox(CVector3& return_min, CVector3& return_max)const {
		return_min = m_minAABB;
		return_max = m_maxAABB;
	}
	/// <summary>
	/// ���f���{���̃o�E���f�B���O�{�b�N�X(AABB)���擾
	/// </summary>
	/// <param name="return_center">�{�b�N�X�̒��S</param>
	/// <param name="return_extents">�{�b�N�X�̒��S����[�܂ł̃x�N�g��</param>
	void GetModelOriginalBoundingBox(CVector3& return_center, CVector3& return_extents)const {
		return_center = m_modelBoxCenter, return_extents = m_modelBoxExtents;
	}

	//�J�����O�O�ɍs��������ݒ�
	void SetPreCullingFunction(std::function<void(SkinModel*)> func) {
		m_preCullingFunc = func;
	}
	/// <summary>
	/// �`��O�ɍs��������ݒ�
	/// </summary>
	/// <param name="funcName">������</param>
	/// <param name="func">����</param>
	/// <returns>�ݒ�ɐ���������?(���O�����Ԃ�ƒǉ��ł��Ȃ�)</returns>
	bool SetPreDrawFunction(const wchar_t* funcName, std::function<void(SkinModel*)> func) {
		return m_preDrawFunc.emplace(Util::MakeHash(funcName), func).second;
	}
	void ErasePreDrawFunction(const wchar_t* funcName) {
		m_preDrawFunc.erase(Util::MakeHash(funcName));
	}
	/// <summary>
	/// �`���ɍs��������ݒ�
	/// </summary>
	/// <param name="funcName">������</param>
	/// <param name="func">����</param>
	/// <returns>�ݒ�ɐ���������?(���O�����Ԃ�ƒǉ��ł��Ȃ�)</returns>
	bool SetPostDrawFunction(const wchar_t* funcName, std::function<void(SkinModel*)> func) {
		return m_postDrawFunc.emplace(Util::MakeHash(funcName), func).second;
	}
	void ErasePostDrawFunction(const wchar_t* funcName) {
		m_postDrawFunc.erase(Util::MakeHash(funcName));
	}

	//�`����s�����ݒ�
	void SetIsDraw(bool flag) { m_isDraw = flag; }
	bool GetIsDraw() const { return m_isDraw; }

	//FBX�̐ݒ�擾
	[[nodiscard]]
	const EnFbxUpAxis& GetFBXUpAxis()const {
		return m_enFbxUpAxis;
	}
	[[nodiscard]]
	const EnFbxCoordinateSystem& GetFBXCoordinateSystem()const {
		return m_enFbxCoordinate;
	}

	/// <summary>
	/// DirectXTK�̃��f�������[�h�ς݂�?
	/// </summary>
	bool IsLoadedDirectXTKModel()const {
		return m_modelDx != nullptr;
	}
	/// <summary>
	/// ���f�������[�h�ς݂�?
	/// </summary>
	bool IsLoadedModel()const {
		return m_model != nullptr;
	}

private:
	/*!
	*@brief	�萔�o�b�t�@�̍쐬�B
	*/
	void InitConstantBuffer();
	/*!
	*@brief	�X�P���g���̏������B
	*@param[in]	filePath		���[�h����cmo�t�@�C���̃t�@�C���p�X�B
	*/
	bool InitSkeleton(const wchar_t* filePath);	

	/// <summary>
	/// �o�E���f�B���O�{�b�N�X�����[���h�s��ōX�V
	/// </summary>
	void UpdateBoundingBoxWithWorldMatrix();

	/// <summary>
	/// ���[���h�s��X�V��̏���
	/// </summary>
	/// <param name="isUpdatedWorldMatrix">���[���h�s�񂪍X�V���ꂽ��?</param>
	/// <param name="RefreshOldPos">���s����X�V���邩?</param>
	void InnerUpdateWorldMatrix(bool isUpdatedWorldMatrix, bool RefreshOldPos);
	
private:
	//�萔�o�b�t�@�B
	//[model.fx:VSPSCb]
	struct SVSConstantBuffer {
		CMatrix mWorld;
		CMatrix mView;
		CMatrix mProj;

		//�O��̍s��
		CMatrix mWorld_old;
		CMatrix mView_old;
		CMatrix mProj_old;

		//�J�����̈ړ���
		CVector4 camMoveVec;

		//�[�x�l�̃o�C�A�X
		CVector4 depthBias;

		//�J�����̃��[���h���W
		CVector3 camWorldPos;

		//�Ȃ�
		float cb_t = 0.0f;

		//�C���|�X�^�[�p
		int imposterPartNum[2];//������
		float imposterParameter[2];//x:�X�P�[��,y:Y����]
		
		//�J������Near(x)�EFar(y)
		CVector2 nearFar;

		//�\�t�g�p�[�e�B�N�����L���ɂȂ�͈�
		float softParticleArea = 0.02f;

		//���[�V�����u���[�X�P�[��
		float MotionBlurScale = MotionBlurRender::DEFAULT_MBLUR_SCALE;
	};
	ID3D11Buffer* m_cb = nullptr;//�萔�o�b�t�@

	//�s��
	CMatrix	m_worldMatrix;		//���[���h�s��
	CMatrix m_worldMatrixOld;	//�O��̃��[���h�s��
	CMatrix m_biasMatrix;
	bool m_isFirstWorldMatRef = true;

	//�V�F�[�_�[�p�ݒ�	
	float m_cb_t = 0.0f;//�V�F�[�_�[�p�Ȃ�		
	int m_imposterPartNum[2] = {};//�C���|�X�^�[�p������
	float m_imposterScale = 1.0f, m_imposterRotY = 0.0f;//�C���|�X�^�[�p	
	float m_softParticleArea = 50.0f;//�\�t�g�p�[�e�B�N�����L���ɂȂ�͈�

	//�}�e���A���ʐݒ�	
	bool isMatSetInit = false;
	bool isMatSetEnable = false;
	std::vector<MaterialSetting> m_materialSetting;

	//���f���t�@�C�����
	EnFbxUpAxis			m_enFbxUpAxis = enFbxUpAxisZ;			//FBX�̏����
	EnFbxCoordinateSystem m_enFbxCoordinate = enFbxRightHanded;	//FBX�̍��W�n
	std::wstring		m_modelName;							//���f���̖��O

	//DirectXTK���񋟂��郂�f���N���X
	DirectX::Model* m_modelDx = nullptr;								
	std::unique_ptr<DirectX::Model> m_modelDxData;

	//���̃G���W���̃��f���N���X
	CModel* m_model = nullptr;
	std::unique_ptr<CModel> m_modelData;

	//�X�P���g��
	Skeleton m_skeleton;

	//�`��ݒ�
	ID3D11DepthStencilState* m_pDepthStencilState = nullptr;//�f�v�X�X�e���V���X�e�[�g
	ID3D11RasterizerState* m_pRasterizerStateCw = nullptr;//���X�^���C�U�X�e�[�g
	ID3D11RasterizerState* m_pRasterizerStateCCw = nullptr;
	ID3D11RasterizerState* m_pRasterizerStateNone = nullptr;
	float m_depthBias = 0.0f;//�[�x�l�o�C�A�X	
	D3D11_CULL_MODE m_cull = D3D11_CULL_FRONT;//�ʂ̌���

	//�C���X�^���X��
	int m_instanceNum = 1;

	//�o�E���f�B���O�{�b�N�X
	CVector3 m_minAABB_Origin, m_maxAABB_Origin;
	CVector3 m_centerAABB, m_extentsAABB;
	CVector3 m_minAABB, m_maxAABB;
	CVector3 m_modelBoxCenter, m_modelBoxExtents;

	//���[�U�[�ݒ�̏���
	std::function<void(SkinModel*)> m_preCullingFunc = nullptr;//�J�����O�O�Ɏ��s
	std::unordered_map<int, std::function<void(SkinModel*)>> m_preDrawFunc;//�`��O�Ɏ��s
	std::unordered_map<int, std::function<void(SkinModel*)>> m_postDrawFunc;//�`���Ɏ��s

	//�ݒ�
	bool m_isDraw = true; //�`�悷�邩?
	bool m_isCalcWorldMatrix = true;//���[���h�s����v�Z���邩?
	bool m_isFrustumCull = false;//������J�����O���邩?

	//���f���f�[�^�}�l�[�W���[
	static SkinModelDataManager m_skinModelDataManager;
};

}