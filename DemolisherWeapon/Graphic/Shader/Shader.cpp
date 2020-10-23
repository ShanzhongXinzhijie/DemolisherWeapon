/*!
 * @brief	�V�F�[�_�[�B
 */
#include "DWstdafx.h"
#include "Shader.h"

#include <stierr.h>
#include <sstream>
#include <fstream>

namespace DemolisherWeapon {
	
bool Shader::Load(const char* filePath, const char* entryFuncName, EnType shaderType, const char* definesIdentifier, const D3D_SHADER_MACRO* pDefines)
{
	bool result = ShaderResources::GetInstance().Load(
		m_pShaderResource,
		filePath,
		entryFuncName,
		shaderType,
		definesIdentifier,
		pDefines
	);
	if (!result) {
		MessageBox(NULL, filePath, "Shader::Load()�Ɏ��s���܂���", MB_OK);
	}
	m_isLoaded = result;
    return result;
}


bool Shader::LoadRaytracing(const wchar_t* filePath)
{
	std::ifstream shaderFile(filePath);
	if (shaderFile.good() == false) {
		std::wstring errormessage = L"�V�F�[�_�[�t�@�C���̃I�[�v���Ɏ��s���܂����B\n";
		errormessage += filePath;
		MessageBoxW(nullptr, errormessage.c_str(), L"�G���[", MB_OK);
		std::abort();
	}

	std::stringstream strStream;
	strStream << shaderFile.rdbuf();
	std::string shader = strStream.str();
	//�V�F�[�_�[�̃e�L�X�g�t�@�C������ABLOB���쐬����B
	Microsoft::WRL::ComPtr<IDxcLibrary> dxclib;
	auto hr = DxcCreateInstance(CLSID_DxcLibrary, IID_PPV_ARGS(&dxclib));
	if (FAILED(hr)) {
		MessageBox(nullptr, "DXCLIB�̍쐬�Ɏ��s���܂����B", "�G���[", MB_OK);
		std::abort();
	}
	Microsoft::WRL::ComPtr< IDxcIncludeHandler> includerHandler;
	hr = dxclib->CreateIncludeHandler(&includerHandler);
	if (FAILED(hr)) {
		MessageBox(nullptr, "CreateIncludeHandler�Ɏ��s���܂����B", "�G���[", MB_OK);
		std::abort();
	}

	//dxc�R���p�C���̍쐬�B
	Microsoft::WRL::ComPtr<IDxcCompiler> dxcCompiler;
	hr = DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&dxcCompiler));
	if (FAILED(hr)) {
		MessageBox(nullptr, "dxc�R���p�C���̍쐬�Ɏ��s���܂����B", "�G���[", MB_OK);
		std::abort();
	}
	//�\�[�X�R�[�h��BLOB���쐬����B
	uint32_t codePage = CP_UTF8;
	Microsoft::WRL::ComPtr< IDxcBlobEncoding> sourceBlob;
	hr = dxclib->CreateBlobFromFile(filePath, &codePage, &sourceBlob);
	if (FAILED(hr)) {
		MessageBox(nullptr, "�V�F�[�_�[�\�[�X��Blob�̍쐬�Ɏ��s���܂����B", "�G���[", MB_OK);
		std::abort();
	}

	Microsoft::WRL::ComPtr<IDxcIncludeHandler> dxcIncludeHandler;
	dxclib->CreateIncludeHandler(&dxcIncludeHandler);
	const wchar_t* args[] = {
		L"-I preset\\shader",//�C���N���[�h�p�X��ǉ�����
	};
	//�R���p�C���B
	Microsoft::WRL::ComPtr<IDxcOperationResult> result;
	hr = dxcCompiler->Compile(
		sourceBlob.Get(), // pSource
		filePath, // pSourceName
		L"",		// pEntryPoint
		L"lib_6_3", // pTargetProfile
		nullptr, 0, //args, 1, // pArguments, argCount
		nullptr, 0, // pDefines, defineCount
		dxcIncludeHandler.Get(), // pIncludeHandler
		&result); // ppResult
	if (SUCCEEDED(hr)) {
		result->GetStatus(&hr);
	}

	if (FAILED(hr))
	{
		if (result)
		{
			Microsoft::WRL::ComPtr<IDxcBlobEncoding> errorsBlob;
			hr = result->GetErrorBuffer(&errorsBlob);
			if (SUCCEEDED(hr) && errorsBlob)
			{
				std::string errormessage = "Compilation failed with errors:\n%hs\n";
				errormessage += (const char*)errorsBlob->GetBufferPointer();
				MessageBoxA(nullptr, errormessage.c_str(), "�G���[", MB_OK);

			}
		}
		// Handle compilation error...
	}
	else {
		result->GetResult(&m_dxcBlob);
	}

	return true;
}

}