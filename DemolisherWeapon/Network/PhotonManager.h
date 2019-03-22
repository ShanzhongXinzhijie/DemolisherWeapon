#pragma once

#include "Client.h"
#include "Common.h"

namespace DemolisherWeapon {

	class PhotonNetworkLogic : private ExitGames::LoadBalancing::Listener
	{
	private:
		// receive and print out debug out here
		virtual void debugReturn(int debugLevel, const ExitGames::Common::JString& string);

		// implement your error-handling here
		virtual void connectionErrorReturn(int errorCode);
		virtual void clientErrorReturn(int errorCode);
		virtual void warningReturn(int warningCode);
		virtual void serverErrorReturn(int errorCode);

		// events, triggered by certain operations of all players in the same room
		virtual void joinRoomEventAction(int playerNr, const ExitGames::Common::JVector<int>& playernrs, const ExitGames::LoadBalancing::Player& player);
		virtual void leaveRoomEventAction(int playerNr, bool isInactive);
		virtual void customEventAction(int playerNr, nByte eventCode, const ExitGames::Common::Object& eventContent);

		// callbacks for operations on PhotonLoadBalancing server
		virtual void connectReturn(int errorCode, const ExitGames::Common::JString& errorString, const ExitGames::Common::JString& region, const ExitGames::Common::JString& cluster);
		virtual void disconnectReturn(void);
		virtual void createRoomReturn(int localPlayerNr, const ExitGames::Common::Hashtable& gameProperties, const ExitGames::Common::Hashtable& playerProperties, int errorCode, const ExitGames::Common::JString& errorString);
		virtual void joinOrCreateRoomReturn(int localPlayerNr, const ExitGames::Common::Hashtable& gameProperties, const ExitGames::Common::Hashtable& playerProperties, int errorCode, const ExitGames::Common::JString& errorString);
		virtual void joinRoomReturn(int localPlayerNr, const ExitGames::Common::Hashtable& gameProperties, const ExitGames::Common::Hashtable& playerProperties, int errorCode, const ExitGames::Common::JString& errorString);
		virtual void joinRandomRoomReturn(int localPlayerNr, const ExitGames::Common::Hashtable& gameProperties, const ExitGames::Common::Hashtable& playerProperties, int errorCode, const ExitGames::Common::JString& errorString);
		virtual void leaveRoomReturn(int errorCode, const ExitGames::Common::JString& errorString);
		virtual void joinLobbyReturn(void);
		virtual void leaveLobbyReturn(void);

		// info, that certain values have been updated
		virtual void onRoomPropertiesChange(const ExitGames::Common::Hashtable& gameProperties) {
			if(m_roomPropetyAction) m_roomPropetyAction(gameProperties);
		}
		virtual void onPlayerPropertiesChange(int playerNr, const ExitGames::Common::Hashtable& changes) {
			if(m_playerPropetyAction) m_playerPropetyAction(playerNr, changes);
		}

	public:
		using EventActionFunc = std::function<void(int playerNr, nByte eventCode, const ExitGames::Common::Object& eventContentObj)>;
		using JoinEventActionFunc = std::function<void(int playerNr, const ExitGames::Common::JVector<int>& playernrs, const ExitGames::LoadBalancing::Player& player)>;
		using LeaveEventActionFunc = std::function<void(int playerNr, bool isInactive)>;
		using RoomPropetyActionFunc = std::function<void(const ExitGames::Common::Hashtable& gameProperties)>;
		using PlayerPropetyActionFunc = std::function<void(int playerNr, const ExitGames::Common::Hashtable& changes)>;

		PhotonNetworkLogic(const ExitGames::Common::JString& appID, const ExitGames::Common::JString& appVersion, EventActionFunc eventAction);
		~PhotonNetworkLogic() {
			if (GetState() != INITIALIZED && GetState() != DISCONNECTED) {
				DisconnectServer();
				while (1) {
					Update();
					if (GetState() == DISCONNECTED) { break; }
					Sleep(16);
				}
			}
		}

		void Update() {
			m_LoadBalancingClient.service();
		}
		
