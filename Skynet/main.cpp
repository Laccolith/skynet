#include <iostream>
#include <thread>
#include <chrono>
#include <string>

#include <BWAPI.h>
#include <BWAPI/Client.h>

#include "Skynet.h"

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

		Skynet skynet( "Skynet" );

		while( BWAPI::BWAPIClient.isConnected() && BWAPI::Broodwar->isInGame() )
		{
			skynet.update();

			BWAPI::BWAPIClient.update();
		}
	}

	return 0;
}