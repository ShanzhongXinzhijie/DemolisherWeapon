/*!
 *@brief	�V�F�[�_�[���\�[�X�B
 *@ Flyweight�p�^�[�����g�p�B
 */

#include "DWstdafx.h"
#include "ShaderResources.h"
#include "util/Util.h"

namespace DemolisherWeapon {

namespace {
	//�t�@�C���ǂݍ���
	std::unique_ptr<char[]> ReadFile(const char* filePath, int& fileSize)
	{
		FILE* fp;
		fopen_s(&fp, filePath, "rb");

		fseek(fp, 0, SEEK_END);
		fpos_t fPos;
		fgetpos(fp, &fPos);
		fseek(fp, 0, SEEK_SET);
		fileSize = (int)fPos;
		std::unique_ptr<char[]> readBuff = std::make_unique<char[]>(fileSize);
		fread(readBuff.get(), fileSize, 1, fp);
		fclose(fp);
		return readBuff;
	}
	/*!
	*@brief	���_�V�F�[�_�[���璸�_���C�A�E�g�𐶐��B
	*/
	HRESULT CreateInputLayoutDescFromVertexShaderSignature(ID3DBlob* pShaderBlob, ID3D11Device* pD3DDevice, ID3D11InputLayout** pInputLayout)
	{
		// �V�F�[�_�[��񂩂烊�t���N�V�������s���B
		ID3D11ShaderReflection* pVertexShaderReflection = NULL;
		if (FAILED(D3DReflect(pShaderBlob->GetBufferPointer(), pShaderBlob->GetBufferSize(), IID_ID3D11ShaderReflection, (void**)&pVertexShaderReflection)))
		{
			return S_FALSE;
		}

		// �V�F�[�_�[�����擾�B
		D3D11_SHADER_DESC shaderDesc;
		pVertexShaderReflection->GetDesc(&shaderDesc);

		// ���͏���`��ǂݍ���
		std::vector<D3D11_INPUT_ELEMENT_DESC> inputLayoutDesc;
		for (unsigned int i = 0; i< shaderDesc.InputParameters; i++)
		{
			D3D11_SIGNATURE_PARAMETER_DESC paramDesc;
			pVertexShaderReflection->GetInputParameterDesc(i, &paramDesc);

			// �G�������g��`��ݒ�B
			D3D11_INPUT_ELEMENT_DESC elementDesc;
			elementDesc.SemanticName = paramDesc.SemanticName;
			elementDesc.SemanticIndex = paramDesc.SemanticIndex;
			elementDesc.InputSlot = 0;
			elementDesc.AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
			elementDesc.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
			elementDesc.InstanceDataStepRate = 0;

			// determine DXGI format
			if (paramDesc.Mask == 1)
			{
				if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_UINT32) elementDesc.Format = DXGI_FORMAT_R32_UINT;
				else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_SINT32) elementDesc.Format = DXGI_FORMAT_R32_SINT;
				else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_FLOAT32) elementDesc.Format = DXGI_FORMAT_R32_FLOAT;
			}
			else if (paramDesc.Mask <= 3)
			{
				if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_UINT32) elementDesc.Format = DXGI_FORMAT_R32G32_UINT;
				else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_SINT32) elementDesc.Format = DXGI_FORMAT_R32G32_SINT;
				else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_FLOAT32) elementDesc.Format = DXGI_FORMAT_R32G32_FLOAT;
			}
			else if (paramDesc.Mask <= 7)
			{
				if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_UINT32) elementDesc.Format = DXGI_FORMAT_R32G32B32_UINT;
				else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_SINT32) elementDesc.Format = DXGI_FORMAT_R32G32B32_SINT;
				else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_FLOAT32) elementDesc.Format = DXGI_FORMAT_R32G32B32_FLOAT;
			}
			else if (paramDesc.Mask <= 15)
			{
				if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_UINT32) elementDesc.Format = DXGI_FORMAT_R32G32B32A32_UINT;
				else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_SINT32) elementDesc.Format = DXGI_FORMAT_R32G32B32A32_SINT;
				else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_FLOAT32) elementDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
			}

			//�G�������g�̒�`��ۑ��B
			inputLayoutDesc.push_back(elementDesc);
		}

		// ���̓��C�A�E�g���쐬�B
		HRESULT hr = pD3DDevice->CreateInputLayout(
			&inputLayoutDesc[0],
			static_cast<UINT>(inputLayoutDesc.size()),
			pShaderBlob->GetBufferPointer(),
			pShaderBlob->GetBufferSize(),
			pInputLayout);

		//���t���N�V�����p�Ɋm�ۂ���������������B
		pVertexShaderReflection->Release();
		return hr;
	}
}

