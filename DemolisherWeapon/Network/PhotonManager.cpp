#include "DWstdafx.h"
#include "PhotonManager.h"

namespace DemolisherWeapon {

	void PhotonNetworkLogic::debugReturn(int /*debugLevel*/, const ExitGames::Common::JString& string)
	{
		//mpOutputListener->writeString(string);
	}

	void PhotonNetworkLogic::connectionErrorReturn(int errorCode)
	{
		EGLOG(ExitGames::Common::DebugLevel::ERRORS, L"code: %d", errorCode);
		//mpOutputListener->writeString(ExitGames::Common::JString(L"received connection error ") + errorCode);
		
		m_isJoinedRoom = false;
		m_isConnected = false;
	}

	void PhotonNetworkLogic::clientErrorReturn(int errorCode)
	{
		EGLOG(ExitGames::Common::DebugLevel::ERRORS, L"code: %d", errorCode);
		//mpOutputListener->writeString(ExitGames::Common::JString(L"received error ") + errorCode + L" from client");
	}

	void PhotonNetworkLogic::warningReturn(int warningCode)
	{
		EGLOG(ExitGames::Common::DebugLevel::WARNINGS, L"code: %d", warningCode);
		//mpOutputListener->writeString(ExitGames::Common::JString(L"received warning ") + warningCode + L" from client");
	}

	void PhotonNetworkLogic::serverErrorReturn(int errorCode)
	{
		EGLOG(ExitGames::Common::DebugLevel::ERRORS, L"code: %d", errorCode);
		//mpOutputListener->writeString(ExitGames::Common::JString(L"received error ") + errorCode + " from server");
	}

	void PhotonNetworkLogic::joinRoomEventAction(int playerNr, const ExitGames::Common::JVector<int>& /*playernrs*/, const ExitGames::LoadBalancing::Player& player)
	{
		EGLOG(ExitGames::Common::DebugLevel::INFO, L"%ls joined the game", player.getName().cstr());
		//mpOutputListener->writeString(L"");
		//mpOutputListener->writeString(ExitGames::Common::JString(L"player ") + playerNr + L" " + player.getName() + L" has joined the game");
	}

	void PhotonNetworkLogic::leaveRoomEventAction(int playerNr, bool isInactive)
	{
		EGLOG(ExitGames::Common::DebugLevel::INFO, L"");
		//mpOutputListener->writeString(L"");
		//mpOutputListener->writeString(ExitGames::Common::JString(L"player ") + playerNr + L" has left the game");
	}

	void PhotonNetworkLogic::customEventAction(int playerNr, nByte eventCode, const ExitGames::Common::Object& eventContentObj)
	{
		ExitGames::Common::Hashtable eventContent = ExitGames::Common::ValueObject<ExitGames::Common::Hashtable>(eventContentObj).getDataCopy();
		/*switch (eventCode)
		{
		case 0:
			if (eventContent.getValue((nByte)0))
				mReceiveCount = ((ExitGames::Common::ValueObject<int64>*)(eventContent.getValue((nByte)0)))->getDataCopy();
			if (mState == State::SENT_DATA && mReceiveCount >= mSendCount)
			{
				mState = State::RECEIVED_DATA;
				mSendCount = 0;
				mReceiveCount = 0;
			}
			break;
		default:
			break;
		}*/

	}

	void PhotonNetworkLogic::connectReturn(int errorCode, const ExitGames::Common::JString& errorString, const ExitGames::Common::JString& region, const ExitGames::Common::JString& cluster)
	{
		EGLOG(ExitGames::Common::DebugLevel::INFO, L"connected to cluster " + cluster + L" of region " + region);
		if (errorCode)
		{
			EGLOG(ExitGames::Common::DebugLevel::ERRORS, L"%ls", errorString.cstr());
			//mState = State::DISCONNECTING;
			return;
		}
		//mpOutputListener->writeString(L"connected to cluster " + cluster);
		
		m_isConnected = true;
	}

	void PhotonNetworkLogic::disconnectReturn(void)
	{
		EGLOG(ExitGames::Common::DebugLevel::INFO, L"");
		//mpOutputListener->writeString(L"disconnected");
		
		m_isJoinedRoom = false;
		m_isConnected = false;
	}

	void PhotonNetworkLogic::createRoomReturn(int localPlayerNr, const ExitGames::Common::Hashtable& /*gameProperties*/, const ExitGames::Common::Hashtable& /*playerProperties*/, int errorCode, const ExitGames::Common::JString& errorString)
	{
		EGLOG(ExitGames::Common::DebugLevel::INFO, L"");
		if (errorCode)
		{
			EGLOG(ExitGames::Common::DebugLevel::ERRORS, L"%ls", errorString.cstr());
			//mpOutputListener->writeString(L"opCreateRoom() failed: " + errorString);
			m_isJoinedRoom = false;
			return;
		}

		EGLOG(ExitGames::Common::DebugLevel::INFO, L"localPlayerNr: %d", localPlayerNr);
		//mpOutputListener->writeString(L"... room " + mLoadBalancingClient.getCurrentlyJoinedRoom().getName() + " has been created");
		//mpOutputListener->writeString(L"regularly sending dummy events now");
		
		m_isJoinedRoom = true;
	}

