#include "DWstdafx.h"
#include "CInstancingModelRender.h"
#include "Graphic/Model/SkinModelShaderConst.h"

namespace DemolisherWeapon {

	GameObj::InstancingModel* InstancingModelManager::Load(
		int instanceMax,
		const wchar_t* filePath,
		AnimationClip* animationClip,
		EnFbxUpAxis fbxUpAxis,
		EnFbxCoordinateSystem fbxCoordinate,
		const wchar_t* identifier
	) {
		//キーを作成
		int modelHash, animHash, idenHash;
		modelHash = Util::MakeHash(filePath);
		if (animationClip) { animHash = Util::MakeHash(animationClip->GetName()); }else{ animHash = Util::MakeHash(L""); }
		if (identifier)    { idenHash = Util::MakeHash(identifier);               }else{ idenHash = Util::MakeHash(L""); }
		std::tuple<int, int, int> key = std::make_tuple(modelHash, animHash, idenHash);

		//既に登録されてないか?
		if (m_instancingModelMap.count(key) > 0) {
			//登録されてたらマップから取得
			return m_instancingModelMap[key];
		}
		else {
			//新規読み込み
			m_instancingModelMap[key] = new GameObj::InstancingModel(instanceMax, filePath, animationClip, fbxUpAxis, fbxCoordinate);
			return m_instancingModelMap[key];
		}
	}

namespace GameObj {

	void InstancingModel::SetInstanceMax(int instanceMax) {

		//設定最大数が現在以下なら数だけ変更
		if (m_instanceMax >= instanceMax) {
			m_instanceMax = instanceMax;
			return;
		}

		//いろいろ再確保
		Release();
		
		m_instanceMax = instanceMax;
		
		//ワールド行列の確保
		m_instancingWorldMatrix.reset(new CMatrix[m_instanceMax]);
		m_instancingWorldMatrixOld.reset(new CMatrix[m_instanceMax]);

		//StructuredBufferの確保
		D3D11_BUFFER_DESC desc;
		ZeroMemory(&desc, sizeof(desc));
		int stride = sizeof(CMatrix);
		desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;					
		desc.ByteWidth = static_cast<UINT>(stride * m_instanceMax);
		desc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
		desc.StructureByteStride = stride;
		GetEngine().GetGraphicsEngine().GetD3DDevice()->CreateBuffer(&desc, NULL, &m_worldMatrixSB);
		GetEngine().GetGraphicsEngine().GetD3DDevice()->CreateBuffer(&desc, NULL, &m_worldMatrixSBOld);
		
		//ShaderResourceViewの確保
		D3D11_SHADER_RESOURCE_VIEW_DESC descSRV;
		ZeroMemory(&descSRV, sizeof(descSRV));
		descSRV.ViewDimension = D3D11_SRV_DIMENSION_BUFFEREX;
		descSRV.BufferEx.FirstElement = 0;
		descSRV.Format = DXGI_FORMAT_UNKNOWN;
		descSRV.BufferEx.NumElements = desc.ByteWidth / desc.StructureByteStride;
		GetEngine().GetGraphicsEngine().GetD3DDevice()->CreateShaderResourceView(m_worldMatrixSB, &descSRV, &m_worldMatrixSRV);
		GetEngine().GetGraphicsEngine().GetD3DDevice()->CreateShaderResourceView(m_worldMatrixSBOld, &descSRV, &m_worldMatrixSRVOld);
	}

	void InstancingModel::PostLoopPostUpdate() {

		m_instanceNum = max(0, m_instanceNum);

		//描画インスタンス数の設定
		m_model.GetSkinModel().SetInstanceNum(m_instanceNum);

		if (m_instanceNum <= 0) { return; }		

		//StructuredBufferを更新。
		GetEngine().GetGraphicsEngine().GetD3DDeviceContext()->UpdateSubresource(
			m_worldMatrixSB, 0, NULL, m_instancingWorldMatrix.get(), 0, 0
		);
		GetEngine().GetGraphicsEngine().GetD3DDeviceContext()->UpdateSubresource(
			m_worldMatrixSBOld, 0, NULL, m_instancingWorldMatrixOld.get(), 0, 0
		);
		//ボーン行列を頂点シェーダーステージに設定。
		GetEngine().GetGraphicsEngine().GetD3DDeviceContext()->VSSetShaderResources(enSkinModelSRVReg_InstancingWorldMatrix, 1, &m_worldMatrixSRV);
		GetEngine().GetGraphicsEngine().GetD3DDeviceContext()->VSSetShaderResources(enSkinModelSRVReg_InstancingWorldMatrixOld, 1, &m_worldMatrixSRVOld);
	
		m_instanceNum = 0;
	}
	
	InstancingModelManager CInstancingModelRender::m_s_instancingModelManager;

	CInstancingModelRender::CInstancingModelRender()
	{
	}
	CInstancingModelRender::~CInstancingModelRender()
	{
	}

}
}