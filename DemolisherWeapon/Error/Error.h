#pragma once

namespace DemolisherWeapon {
	namespace Error
	{	
		//���b�Z�[�W�{�b�N�X�����ăv���O�������~�߂�
		inline void Box(const char* str) {
#ifndef DW_MASTER
			MessageBox(NULL, str, "Error", MB_OK);
			std::abort();
#endif
		}
		//�x�����b�Z�[�W���o��
		inline void WarningMessage(const char* str) {
#ifndef DW_MASTER
			OutputDebugStringA(str);
#endif
		}	
	}
}

#ifndef DW_MASTER
#define 	DW_ERRORBOX( flg, str );	if(flg){DemolisherWeapon::Error::Box(str);}
#define 	DW_WARNING_MESSAGE( flg, str );	if(flg){DemolisherWeapon::Error::WarningMessage(str);}
#else
#define 	DW_ERRORBOX( flg, str );
#define 	DW_WARNING_MESSAGE( flg, str );
#endif 