#pragma once

namespace DemolisherWeapon {
	namespace GameObj {
		namespace Suicider {

			template <class X>
			class CParticle :
				public IQSGameObject
			{
			public:
				void Update()override {
					//毎フレームの変化
					if (m_graphic) {
						m_graphic->SetPos(m_graphic->GetPos() + m_move);
						m_graphic->SetScale(m_graphic->GetScale()*m_scaling);
						m_graphic->SetRot(m_rotation * m_graphic->GetRot());
					}
					//寿命
					if (m_lifeTime < 0) { return; }
					m_lifeTime--; if (m_lifeTime <= 0) { delete this; return; }
				}

			public:
				/// <summary>
				/// コンストラクタ
				/// </summary>
				CParticle() = default;
				/// <param name="graphic">パーティクルの見た目</param>
				/// <param name="lifetime">寿命(lifetime＜0で寿命なし)</param>
				CParticle(std::unique_ptr<X>&& graphic, int lifetime = -1) {
					Play(std::move(graphic), lifetime);
				}

				/// <summary>
				/// 再生
				/// </summary>
				/// <param name="graphic">パーティクルの見た目</param>
				/// <param name="lifetime">寿命(lifetime＜0で寿命なし)</param>
				void Play(std::unique_ptr<X>&& graphic, int lifetime = -1) {
					m_graphic.reset();
					m_graphic = std::move(graphic); m_lifeTime = lifetime;
				}

				//毎フレームの移動量を設定
				void SetMove(const CVector3& vec) { m_move = vec; }
				//毎フレームの拡大率を設定
				void SetScaling(const CVector3& scale) { m_scaling = scale; }
				//毎フレームの回転量を設定
				void SetRotation(const CQuaternion& rot) { m_rotation = rot; }

				//パーティクルの見た目を取得
				X* GetGraphic() { return m_graphic.get(); }

				//残り寿命を取得
				int GetLifeTime()const { return m_lifeTime; }

			private:
				std::unique_ptr<X> m_graphic;
				int m_lifeTime = -1;
				CVector3 m_move, m_scaling = CVector3::One();
				CQuaternion m_rotation;
			};

		}
	}
}
