#pragma once

class ControlTask
{
public:
	virtual ~ControlTask() {}

	virtual bool isInProgress() const = 0;
	virtual bool isFinished() const = 0;
};
