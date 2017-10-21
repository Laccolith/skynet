#include "Skynet.h"

#include "SkynetInterface.h"
#include "StringUtils.h"

#include <BWAPI.h>

Skynet::Skynet( std::string name )
{
	BWAPI::Broodwar->enableFlag( BWAPI::Flag::UserInput );

	m_access = std::make_unique<Access>( *this );

	std::sort( m_update_processes.begin(), m_update_processes.end(), []( const std::pair<float, std::function<void()>> & lhs, const std::pair<float, std::function<void()>> & rhs )
	{
		return lhs.first < rhs.first;
	} );

	m_in_startup = false;
	BWAPI::Broodwar->printf( "%s is online.", name.c_str() );
}

void Skynet::update()
{
	for( auto & e : BWAPI::Broodwar->getEvents() )
	{
		if( e.getType() == BWAPI::EventType::SendText )
		{
			auto tokens = StringUtils::split( e.getText(), "\t " );

			if( tokens.empty() )
				continue;

			if( tokens[0] == "debug" )
			{
				if( tokens.size() < 3 )
				{
					BWAPI::Broodwar->printf( "Not enough commands for debug, expects interface and command." );
					continue;
				}

				auto it = m_interfaces.find( tokens[1] );
				if( it != m_interfaces.end() )
					it->second->debugCommand( tokens[2] );
				else
					BWAPI::Broodwar->printf( "%s is not a known interface.", tokens[1].c_str() );
			}
			else
				BWAPI::Broodwar->printf( "%s is not a command.", tokens[0].c_str() );
		}
	}

	for( auto & update : m_update_processes )
		update.second();
}

void Skynet::registerInterface( SkynetInterface & inter )
{
	if( m_in_startup )
		m_interfaces[inter.getName()] = &inter;
	else
		BWAPI::Broodwar->printf( "Interface %s has attempted to register itself after startup.", inter.getName().c_str() );
}

void Skynet::registerUpdateProcess( float priority, std::function<void()> update_function )
{
	if( m_in_startup )
		m_update_processes.emplace_back( priority, std::move( update_function ) );
	else
		BWAPI::Broodwar->printf( "An update process has been attempted to register after startup." );
}