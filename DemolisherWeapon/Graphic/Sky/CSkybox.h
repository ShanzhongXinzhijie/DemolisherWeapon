#pragma once

namespace DemolisherWeapon {
namespace GameObj {

	class CSkybox : public IGameObject
	{
	public:
		CSkybox(const wchar_t* filePass, float size = -1.0f);
		~CSkybox();

		void Update()override;

	private:
		GameObj::CSkinModelRender m_skyModel;
		Shader m_psShader;
	};

}
}