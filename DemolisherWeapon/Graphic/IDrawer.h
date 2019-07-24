#pragma once

/// <summary>
/// 描画物のインターフェースクラス
/// </summary>
class IDrawer
{
public:
	IDrawer() = default;
	virtual ~IDrawer() {}

	/// <summary>
	/// 描画
	/// </summary>
	/// <param name="reverseCull">面を反転するか</param>
	/// <param name="instanceNum">描画インスタンス数</param>
	/// <param name="pBlendState">ブレンドステート</param>
	/// <param name="pDepthStencilState">デプスステンシルステート</param>
	virtual void Draw(	bool reverseCull = false,
						int instanceNum = 1,
						ID3D11BlendState* pBlendState = nullptr,
						ID3D11DepthStencilState* pDepthStencilState = nullptr	) = 0;
};

/// <summary>
/// Level of Detail を実行するDrawer
/// </summary>
class LODDrawer : public IDrawer {
public:
	/// <summary>
	/// 描画
	/// </summary>
	void Draw(bool reverseCull = false,
		int instanceNum = 1,
		ID3D11BlendState* pBlendState = nullptr,
		ID3D11DepthStencilState* pDepthStencilState = nullptr)override {

		//画面に対する大きさから描画するドロワーを決める

	}

	/// <summary>
	/// ドロワーの追加
	/// </summary>
	void AddDrawer(IDrawer* drawer, int lodLevel) {

	}

	/// <summary>
	/// ドロワーの切り替え設定
	/// </summary>

private:
	std::unordered_map<int, IDrawer*> m_drawer;
};