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
	/*!
	*@brief	���f�������[�h�B
	*@param[in]	filePath	�t�@�C���p�X�B
	*@param[in]	skeleton	�X�P���g���B
	*/
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