		//�T�[�o�[�ɐڑ�
		bool ConnectServer(const wchar_t* userName);
		//�T�[�o�[�Ƃ̐ڑ���ؒf
		void DisconnectServer() {
			if (m_state == INITIALIZED || m_state == DISCONNECTING || m_state == DISCONNECTED) { return; }//!GetConnected() || 
			m_LoadBalancingClient.disconnect();
			m_state = DISCONNECTING;
		}

		//���[���ɓ���(�܂��͗��Ă�)
		void JoinRoom(const ExitGames::Common::JString& roomName, nByte maxPlayers) {
			if (GetJoinedRoom() || m_state == JOINING) { return; }
			m_LoadBalancingClient.opJoinOrCreateRoom(roomName, ExitGames::LoadBalancing::RoomOptions().setMaxPlayers(maxPlayers));
			m_state = JOINING;
		}
		//���[������ޏo
		void LeaveRoom() {
			if (!GetJoinedRoom() || m_state == LEAVING) { return; }
			m_LoadBalancingClient.opLeaveRoom();
			m_state = LEAVING;
		}
		//���[���ɓ���
		void JoinRandomRoom() {
			if (GetJoinedRoom() || m_state == JOINING) { return; }
			m_LoadBalancingClient.opJoinRandomRoom();
			m_state = JOINING;
		}
		//���[�������
		void CreateRoom(const ExitGames::Common::JString& roomName, nByte maxPlayers) {
			if (GetJoinedRoom() || m_state == JOINING) { return; }
			m_LoadBalancingClient.opCreateRoom(roomName, ExitGames::LoadBalancing::RoomOptions().setMaxPlayers(maxPlayers));
			m_state = JOINING;
		}

		//�C�x���g�𑗐M
		template<typename Ftype>
		void Send(nByte eventCode, const Ftype& parameters, bool sendReliable = false, const ExitGames::LoadBalancing::RaiseEventOptions& options = ExitGames::LoadBalancing::RaiseEventOptions()){
			if (!GetJoinedRoom()) { return; }
			m_LoadBalancingClient.opRaiseEvent(sendReliable, parameters, eventCode, options);
		}
		//�C�x���g��M���Ɏ��s����֐���ݒ�
		void SetEventAction(EventActionFunc eventAction) {
			m_eventAction = eventAction;
		}

		//���[���v���p�e�B��ݒ�
		template<typename ktype, typename vtype>
		void SetRoomProperty(const ktype& key, const vtype& value) {
			GetMutableRoom().addCustomProperty(key, value);
			//GetMutableRoom().getCustomProperties();
			//GetMutableRoom().mergeCustomProperties();
		}
		//���[���v���p�e�B���ύX���ꂽ�ۂɎ��s����֐���ݒ�
		void SetRoomPropetyAction(RoomPropetyActionFunc roomPropetyAction) {
			m_roomPropetyAction = roomPropetyAction;
		}
		//���[���v���p�e�B���擾
		const ExitGames::Common::Hashtable& GetRoomProperty() {
			return GetMutableRoom().getCustomProperties();
		}

		//�v���C���[�v���p�e�B��ݒ�
		template<typename ktype, typename vtype>
		void SetPlayerProperty(const ktype& key, const vtype& value) {
			GetLocalPlayer().addCustomProperty(key, value);
		}
		//�v���C���[�v���p�e�B���ύX���ꂽ�ۂɎ��s����֐���ݒ�
		void SetPlayerPropetyAction(PlayerPropetyActionFunc playerPropetyAction) {
			m_playerPropetyAction = playerPropetyAction;
		}
		//�v���C���[�v���p�e�B���擾
		const ExitGames::Common::Hashtable& GetPlayerProperty(int plyNum) {
			return m_LoadBalancingClient.getCurrentlyJoinedRoom().getPlayerForNumber(plyNum)->getCustomProperties();
		}

		//���҂����Q�������ۂɌĂ΂��֐���ݒ�
		void SetJoinEventAction(JoinEventActionFunc func) {
			m_joinEventAction = func;
		}
		//���҂������E�����ۂɌĂ΂��֐���ݒ�
		void SetLeaveEventAction(LeaveEventActionFunc func) {
			m_leaveEventAction = func;
		}

