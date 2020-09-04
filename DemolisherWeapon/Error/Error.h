#pragma once

namespace DemolisherWeapon {
	namespace Error
	{	
#ifndef DW_MASTER
		inline char log[1024 * 10];
#endif

		//���b�Z�[�W�{�b�N�X�����ăv���O�������~�߂�
		inline void Box(const char* str) {
#ifndef DW_MASTER
			MessageBox(NULL, str, "Error", MB_OK);
			std::abort();
#endif
		}

		//���b�Z�[�W�{�b�N�X�����ăv���O�������~�߂�
		inline void Box(const char* file, long line, const char* format, ...) {
#ifndef DW_MASTER
			va_list va;
			va_start(va, format);
			char fileLineInfo[256];
			sprintf_s(fileLineInfo, "\n\n%s, %d�s��", file, line);
			vsprintf_s(log, format, va);
			strcat_s(log, fileLineInfo);
			va_end(va);

			MessageBox(NULL, log, "Error", MB_OK);
			std::abort();
#endif
		}

		//�x�����b�Z�[�W�{�b�N�X
		inline void WarningBox(const char* file, long line, const char* format, ...){
#ifndef DW_MASTER
			va_list va;
			va_start(va, format);
			char fileLineInfo[256];
			sprintf_s(fileLineInfo, "\n\n%s, %d�s��", file, line);
			vsprintf_s(log, format, va);
			strcat_s(log, fileLineInfo);
			va_end(va);

			MessageBox(NULL, log, "Warning", MB_OK);
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
#define 	DW_ERRORBOX( flg, format, ... );	if(flg){DemolisherWeapon::Error::Box(__FILE__, __LINE__,  format, __VA_ARGS__);}
#define 	DW_WARNING_BOX( flg, format, ... );	if(flg){DemolisherWeapon::Error::WarningBox(__FILE__, __LINE__,  format, __VA_ARGS__);}
#define 	DW_WARNING_MESSAGE( flg, str );	if(flg){DemolisherWeapon::Error::WarningMessage(str);}
#else
#define 	DW_ERRORBOX( flg, str );
#define 	DW_WARNING_MESSAGE( flg, str );
#endif 