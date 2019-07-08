#pragma once

namespace DemolisherWeapon {
	namespace Error
	{	
		//メッセージボックスだしてプログラムを止める
		static inline void Box(const char* str) {
#ifndef DW_MASTER
			MessageBox(NULL, str, "Error", MB_OK);
			std::abort();
#endif
		}
		//警告メッセージを出力
		static inline void WarningMessage(const char* str) {
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