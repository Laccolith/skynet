#include "Logger.h"

#include <stdarg.h>
#include <fstream>
#include <filesystem>

Logger::Logger( std::string filename )
	: m_output_filename( std::move( filename ) )
{
	std::experimental::filesystem::path fs_dir( "bwapi-data/Skynet/Logs/" );
	if( !std::experimental::filesystem::exists( fs_dir ) )
	{
		std::experimental::filesystem::create_directories( fs_dir );
	}
}

void Logger::log( const char * str, ... )
{
	char buffer[502];

	va_list args;
	va_start( args, str );

	int i = vsnprintf( buffer, 500, str, args );

	va_end( args );

	if( i > 0 && i < 500 )
	{
		buffer[i] = '\n';
		buffer[i + 1] = '\0';
		printf( buffer );

		if( !m_output_filename.empty() )
		{
			std::ofstream output_file{ "bwapi-data/Skynet/Logs/" + m_output_filename };

			output_file << std::string{ buffer };
		}
	}
}
