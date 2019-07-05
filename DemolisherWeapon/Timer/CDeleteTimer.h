#pragma once

namespace DemolisherWeapon {
	namespace GameObj {
		namespace Suicider {

			class CDeleteTimer :
				public IGameObject
			{
			public:
				/// <summary>
				/// コンストラクタ
				/// </summary>
				CDeleteTimer() = default;
				/// <param name="target">削除するゲームオブジェクト</param>
				/// <param name="timer">削除までの猶予時間(フレーム数)</param>
				/// <param name="isSuicide">ゲームオブジェクトの削除後にこのオブジェクトも自殺するか?</param>
				CDeleteTimer(IGameObject* target, int timer, bool isSuicide = false) {
					Init(target, timer, isSuicide);
				}

				/// <summary>
				/// 初期化
				/// </summary>
				/// <param name="target">削除するゲームオブジェクト</param>
				/// <param name="timer">削除までの猶予時間(フレーム数)</param>
				/// <param name="isSuicide">ゲームオブジェクトの削除後にこのオブジェクトも自殺するか?</param>
				void Init(IGameObject* target, int timer, bool isSuicide = false) {
					m_isSuicide = isSuicide;
					m_timer = timer;
					m_deleteTarget = target;
				}

				void PostUpdate()override {
					if (!m_deleteTarget) { return; }
					m_timer--;
					if (m_timer <= 0) {
						delete m_deleteTarget;
						if (m_isSuicide) { delete this; }
						return;
					}
				}

				/// <summary>
				/// 削除までの猶予時間(フレーム数)を取得
				/// </summary>
				int GetTimer()const { return m_timer; }

			private:
				bool m_isSuicide = false;
				int m_timer = -1;
				IGameObject* m_deleteTarget = nullptr;
			};

		}
	}
}