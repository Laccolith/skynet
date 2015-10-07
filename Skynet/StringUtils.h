#pragma once

#include <string>
#include <vector>

namespace StringUtils
{
	inline void trimWhitespace( std::string & str )
	{
		auto start_pos = str.find_first_not_of( " \t" );
		if( start_pos == std::string::npos )
			str.clear();
		else
		{
			auto end_pos = str.find_last_not_of( " \t" );
			str = str.substr( start_pos, end_pos - start_pos + 1 );
		}
	}

	inline std::string extractEntry( std::string & values_string )
	{
		std::string result;
		auto next_comma_pos = values_string.find( ',' );

		if( next_comma_pos != std::string::npos )
		{
			result = values_string.substr( 0, next_comma_pos );
			values_string.erase( 0, next_comma_pos + 1 );
			trimWhitespace( result );
		}
		else
		{
			std::swap( result, values_string );
			trimWhitespace( result );
			values_string.clear();
		}

		return result;
	}

	inline std::vector<std::string> split( const std::string & str, const std::string & delims )
	{
		std::vector<std::string> tokens;

		auto first_pos = str.find_first_not_of( delims );
		while( first_pos != std::string::npos )
		{
			auto next_pos = str.find_first_of( delims, first_pos + 1 );

			if( next_pos == std::string::npos )
			{
				tokens.push_back( str.substr( first_pos ) );
				break;
			}
			else
			{
				tokens.push_back( str.substr( first_pos, next_pos - first_pos ) );

				first_pos = str.find_first_not_of( delims, next_pos + 1 );
			}
		}

		return tokens;
	}
}