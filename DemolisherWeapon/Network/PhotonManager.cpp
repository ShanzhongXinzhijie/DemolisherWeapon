#include "DWstdafx.h"
#include "PhotonManager.h"

namespace DemolisherWeapon {

	void PhotonNetworkLogic::debugReturn(int /*debugLevel*/, const ExitGames::Common::JString& string)
	{
		if (m_debugReturnAction) { m_debugReturnAction(string); }
	}

	void PhotonNetworkLogic::connectionErrorReturn(int errorCode)
	{
		EGLOG(ExitGames::Common::DebugLevel::ERRORS, L"code: %d", errorCode);
		//mpOutputListener->writeString(ExitGames::Common::JString(L"received connection error ") + errorCode);

		if (m_errorReturnAction) { m_errorReturnAction(errorCode, L"", L"connectionErrorReturn"); }
		
		m_isJoinedRoom = false;
		m_isConnected = false;
		m_state = DISCONNECTED;
	}

	void PhotonNetworkLogic::clientErrorReturn(int errorCode)
	{
		EGLOG(ExitGames::Common::DebugLevel::ERRORS, L"code: %d", errorCode);
		//mpOutputListener->writeString(ExitGames::Common::JString(L"received error ") + errorCode + L" from client");

		if (m_errorReturnAction) { m_errorReturnAction(errorCode, L"", L"clientErrorReturn"); }

		m_isJoinedRoom = false;
		m_isConnected = false;
	}

	void PhotonNetworkLogic::warningReturn(int warningCode)
	{
		EGLOG(ExitGames::Common::DebugLevel::WARNINGS, L"code: %d", warningCode);
		//mpOutputListener->writeString(ExitGames::Common::JString(L"received warning ") + warningCode + L" from client");

		if (m_warningReturnAction) { m_warningReturnAction(warningCode); }
	}

	void PhotonNetworkLogic::serverErrorReturn(int errorCode)
	{
		EGLOG(ExitGames::Common::DebugLevel::ERRORS, L"code: %d", errorCode);
		//mpOutputListener->writeString(ExitGames::Common::JString(L"received error ") + errorCode + " from server");

		if (m_errorReturnAction) { m_errorReturnAction(errorCode, L"", L"serverErrorReturn"); }

		m_isJoinedRoom = false;
		m_isConnected = false;
	}

	void PhotonNetworkLogic::joinRoomEventAction(int playerNr, const ExitGames::Common::JVector<int>& playernrs, const ExitGames::LoadBalancing::Player& player)
	{
		EGLOG(ExitGames::Common::DebugLevel::INFO, L"%ls joined the game", player.getName().cstr());
		//mpOutputListener->writeString(L"");
		//mpOutputListener->writeString(ExitGames::Common::JString(L"player ") + playerNr + L" " + player.getName() + L" has joined the game");

		//誰かが参加した時
		if(m_joinEventAction) m_joinEventAction(playerNr, playernrs, player);
	}

	void PhotonNetworkLogic::leaveRoomEventAction(int playerNr, bool isInactive)
	{
		EGLOG(ExitGames::Common::DebugLevel::INFO, L"");
		//mpOutputListener->writeString(L"");
		//mpOutputListener->writeString(ExitGames::Common::JString(L"player ") + playerNr + L" has left the game");

		//誰かが退出した時
		if(m_leaveEventAction) m_leaveEventAction(playerNr, isInactive);
	}

