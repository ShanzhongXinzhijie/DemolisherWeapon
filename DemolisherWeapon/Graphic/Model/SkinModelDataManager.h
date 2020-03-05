#pragma once

namespace DemolisherWeapon {

class Skeleton;

/*!
*@brief	�X�L�����f���f�[�^�}�l�[�W���[�B�B
*@Flyweight�p�^�[����FlyweightFactory�N���X�A�B
*/
class SkinModelDataManager
{
public:
	/// <summary>
	/// ���f�����쐬
	/// (Map�Ƀ��f�����o�^����܂���)
	/// </summary>
	/// <param name="filePath">�t�@�C���p�X</param>
	/// <param name="skeleton">�X�P���g��</param>
	/// <returns>�쐬�������f���f�[�^</returns>
	std::unique_ptr<DirectX::Model> CreateModel(const wchar_t* filePath, const Skeleton& skeleton);
	
	/// <summary>
	/// ���f�������[�h
	/// (Map�Ƀ��f�����o�^����܂��B���łɓo�^����Ă���ꍇ��������Q�Ƃ����)
	/// </summary>
	/// <param name="filePath">�t�@�C���p�X</param>
	/// <param name="sketon">�X�P���g��</param>
	/// <returns>���[�h�������f���̎Q��</returns>
	DirectX::Model* Load(const wchar_t* filePath, const Skeleton& sketon);

	/*!
	*@brief	���f���f�[�^��S�J���B
	*/
	void Release();

private:
	//�t�@�C���p�X���L�[�ADirectXModel�̃C���X�^���X��l�Ƃ���}�b�v�B
	//�����݂����Ȃ��̂ł��B
	std::unordered_map<
		std::wstring,					//wstring���L�[�B 
		std::unique_ptr<DirectX::Model>	//���ꂪ�l�B
	> m_directXModelMap;
};

}

