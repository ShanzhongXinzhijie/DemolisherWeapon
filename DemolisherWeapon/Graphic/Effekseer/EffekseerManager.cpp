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

	// ���W�n�̎w��(RH�ŉE��n�ALH�ō���n)
	m_manager->SetCoordinateSystem(Effekseer::CoordinateSystem::LH);
}

void EffekseerManager::Release() {
	// �G�t�F�N�g��������܂��B�Đ����̏ꍇ�́A�Đ����I��������A�����I�ɉ������܂��B
	ES_SAFE_RELEASE(effect);
	// �G�t�F�N�g�Ǘ��p�C���X�^���X��j��
	m_manager->Destroy();
	// �T�E���h�p�C���X�^���X��j��
	m_sound->Destroy();
	// �`��p�C���X�^���X��j��
	m_renderer->Destroy();
}

void EffekseerManager::Update() {
	// ���e�s��̍X�V
	m_renderer->SetProjectionMatrix(GetMainCamera()->GetProjMatrix());
	// �J�����s��̍X�V
	m_renderer->SetCameraMatrix(GetMainCamera()->GetViewMatrix());
	
	// 3D�T�E���h�p���X�i�[�ݒ�̍X�V
	m_sound->SetListener(GetMainCamera()->GetPos(), GetMainCamera()->GetTarget(), GetMainCamera()->GetUp());

	// �Đ����̃G�t�F�N�g�̈ړ���(::Effekseer::Manager�o�R�ŗl�X�ȃp�����[�^�[���ݒ�ł��܂��B)
	//m_manager->AddLocation(handle, ::Effekseer::Vector3D);

	// �S�ẴG�t�F�N�g�̍X�V
	m_manager->Update();
}

void EffekseerRender::Render() {
	m_renderer->BeginRendering();
	m_manager->Draw();
	m_renderer->EndRendering();
}

// �G�t�F�N�g�̓Ǎ�
Effekseer::Effect* effect = Effekseer::Effect::Create(manager, �Ǎ���p�X);
// �G�t�F�N�g�̍Đ�
Effekseer::Handle handle = manager->Play(effect, �����ʒu);

}