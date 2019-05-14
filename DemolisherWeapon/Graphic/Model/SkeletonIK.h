#pragma once

namespace DemolisherWeapon {
	class Bone;

	class SkeletonIK
	{
	public:
		struct IKSetting {
			Bone* tipBone = nullptr; //先端ボーン
			Bone* rootBone = nullptr;//根本ボーン
			bool isFootIK = false;
			bool isEnable = true;
			CVector3 targetPos;//IKの目標地点
			float targetSize = 1.0f;//目標位置球の大きさ(誤差許容値)
			int iteration = 5;//反復回数
		};

	public:
		SkeletonIK();
		~SkeletonIK();

		/// <summary>
		/// IKを実行
		/// </summary>
		void Update();

		/// <summary>
		/// 実行するIKの初期化
		/// </summary>
		/// <param name="setting">IKの設定</param>
		/// <returns>実行するIKの設定構造体へのポインタ</returns>
		IKSetting* CreateIK(const IKSetting& setting) {
			m_ikList.emplace_back(setting);
			return &m_ikList.back();
		}

		/// <summary>
		/// 実行するか設定
		/// </summary>
		/// <param name="enable">実行するか</param>
		void SetEnable(bool enable) { m_isEnable = enable; }

	private:
		/// <summary>
		/// CCD法によるIKの実行
		/// </summary>
		/// <param name="ik">実行するIK</param>
		void CalcCCD(const IKSetting& ik);

		/// <summary>
		/// FootIKの接触点の計算
		/// </summary>
		/// <param name="ik">実行するIK</param>
		void CalcFootIKTarget(const IKSetting& ik);

	private:
		bool m_isEnable = true;
		std::list<IKSetting> m_ikList;//実行するIKのリスト
	};
}
