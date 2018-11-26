#include "DWstdafx.h"
#include "CEffekseer.h"

namespace DemolisherWeapon {
namespace GameObj {
namespace Suicider {

CEffekseer::CEffekseer()
{
}
CEffekseer::CEffekseer(const wchar_t* filePath, const CVector3& pos, const CQuaternion& rot, const CVector3& scale) 
{
	Play(filePath,pos,rot,scale);
}

CEffekseer::~CEffekseer()
{
	Release();
}

void CEffekseer::Release()
{
	if (m_handle != -1) {
		GetEngine().GetEffekseer().Stop(m_handle);
		m_handle = -1;
	}
}

void CEffekseer::Play(const wchar_t* filePath, const CVector3& pos, const CQuaternion& rot, const CVector3& scale)
{
	EffekseerManager& effek = GetEngine().GetEffekseer();

	Effekseer::Effect* effect = effek.Load(filePath);
	if (!effect) { 
#ifndef DW_MASTER
		OutputDebugStringA("�Ȃ�CEffekseer::Play�Ɏ��s�B\n");
#endif
		return; 
	}

	SetPRS(pos,rot,scale);

	m_handle = effek.Play(effect,m_pos);
}

void CEffekseer::Update()
{
	CMatrix mTrans, mRot, mScale, mBase;
	mTrans.MakeTranslation(m_pos);
	mRot.MakeRotationFromQuaternion(m_rot);
	mScale.MakeScaling(m_scale);
	mBase = mScale * mRot;
	mBase = mBase * mTrans;
	GetEngine().GetEffekseer().GetManager()->SetMatrix(m_handle, mBase);//SetBaseMatrix
	
	if (IsPlay() == false) {
		delete this;
	}
}

}
}
}