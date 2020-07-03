#pragma once

namespace DemolisherWeapon::GameObj::Suicider {

	/// <summary>
	/// �V�F�[�_�[�̃v�����[�h������N���X
	/// �����[�h���I���Ǝ���
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