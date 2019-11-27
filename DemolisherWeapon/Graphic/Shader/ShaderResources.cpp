/*!
 *@brief	シェーダーリソース。
 *@ Flyweightパターンを使用。
 */

#include "DWstdafx.h"
#include "ShaderResources.h"
#include "util/Util.h"

#include <iostream>
#include <fstream>
#include <charconv>

namespace DemolisherWeapon {

namespace {
	//ファイル読み込み
	[[nodiscard]]
	std::unique_ptr<char[]> ReadFile(const char* filePath, int& fileSize)
	{
		FILE* fp = nullptr;
		errno_t err = fopen_s(&fp, filePath, "rb");
		DW_ERRORBOX(err != 0, "<ShaderResources.cpp>ReadFile\nファイルが開けませんでした。");
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
	//ファイル書き込み
	void WriteFile(std::filesystem::path filePath, const char * data, size_t dataSize)
	{
		//フォルダ作成
		std::filesystem::create_directories(filePath.parent_path());
		//ファイル開く
		std::ofstream ofs(filePath, std::ios::binary);
		if (!ofs) {
			//失敗
			return;
		}
		// バイナリとして書き込む
		ofs.write(data, dataSize);
	}
	/*!
	*@brief	頂点シェーダーから頂点レイアウトを生成。
	*/
	HRESULT CreateInputLayoutDescFromVertexShaderSignature(LPCVOID blob, SIZE_T blobSize, ID3D11Device* pD3DDevice, ID3D11InputLayout** pInputLayout)
	{
		// シェーダー情報からリフレクションを行う。
		ID3D11ShaderReflection* pVertexShaderReflection = NULL;
		if (FAILED(D3DReflect(blob, blobSize, IID_ID3D11ShaderReflection, (void**)&pVertexShaderReflection)))
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
			blob,
			blobSize,
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
	GetEngine().GetGraphicsEngine().GetD3DDevice()->CreateClassLinkage(&m_pClassLinkage);
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
	fileblob.reset(); fileblobSize = 0;

#ifndef DW_MASTER
	if (fullRelease) {
		entryFuncName.reset();
		pDefines.reset();
		macroNum = 0;
		shaderResourceHash = 0;
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

	//シェーダータイプに合わせて作成
	resource->type = shaderType;
	ID3D11Device* pD3DDevice = GetEngine().GetGraphicsEngine().GetD3DDevice();
	switch (shaderType) {
	case EnType::VS: {
		//頂点シェーダー。
		hr = pD3DDevice->CreateVertexShader(blob, blobSize, m_pClassLinkage, (ID3D11VertexShader**)&resource->shader);
		if (FAILED(hr)) {
			return false;
		}
		//入力レイアウトを作成。
		hr = CreateInputLayoutDescFromVertexShaderSignature(blob, blobSize, pD3DDevice, &resource->inputLayout);
		if (FAILED(hr)) {
			return false;
		}
	}break;
	case EnType::PS: {
		//ピクセルシェーダー。
		hr = pD3DDevice->CreatePixelShader(blob, blobSize, m_pClassLinkage, (ID3D11PixelShader**)&resource->shader);
		if (FAILED(hr)) {
			return false;
		}
	}break;
	case EnType::CS: {
		//コンピュートシェーダー。
		hr = pD3DDevice->CreateComputeShader(blob, blobSize, m_pClassLinkage, (ID3D11ComputeShader**)&resource->shader);
		if (FAILED(hr)) {
			return false;
		}
	}break;
	}

	//動的リンク関係
	{
		ID3D11ShaderReflection* pReflector = nullptr;
		hr = D3DReflect(blob, blobSize, IID_ID3D11ShaderReflection, (void**)&pReflector);
		if (FAILED(hr)) {
			if (pReflector) { pReflector->Release(); }
			return false;
		}
		//インターフェイスインスタンスの数を取得
		resource->numInterfaces = pReflector->GetNumInterfaceSlots();
		//配列確保
		if (resource->numInterfaces > 0) {
			resource->dynamicLinkageArray = (ID3D11ClassInstance**)malloc(sizeof(ID3D11ClassInstance*) * resource->numInterfaces);
		}
		if (pReflector) { pReflector->Release(); }
	}

	//返す
#ifndef DW_MASTER
	if (!isHotReload) {
		resource->entryFuncName = std::make_unique<std::string>(entryFuncName);
		if (pDefines) {
			//マクロの数カウント
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
					//文字数
					size_t size[2] = { strlen(pDefines[i].Name) + 1, strlen(pDefines[i].Definition) + 1 };
					resource->pDefines[i].Name = std::make_unique<char[]>(size[0]);
					resource->pDefines[i].Definition = std::make_unique<char[]>(size[1]);
					//コピー
					strcpy_s(resource->pDefines[i].Name.get(), size[0], pDefines[i].Name);
					strcpy_s(resource->pDefines[i].Definition.get(), size[1], pDefines[i].Definition);
				}
			}
		}
	}
#endif

	return true;
}

bool ShaderResources::CompileShader(const SShaderProgram* shaderProgram, const char* filePath, const D3D_SHADER_MACRO* pDefines, const char* entryFuncName, EnType shaderType, std::string_view hashString, SShaderResource* resource, bool isHotReload) {
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

	//コンパイル
	hr = D3DCompile(shaderProgram->program.get(), shaderProgram->programSize,
		filePath, pDefines, D3D_COMPILE_STANDARD_FILE_INCLUDE, entryFuncName,
		shaderModelNames[(int)shaderType], dwShaderFlags, 0, &blobOut, &errorBlob);

	//エラー!
	if (FAILED(hr))
	{
		if (errorBlob != nullptr) {
			static char errorMessage[10 * 1024];
			sprintf_s(errorMessage, "filePath : %s, %s", filePath, (char*)errorBlob->GetBufferPointer());
			MessageBox(NULL, errorMessage, "シェーダーコンパイルエラー", MB_OK);
		}
		if (blobOut) { blobOut->Release(); blobOut = nullptr; }
		if (errorBlob) { errorBlob->Release(); errorBlob = nullptr; }
		return false;
	}
	if (errorBlob) { errorBlob->Release(); errorBlob = nullptr; }

	//コンパイル済みシェーダー保存
	//ファイルパス作成
#ifndef DW_MASTER
	std::string path = "Preset/shader/meta/debug/";
#else
	std::string path = "Preset/shader/meta/master/";
#endif
	path += hashString;
	path += ".cso";
	//保存
	WriteFile(path.c_str(), reinterpret_cast<char*>(blobOut->GetBufferPointer()), blobOut->GetBufferSize());
	//TODO .metaももも

	//読み込み後の処理
	if (!PostLoadShader(blobOut->GetBufferPointer(), blobOut->GetBufferSize(), shaderType, entryFuncName, pDefines, isHotReload, resource)) {
		//エラー!
		blobOut->Release();
		return false;
	}

	resource->blobOut = blobOut;
	return true;
}

bool ShaderResources::LoadShaderResource(const char* filePath, const D3D_SHADER_MACRO* pDefines, const char* entryFuncName, EnType shaderType, SShaderResource* return_resource) {
	return_resource->Release(false);
	
	int filesize;
	return_resource->fileblob = ReadFile(filePath, filesize);
	return_resource->fileblobSize = filesize;
	//読み込み後の処理
	if (!PostLoadShader(return_resource->fileblob.get(), return_resource->fileblobSize, shaderType, entryFuncName, pDefines, false, return_resource)) {
		//エラー!
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
	//ファイルパスをエンジン側のものにするモード
	std::string pathstring;
	if (m_replaceForEngineFilePath) {
		std::filesystem::path path(filePath);
		path = "../../DemolisherWeapon/DemolisherWeapon/Preset/shader" / path.filename();
		pathstring = path.string();
		filePath = pathstring;
	}
#endif

	//ファイルパスからハッシュ値を作成する。
	int shaderProgramHash = Util::MakeHash(filePath.data());
	//シェーダープログラムをロード済みか調べる。
	auto it = m_shaderProgramMap.find(shaderProgramHash);
	SShaderProgram* shaderProgram;
	if (it == m_shaderProgramMap.end()) {
		//新規。
		SShaderProgramPtr prog = std::make_unique<SShaderProgram>();
		//ファイル読み込み
		LoadShaderProgram(filePath.data(), prog);
		//返す
		shaderProgram = prog.get();
		//mapに登録
		std::pair<int, SShaderProgramPtr> pair;
		pair.first = shaderProgramHash;
		pair.second = std::move(prog);
		m_shaderProgramMap.insert(std::move(pair));
	}
	else {
		//すでに読み込み済み。
		shaderProgram = it->second.get();
	}

	//ファイルパス＋エントリーポイントの関数名＋マクロの識別名でハッシュ値を作成する。
	static char buff[1024];
	strcpy_s(buff, filePath.data());
	strcat_s(buff, entryFuncName);
	strcat_s(buff, definesIdentifier);
	int shaderResourceHash = Util::MakeHash(buff);
	
	//ハッシュを10進数文字列へ変換
	auto begin = std::begin(buff);
	auto end = std::end(buff);
	auto[ptr, ec] = std::to_chars(begin, end, shaderResourceHash);
	std::string_view hashString = std::string_view(begin, ptr - begin);
	if (ec != std::errc{}) {
		//エラー!
		return false;
	}

	bool isCompiled = false;
	bool isMeta = false;

	//シェーダーがコンパイル済みか調べる
	auto itShaderResource = m_shaderResourceMap.find(shaderResourceHash);
	if (itShaderResource != m_shaderResourceMap.end()) {
		//コンパイル済み!
		isCompiled = true;
	}
	//コンパイルしてない!
	if (!isCompiled) {
		//メタファイルの読み込み
		{
			//メタファイルへのファイルパスを作成
#ifndef DW_MASTER
			std::string path("Preset/shader/meta/debug/");
#else
			std::string path("Preset/shader/meta/master/");
#endif
			path += hashString;
			path += ".dwsmeta";
			{
				//メタファイルがあるか?
				std::ifstream ifs(path);
				if (ifs) {
					//メタファイルはあります
					isMeta = true;
					
					//更新日時ロード
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
						//更新日時が一致する場合はファイルからblobを読み込む
						isMeta = true;
					}
					else {
						//更新日時が一致しない場合はファイルがないものとして扱う
						isMeta = false;
					}
				}
			}
			//メタファイルあり
			if (isMeta) {
				//ファイルパス作成
#ifndef DW_MASTER
				path = "Preset/shader/meta/debug/";
#else
				path = "Preset/shader/meta/master/";
#endif
				path += hashString;
				path += ".cso";

				//シェーダー読み込み
				SShaderResourcePtr resource = std::make_unique<SShaderResource>();
				if (!LoadShaderResource(path.c_str(), pDefines, entryFuncName, shaderType, resource.get())) {
					return false;
				}
				//戻り値
				return_resource = resource.get();
#ifndef DW_MASTER
				//シェーダープログラムのリストに登録
				shaderProgram->shaderResourceList.emplace_back(resource.get());
#endif
				//マップに登録
				std::pair<int, SShaderResourcePtr> pair;
				pair.first = shaderResourceHash;
				pair.second = std::move(resource);
				m_shaderResourceMap.insert(std::move(pair));
				//コンパイル済みにする
				isCompiled = true;
			}
		}		
	}	

	//シェーダーがコンパイル済みでなければコンパイル	
	if (!isCompiled) {
		//新規。
		SShaderResourcePtr resource = std::make_unique<SShaderResource>();

		//コンパイル
		if (!CompileShader(shaderProgram, filePath.data(), pDefines, entryFuncName, shaderType, hashString, resource.get())) {
			return false;
		}

#ifndef DW_MASTER
		resource->shaderResourceHash = shaderResourceHash;
#endif

		//戻り値
		return_resource = resource.get();

#ifndef DW_MASTER
		//シェーダープログラムのリストに登録
		shaderProgram->shaderResourceList.emplace_back(resource.get());
#endif

		//マップに登録
		std::pair<int, SShaderResourcePtr> pair;
		pair.first = shaderResourceHash;
		pair.second = std::move(resource);
		m_shaderResourceMap.insert(std::move(pair));		 
	}
	else if (itShaderResource != m_shaderResourceMap.end()) {
		//すでに読み込み済み。
		return_resource = itShaderResource->second.get();
	}

	return true;
}

#ifndef DW_MASTER
void ShaderResources::HotReload() {
	for (auto& shader : m_shaderProgramMap) {
		//ファイルの最終更新日が一致しなければ更新
		auto file_time = std::filesystem::last_write_time(shader.second->filepath);
		if (file_time != shader.second->file_time) {
			//シェーダープログラムの更新
			LoadShaderProgram(shader.second->filepath.c_str(), shader.second);
			//シェーダーの再コンパイル
			for (auto& resource : shader.second->shaderResourceList) {
				//マクロ
				std::unique_ptr<D3D_SHADER_MACRO[]> macros;
				if (resource->macroNum > 0) {
					macros = std::make_unique<D3D_SHADER_MACRO[]>(resource->macroNum + 1);
					macros[resource->macroNum].Name = NULL; macros[resource->macroNum].Definition = NULL;
					for (int i = 0; i < resource->macroNum; i++) {
						macros[i].Name = resource->pDefines[i].Name.get();
						macros[i].Definition = resource->pDefines[i].Definition.get();
					}
				}
				//ハッシュを10進数文字列へ変換
				static char buff[24];
				auto begin = std::begin(buff);
				auto end = std::end(buff);
				auto[ptr, ec] = std::to_chars(begin, end, resource->shaderResourceHash);
				if (ec != std::errc{}) {
					//エラー!
					MessageBox(NULL, "ハッシュの文字列変換に失敗しました。", "HotReloadエラー", MB_OK);
					continue;
				}
				//コンパイル
				bool ok = CompileShader(shader.second.get(), shader.second->filepath.c_str(), resource->macroNum > 0 ? macros.get() : nullptr, resource->entryFuncName->c_str(), resource->type, std::string_view(begin, ptr - begin), resource, true);
				if (!ok) {
					//ERR!
					MessageBox(NULL, "シェーダーの再コンパイルに失敗しました。", "CompileShaderエラー", MB_OK);
					continue;
				}
			}
		}
	}
}
#endif

}