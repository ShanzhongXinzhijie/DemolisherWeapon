#pragma once

//�A�N�Z�T�쐬�}�N��
#define DW_GETSET(varType, varName, funName) public: virtual inline varType get##funName(void) const { return varName; } inline void set##funName(varType var) { varName = var; } private: varType varName; 
#define DW_GETSETCONSREF(varType, varName, funName) public: virtual inline const varType& get##funName(void) const { return varName; } inline void set##funName(const varType& var) { varName = var; } private: varType varName; 