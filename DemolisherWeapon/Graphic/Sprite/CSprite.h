#pragma once

#include <filesystem>

namespace DemolisherWeapon {

	class CSprite
	{
	public:
		CSprite() = default;
		~CSprite();

		void Release();

		//������
		void Init(const wchar_t* fileName);
		void Init(const TextueData& texture);
		void Init(ID3D11ShaderResourceView* srv, UINT width, UINT height, bool isPMA = false);

		/// <summary>
		/// ���f������X�v���C�g�̍쐬
		/// </summary>
		/// <param name="identifier">���ʖ�</param>
		/// <param name="model">3d���f��</param>
		/// <param name="resolution">�e�N�X�`���̉𑜓x</param>
		/// <param name="rotOffset">��]�I�t�Z�b�g</param>
		void Init(const wchar_t* identifier, SkinModel& model, UINT resolusuon, const CQuaternion& rotOffset = CQuaternion::Identity());

		//�`�悷��
		//��layerDepth�̑�O�����_�ȉ��͎g��Ȃ��ق��������Ǝv��
		//const CVector2& pos ��0.0f�`1.0f�͈̔́B{0.0f,0.0f}�ŉ�ʍ���
		void Draw(const CVector2& pos,
			const CVector2& scale = CVector2::One(),
			const CVector2& pivot = CVector2::Zero(),
			float rotation = 0.0f,
			const CVector4& color = CVector4::White(),
			DirectX::SpriteEffects effects = DirectX::SpriteEffects_None,
			float layerDepth = 0.5f);
		//const CVector2& pos ���X�N���[�����W�Ŏw�肷��ŁB�E���̍��W=��ʉ𑜓x�@
		void DrawScreenPos(const CVector2& pos,
			const CVector2& scale = CVector2::One(),
			const CVector2& pivot = CVector2::Zero(),
			float rotation = 0.0f,
			const CVector4& color = CVector4::White(),
			DirectX::SpriteEffects effects = DirectX::SpriteEffects_None,
			float layerDepth = 0.5f);

		//�摜�̕`�悷��͈͂�ݒ�
		//0.0f�`1.0f�͈̔͂Ŏw��
		void SetSourceRectangle(const float left, const float top, const float right, const float bottom) {
			m_sourceRectangle.left		= (LONG)(m_texdata.width  * left);
			m_sourceRectangle.right		= (LONG)(m_texdata.width  * right);
			m_sourceRectangle.top		= (LONG)(m_texdata.height * top);
			m_sourceRectangle.bottom	= (LONG)(m_texdata.height * bottom);
		}
		//�摜�̉𑜓x�Ŏw��
		void SetSourceRectangleSpriteSize(const RECT& sourceRectangle){
			m_sourceRectangle = sourceRectangle;
		}

		//�摜�̕����擾
		UINT GetWidth()const {
			return m_texdata.width;
		}
		//�摜�̍������擾
		UINT GetHeight()const {
			return m_texdata.height;
		}

		//�摜�f�[�^���擾
		const TextueData& GetTextueData()const {
			return m_texdata;
		}

	private:
		DirectX::SpriteBatch* m_spriteBatch = nullptr;

		TextueData m_texdata;

		D3D12_CPU_DESCRIPTOR_HANDLE m_cpuHandle;
		D3D12_GPU_DESCRIPTOR_HANDLE m_gpuHandle;

		RECT m_sourceRectangle;
	};

	namespace GameObj {
		class CSpriteRender : public IGameObject
		{
		public:
			void Init(const wchar_t* fileName) {
				m_sprite.Init(fileName);
			}
			void Init(const wchar_t* identifier, SkinModel& model, UINT resolusuon, const CQuaternion& rotOffset = CQuaternion::Identity())
			{
				m_sprite.Init(identifier, model, resolusuon, rotOffset);
			}

			//�X�e�[�^�X�ݒ�
			void SetPos(const CVector2& pos) {
				m_pos = pos;
			}
			void SetScale(const CVector2& scale) {
				m_scale = scale;
			}
			void SetPivot(const CVector2& pivot) {
				m_pivot = pivot;
			}
			void SetRot(float rot) {
				m_rotation = rot;
			}
			void SetColor(const CVector4& color) {
				m_color = color;
			}
			//�X�e�[�^�X�擾
			const CVector2& GetPos()const {
				return m_pos;
			}
			const CVector2& GetScale()const {
				return m_scale;
			}
			const CVector2& GetPivot()const {
				return m_pivot;
			}
			float GetRot() const {
				return m_rotation;
			}
			const CVector4& GetColor() const {
				return m_color;
			}

			//�G�t�F�N�g��ݒ�
			void SetSpriteEffects(DirectX::SpriteEffects effects) {
				m_effects = effects;
			}
			DirectX::SpriteEffects GetSpriteEffects()const {
				return m_effects;
			}

			//�`��[�x��ݒ�
			void SetLayerDepth(float depth) {
				m_layerDepth = depth;
			}
			float GetLayerDepth()const {
				return m_layerDepth;
			}

			/// <summary>
			/// �X�N���[�����W�ňʒu���w�肷�邩
			/// �E���̍��W=��ʉ𑜓x
			/// </summary>
			/// <param name="isuse"></param>
			void SetUseScreenPos(bool isuse) {
				m_isUseScreenPos = isuse;
			}

			//�摜�̕`�悷��͈͂�ݒ�
			//0.0f�`1.0f�͈̔͂Ŏw��
			void SetSourceRectangle(const float left, const float top, const float right, const float bottom) {
				m_sprite.SetSourceRectangle(left, top, right, bottom);
			}
			//�摜�̉𑜓x�Ŏw��
			void SetSourceRectangleSpriteSize(const RECT& sourceRectangle) {
				m_sprite.SetSourceRectangleSpriteSize(sourceRectangle);
			}			

			void PostRender()override {
				if (!m_isUseScreenPos) {
					m_sprite.Draw(m_pos,
						m_scale,
						m_pivot,
						m_rotation,
						m_color,
						m_effects,
						m_layerDepth
					);
				}
				else {
					m_sprite.DrawScreenPos(m_pos,
						m_scale,
						m_pivot,
						m_rotation,
						m_color,
						m_effects,
						m_layerDepth
					);
				}
			}

		private:
			CSprite m_sprite;

			bool m_isUseScreenPos = false;
			CVector2 m_pos;
			CVector2 m_scale = CVector2::One();
			CVector2 m_pivot = CVector2::One() * 0.5f;
			float m_rotation = 0.0f;
			CVector4 m_color = CVector4::White();
			DirectX::SpriteEffects m_effects = DirectX::SpriteEffects_None;
			float m_layerDepth = 0.5f;
		};
	}
}
