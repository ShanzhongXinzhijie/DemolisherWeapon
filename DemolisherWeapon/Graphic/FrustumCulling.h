#pragma once

namespace DemolisherWeapon {

	class FrustumCulling
	{
	public:
		/// <summary>
		/// AABB�ɑ΂��Ď�����J�����O�e�X�g���s��
		/// </summary>
		/// <param name="camera">�J����</param>
		/// <param name="aabbMin">AABB�̍ŏ����W</param>
		/// <param name="aabbMax">AABB�̍ő���W</param>
		/// <returns>�������AABB�������Ă��邩�H</returns>
		static bool AABBTest(GameObj::ICamera* camera, const CVector3& aabbMin, const CVector3& aabbMax);
	};

}
