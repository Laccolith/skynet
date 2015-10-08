#pragma once

#include "Access.h"

#include <unordered_map>
#include <vector>
#include <functional>

class SkynetInterface;
class Skynet
{
	bool in_startup = true;
	std::unique_ptr<Access> m_access;

	std::unordered_map<std::string, SkynetInterface*> m_interfaces;
	std::vector<std::pair<float, std::function<void()>>> m_update_processes;

public:
	Skynet( std::string name );

	void update();

	void registerInterface( SkynetInterface & inter );
	void registerUpdateProcess( float priority, std::function<void()> update_function );
};