	void PhotonNetworkLogic::joinOrCreateRoomReturn(int localPlayerNr, const ExitGames::Common::Hashtable& /*gameProperties*/, const ExitGames::Common::Hashtable& /*playerProperties*/, int errorCode, const ExitGames::Common::JString& errorString)
	{
		EGLOG(ExitGames::Common::DebugLevel::INFO, L"");
		if (errorCode)
		{
			EGLOG(ExitGames::Common::DebugLevel::ERRORS, L"%ls", errorString.cstr());
			//mpOutputListener->writeString(L"opJoinOrCreateRoom() failed: " + errorString);
			m_isJoinedRoom = false;
			return;
		}

		EGLOG(ExitGames::Common::DebugLevel::INFO, L"localPlayerNr: %d", localPlayerNr);
		//mpOutputListener->writeString(L"... room " + mLoadBalancingClient.getCurrentlyJoinedRoom().getName() + " has been entered");
		//mpOutputListener->writeString(L"regularly sending dummy events now");
		
		m_isJoinedRoom = true;
	}

	void PhotonNetworkLogic::joinRoomReturn(int localPlayerNr, const ExitGames::Common::Hashtable& /*gameProperties*/, const ExitGames::Common::Hashtable& /*playerProperties*/, int errorCode, const ExitGames::Common::JString& errorString)
	{
		EGLOG(ExitGames::Common::DebugLevel::INFO, L"");
		if (errorCode)
		{
			EGLOG(ExitGames::Common::DebugLevel::ERRORS, L"%ls", errorString.cstr());
			//mpOutputListener->writeString(L"opJoinRoom() failed: " + errorString);
			m_isJoinedRoom = false;
			return;
		}
		EGLOG(ExitGames::Common::DebugLevel::INFO, L"localPlayerNr: %d", localPlayerNr);
		//mpOutputListener->writeString(L"... room " + mLoadBalancingClient.getCurrentlyJoinedRoom().getName() + " has been successfully joined");
		//mpOutputListener->writeString(L"regularly sending dummy events now");

		m_isJoinedRoom = true;
	}

	void PhotonNetworkLogic::joinRandomRoomReturn(int localPlayerNr, const ExitGames::Common::Hashtable& /*gameProperties*/, const ExitGames::Common::Hashtable& /*playerProperties*/, int errorCode, const ExitGames::Common::JString& errorString)
	{
		EGLOG(ExitGames::Common::DebugLevel::INFO, L"");
		if (errorCode)
		{
			EGLOG(ExitGames::Common::DebugLevel::ERRORS, L"%ls", errorString.cstr());
			//mpOutputListener->writeString(L"opJoinRandomRoom() failed: " + errorString);
			m_isJoinedRoom = false;
			return;
		}

		EGLOG(ExitGames::Common::DebugLevel::INFO, L"localPlayerNr: %d", localPlayerNr);
		//mpOutputListener->writeString(L"... room " + mLoadBalancingClient.getCurrentlyJoinedRoom().getName() + " has been successfully joined");
		//mpOutputListener->writeString(L"regularly sending dummy events now");
		
		m_isJoinedRoom = true;
	}

	void PhotonNetworkLogic::leaveRoomReturn(int errorCode, const ExitGames::Common::JString& errorString)
	{
		EGLOG(ExitGames::Common::DebugLevel::INFO, L"");
		if (errorCode)
		{
			EGLOG(ExitGames::Common::DebugLevel::ERRORS, L"%ls", errorString.cstr());
			//mpOutputListener->writeString(L"opLeaveRoom() failed: " + errorString);
			//mState = State::DISCONNECTING;
			return;
		}
		//mState = State::LEFT;
		//mpOutputListener->writeString(L"room has been successfully left");
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

	PhotonNetworkLogic::PhotonNetworkLogic(const ExitGames::Common::JString& appID, const ExitGames::Common::JString& appVersion)
		: m_LoadBalancingClient(*this, appID, appVersion, ExitGames::Photon::ConnectionProtocol::DEFAULT, true, ExitGames::LoadBalancing::RegionSelectionMode::BEST)
	{
		m_LoadBalancingClient.setDebugOutputLevel(DEBUG_RELEASE(ExitGames::Common::DebugLevel::INFO, ExitGames::Common::DebugLevel::WARNINGS)); // that instance of LoadBalancingClient and its implementation details
		mLogger.setListener(*this);
		mLogger.setDebugOutputLevel(DEBUG_RELEASE(ExitGames::Common::DebugLevel::INFO, ExitGames::Common::DebugLevel::WARNINGS)); // this class
		ExitGames::Common::Base::setListener(this);
		ExitGames::Common::Base::setDebugOutputLevel(DEBUG_RELEASE(ExitGames::Common::DebugLevel::INFO, ExitGames::Common::DebugLevel::WARNINGS)); // all classes that inherit from Base
	}

	void PhotonNetworkLogic::Connect(void)
	{
		// connect() is asynchronous - the actual result arrives in the Listener::connectReturn() or the Listener::connectionErrorReturn() callback
		//userNameê›íË
		if (!m_LoadBalancingClient.connect()) {
			EGLOG(ExitGames::Common::DebugLevel::ERRORS, L"Could not connect.");
		}
	}

}