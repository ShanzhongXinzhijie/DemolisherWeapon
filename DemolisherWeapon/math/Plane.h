#pragma once

namespace DemolisherWeapon {

	//���ʂ�\���N���X
	class Plane
	{
	public:
		//�v�f
		CVector3 m_normal;	//�@��
		CVector3 m_position;//���ʏ�̈�_

	public:
		Plane() = default;
		~Plane() = default;//�f�X�g���N�^�̓f�t�H���g��noexcept

		constexpr Plane(const CVector3& normal, const CVector3& position) 
			: m_normal(normal), m_position(position){}
	};

}