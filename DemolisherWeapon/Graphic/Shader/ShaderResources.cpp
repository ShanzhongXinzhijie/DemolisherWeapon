/*!
 *@brief	シェーダーリソース。
 *@ Flyweightパターンを使用。
 */

#include "DWstdafx.h"
#include "ShaderResources.h"
#include "Shader.h"
#include "util/Util.h"

namespace DemolisherWeapon {

namespace {
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
	*@brief	頂点シェーダーから頂点レイアウトを生成。
	*/
	HRESULT CreateInputLayoutDescFromVertexShaderSignature(ID3DBlob* pShaderBlob, ID3D11Device* pD3DDevice, ID3D11InputLayout** pInputLayout)
	{
		// シェーダー情報からリフレクションを行う。
		ID3D11ShaderReflection* pVertexShaderReflection = NULL;
		if (FAILED(D3DReflect(pShaderBlob->GetBufferPointer(), pShaderBlob->GetBufferSize(), IID_ID3D11ShaderReflection, (void**)&pVertexShaderReflection)))
		{
			return S_FALSE;
		}

		// シェーダー情報を取得。
		D3D11_SHADER_DESC shaderDesc;
		pVertexShaderReflection->GetDesc(&shaderDesc);

		// 入力情報定義を読み込み
		std::vector<D3D11_INPUT_ELEMENT_DESC> inputLayoutDesc;
		for (unsigned int i = 0; i< shaderDesc.InputParameters; i++)
		{
			D3D11_SIGNATURE_PARAMETER_DESC paramDesc;
			pVertexShaderReflection->GetInputParameterDesc(i, &paramDesc);

			// エレメント定義を設定。
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

			//エレメントの定義を保存。
			inputLayoutDesc.push_back(elementDesc);
		}

		// 入力レイアウトを作成。
		HRESULT hr = pD3DDevice->CreateInputLayout(
			&inputLayoutDesc[0],
			static_cast<UINT>(inputLayoutDesc.size()),
			pShaderBlob->GetBufferPointer(),
			pShaderBlob->GetBufferSize(),
			pInputLayout);

		//リフレクション用に確保したメモリを解放。
		pVertexShaderReflection->Release();
		return hr;
	}
}

/*!
 *@brief	コンストラクタ。
 */
ShaderResources::ShaderResources()
{
	GetEngine().GetGraphicsEngine().GetD3DDevice()->CreateClassLinkage(&pClassLinkage);
}
/*!
 *@brief	デストラクタ。
 */
ShaderResources::~ShaderResources()
{
	Release();
}
void ShaderResources::Release()
{
	for (auto it = m_shaderResourceMap.begin(); it != m_shaderResourceMap.end(); it++) {
		switch (it->second->type) {
		case Shader::EnType::VS:
			((ID3D11VertexShader*)it->second->shader)->Release();
			break;
		case Shader::EnType::PS:
			((ID3D11PixelShader*)it->second->shader)->Release();
			break;
		case Shader::EnType::CS:
			((ID3D11ComputeShader*)it->second->shader)->Release();
			break;
		}
		if (it->second->inputLayout) {
			it->second->inputLayout->Release();
		}
		it->second->blobOut->Release();

		it->second->numInterfaces = 0;
		free(it->second->dynamicLinkageArray);
	}
	m_shaderResourceMap.clear();
	m_shaderProgramMap.clear();

	pClassLinkage->Release();
}
bool ShaderResources::Load(
	void*& shader,
	ID3D11InputLayout*& inputLayout,
	ID3DBlob*& blob,
	const char* filePath, 
	const char* entryFuncName,
	Shader::EnType shaderType,
	int& numInterfaces,
	ID3D11ClassInstance**& dynamicLinkageArray
)
{
	//ファイルパスからハッシュ値を作成する。
	int hash = Util::MakeHash(filePath);
	//シェーダープログラムをロード済みか調べる。
	auto it = m_shaderProgramMap.find(hash);
	SShaderProgram* shaderProgram;
	if (it == m_shaderProgramMap.end()) {
		//新規。
		SShaderProgramPtr prog = std::make_unique<SShaderProgram>();
		prog->program = ReadFile(filePath, prog->programSize);
		shaderProgram = prog.get();
		std::pair<int, SShaderProgramPtr> pair;
		pair.first = hash;
		pair.second = std::move(prog);
		m_shaderProgramMap.insert(std::move(pair));
		
	}
	else {
		//すでに読み込み済み。
		shaderProgram = it->second.get();
	}

	//続いて、シェーダーをコンパイル済み調べる。
	static char buff[1024];
	strcpy_s(buff, filePath);
	strcat_s(buff, entryFuncName );
	//ファイルパス＋エントリーポイントの関数名でハッシュ値を作成する。
	hash = Util::MakeHash(buff);
	auto itShaderResource = m_shaderResourceMap.find(hash);
	if (itShaderResource == m_shaderResourceMap.end()) {
		//新規。
		HRESULT hr = S_OK;

		DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS | D3DCOMPILE_OPTIMIZATION_LEVEL3;
		/*if (strcmp(filePath, "Resource/shader/bloom.fx") == 0
		|| strcmp(filePath, "Resource/shader/model.fx") == 0) {
		dwShaderFlags |= D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
		}*/
#if BUILD_LEVEL == BUILD_LEVEL_DEBUG
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

		hr = D3DCompile(shaderProgram->program.get(), shaderProgram->programSize, filePath, nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, entryFuncName,
			shaderModelNames[(int)shaderType], dwShaderFlags, 0, &blobOut, &errorBlob);
		
		if (FAILED(hr))
		{
			if (errorBlob != nullptr) {
				static char errorMessage[10 * 1024];
				sprintf_s(errorMessage, "filePath : %s, %s", filePath, (char*)errorBlob->GetBufferPointer());
				MessageBox(NULL, errorMessage, "シェーダーコンパイルエラー", MB_OK);
			}
			return false;
		}
		SShaderResourcePtr resource = std::make_unique<SShaderResource>();
		resource->inputLayout = nullptr;
		resource->type = shaderType;
		ID3D11Device* pD3DDevice = GetEngine().GetGraphicsEngine().GetD3DDevice();
		switch (shaderType) {
		case Shader::EnType::VS: {
			//頂点シェーダー。
			hr = pD3DDevice->CreateVertexShader(blobOut->GetBufferPointer(), blobOut->GetBufferSize(), pClassLinkage, (ID3D11VertexShader**)&resource->shader);
			if (FAILED(hr)) {
				return false;
			}
			//入力レイアウトを作成。
			hr = CreateInputLayoutDescFromVertexShaderSignature(blobOut, pD3DDevice, &resource->inputLayout);
			if (FAILED(hr)) {
				//入力レイアウトの作成に失敗した。
				return false;
			}
		}break;
		case Shader::EnType::PS: {
			//ピクセルシェーダー。
			hr = pD3DDevice->CreatePixelShader(blobOut->GetBufferPointer(), blobOut->GetBufferSize(), pClassLinkage, (ID3D11PixelShader**)&resource->shader);
			if (FAILED(hr)) {
				return false;
			}
		}break;
		case Shader::EnType::CS: {
			//コンピュートシェーダー。
			hr = pD3DDevice->CreateComputeShader(blobOut->GetBufferPointer(), blobOut->GetBufferSize(), pClassLinkage, (ID3D11ComputeShader**)&resource->shader);
			if (FAILED(hr)) {
				return false;
			}
		}break;
		}

		//動的リンク関係
		{
			ID3D11ShaderReflection* pReflector = nullptr;
			D3DReflect(blobOut->GetBufferPointer(), blobOut->GetBufferSize(), IID_ID3D11ShaderReflection, (void**)&pReflector);
			//インターフェイスインスタンスの数を取得
			resource->numInterfaces = pReflector->GetNumInterfaceSlots();
			//配列確保
			if (resource->numInterfaces > 0) {
				resource->dynamicLinkageArray = (ID3D11ClassInstance**)malloc(sizeof(ID3D11ClassInstance*) * resource->numInterfaces);
			}
			pReflector->Release();
		}

		resource->blobOut = blobOut;
		shader = resource->shader;
		inputLayout = resource->inputLayout;
		blob = blobOut;

		numInterfaces = resource->numInterfaces;
		dynamicLinkageArray = resource->dynamicLinkageArray;

		std::pair<int, SShaderResourcePtr> pair;
		pair.first = hash;
		pair.second = std::move(resource);
		m_shaderResourceMap.insert(std::move(pair));		 
	}
	else {
		//すでに読み込み済み。
		shader = itShaderResource->second->shader;
		inputLayout = itShaderResource->second->inputLayout;
		blob = itShaderResource->second->blobOut;

		numInterfaces = itShaderResource->second->numInterfaces;
		dynamicLinkageArray = itShaderResource->second->dynamicLinkageArray;
	}
	return true;
}

}