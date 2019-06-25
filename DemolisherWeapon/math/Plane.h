#pragma once

namespace DemolisherWeapon {

	//平面を表すクラス
	class Plane
	{
	public:
		//要素
		CVector3 m_normal;	//法線
		CVector3 m_position;//平面上の一点

	public:
		Plane() = default;
		~Plane() = default;//デストラクタはデフォルトでnoexcept

		constexpr Plane(const CVector3& normal, const CVector3& position) 
			: m_normal(normal), m_position(position){}
	};

}