	void PhotonNetworkLogic::customEventAction(int playerNr, nByte eventCode, const ExitGames::Common::Object& eventContentObj)
	{
		//イベントを受信
		m_eventAction(playerNr, eventCode, eventContentObj);

		/*switch (eventCode)
		{
		case 0:
		{
			ExitGames::Common::Hashtable eventContent = ExitGames::Common::ValueObject<ExitGames::Common::Hashtable>(eventContentObj).getDataCopy();
			if (eventContent.getValue(0)) {
				int64 mReceiveCount = ((ExitGames::Common::ValueObject<int64>*)(eventContent.getValue(0)))->getDataCopy();
				mReceiveCount = mReceiveCount;
			}
			//ExitGames::Common::Hashtable* event{ ExitGames::Common::ValueObject<ExitGames::Common::Hashtable*>(eventContentObj).getDataCopy() };
			//float x{ ExitGames::Common::ValueObject<float>(event->getValue(0)).getDataCopy() };
		}
		break;
		default:
			break;
		}*/
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

	void PhotonNetworkLogic::connectReturn(int errorCode, const ExitGames::Common::JString& errorString, const ExitGames::Common::JString& region, const ExitGames::Common::JString& cluster)
	{
		EGLOG(ExitGames::Common::DebugLevel::INFO, L"connected to cluster " + cluster + L" of region " + region);
		if (errorCode)
		{
			EGLOG(ExitGames::Common::DebugLevel::ERRORS, L"%ls", errorString.cstr());
			if (m_errorReturnAction) { m_errorReturnAction(errorCode, errorString.cstr(), L"connectReturn"); }

			m_isConnected = false;
			m_state = DISCONNECTING;
			return;
		}
		
		m_isConnected = true;
		m_state = CONNECTED;
	}

	void PhotonNetworkLogic::disconnectReturn(void)
	{
		EGLOG(ExitGames::Common::DebugLevel::INFO, L"");
		//mpOutputListener->writeString(L"disconnected");
		
		m_isJoinedRoom = false;
		m_isConnected = false;
		m_state = DISCONNECTED;
	}

	void PhotonNetworkLogic::createRoomReturn(int localPlayerNr, const ExitGames::Common::Hashtable& /*gameProperties*/, const ExitGames::Common::Hashtable& /*playerProperties*/, int errorCode, const ExitGames::Common::JString& errorString)
	{
		EGLOG(ExitGames::Common::DebugLevel::INFO, L"");
		if (errorCode)
		{
			EGLOG(ExitGames::Common::DebugLevel::ERRORS, L"%ls", errorString.cstr());
			//mpOutputListener->writeString(L"opCreateRoom() failed: " + errorString);
			if (m_errorReturnAction) { m_errorReturnAction(errorCode, errorString.cstr(), L"createRoomReturn"); }
			
			m_isJoinedRoom = false;
			m_state = CONNECTED;
			return;
		}

		EGLOG(ExitGames::Common::DebugLevel::INFO, L"localPlayerNr: %d", localPlayerNr);
		//mpOutputListener->writeString(L"... room " + mLoadBalancingClient.getCurrentlyJoinedRoom().getName() + " has been created");
		//mpOutputListener->writeString(L"regularly sending dummy events now");
		
		m_localPlayerNum = localPlayerNr;
		m_isJoinedRoom = true;
		m_state = JOINED;
	}

	void PhotonNetworkLogic::joinOrCreateRoomReturn(int localPlayerNr, const ExitGames::Common::Hashtable& /*gameProperties*/, const ExitGames::Common::Hashtable& /*playerProperties*/, int errorCode, const ExitGames::Common::JString& errorString)
	{
		EGLOG(ExitGames::Common::DebugLevel::INFO, L"");
		if (errorCode)
		{
			EGLOG(ExitGames::Common::DebugLevel::ERRORS, L"%ls", errorString.cstr());
			//mpOutputListener->writeString(L"opJoinOrCreateRoom() failed: " + errorString);
			if (m_errorReturnAction) { m_errorReturnAction(errorCode, errorString.cstr(), L"joinOrCreateRoomReturn"); }
			
			m_isJoinedRoom = false;
			m_state = CONNECTED;
			return;
		}

		EGLOG(ExitGames::Common::DebugLevel::INFO, L"localPlayerNr: %d", localPlayerNr);
		//mpOutputListener->writeString(L"... room " + mLoadBalancingClient.getCurrentlyJoinedRoom().getName() + " has been entered");
		//mpOutputListener->writeString(L"regularly sending dummy events now");

		//const ExitGames::Common::JVector<ExitGames::LoadBalancing::Player*>& P = m_LoadBalancingClient.getCurrentlyJoinedRoom().getPlayers();
		
		m_localPlayerNum = localPlayerNr;
		m_isJoinedRoom = true;
		m_state = JOINED;
	}

	void PhotonNetworkLogic::joinRoomReturn(int localPlayerNr, const ExitGames::Common::Hashtable& /*gameProperties*/, const ExitGames::Common::Hashtable& /*playerProperties*/, int errorCode, const ExitGames::Common::JString& errorString)
	{
		EGLOG(ExitGames::Common::DebugLevel::INFO, L"");
		if (errorCode)
		{
			EGLOG(ExitGames::Common::DebugLevel::ERRORS, L"%ls", errorString.cstr());
			//mpOutputListener->writeString(L"opJoinRoom() failed: " + errorString);
			if (m_errorReturnAction) { m_errorReturnAction(errorCode, errorString.cstr(), L"joinRoomReturn"); }
			
			m_isJoinedRoom = false;
			m_state = CONNECTED;
			return;
		}
		EGLOG(ExitGames::Common::DebugLevel::INFO, L"localPlayerNr: %d", localPlayerNr);
		//mpOutputListener->writeString(L"... room " + mLoadBalancingClient.getCurrentlyJoinedRoom().getName() + " has been successfully joined");
		//mpOutputListener->writeString(L"regularly sending dummy events now");

		m_localPlayerNum = localPlayerNr;
		m_isJoinedRoom = true;
		m_state = JOINED;
	}

	void PhotonNetworkLogic::joinRandomRoomReturn(int localPlayerNr, const ExitGames::Common::Hashtable& /*gameProperties*/, const ExitGames::Common::Hashtable& /*playerProperties*/, int errorCode, const ExitGames::Common::JString& errorString)
	{
		EGLOG(ExitGames::Common::DebugLevel::INFO, L"");
		if (errorCode)
		{
			EGLOG(ExitGames::Common::DebugLevel::ERRORS, L"%ls", errorString.cstr());
			//mpOutputListener->writeString(L"opJoinRandomRoom() failed: " + errorString);
			if (m_errorReturnAction) { m_errorReturnAction(errorCode, errorString.cstr(), L"joinRandomRoomReturn"); }
			
			m_isJoinedRoom = false;
			m_state = CONNECTED;
			return;
		}

		EGLOG(ExitGames::Common::DebugLevel::INFO, L"localPlayerNr: %d", localPlayerNr);
		//mpOutputListener->writeString(L"... room " + mLoadBalancingClient.getCurrentlyJoinedRoom().getName() + " has been successfully joined");
		//mpOutputListener->writeString(L"regularly sending dummy events now");
		
		m_localPlayerNum = localPlayerNr;
		m_isJoinedRoom = true;
		m_state = JOINED;
	}

	void PhotonNetworkLogic::leaveRoomReturn(int errorCode, const ExitGames::Common::JString& errorString)
	{
		EGLOG(ExitGames::Common::DebugLevel::INFO, L"");
		if (errorCode)
		{
			EGLOG(ExitGames::Common::DebugLevel::ERRORS, L"%ls", errorString.cstr());
			//mpOutputListener->writeString(L"opLeaveRoom() failed: " + errorString);
			if (m_errorReturnAction) { m_errorReturnAction(errorCode, errorString.cstr(), L"leaveRoomReturn"); }
			
			m_isJoinedRoom = false;
			m_state = DISCONNECTING;
			return;
		}
		//mState = State::LEFT;
		//mpOutputListener->writeString(L"room has been successfully left");

		m_isJoinedRoom = false;
		m_state = LEFT;
	}

	void PhotonNetworkLogic::joinLobbyReturn(void)
	{
		EGLOG(ExitGames::Common::DebugLevel::INFO, L"");
		//mpOutputListener->writeString(L"joined lobby");
	}

	void PhotonNetworkLogic::leaveLobbyReturn(void)
	{
		EGLOG(ExitGames::Common::DebugLevel::INFO, L"");
		//mpOutputListener->writeString(L"left lobby");
	}

	//コンストラクタ
	PhotonNetworkLogic::PhotonNetworkLogic(const ExitGames::Common::JString& appID, const ExitGames::Common::JString& appVersion, EventActionFunc eventAction)
		: m_LoadBalancingClient(*this, appID, appVersion, ExitGames::Photon::ConnectionProtocol::DEFAULT, true, ExitGames::LoadBalancing::RegionSelectionMode::BEST)
		, m_eventAction(eventAction)
	{
		m_LoadBalancingClient.setDebugOutputLevel(DEBUG_RELEASE(ExitGames::Common::DebugLevel::INFO, ExitGames::Common::DebugLevel::WARNINGS)); // that instance of LoadBalancingClient and its implementation details
		mLogger.setListener(*this);
		mLogger.setDebugOutputLevel(DEBUG_RELEASE(ExitGames::Common::DebugLevel::INFO, ExitGames::Common::DebugLevel::WARNINGS)); // this class
		ExitGames::Common::Base::setListener(this);
		ExitGames::Common::Base::setDebugOutputLevel(DEBUG_RELEASE(ExitGames::Common::DebugLevel::INFO, ExitGames::Common::DebugLevel::WARNINGS)); // all classes that inherit from Base
	}

	bool PhotonNetworkLogic::ConnectServer(const wchar_t* userName)
	{
		if (!m_LoadBalancingClient.connect(ExitGames::LoadBalancing::AuthenticationValues(), userName)) {
			EGLOG(ExitGames::Common::DebugLevel::ERRORS, L"Could not connect.");
			return false;
		}
		else {
			m_state = CONNECTING;
			return true;
		}
	}

}