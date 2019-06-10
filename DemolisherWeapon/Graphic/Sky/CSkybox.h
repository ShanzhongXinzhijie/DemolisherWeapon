#pragma once

namespace DemolisherWeapon {
namespace GameObj {

	class CSkybox : public IGameObject
	{
	public:
		CSkybox() = default;
		CSkybox(const wchar_t* filePass, float size = -1.0f, bool isSetAmbientCube = true);
		~CSkybox();

		void Init(const wchar_t* filePass, float size = -1.0f, bool isSetAmbientCube = true);

		void Update()override;

	private:
		bool m_isInit = false;

		GameObj::CSkinModelRender m_skyModel;
		Shader m_psShader;
		Shader m_vsShader;
	};

}
}