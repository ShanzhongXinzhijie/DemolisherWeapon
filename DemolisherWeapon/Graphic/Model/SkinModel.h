#pragma once

#include "Skeleton.h"
#include "SkinModelDataManager.h"
#include "SkinModelEffect.h"
#include "../Render/MotionBlurRender.h"

namespace DemolisherWeapon {

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
	
	/*!
	*@brief	�������B
	*@param[in]	filePath		���[�h����cmo�t�@�C���̃t�@�C���p�X�B
	*@param[in] enFbxUpAxis		fbx�̏㎲�B�f�t�H���g��enFbxUpAxisZ�B
	*/
	void Init(const wchar_t* filePath, EnFbxUpAxis enFbxUpAxis = enFbxUpAxisZ, EnFbxCoordinateSystem enFbxCoordinate = enFbxRightHanded);
	
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
	/// <returns>���[���h�s��</returns>
	const CMatrix& GetWorldMatrix()const {
		return m_worldMatrix;
	}
	/// <summary>
	/// ���[���h�s��̐ݒ�
	/// </summary>
	/// <param name="mat">�ݒ肷��s��</param>
	void SetWorldMatrix(const CMatrix& mat) {
		m_worldMatrix = mat;
	}

	/*!
	*@brief	�{�[���������B
	*@param[in]		boneName	�{�[���̖��O�B
	*@return	���������{�[���B������Ȃ������ꍇ��nullptr��Ԃ��܂��B
	*/
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
	const wchar_t* GetModelName()const
	{
		return m_modelName.c_str();
	}
	//���f���̖��O�̈�v�𔻒�
	bool EqualModelName(const wchar_t* name) const
	{
		return wcscmp(name, m_modelName.c_str()) == 0;
	}

	/*!
	*@brief	�X�P���g���̎擾�B
	*/
	Skeleton& GetSkeleton()
	{
		return m_skeleton;
	}

	/*!
	*@brief	���b�V������������B
	*@param[in] onFindMesh		���b�V�������������Ƃ��̃R�[���o�b�N�֐�
	*/
	void FindMesh(OnFindMesh onFindMesh) const
	{
		for (auto& modelMeshs : m_modelDx->meshes) {
			for (std::unique_ptr<DirectX::ModelMeshPart>& mesh : modelMeshs->meshParts) {
				onFindMesh(mesh);
			}
		}
	}
	/// <summary>
	/// ���b�V���̏W������������B
	/// </summary>
	/// <param name="onFindMeshes">���b�V���̏W�������������Ƃ��̃R�[���o�b�N�֐�</param>
	void FindMeshes(std::function<void(const std::shared_ptr<DirectX::ModelMesh>&)> onFindMeshes)const {
		for (auto& modelMeshs : m_modelDx->meshes) {
			onFindMeshes(modelMeshs);
		}
	}
	/*!
	*@brief	�}�e���A������������B
	*@param[in] onFindMaterial	�}�e���A�������������Ƃ��̃R�[���o�b�N�֐�
	*/
	void FindMaterial(OnFindMaterial onFindMaterial) const
	{
		FindMesh([&](auto& mesh) {
			ModelEffect* effect = reinterpret_cast<ModelEffect*>(mesh->effect.get());
			onFindMaterial(effect);
		});
	}

