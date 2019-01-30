#include "DWstdafx.h"
#include "CEffekseer.h"

namespace DemolisherWeapon {
namespace GameObj {
namespace Suicider {

CEffekseer::CEffekseer()
{
}
CEffekseer::CEffekseer(const wchar_t* filePath, const float magnification, const CVector3& pos, const CQuaternion& rot, const CVector3& scale)
{
	Play(filePath, magnification,pos,rot,scale);
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

void CEffekseer::Play(const wchar_t* filePath, const float magnification, const CVector3& pos, const CQuaternion& rot, const CVector3& scale)
{
	EffekseerManager& effek = GetEngine().GetEffekseer();

	Effekseer::Effect* effect = effek.Load(filePath, magnification);
	if (!effect) { 
#ifndef DW_MASTER
		OutputDebugStringA("‚È‚ñ‚©CEffekseer::Play‚ÉŽ¸”sB\n");
#endif
		return; 
	}

	SetPRS(pos,rot,scale);

	m_handle = effek.Play(effect,m_pos);
	SetBaseMatrix();
}

void CEffekseer::Update()
{
	SetBaseMatrix();

	if (IsPlay() == false) {
		delete this;
	}
}

void CEffekseer::SetBaseMatrix(){
	CMatrix mTrans, mRot, mScale, mBase;
	mTrans.MakeTranslation(m_pos);
	mRot.MakeRotationFromQuaternion(m_rot);
	mScale.MakeScaling(m_scale);
	mBase = mScale * mRot;
	mBase = mBase * mTrans;
	GetEngine().GetEffekseer().GetManager()->SetBaseMatrix(m_handle, mBase);//SetMatrix
}

}
}
}