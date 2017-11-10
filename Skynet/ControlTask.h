#pragma once

class TaskPriority;
class ControlTask
{
public:
	virtual ~ControlTask() {}

	virtual int timeTillStart() const = 0;
	virtual bool isInProgress() const = 0;
	virtual bool isComplete() const = 0;
};
