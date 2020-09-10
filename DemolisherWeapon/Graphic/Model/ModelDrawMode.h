#pragma once

class ModelDrawMode {
public:
	//�V�F�[�_�[���[�h
	enum enShaderMode {
		enNormalShader,
		enZShader,
	};

	//�V�F�[�_�[���[�h��ݒ�
	void SetShaderMode(enShaderMode sm) {
		m_s_shadermode = sm;
	}
	enShaderMode GetShaderMode() const {
		return m_s_shadermode;
	}

private:
	enShaderMode m_s_shadermode = enNormalShader;//�V�F�[�_�[���[�h
};