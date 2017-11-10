#include "SkynetControlTaskFactory.h"

#include "SkynetControlTask.h"
#include "SkynetControlTaskTrain.h"
#include "SkynetControlTaskConstruct.h"
#include "SkynetControlTaskResearch.h"
#include "SkynetControlTaskUpgrade.h"

SkynetControlTaskFactory::SkynetControlTaskFactory( Core & core )
	: ControlTaskFactoryInterface( core )
{
	core.registerUpdateProcess( 3.0f, [this]() { preUpdate(); } );
	core.registerUpdateProcess( 5.0f, [this]() { postUpdate(); } );
}

void SkynetControlTaskFactory::preUpdate()
{
	for( auto task : m_control_tasks )
	{
		task->preUpdate();
	}
}

void SkynetControlTaskFactory::postUpdate()
{
	for( auto task : m_control_tasks )
	{
		task->postUpdate();
	}
}

std::unique_ptr<ControlTask> SkynetControlTaskFactory::createBuildControlTask( TaskPriority * priority, UnitType unit_type, BuildLocationType build_location_type )
{
	std::unique_ptr<SkynetControlTask> control_task;

	if( unit_type.getRace() == BWAPI::Races::Zerg && unit_type.isBuilding() == unit_type.whatBuilds().first.isBuilding() )
	{
		// TODO: Morph
	}
	else if( unit_type.isAddon() )
	{
		// TODO: Addon
	}
	else if( unit_type.isBuilding() )
	{
		control_task = std::make_unique<SkynetControlTaskConstruct>( *this, priority, unit_type, build_location_type );
	}
	else
	{
		control_task = std::make_unique<SkynetControlTaskTrain>( *this, priority, unit_type );
	}

	if( control_task )
		m_control_tasks.push_back( control_task.get() );

	return control_task;
}

std::unique_ptr<ControlTask> SkynetControlTaskFactory::createResearchControlTask( TaskPriority * priority, TechType tech_type )
{
	auto control_task = std::make_unique<SkynetControlTaskResearch>( *this, priority, tech_type );
	m_control_tasks.push_back( control_task.get() );
	return control_task;
}

std::unique_ptr<ControlTask> SkynetControlTaskFactory::createUpgradeControlTask( TaskPriority * priority, UpgradeType upgrade_type, int upgrade_level )
{
	auto control_task = std::make_unique<SkynetControlTaskUpgrade>( *this, priority, upgrade_type, upgrade_level );
	m_control_tasks.push_back( control_task.get() );
	return control_task;
}

void SkynetControlTaskFactory::onTaskDestroyed( SkynetControlTask * task )
{
	auto it = std::find( m_control_tasks.begin(), m_control_tasks.end(), task );
	std::swap( *it, m_control_tasks.back() );
	m_control_tasks.pop_back();
}
