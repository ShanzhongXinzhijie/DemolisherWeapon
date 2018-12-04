#pragma once

namespace DemolisherWeapon {

	class KeyState
	{
	public:
		KeyState();
		~KeyState();

		void Update();
		void InLoopUpdate();

		bool GetInput(const int n)const;//���͏�Ԏ擾
		bool GetDown(const int n)const;//���̃t���[���ɉ����ꂽ�H
		bool GetUp(const int n)const;//���̃t���[���ɗ����ꂽ�H

	private:
		BYTE m_key[256] = {};
		BYTE m_keyOld[256] = {};
	};

}