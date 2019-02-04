#include "DWstdafx.h"
#include "PhysicsDebugDraw.h"

namespace DemolisherWeapon {

	PhysicsDebugDraw::~PhysicsDebugDraw() {
		Release();
	}

	void PhysicsDebugDraw::Init()
	{

		//ラインストリップのテスト。
		m_primitive.Init(D3D_PRIMITIVE_TOPOLOGY_LINELIST, static_cast<int>(m_vertexBuffer.size()), &m_vertexBuffer[0]);
		
		m_vs.Load("Preset/shader/linePrimitive.fx", "VSMain", Shader::EnType::VS);
		m_ps.Load("Preset/shader/linePrimitive.fx", "PSMain", Shader::EnType::PS);

		//定数バッファ
		int bufferSize = sizeof(SConstantBuffer);
		D3D11_BUFFER_DESC bufferDesc;
		ZeroMemory(&bufferDesc, sizeof(bufferDesc));
		bufferDesc.Usage = D3D11_USAGE_DEFAULT;
		bufferDesc.ByteWidth = (((bufferSize - 1) / 16) + 1) * 16;
		bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		GetEngine().GetGraphicsEngine().GetD3DDevice()->CreateBuffer(&bufferDesc, NULL, &m_cb);
	}
	void PhysicsDebugDraw::Release() {
		if (m_cb) { m_cb->Release(); m_cb = nullptr; }
	}
	void PhysicsDebugDraw::drawLine(const btVector3& from, const btVector3& to, const btVector3& color)
	{
		int baseIndex = m_numLine * 2;
		m_vertexBuffer[baseIndex].position[0] = from.x();
		m_vertexBuffer[baseIndex].position[1] = from.y();
		m_vertexBuffer[baseIndex].position[2] = from.z();
		m_vertexBuffer[baseIndex].position[3] = 1.0f;
		m_vertexBuffer[baseIndex + 1].position[0] = to.x();
		m_vertexBuffer[baseIndex + 1].position[1] = to.y();
		m_vertexBuffer[baseIndex + 1].position[2] = to.z();
		m_vertexBuffer[baseIndex + 1].position[3] = 1.0f;
		m_numLine++;
	}
	void PhysicsDebugDraw::EndDraw()
	{
		ID3D11DeviceContext* rc = GetEngine().GetGraphicsEngine().GetD3DDeviceContext();

#ifndef DW_MASTER
		if (!GetMainCamera()) {
			MessageBox(NULL, "カメラが設定されていません!!", "Error", MB_OK);
			std::abort();
		}
#endif

		//定数バッファの更新。
		SConstantBuffer cb;
		cb.mView = GetMainCamera()->GetViewMatrix();
		cb.mProj = GetMainCamera()->GetProjMatrix();
		rc->UpdateSubresource(m_cb, 0, nullptr, &cb, 0, 0);
		rc->VSSetConstantBuffers(0, 1, &m_cb);

		//シェーダーセット
		rc->VSSetShader((ID3D11VertexShader*)m_vs.GetBody(), NULL, 0);
		rc->PSSetShader((ID3D11PixelShader*)m_ps.GetBody(), NULL, 0);

		//入力レイアウトを設定。
		rc->IASetInputLayout(m_vs.GetInputLayout());

		//頂点をセット
		rc->UpdateSubresource(m_primitive.GetVertexBuffer(), 0, nullptr, &m_vertexBuffer[0], 0, 0);

		m_primitive.Draw(m_numLine * 2);
	}

}