#pragma once

namespace DemolisherWeapon {

	class KeyState
	{
	public:
		KeyState();
		~KeyState();

		void Update();
		void InLoopUpdate();

		bool GetInput(const int n)const;//入力状態取得
		bool GetDown(const int n)const;//このフレームに押された？
		bool GetUp(const int n)const;//このフレームに離された？

	private:
		BYTE m_key[256] = {};
		BYTE m_keyOld[256] = {};
	};

}