/*!
 *@brief	�R���X�g���N�^�B
 */
ShaderResources::ShaderResources()
{
	GetEngine().GetGraphicsEngine().GetD3DDevice()->CreateClassLinkage(&m_pClassLinkage);
}
/*!
 *@brief	�f�X�g���N�^�B
 */
ShaderResources::~ShaderResources()
{
	Release();
}
void ShaderResources::Release()
{
	for (auto it = m_shaderResourceMap.begin(); it != m_shaderResourceMap.end(); it++) {
		it->second->Release();
	}
	m_shaderResourceMap.clear();
	m_shaderProgramMap.clear();

	m_pClassLinkage->Release();
}

void ShaderResources::SShaderResource::Release(bool fullRelease) {
	if (shader) {
		switch (type) {
		case EnType::VS:
			((ID3D11VertexShader*)shader)->Release();
			shader = nullptr;
			break;
		case EnType::PS:
			((ID3D11PixelShader*)shader)->Release();
			shader = nullptr;
			break;
		case EnType::CS:
			((ID3D11ComputeShader*)shader)->Release();
			shader = nullptr;
			break;
		}
	}
	if (inputLayout) {
		inputLayout->Release();
		inputLayout = nullptr;
	}
	if (blobOut) {
		blobOut->Release();
		blobOut = nullptr;
	}

#ifndef DW_MASTER
	if (fullRelease) {
		entryFuncName.reset();
		pDefines.reset();
	}
	macroNum = 0;
#endif

	if (numInterfaces > 0) {
		free(dynamicLinkageArray); 
		dynamicLinkageArray = nullptr;
	}
	numInterfaces = 0;
}

void ShaderResources::LoadShaderProgram(const char* filePath, SShaderProgramPtr& prog) {
	prog->program = ReadFile(filePath, prog->programSize);
#ifndef DW_MASTER
	prog->filepath = filePath; prog->file_time = std::filesystem::last_write_time(prog->filepath);
#endif
}