	//�}�e���A���ݒ�����������ėL����
	void InitMaterialSetting() {
		isMatSetInit = true;
		isMatSetEnable = true;

		int i = 0;
		FindMaterial(
			[&](ModelEffect* mat) {
				mat->MaterialSettingInit(m_materialSetting[i]);
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

	//���X�^���C�U�[�X�e�[�g���Z�b�g
	void SetRasterizerState(ID3D11RasterizerState* RSCw, ID3D11RasterizerState* RSCCw) {
		m_pRasterizerStateCw = RSCw;
		m_pRasterizerStateCCw = RSCCw;
	}
	void GetRasterizerState(ID3D11RasterizerState*& RSCw, ID3D11RasterizerState*& RSCCw) const{
		RSCw = m_pRasterizerStateCw;
		RSCCw = m_pRasterizerStateCCw;
	}
	//�[�x�o�C�A�X���Z�b�g(�V�F�[�_�p)
	void SetDepthBias(float bias) {
		m_depthBias = bias;
	}
	float GetDepthBias()const {
		return m_depthBias;
	}

	/// <summary>
	/// �C���|�X�^�[�̖�����ݒ�
	/// </summary>
	/// <param name="x">���̖���</param>
	/// <param name="y">�c�̖���</param>
	void SetImposterPartNum(int x, int y) {
		m_imposterPartNum[0] = x;
		m_imposterPartNum[1] = y;
	}
	//�C���|�X�^�[�̊g�嗦��ݒ�
	void SetImposterScale(float scale) {
		m_imposterScale = scale;
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

	//�o�E���f�B���O�{�b�N�X��ݒ�
	void SetBoundingBox(const CVector3& min, const CVector3& max) {
		m_minAABB = min;
		m_maxAABB = max;
		m_centerAABB = m_minAABB + m_maxAABB; m_centerAABB /= 2.0f;
		m_extentsAABB = m_maxAABB - m_centerAABB;
	}
	void GetBoundingBox(CVector3& return_min, CVector3& return_max) {
		return_min = m_minAABB;
		return_max = m_maxAABB;
	}

	//�`��O�ɍs��������ݒ�
	void SetPreDrawFunction(std::function<void(SkinModel*)> func) {
		m_preDrawFunc = func;
	}

	//FBX�̐ݒ�擾
	const EnFbxUpAxis& GetFBXUpAxis()const {
		return m_enFbxUpAxis;
	}
	const EnFbxCoordinateSystem& GetFBXCoordinateSystem()const {
		return m_enFbxCoordinate;
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

		float padding = 0.0f;

		//�C���|�X�^�[�p
		int imposterPartNum[2];//������
		float imposterScale;//�X�P�[��
	};
	CMatrix	m_worldMatrix;		//���[���h�s��
	CMatrix m_worldMatrixOld;	//�O��̃��[���h�s��
	CMatrix m_biasMatrix;
	//CMatrix m_SRTMatrix;
	bool m_isFirstWorldMatRef = true;

	//�}�e���A���ʐݒ�	
	bool isMatSetInit = false;
	bool isMatSetEnable = false;
	std::vector<MaterialSetting> m_materialSetting;

	EnFbxUpAxis			m_enFbxUpAxis = enFbxUpAxisZ;			//!<FBX�̏�����B
	EnFbxCoordinateSystem m_enFbxCoordinate = enFbxRightHanded;	//FBX�̍��W�n
	ID3D11Buffer*		m_cb = nullptr;							//!<�萔�o�b�t�@�B
	Skeleton			m_skeleton;								//!<�X�P���g���B
	DirectX::Model*		m_modelDx;								//!<DirectXTK���񋟂��郂�f���N���X�B
	std::wstring		m_modelName;							//!<���f���̖��O�B

	ID3D11RasterizerState* m_pRasterizerStateCw = nullptr;//���X�^���C�U�X�e�[�g
	ID3D11RasterizerState* m_pRasterizerStateCCw = nullptr;
	float m_depthBias = 0.0f;//�[�x�l�o�C�A�X	
	
	//�C���|�X�^�[�p
	int m_imposterPartNum[2] = {};//������
	float m_imposterScale = 1.0f;

	int m_instanceNum = 1;//�C���X�^���X��

	bool m_isCalcWorldMatrix = true;//���[���h�s����v�Z���邩?

	bool m_isFrustumCull = false;//������J�����O���邩?
	CVector3 m_minAABB, m_maxAABB;//�o�E���f�B���O�{�b�N�X
	CVector3 m_centerAABB, m_extentsAABB;

	std::function<void(SkinModel*)> m_preDrawFunc = nullptr;//���[�U�[�ݒ�̏���

	static SkinModelDataManager m_skinModelDataManager;
};

}