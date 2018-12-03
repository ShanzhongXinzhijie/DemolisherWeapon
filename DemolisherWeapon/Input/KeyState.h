#pragma once
class KeyState
{
public:
	KeyState();
	~KeyState();

	void Update();
	void InLoopUpdate();

	bool GetInput(const int n);//入力状態取得
	bool GetDown(const int n);//このフレームに押された？
	bool GetUp(const int n);//このフレームに離された？

private:
	BYTE m_key[256] = {};
	BYTE m_keyOld[256] = {};
};

