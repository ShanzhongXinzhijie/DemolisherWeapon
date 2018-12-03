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
		PhotonNetworkLogic(const ExitGames::Common::JString& appID, const ExitGames::Common::JString& appVersion);
		
		void Connect(void);
		void Disconnect(void) {
			m_LoadBalancingClient.disconnect();
		}

		void Update() {
			m_LoadBalancingClient.service();
		}

		void Join(const ExitGames::Common::JString& roomName, nByte maxPlayers) {
			m_LoadBalancingClient.opJoinOrCreateRoom(roomName, ExitGames::LoadBalancing::RoomOptions().setMaxPlayers(maxPlayers));
		}
		void Leave() {
			m_LoadBalancingClient.opLeaveRoom();
		}

		void Send(){
			nByte eventCode = 1; // use distinct event codes to distinguish between different types of events (for example 'move', 'shoot', etc.)
			ExitGames::Common::Hashtable evData; // organize your payload data in any way you like as long as it is supported by Photons serialization
			bool sendReliable = false; // send something reliable if it has to arrive everywhere
			m_LoadBalancingClient.opRaiseEvent(sendReliable, evData, eventCode);
		}

		/*void customEventAction(int playerNr, nByte eventCode, const ExitGames::Common::Object& eventContent)
		{

			ExitGames::Client::Photon::LoadBalancing::EventCode

			// logging the string representation of the eventContent can be really useful for debugging, but use with care: for big events this might get expensive
			EGLOG(ExitGames::Common::DebugLevel::ALL, L"an event of type %d from player Nr %d with the following content has just arrived: %ls", eventCode, playerNr, eventContent.toString(true).cstr());

			switch (eventCode)
			{
			case 1:
			{
				// you can access the content as a copy (might be a bit expensive for really big data constructs)
				ExitGames::Common::Hashtable content = ExitGames::Common::ValueObject<ExitGames::Common::Hashtable>(eventContent).getDataCopy();
				// or you access it by address (it will become invalid as soon as this function returns, so (any part of the) data that you need to continue having access to later on needs to be copied)
				ExitGames::Common::Hashtable* pContent = ExitGames::Common::ValueObject<ExitGames::Common::Hashtable>(eventContent).getDataAddress();
			}
			break;
			case 2:
			{
				// of course the payload does not need to be a Hashtable - how about just sending around for example a plain 64bit integer?
				long long content = ExitGames::Common::ValueObject<long long>(eventContent).getDataCopy();
			}
			break;
			case 3:
			{
				// or an array of floats?
				float* pContent = ExitGames::Common::ValueObject<float*>(eventContent).getDataCopy();
				float** ppContent = ExitGames::Common::ValueObject<float*>(eventContent).getDataAddress();
				short contentElementCount = *ExitGames::Common::ValueObject<float*>(eventContent).getSizes();
				// when calling getDataCopy() on Objects that hold an array as payload, then you must deallocate the copy of the array yourself using deallocateArray()!
				ExitGames::Common::MemoryManagement::deallocateArray(pContent);
			}
			break;
			default:
			{
				// have a look at demo_typeSupport inside the C++ client SDKs for example code on how to send and receive more fancy data types
			}
			break;
			}
		}*/

	private:
		ExitGames::LoadBalancing::Client m_LoadBalancingClient;
		ExitGames::Common::Logger mLogger; // accessed by EGLOG()

		bool m_isConnected = false;
		bool m_isJoinedRoom = false;

	public:
		bool GetConnected()const { return m_isConnected; }
		bool GetJoinedRoom()const { return m_isJoinedRoom; }
	};

}