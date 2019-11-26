/*!
 *@brief	�V�F�[�_�[���\�[�X�B
 *@ Flyweight�p�^�[�����g�p�B
 */

#include "DWstdafx.h"
#include "ShaderResources.h"
#include "util/Util.h"

#include <iostream>
#include <fstream>
#include <charconv>

namespace DemolisherWeapon {

namespace {
	//�t�@�C���ǂݍ���
	[[nodiscard]]
	std::unique_ptr<char[]> ReadFile(const char* filePath, int& fileSize)
	{
		FILE* fp = nullptr;
		errno_t err = fopen_s(&fp, filePath, "rb");
		DW_ERRORBOX(err != 0, "<ShaderResources.cpp>ReadFile\n�t�@�C�����J���܂���ł����B");
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
	HRESULT CreateInputLayoutDescFromVertexShaderSignature(LPCVOID blob, SIZE_T blobSize, ID3D11Device* pD3DDevice, ID3D11InputLayout** pInputLayout)
	{
		// �V�F�[�_�[��񂩂烊�t���N�V�������s���B
		ID3D11ShaderReflection* pVertexShaderReflection = NULL;
		if (FAILED(D3DReflect(blob, blobSize, IID_ID3D11ShaderReflection, (void**)&pVertexShaderReflection)))
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
			blob,
			blobSize,
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
		macroNum = 0;
	}
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

bool ShaderResources::PostLoadShader(void* blob, size_t blobSize, EnType shaderType, const char* entryFuncName, const D3D_SHADER_MACRO* pDefines, bool isHotReload, SShaderResource* resource) {
	HRESULT hr = S_OK;

	//�V�F�[�_�[�^�C�v�ɍ��킹�č쐬
	resource->type = shaderType;
	ID3D11Device* pD3DDevice = GetEngine().GetGraphicsEngine().GetD3DDevice();
	switch (shaderType) {
	case EnType::VS: {
		//���_�V�F�[�_�[�B
		hr = pD3DDevice->CreateVertexShader(blob, blobSize, m_pClassLinkage, (ID3D11VertexShader**)&resource->shader);
		if (FAILED(hr)) {
			return false;
		}
		//���̓��C�A�E�g���쐬�B
		hr = CreateInputLayoutDescFromVertexShaderSignature(blob, blobSize, pD3DDevice, &resource->inputLayout);
		if (FAILED(hr)) {
			return false;
		}
	}break;
	case EnType::PS: {
		//�s�N�Z���V�F�[�_�[�B
		hr = pD3DDevice->CreatePixelShader(blob, blobSize, m_pClassLinkage, (ID3D11PixelShader**)&resource->shader);
		if (FAILED(hr)) {
			return false;
		}
	}break;
	case EnType::CS: {
		//�R���s���[�g�V�F�[�_�[�B
		hr = pD3DDevice->CreateComputeShader(blob, blobSize, m_pClassLinkage, (ID3D11ComputeShader**)&resource->shader);
		if (FAILED(hr)) {
			return false;
		}
	}break;
	}

	//���I�����N�֌W
	{
		ID3D11ShaderReflection* pReflector = nullptr;
		hr = D3DReflect(blob, blobSize, IID_ID3D11ShaderReflection, (void**)&pReflector);
		if (FAILED(hr)) {
			if (pReflector) { pReflector->Release(); }
			return false;
		}
		//�C���^�[�t�F�C�X�C���X�^���X�̐����擾
		resource->numInterfaces = pReflector->GetNumInterfaceSlots();
		//�z��m��
		if (resource->numInterfaces > 0) {
			resource->dynamicLinkageArray = (ID3D11ClassInstance**)malloc(sizeof(ID3D11ClassInstance*) * resource->numInterfaces);
		}
		if (pReflector) { pReflector->Release(); }
	}

	//�Ԃ�
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
}

bool ShaderResources::CompileShader(const SShaderProgram* shaderProgram, const char* filePath, const D3D_SHADER_MACRO* pDefines, const char* entryFuncName, EnType shaderType, SShaderResource* resource, bool isHotReload) {
	resource->Release(!isHotReload);
	
	DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS | D3DCOMPILE_OPTIMIZATION_LEVEL3;
#ifndef DW_MASTER
	dwShaderFlags |= D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

	static const char* shaderModelNames[] = {
		"vs_5_0",
		"ps_5_0",
		"cs_5_0"
	};
	ID3DBlob* blobOut = nullptr;
	ID3DBlob* errorBlob = nullptr;

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
		if (blobOut) { blobOut->Release(); blobOut = nullptr; }
		if (errorBlob) { errorBlob->Release(); errorBlob = nullptr; }
		return false;
	}
	if (errorBlob) { errorBlob->Release(); errorBlob = nullptr; }

	//�ǂݍ��݌�̏���
	if (!PostLoadShader(blobOut->GetBufferPointer(), blobOut->GetBufferSize(), shaderType, entryFuncName, pDefines, isHotReload, resource)) {
		//�G���[!
		blobOut->Release();
		return false;
	}

	resource->blobOut = blobOut;
	return true;
}

bool ShaderResources::LoadShaderResource(const char* filePath, const D3D_SHADER_MACRO* pDefines, const char* entryFuncName, EnType shaderType, SShaderResource* return_resource) {
	int filesize;
	return_resource->fileblob = ReadFile(filePath, filesize);
	return_resource->fileblobSize = filesize;
	//�ǂݍ��݌�̏���
	if (!PostLoadShader(return_resource->fileblob.get(), return_resource->fileblobSize, shaderType, entryFuncName, pDefines, false, return_resource)) {
		//�G���[!
		return_resource->fileblob.reset();
		return_resource->fileblobSize = 0;
		return false;
	}
	return true;
}

bool ShaderResources::Load(
	const ShaderResources::SShaderResource*& return_resource,
	std::string_view filePath, 
	const char* entryFuncName,
	EnType shaderType,
	const char* definesIdentifier,
	const D3D_SHADER_MACRO* pDefines
)
{
#ifndef DW_MASTER
	//�t�@�C���p�X���G���W�����̂��̂ɂ��郂�[�h
	std::string pathstring;
	if (m_replaceForEngineFilePath) {
		std::filesystem::path path(filePath);
		path = "../../DemolisherWeapon/DemolisherWeapon/Preset/shader" / path.filename();
		pathstring = path.string();
		filePath = pathstring;
	}
#endif

	//�t�@�C���p�X�{�G���g���[�|�C���g�̊֐����{�}�N���̎��ʖ��Ńn�b�V���l���쐬����B
	static char buff[1024];
	strcpy_s(buff, filePath.data());
	strcat_s(buff, entryFuncName);
	strcat_s(buff, definesIdentifier);
	int shaderResourceHash = Util::MakeHash(buff);

	//�V�F�[�_�[���R���p�C���ς݂����ׂ�
	bool isCompiled = false;
	auto itShaderResource = m_shaderResourceMap.find(shaderResourceHash);
	if (itShaderResource != m_shaderResourceMap.end()) {
		//�R���p�C���ς�!
		isCompiled = true;
	}
	//�R���p�C�����ĂȂ�!
	if (!isCompiled) {
		//���^�t�@�C���̓ǂݍ���
		bool isMeta = false;
		//�n�b�V����10�i��������֕ϊ�
		auto begin = std::begin(buff);
		auto end = std::end(buff);
		if (auto[ptr, ec] = std::to_chars(begin, end, shaderResourceHash); ec == std::errc{}) {
			//���^�t�@�C���ւ̃t�@�C���p�X���쐬
#ifndef DW_MASTER
			std::string path("Preset/shader/meta/debug");
#else
			std::string path("Preset/shader/meta/master");
#endif
			path += std::string_view(begin, ptr - begin);
			path += ".dwsmeta";
			{
				//���^�t�@�C�������邩?
				std::ifstream ifs(path);
				if (ifs) {
					//���^�t�@�C���͂���܂�
					isMeta = true;
					
					//�X�V�������[�h
					bool isLoad = false;
					std::string lastwritetimeString;
					long long lastwritetime = 0;
					while (!ifs.eof())
					{
						std::getline(ifs, lastwritetimeString);
						lastwritetime = std::stoll(lastwritetimeString);
						isLoad = true;
						break;
					}

					if (isLoad && lastwritetime == std::chrono::duration_cast<std::chrono::seconds>(std::filesystem::last_write_time(filePath).time_since_epoch()).count()) {
						//�X�V��������v����ꍇ�̓t�@�C������blob��ǂݍ���
						isMeta = true;
					}
					else {
						//�X�V��������v���Ȃ��ꍇ�̓t�@�C�����Ȃ����̂Ƃ��Ĉ���
						isMeta = false;
					}
				}
			}
			//���^�t�@�C������
			if (isMeta) {
				//�t�@�C���p�X�쐬
#ifndef DW_MASTER
				path = "Preset/shader/meta/debug";
#else
				path = "Preset/shader/meta/master";
#endif
				path += std::string_view(begin, ptr - begin);
				path += ".cso";

				//�V�F�[�_�[�ǂݍ���

			}
		}
		
		//���^�t�@�C���Ȃ�
		if (!isMeta) {
			//�ʏ�̃��[�g
			//TODO �ۑ�����
		}
	}

	//�t�@�C���p�X����n�b�V���l���쐬����B
	int shaderProgramHash = Util::MakeHash(filePath.data());
	//�V�F�[�_�[�v���O���������[�h�ς݂����ׂ�B
	auto it = m_shaderProgramMap.find(shaderProgramHash);
	SShaderProgram* shaderProgram;
	if (it == m_shaderProgramMap.end()) {
		//�V�K�B
		SShaderProgramPtr prog = std::make_unique<SShaderProgram>();
		//�t�@�C���ǂݍ���
		LoadShaderProgram(filePath.data(), prog);
		//�Ԃ�
		shaderProgram = prog.get();
		//map�ɓo�^
		std::pair<int, SShaderProgramPtr> pair;
		pair.first = shaderProgramHash;
		pair.second = std::move(prog);
		m_shaderProgramMap.insert(std::move(pair));		
	}
	else {
		//���łɓǂݍ��ݍς݁B
		shaderProgram = it->second.get();
	}

	//�����āA�V�F�[�_�[���R���p�C���ςݒ��ׂ�B	
	//auto itShaderResource = m_shaderResourceMap.find(shaderResourceHash);
	if (itShaderResource == m_shaderResourceMap.end()) {
		//�V�K�B
		SShaderResourcePtr resource = std::make_unique<SShaderResource>();

		//�R���p�C��
		if (!CompileShader(shaderProgram, filePath.data(), pDefines, entryFuncName, shaderType, resource.get())) {
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
		pair.first = shaderResourceHash;
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
				//�}�N��
				std::unique_ptr<D3D_SHADER_MACRO[]> macros;
				if (resource->macroNum > 0) {
					macros = std::make_unique<D3D_SHADER_MACRO[]>(resource->macroNum + 1);
					macros[resource->macroNum].Name = NULL; macros[resource->macroNum].Definition = NULL;
					for (int i = 0; i < resource->macroNum; i++) {
						macros[i].Name = resource->pDefines[i].Name.get();
						macros[i].Definition = resource->pDefines[i].Definition.get();
					}
				}
				//�R���p�C��
				CompileShader(shader.second.get(), shader.second->filepath.c_str(), resource->macroNum > 0 ? macros.get() : nullptr, resource->entryFuncName->c_str(), resource->type, resource, true);
			}
		}
	}
}
#endif

}