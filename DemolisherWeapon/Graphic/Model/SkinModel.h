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
	*@param[in]	position	���f���̍��W�B
	*@param[in]	rotation	���f���̉�]�B
	*@param[in]	scale		���f���̊g�嗦�B
	*/
	void UpdateWorldMatrix(const CVector3& position, const CQuaternion& rotation, const CVector3& scale, bool RefreshOldPos = false);

	//���[���h�s����v�Z����
	void CalcWorldMatrix(const CVector3& position, const CQuaternion& rotation, const CVector3& scale, CMatrix& returnWorldMatrix);

	//���s��̋L�^
	void UpdateOldMatrix() {
		//�O��̃��[���h�s����L�^
		m_worldMatrixOld = m_worldMatrix;
		//�{�[��
		m_skeleton.UpdateBoneMatrixOld();
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

	//�C���X�^���X����ݒ�
	void SetInstanceNum(int num) {
		m_instanceNum = num;
	}

	//���[���h�s����v�Z���邩�ݒ�
	void SetIsCalcWorldMatrix(bool enable) {
		m_isCalcWorldMatrix = enable;
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
	*@brief	�T���v���X�e�[�g�̏������B
	*/
	void InitSamplerState();
	/*!
	*@brief	�萔�o�b�t�@�̍쐬�B
	*/
	void InitConstantBuffer();
	/*!
	*@brief	�X�P���g���̏������B
	*@param[in]	filePath		���[�h����cmo�t�@�C���̃t�@�C���p�X�B
	*/
	void InitSkeleton(const wchar_t* filePath);	
	
private:
	//�萔�o�b�t�@�B
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

		CVector4 depthBias;

		//�J�����̃��[���h���W
		CVector3 camWorldPos;
	};
	CMatrix	m_worldMatrix;		//���[���h�s��
	CMatrix m_worldMatrixOld;	//�O��̃��[���h�s��
	bool m_isFirstWorldMatRef = true;

	//�}�e���A���ʐݒ�	
	bool isMatSetInit = false;
	bool isMatSetEnable = false;
	std::vector<MaterialSetting> m_materialSetting;

	EnFbxUpAxis			m_enFbxUpAxis = enFbxUpAxisZ;	//!<FBX�̏�����B
	EnFbxCoordinateSystem m_enFbxCoordinate = enFbxRightHanded;//FBX�̍��W�n
	ID3D11Buffer*		m_cb = nullptr;					//!<�萔�o�b�t�@�B
	Skeleton			m_skeleton;						//!<�X�P���g���B
	DirectX::Model*		m_modelDx;						//!<DirectXTK���񋟂��郂�f���N���X�B
	std::wstring		m_modelName;					//!<���f���̖��O�B
	ID3D11SamplerState* m_samplerState = nullptr;		//!<�T���v���X�e�[�g�B
	ID3D11RasterizerState* m_pRasterizerStateCw = nullptr;
	ID3D11RasterizerState* m_pRasterizerStateCCw = nullptr;
	float m_depthBias = 0.0f;

	int m_instanceNum = 1;//�C���X�^���X��

	bool m_isCalcWorldMatrix = true;//���[���h�s����v�Z���邩?

	std::function<void(SkinModel*)> m_preDrawFunc = nullptr;//���[�U�[�ݒ�̏���

	static SkinModelDataManager m_skinModelDataManager;
};

}