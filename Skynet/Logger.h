#pragma once

#include <string>

class Logger
{
public:
	Logger( std::string filename );

	void log( const char * str, ... );

private:
	std::string m_output_filename;
};