bool ShaderResources::CompileShader(const SShaderProgram* shaderProgram, const char* filePath, const D3D_SHADER_MACRO* pDefines, const char* entryFuncName, EnType shaderType, SShaderResource* resource, bool isHotReload) {
	resource->Release(!isHotReload);
	
	DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS | D3DCOMPILE_OPTIMIZATION_LEVEL3;

#ifndef DW_MASTER //#if BUILD_LEVEL == BUILD_LEVEL_DEBUG
	// Set the D3DCOMPILE_DEBUG flag to embed debug information in the shaders.
	// Setting this flag improves the shader debugging experience, but still allows 
	// the shaders to be optimized and to run exactly the way they will run in 
	// the release configuration of this program.
	dwShaderFlags |= D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

	static const char* shaderModelNames[] = {
		"vs_5_0",
		"ps_5_0",
		"cs_5_0"
	};
	ID3DBlob* blobOut;
	ID3DBlob* errorBlob;

	HRESULT hr = S_OK;

	//�R���p�C��
	hr = D3DCompile(shaderProgram->program.get(), shaderProgram->programSize,
		filePath, pDefines, D3D_COMPILE_STANDARD_FILE_INCLUDE, entryFuncName,
		shaderModelNames[(int)shaderType], dwShaderFlags, 0, &blobOut, &errorBlob);

	//�G���[!
	if (FAILED(hr))
	{
		if (errorBlob != nullptr) {
			static char errorMessage[10 * 1024];
			sprintf_s(errorMessage, "filePath : %s, %s", filePath, (char*)errorBlob->GetBufferPointer());
			MessageBox(NULL, errorMessage, "�V�F�[�_�[�R���p�C���G���[", MB_OK);
		}
		return false;
	}

	//�V�F�[�_�[�^�C�v�ɍ��킹�č쐬
	resource->type = shaderType;
	ID3D11Device* pD3DDevice = GetEngine().GetGraphicsEngine().GetD3DDevice();
	switch (shaderType) {
	case EnType::VS: {
		//���_�V�F�[�_�[�B
		hr = pD3DDevice->CreateVertexShader(blobOut->GetBufferPointer(), blobOut->GetBufferSize(), m_pClassLinkage, (ID3D11VertexShader**)&resource->shader);
		if (FAILED(hr)) {
			blobOut->Release();
			return false;
		}
		//���̓��C�A�E�g���쐬�B
		hr = CreateInputLayoutDescFromVertexShaderSignature(blobOut, pD3DDevice, &resource->inputLayout);
		if (FAILED(hr)) {
			//���̓��C�A�E�g�̍쐬�Ɏ��s�����B
			blobOut->Release(); 
			return false;
		}
	}break;
	case EnType::PS: {
		//�s�N�Z���V�F�[�_�[�B
		hr = pD3DDevice->CreatePixelShader(blobOut->GetBufferPointer(), blobOut->GetBufferSize(), m_pClassLinkage, (ID3D11PixelShader**)&resource->shader);
		if (FAILED(hr)) {
			blobOut->Release();
			return false;
		}
	}break;
	case EnType::CS: {
		//�R���s���[�g�V�F�[�_�[�B
		hr = pD3DDevice->CreateComputeShader(blobOut->GetBufferPointer(), blobOut->GetBufferSize(), m_pClassLinkage, (ID3D11ComputeShader**)&resource->shader);
		if (FAILED(hr)) {
			blobOut->Release();
			return false;
		}
	}break;
	}

	//���I�����N�֌W
	{
		ID3D11ShaderReflection* pReflector = nullptr;
		D3DReflect(blobOut->GetBufferPointer(), blobOut->GetBufferSize(), IID_ID3D11ShaderReflection, (void**)&pReflector);
		//�C���^�[�t�F�C�X�C���X�^���X�̐����擾
		resource->numInterfaces = pReflector->GetNumInterfaceSlots();
		//�z��m��
		if (resource->numInterfaces > 0) {
			resource->dynamicLinkageArray = (ID3D11ClassInstance**)malloc(sizeof(ID3D11ClassInstance*) * resource->numInterfaces);
		}
		pReflector->Release();
	}

	//�Ԃ�
	resource->blobOut = blobOut;
#ifndef DW_MASTER
	if (!isHotReload) {
		resource->entryFuncName = std::make_unique<std::string>(entryFuncName);
		if (pDefines) {
			//�}�N���̐��J�E���g
			int cnt = 0;
			while (1) {
				if (pDefines[cnt].Name == NULL || pDefines[cnt].Definition == NULL) { break; }
				cnt++;
			}
			resource->macroNum = cnt;
			if (cnt > 0) {
				resource->pDefines = std::make_unique<SShaderResource::D3D_SHADER_MACRO_SAVE[]>(cnt + 1);
				resource->pDefines[cnt].Name = NULL; resource->pDefines[cnt].Definition = NULL;
				for (int i = 0; i < cnt; i++) {
					//������
					size_t size[2] = { strlen(pDefines[i].Name) + 1, strlen(pDefines[i].Definition) + 1 };
					resource->pDefines[i].Name = std::make_unique<char[]>(size[0]);
					resource->pDefines[i].Definition = std::make_unique<char[]>(size[1]);
					//�R�s�[
					strcpy_s(resource->pDefines[i].Name.get(), size[0], pDefines[i].Name);
					strcpy_s(resource->pDefines[i].Definition.get(), size[1], pDefines[i].Definition);
				}
			}
		}
	}
#endif

	return true;
}


