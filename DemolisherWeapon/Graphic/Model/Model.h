#pragma once
#include"tktkmfile.h"

namespace DemolisherWeapon{

	/// <summary>
	/// ���b�V���p�[�c
	/// </summary>
	class CMeshParts {
	public:
		/// <summary>
		/// ���b�V��
		/// </summary>
		struct SMesh {
			std::unique_ptr<IVertexBuffer>					m_vertexBuffer;		//���_�o�b�t�@�B
			std::vector<std::unique_ptr<IIndexBuffer>>		m_indexBufferArray;	//�C���f�b�N�X�o�b�t�@�B
			std::vector<std::unique_ptr<MaterialSetting>>	m_materials;		//�}�e���A���B
			std::vector<int>								m_skinFlags;		//�X�L���������Ă��邩�ǂ����̃t���O�B
		};

		/// <summary>
		/// tkm�t�@�C�����珉�����B
		/// </summary>
		/// <param name="tkmFile"></param>
		void InitFromTkmFile(const tkEngine::CTkmFile& tkmFile);

		/// <summary>
		/// �`��B
		/// </summary>
		void Draw();

		std::vector< std::unique_ptr<SMesh> > m_meshs;//���b�V��

	private:
		void CreateMeshFromTkmMesh(const tkEngine::CTkmFile::SMesh& tkmMesh, int meshNo);

	};

	/// <summary>
	/// ���f���N���X�B
	/// </summary>
	class CModel {
	public:
		//���b�V���E�}�e���A���̌���

		/// <summary>
		/// tkm�t�@�C����񓯊����\�h�B
		/// </summary>
		/// <remarks>
		/// IsInited�֐��𗘗p���āA�������Ƃ�悤�ɂ��Ă��������B
		/// </remarks>
		/// <param name="filePath">tkm�t�@�C���̃t�@�C���p�X�B</param>
		void LoadTkmFileAsync(const char* filePath);

		/// <summary>
		/// tkm�t�@�C�������[�h�B
		/// </summary>
		/// <param name="filePath">tkm�t�@�C���̃t�@�C���p�X�B</param>
		void LoadTkmFile(const char* filePath);

		/// <summary>
		/// ���b�V���p�[�c���쐬����B
		/// </summary>
		/// <remarks>
		/// ���̊֐���LoadTkmFileAsync�֐��Ȃǂ𗘗p���āA
		/// tkm�t�@�C�������[�h������ŌĂяo���Ă��������B
		/// </remarks>
		void CreateMeshParts();

		/// <summary>
		/// ���������I����Ă��邩����B
		/// </summary>
		/// <returns></returns>
		bool IsInited() const;

		/// <summary>
		/// �`��
		/// </summary>
		void Draw();

		/// <summary>
		/// tkm�t�@�C�����擾�B
		/// </summary>
		/// <returns></returns>
		const tkEngine::CTkmFile& GetTkmFile() const
		{
			return m_tkmFile;
		}

	private:
		tkEngine::CTkmFile m_tkmFile;	//tkm�t�@�C���B
		CMeshParts m_meshParts;			//���b�V���p�[�c�B
	};
}