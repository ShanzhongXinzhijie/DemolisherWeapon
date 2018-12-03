#pragma once
class KeyState
{
public:
	KeyState();
	~KeyState();

	void Update();
	void InLoopUpdate();

	bool GetInput(const int n);//���͏�Ԏ擾
	bool GetDown(const int n);//���̃t���[���ɉ����ꂽ�H
	bool GetUp(const int n);//���̃t���[���ɗ����ꂽ�H

private:
	BYTE m_key[256] = {};
	BYTE m_keyOld[256] = {};
};

