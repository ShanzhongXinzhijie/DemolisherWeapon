#include "stdafx.h"

#include "CEnemy.h"
#include "CPlayer.h"

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow)
{
	//�Q�[���̏������B
	InitEngineParameter initparam;
	GetEngine().InitGame(hInstance, hPrevInstance, lpCmdLine, nCmdShow, "Game", initparam);	
	
	//���C�g�쐬
	GameObj::CDirectionLight l;
	l.SetDirection(CVector3::Down());
	l.SetColor(CVector3::One()*0.5f);

	//�J�����쐬
	GameObj::PerspectiveCamera cam;
	SetMainCamera(&cam);
	cam.SetPos(CVector3::Up()*200.0f + CVector3::AxisZ()*-400.0f);

	//�L�����N�^�[�쐬
	CPlayer plyer;
	CEnemy*  enemy = NewGO<CEnemy>();

	//�R���W������\������
	SetPhysicsDebugDrawMode(btIDebugDraw::DBG_DrawWireframe);

	//�Q�[�����[�v�B
	GetEngine().RunGameLoop();

}