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
		
		//サーバーに接続
		bool ConnectServer(const wchar_t* userName);
		//サーバーとの接続を切断
		void DisconnectServer() {
			m_LoadBalancingClient.disconnect();
			m_state = DISCONNECTING;
		}

		//ルームに入る
		void JoinRoom(const ExitGames::Common::JString& roomName, nByte maxPlayers) {
			m_LoadBalancingClient.opJoinOrCreateRoom(roomName, ExitGames::LoadBalancing::RoomOptions().setMaxPlayers(maxPlayers));
			m_state = JOINING;
		}
		//ルームから退出
		void LeaveRoom() {
			m_LoadBalancingClient.opLeaveRoom();
			m_state = LEAVING;
		}

		//イベントを送信
		template<typename Ftype>
		void Send(nByte eventCode, const Ftype& parameters, bool sendReliable = false, const ExitGames::LoadBalancing::RaiseEventOptions& options = ExitGames::LoadBalancing::RaiseEventOptions()){
			m_LoadBalancingClient.opRaiseEvent(sendReliable, parameters, eventCode, options);
		}
		//イベント受信時に実行する関数を設定
		void SetEventAction(EventActionFunc eventAction) {
			m_eventAction = eventAction;
		}

		//何者かが参加した際に呼ばれる関数を設定
		void SetJoinEventAction(JoinEventActionFunc func) {
			m_joinEventAction = func;
		}
		//何者かが離脱した際に呼ばれる関数を設定
		void SetLeaveEventAction(LeaveEventActionFunc func) {
			m_leaveEventAction = func;
		}

		//エラーを受け取る関数を設定
		//コードと文字＋どこで
		void SetErrorReturnAction(std::function<void(int errorCode, const wchar_t* errorString, const wchar_t* errorPoint)> func) {
			m_errorReturnAction = func;
		}
		//警告を受け取る関数を設定
		void SetWarningReturnAction(std::function<void(int warningCode)> func) {
			m_warningReturnAction = func;
		}
		//デバッグ出力を受け取る関数を設定
		void SetDebugReturnAction(std::function<void(const ExitGames::Common::JString& string)> func) {
			m_debugReturnAction = func;
		}

		//サーバーに接続しているか取得
		bool GetConnected()const { return m_LoadBalancingClient.getIsInLobby(); }//return m_isConnected; }
		//ルームに入っているか取得
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
		//ステートを取得
		States GetState()const { return m_state; }

		//ローカルプレイヤー番号の取得
		int GetLocalPlayerNumber()const { return m_localPlayerNum; }

		//Clientを取得
		ExitGames::LoadBalancing::Client& GetClient() {
			return m_LoadBalancingClient;
		}

	private:
		ExitGames::LoadBalancing::Client m_LoadBalancingClient;
		ExitGames::Common::Logger mLogger; // accessed by EGLOG()

		EventActionFunc m_eventAction = nullptr;
		JoinEventActionFunc m_joinEventAction = nullptr;
		LeaveEventActionFunc m_leaveEventAction = nullptr;

		std::function<void(int errorCode, const wchar_t* errorString, const wchar_t* errorPoint)> m_errorReturnAction = nullptr;
		std::function<void(int warningCode)> m_warningReturnAction = nullptr;
		std::function<void(const ExitGames::Common::JString& string)> m_debugReturnAction = nullptr;

		bool m_isConnected = false;
		bool m_isJoinedRoom = false;
		States m_state = INITIALIZED;
		int m_localPlayerNum = -1;
	};

}