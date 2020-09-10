#pragma once

class ModelDrawMode {
public:
	//シェーダーモード
	enum enShaderMode {
		enNormalShader,
		enZShader,
	};

	//シェーダーモードを設定
	void SetShaderMode(enShaderMode sm) {
		m_s_shadermode = sm;
	}
	enShaderMode GetShaderMode() const {
		return m_s_shadermode;
	}

private:
	enShaderMode m_s_shadermode = enNormalShader;//シェーダーモード
};