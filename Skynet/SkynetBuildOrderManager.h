#pragma once

#include "BuildOrderManager.h"

#include "SkynetBuildOrder.h"

class SkynetBuildOrderManager : public BuildOrderManagerInterface
{
public:
	SkynetBuildOrderManager( Core & core );

	SkynetBuildOrder & createBuildOrder( std::string_view name );

private:
	std::vector<std::unique_ptr<SkynetBuildOrder>> m_build_orders;
};