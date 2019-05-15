#pragma once

namespace DemolisherWeapon {
	class Bone;

	class SkeletonIK
	{
	public:
		struct IKSetting {
			Bone* tipBone = nullptr; //先端ボーン
			Bone* rootBone = nullptr;//根本ボーン
			bool isEnable = true;
			CVector3 targetPos;//IKの目標地点
			float targetSize = 1.0f;//目標位置球の大きさ(誤差許容値)
			int iteration = 5;//反復回数

			/// <summary>
			/// FootIKを初期化
			/// ※tipBoneとrootBoneを設定してください
			/// </summary>
			void InitFootIK();
			/// <summary>
			/// FootIKを解除
			/// </summary>
			void ReleaseFootIK();
			
			bool GetIsFootIK() const { return isFootIK; }
			const std::list<Bone*>& GetIsFootIKBoneList() const { return footIKBoneList; }

		private:
			bool isFootIK = false;
			std::list<Bone*> footIKBoneList;//FootIKに使うボーンのリスト(つま先から始まる)
		public:
			CVector3 footIKRayEndOffset;//FootIK:地面との判定レイ終点(tipBoneの位置)に対するオフセット
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
		/// <returns>IKの設定へのポインタ</returns>
		IKSetting* CreateIK() {
			m_ikList.emplace_back();
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
		/// <returns>接触しているか?</returns>
		bool CalcFootIKTarget(IKSetting& ik);

	private:
		bool m_isEnable = true;
		std::list<IKSetting> m_ikList;//実行するIKのリスト
	};
}
