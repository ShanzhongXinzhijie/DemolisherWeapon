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

	public:
		using EventActionFunc = std::function<void(int playerNr, nByte eventCode, const ExitGames::Common::Object& eventContentObj)>;
		using JoinEventActionFunc = std::function<void(int playerNr, const ExitGames::Common::JVector<int>& playernrs, const ExitGames::LoadBalancing::Player& player)>;
		using LeaveEventActionFunc = std::function<void(int playerNr, bool isInactive)>;

		PhotonNetworkLogic(const ExitGames::Common::JString& appID, const ExitGames::Common::JString& appVersion, EventActionFunc eventAction);

		void Update() {
			m_LoadBalancingClient.service();
		}
		
		//�T�[�o�[�ɐڑ�
		bool ConnectServer(const wchar_t* userName);
		//�T�[�o�[�Ƃ̐ڑ���ؒf
		void DisconnectServer() {
			m_LoadBalancingClient.disconnect();
			m_state = DISCONNECTING;
		}

		//���[���ɓ���
		void JoinRoom(const ExitGames::Common::JString& roomName, nByte maxPlayers) {
			m_LoadBalancingClient.opJoinOrCreateRoom(roomName, ExitGames::LoadBalancing::RoomOptions().setMaxPlayers(maxPlayers));
			m_state = JOINING;
		}
		//���[������ޏo
		void LeaveRoom() {
			m_LoadBalancingClient.opLeaveRoom();
			m_state = LEAVING;
		}

		//�C�x���g�𑗐M
		template<typename Ftype>
		void Send(nByte eventCode, const Ftype& parameters, bool sendReliable = false, const ExitGames::LoadBalancing::RaiseEventOptions& options = ExitGames::LoadBalancing::RaiseEventOptions()){
			m_LoadBalancingClient.opRaiseEvent(sendReliable, parameters, eventCode, options);
		}
		//�C�x���g��M���Ɏ��s����֐���ݒ�
		void SetEventAction(EventActionFunc eventAction) {
			m_eventAction = eventAction;
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

		//�x�����󂯎��֐���ݒ�

		//�f�o�b�O�o�͂��󂯎��֐���ݒ�

		//�T�[�o�[�ɐڑ����Ă��邩�擾
		bool GetConnected()const { return m_LoadBalancingClient.getIsInLobby(); }//return m_isConnected; }
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

		//Client���擾
		ExitGames::LoadBalancing::Client& GetClient() {
			return m_LoadBalancingClient;
		}

	private:
		ExitGames::LoadBalancing::Client m_LoadBalancingClient;
		ExitGames::Common::Logger mLogger; // accessed by EGLOG()

		EventActionFunc m_eventAction = nullptr;
		JoinEventActionFunc m_joinEventAction = nullptr;
		LeaveEventActionFunc m_leaveEventAction = nullptr;

		bool m_isConnected = false;
		bool m_isJoinedRoom = false;
		States m_state = INITIALIZED;
	};

}