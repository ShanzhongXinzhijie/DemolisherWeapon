/*!
 * @brief	���[�e�B���e�B�֐��B
 */
#pragma once


class Util{
public:
	/*!
	 * @brief	�����񂩂�32bit�̃n�b�V���l���쐬�B
	 */
	static int MakeHash( const char* string )
	{
		int hash = 0;
		int l = (int)strlen(string);
		for( int i = 0; i < l; i++ ){
			hash = hash * 37 + string[i];
		}
		return hash;
	}

	/*!
	* @brief	�����񂩂�32bit�̃n�b�V���l���쐬�B
	*/
	static int MakeHash(const wchar_t* string)
	{
		int hash = 0;
		int l = (int)wcslen(string);
		for (int i = 0; i < l; i++) {
			hash = hash * 37 + string[i];
		}
		return hash;
	}

	//�n�b�V���l������
	static std::size_t HashCombine(std::size_t hash, std::size_t hash2)
	{
		return hash ^ (hash2 + 0x9e3779b9 + (hash << 6) + (hash >> 2));
	}
};

