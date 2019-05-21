#pragma once

namespace DemolisherWeapon {

	class ShaderUtil
	{
	public:
		/// <summary>
		/// �萔�o�b�t�@�̍쐬
		/// �T�C�Y��16�o�C�g�̔{���ɂȂ�悤�Ɋm�ۂ����B
		/// ���Ȃ��A�f�[�^��16�o�C�g���E���ׂ��ł͂Ȃ�Ȃ��B
		/// https://www.3dgep.com/texturing-lighting-directx-11/#Packing_Rules_for_Constant_Buffers
		/// </summary>
		/// <param name="size">�萔�o�b�t�@�̃T�C�Y</param>
		/// <param name="buffer">�萔�o�b�t�@�̃|�C���^���Ԃ��Ă���</param>
		static void CreateConstantBuffer(int size, ID3D11Buffer** buffer);
	};

}