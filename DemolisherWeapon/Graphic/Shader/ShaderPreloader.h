#pragma once

namespace DemolisherWeapon::GameObj::Suicider {

	/// <summary>
	/// シェーダーのプリロードをするクラス
	/// ※ロードが終わると死ぬ
	/// </summary>
	class ShaderPreloader :	public IGameObject
	{
	public:
		void PreLoopUpdate()override;
		//void PostLoopUpdate()override;
		void PostRender()override;

	private:
		void PreLoadShader();

		bool m_isDrawed = false;
	};

}