		//�G���[���󂯎��֐���ݒ�
		//�R�[�h�ƕ����{�ǂ���
		void SetErrorReturnAction(std::function<void(int errorCode, const wchar_t* errorString, const wchar_t* errorPoint)> func) {
			m_errorReturnAction = func;
		}
		//�x�����󂯎��֐���ݒ�
		void SetWarningReturnAction(std::function<void(int warningCode)> func) {
			m_warningReturnAction = func;
		}
		//�f�o�b�O�o�͂��󂯎��֐���ݒ�
		void SetDebugReturnAction(std::function<void(const ExitGames::Common::JString& string)> func) {
			m_debugReturnAction = func;
		}

		//�T�[�o�[�ɐڑ����Ă��邩�擾
		bool GetConnected()const { return m_LoadBalancingClient.getIsInRoom(); }//getIsInLobby�@return m_isConnected; }
		//���[���ɓ����Ă��邩�擾
		bool GetJoinedRoom()const { return m_LoadBalancingClient.getIsInGameRoom(); }//return m_isJoinedRoom; }

		enum States
		{
			INITIALIZED = 0,
			CONNECTING,
			CONNECTED,
			JOINING,
			JOINED,
			LEAVING,
			LEFT,
			DISCONNECTING,
			DISCONNECTED
		};
		//�X�e�[�g���擾
		States GetState()const { return m_state; }

		//���[�J���v���C���[�ԍ��̎擾
		int GetLocalPlayerNumber()const { return m_localPlayerNum; }

		//�������}�X�^�[�N���C�A���g�����擾
		bool GetIsMasterClient() {
			return m_LoadBalancingClient.getLocalPlayer().getIsMasterClient();
		}

		//���݂̃��[���̃v���C���[�����擾
		unsigned int GetCountLocalPlayer() {
			return m_LoadBalancingClient.getCurrentlyJoinedRoom().getPlayers().getSize();
		}
		//���݂̃��[���̃v���C���[���擾
		const ExitGames::Common::JVector<ExitGames::LoadBalancing::Player*>& GetPlayers() {
			return m_LoadBalancingClient.getCurrentlyJoinedRoom().getPlayers();
		}

		//Client���擾
		ExitGames::LoadBalancing::Client& GetClient() {
			return m_LoadBalancingClient;
		}
		//Room���擾
		ExitGames::LoadBalancing::MutableRoom& GetMutableRoom() {
			return m_LoadBalancingClient.getCurrentlyJoinedRoom();
		}
		//LocalPlayer���擾
		ExitGames::LoadBalancing::MutablePlayer& GetLocalPlayer() {
			return m_LoadBalancingClient.getLocalPlayer();
		}

		//Ping���擾
		int GetPing_ms() const{
			return m_LoadBalancingClient.getRoundTripTime();
		}
		//PhotonSever�̎���(ms)���擾
		int GetSeverTime_ms()const {
			return m_LoadBalancingClient.getServerTime();
		}

	private:
		ExitGames::LoadBalancing::Client m_LoadBalancingClient;
		ExitGames::Common::Logger mLogger; // accessed by EGLOG()

		EventActionFunc m_eventAction = nullptr;
		JoinEventActionFunc m_joinEventAction = nullptr;
		LeaveEventActionFunc m_leaveEventAction = nullptr;
		RoomPropetyActionFunc m_roomPropetyAction = nullptr;
		PlayerPropetyActionFunc m_playerPropetyAction = nullptr;

		std::function<void(int errorCode, const wchar_t* errorString, const wchar_t* errorPoint)> m_errorReturnAction = nullptr;
		std::function<void(int warningCode)> m_warningReturnAction = nullptr;
		std::function<void(const ExitGames::Common::JString& string)> m_debugReturnAction = nullptr;

		bool m_isConnected = false;
		bool m_isJoinedRoom = false;
		States m_state = INITIALIZED;

		int m_localPlayerNum = -1;

	};

}