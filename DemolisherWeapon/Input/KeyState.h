#pragma once
class KeyState
{
public:
	KeyState();
	~KeyState();

	void Update();

	bool GetInput(const int n);

private:
	BYTE m_key[256];

};

