#pragma once

namespace DemolisherWeapon {
	namespace Error
	{	
		//���b�Z�[�W�{�b�N�X�����ăv���O�������~�߂�
		static inline void Box(const char* str) {
#ifndef DW_MASTER
			MessageBox(NULL, str, "Error", MB_OK);
			std::abort();
#endif
		}
	
	}
}

#ifndef DW_MASTER
#define 	DW_ERRORBOX( flg, str );	if(flg){DemolisherWeapon::Error::Box(str);}
#else
#define 	DW_ERRORBOX( flg, str );
#endif 