bool ShaderResources::Load(
	const ShaderResources::SShaderResource*& return_resource,
	const char* filePath, 
	const char* entryFuncName,
	EnType shaderType,
	const char* definesIdentifier,
	const D3D_SHADER_MACRO* pDefines
)
{
	//�t�@�C���p�X����n�b�V���l���쐬����B
	int hash = Util::MakeHash(filePath);
	//�V�F�[�_�[�v���O���������[�h�ς݂����ׂ�B
	auto it = m_shaderProgramMap.find(hash);
	SShaderProgram* shaderProgram;
	if (it == m_shaderProgramMap.end()) {
		//�V�K�B
		SShaderProgramPtr prog = std::make_unique<SShaderProgram>();
		//�t�@�C���ǂݍ���
		LoadShaderProgram(filePath, prog);
		//�Ԃ�
		shaderProgram = prog.get();
		//map�ɓo�^
		std::pair<int, SShaderProgramPtr> pair;
		pair.first = hash;
		pair.second = std::move(prog);
		m_shaderProgramMap.insert(std::move(pair));		
	}
	else {
		//���łɓǂݍ��ݍς݁B
		shaderProgram = it->second.get();
	}

	//�����āA�V�F�[�_�[���R���p�C���ςݒ��ׂ�B
	static char buff[1024];
	strcpy_s(buff, filePath);
	strcat_s(buff, entryFuncName);
	strcat_s(buff, definesIdentifier);
	//�t�@�C���p�X�{�G���g���[�|�C���g�̊֐����{�}�N���̎��ʖ��Ńn�b�V���l���쐬����B
	hash = Util::MakeHash(buff);
	auto itShaderResource = m_shaderResourceMap.find(hash);
	if (itShaderResource == m_shaderResourceMap.end()) {
		//�V�K�B
		SShaderResourcePtr resource = std::make_unique<SShaderResource>();

		//�R���p�C��
		if (!CompileShader(shaderProgram, filePath, pDefines, entryFuncName, shaderType, resource.get())) {
			return false;
		}

		//�߂�l
		return_resource = resource.get();

#ifndef DW_MASTER
		//�V�F�[�_�[�v���O�����̃��X�g�ɓo�^
		shaderProgram->shaderResourceList.emplace_back(resource.get());
#endif

		//�}�b�v�ɓo�^
		std::pair<int, SShaderResourcePtr> pair;
		pair.first = hash;
		pair.second = std::move(resource);
		m_shaderResourceMap.insert(std::move(pair));		 
	}
	else {
		//���łɓǂݍ��ݍς݁B
		return_resource = itShaderResource->second.get();
	}

	return true;
}

#ifndef DW_MASTER
void ShaderResources::HotReload() {
	for (auto& shader : m_shaderProgramMap) {
		//�t�@�C���̍ŏI�X�V������v���Ȃ���΍X�V
		auto file_time = std::filesystem::last_write_time(shader.second->filepath);
		if (file_time != shader.second->file_time) {
			//�V�F�[�_�[�v���O�����̍X�V
			LoadShaderProgram(shader.second->filepath.c_str(), shader.second);
			//�V�F�[�_�[�̍ăR���p�C��
			for (auto& resource : shader.second->shaderResourceList) {
				std::unique_ptr<D3D_SHADER_MACRO[]> macros;
				if (resource->macroNum > 0) {
					macros = std::make_unique<D3D_SHADER_MACRO[]>(resource->macroNum + 1);
					macros[resource->macroNum].Name = NULL; macros[resource->macroNum].Definition = NULL;
					for (int i = 0; i < resource->macroNum; i++) {
						//����炪�����ق������....
						macros[i].Name = resource->pDefines[i].Name.get();
						macros[i].Definition = resource->pDefines[i].Definition.get();
					}
				}
				CompileShader(shader.second.get(), shader.second->filepath.c_str(), resource->macroNum > 0 ? macros.get() : nullptr, resource->entryFuncName->c_str(), resource->type, resource, true);
			}
		}
	}
}
#endif

}