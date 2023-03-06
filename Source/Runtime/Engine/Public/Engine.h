#pragma once

#include "EngineModule.h"

#include <iostream>
#include <memory>

/**
 * Abstract class.
 * The Engine own the lifetime of the process.
 */
class ENGINE_API Engine
{

protected:
	Engine() = default;
	virtual ~Engine();

public:
	// Remove copy constructor and copy assignment operator
	// A single engine instance is allowed
	Engine(const Engine&) = delete;
	Engine& operator=(const Engine&) = delete;

	static std::shared_ptr<Engine>& Get();

	static std::shared_ptr<Engine> CreateInstance();

#pragma region Methods
public:
	/** Create and start the engine, called constructor */
	void Start();
	/** Stop the Engine from running */
	void Stop();
	/** Whether or not the engine should stop running */
	virtual bool IsEngineRequestedToStop() { return (m_IsRequestToStop); }

	/** Called after the engine started */
	virtual void OnStart() = 0;
	/** Called after start, the engine will cleanup automatically when this function die */
	virtual void EngineLoop() = 0;
#pragma endregion

#pragma region Properties
protected:
	bool m_IsRequestToStop = false;
#pragma endregion
};
