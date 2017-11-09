#pragma once

#include <string>

class SkynetBuildOrder
{
public:
	SkynetBuildOrder( std::string_view name );

private:
	std::string m_name;
};