#include "DWstdafx.h"
#include "EffekseerManager.h"

namespace DemolisherWeapon {

EffekseerManager::EffekseerManager()
{
}
EffekseerManager::~EffekseerManager()
{
	Release();
}

void EffekseerManager::Init() {

	// �`��Ǘ��C���X�^���X�̐���
	m_renderer = ::EffekseerRendererDX11::Renderer::Create(GetEngine().GetGraphicsEngine().GetD3DDevice(), GetEngine().GetGraphicsEngine().GetD3DDeviceContext(), MAX_SPRITE);
	// �T�E���h�Ǘ��C���X�^���X�̐���
	m_sound = ::EffekseerSound::Sound::Create(GetEngine().GetSoundEngine().GetIXAudio2(), MAX_MONO_VOICE, MAX_STER_VOICE);
	// �G�t�F�N�g�Ǘ��p�C���X�^���X�̐���
	m_manager = ::Effekseer::Manager::Create(MAX_SPRITE);

	// �`����@���w�肵�܂��B�Ǝ��Ɋg�����邱�Ƃ��ł��܂��B
	m_manager->SetSpriteRenderer(m_renderer->CreateSpriteRenderer());
	m_manager->SetRibbonRenderer(m_renderer->CreateRibbonRenderer());
	m_manager->SetRingRenderer(m_renderer->CreateRingRenderer());
	m_manager->SetTrackRenderer(m_renderer->CreateTrackRenderer());
	m_manager->SetModelRenderer(m_renderer->CreateModelRenderer());

	// �e�N�X�`���摜�̓Ǎ����@�̎w��(�p�b�P�[�W������ǂݍ��ޏꍇ�g������K�v������܂��B)
	m_manager->SetTextureLoader(m_renderer->CreateTextureLoader());

	// �T�E���h�Đ��p�C���X�^���X�̎w��
	m_manager->SetSoundPlayer(m_sound->CreateSoundPlayer());

	// �T�E���h�f�[�^�̓Ǎ����@�̎w��(���k�t�H�[�}�b�g�A�p�b�P�[�W������ǂݍ��ޏꍇ�g������K�v������܂��B)
	m_manager->SetSoundLoader(m_sound->CreateSoundLoader());

	// 3D�T�E���h�p���X�i�[�ݒ�̍X�V
	m_sound->SetListener({ 0.0f, 0.0f, 500.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f });

	// ���W�n�̎w��(RH�ŉE��n�ALH�ō���n)
	m_manager->SetCoordinateSystem(Effekseer::CoordinateSystem::LH);
}

void EffekseerManager::Release() {
	// �G�t�F�N�g�̒�~
	if (m_manager) m_manager->StopAllEffects();
	// �G�t�F�N�g��������܂��B�Đ����̏ꍇ�́A�Đ����I��������A�����I�ɉ������܂��B
	for (auto& effect : m_effects) {
		ES_SAFE_RELEASE(effect.second);
	}
	m_effects.clear();

	// �G�t�F�N�g�Ǘ��p�C���X�^���X��j��
	if (m_manager) m_manager->Destroy();
	// �T�E���h�p�C���X�^���X��j��
	if (m_sound) m_sound->Destroy();
	// �`��p�C���X�^���X��j��
	if (m_renderer) m_renderer->Destroy();
}

void EffekseerManager::Update() {
	if (GetMainCamera()) {
		// 3D�T�E���h�p���X�i�[�ݒ�̍X�V

		//�E��n��
		CVector3 pos, target, up;
		pos = GetMainCamera()->GetPos();
		target = GetMainCamera()->GetTarget();
		up = GetMainCamera()->GetUp(); up *= -1.0f;

		m_sound->SetListener( pos, target, up);
	}

	// �Đ����̃G�t�F�N�g�̈ړ���(::Effekseer::Manager�o�R�ŗl�X�ȃp�����[�^�[���ݒ�ł��܂��B)
	//m_manager->AddLocation(handle, ::Effekseer::Vector3D);

	// �S�ẴG�t�F�N�g�̍X�V
	m_manager->Update(60.0f / GetStandardFrameRate());
}

void EffekseerManager::Draw() {

#ifndef DW_MASTER
	if (!GetMainCamera()) {
		MessageBox(NULL, "�J�������ݒ肳��Ă��܂���!!", "Error", MB_OK);
		std::abort();
	}
#endif

	// ���e�s��̍X�V
	m_renderer->SetProjectionMatrix(GetMainCamera()->GetProjMatrix());
	// �J�����s��̍X�V
	m_renderer->SetCameraMatrix(GetMainCamera()->GetViewMatrix());

	m_renderer->BeginRendering();
	m_manager->Draw();
	m_renderer->EndRendering();
}

Effekseer::Effect* EffekseerManager::Load(const wchar_t* filePath, float scale) {
	Effekseer::Effect* effect = nullptr;
	int index = Util::MakeHash(filePath);

	//���ɓo�^����ĂȂ���?
	if (m_effects.count(index) > 0) {
		//�o�^����Ă���}�b�v����擾
		effect = m_effects[index];
	}
	else {
		//�V�K�ǂݍ���

		// �G�t�F�N�g�̓Ǎ�	
		effect = Effekseer::Effect::Create(m_manager, (const EFK_CHAR*)filePath, scale);
		if (effect == nullptr) {
#ifndef DW_MASTER
			char message[256];
			sprintf_s(message, "Effekseer::Effect::Create �Ɏ��s�B\nFilePath:%ls", filePath);
			MessageBox(NULL, message, "Error", MB_OK);
			std::abort();
#endif
			return nullptr;
		}

		//�G�t�F�N�g��o�^
		m_effects[index] = effect;
	}

	return effect;
}

Effekseer::Handle EffekseerManager::Play(Effekseer::Effect* effect, const CVector3& pos) {
	// �G�t�F�N�g�̍Đ�
	return m_manager->Play(effect,0.0f, 0.0f, 0.0f);// , pos.x, pos.y, pos.z); //Effekseer��΋����˂��c
}

void EffekseerManager::Stop(Effekseer::Handle handle) {
	// �G�t�F�N�g�̒�~
	m_manager->StopEffect(handle);
}

}