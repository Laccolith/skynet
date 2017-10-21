#include <iostream>
#include <thread>
#include <chrono>
#include <string>

#include <BWAPI.h>
#if !defined( SKYNET_DLL )
#include <BWAPI/Client.h>
#else
#include <Windows.h>
#endif

#include "SkynetCore.h"

#if !defined( SKYNET_DLL )
void connect()
{
	std::cout << "Connecting..." << std::endl;
	while( !BWAPI::BWAPIClient.connect() )
		std::this_thread::sleep_for( std::chrono::milliseconds{ 50 } );
}

int main()
{
	connect();

	while( true )
	{
		BWAPI::BWAPIClient.update();
		while( !BWAPI::BWAPIClient.isConnected() )
			connect();

		if( !BWAPI::Broodwar->isInGame() )
			continue;

		SkynetCore skynet;

		while( BWAPI::BWAPIClient.isConnected() && BWAPI::Broodwar->isInGame() )
		{
			skynet.update();

			BWAPI::BWAPIClient.update();
		}
	}

	return 0;
}
#else
extern "C" __declspec(dllexport) void gameInit( BWAPI::Game* game )
{
	BWAPI::BroodwarPtr = game;
}

BOOL APIENTRY DllMain( HANDLE, DWORD, LPVOID )
{
	return TRUE;
}

class SkynetAIModule : public BWAPI::AIModule
{
	std::unique_ptr<SkynetCore> m_skynet;

public:
	void onStart() override
	{
		m_skynet = std::make_unique<SkynetCore>();
	}

	void onFrame() override
	{
		if( m_skynet )
		{
			m_skynet->update();
		}
	}

	void onEnd( bool isWinner ) override
	{
		m_skynet.reset();
	}
};

extern "C" __declspec(dllexport) BWAPI::AIModule* newAIModule()
{
	return new SkynetAIModule();
}
#endif