#pragma once

#include "Engine_API.h"

#include <iostream>
#include <memory>

namespace EngineState
{
	enum Type : uint8_t
	{
		Stopped = 0,
		Starting = 1,
		Running = 2,
		Stopping = 3,
	};
}

/**
 * Abstract class.
 * The Engine own the lifetime of the process.
 */
class ENGINE_API Engine
{

protected:
	Engine() = default;

public:
	virtual ~Engine() = default;

	// Remove copy constructor and copy assignment operator
	// A single engine instance is allowed
	Engine(const Engine&) = delete;
	Engine& operator=(const Engine&) = delete;

#pragma region Methods
public:
	/** Called before starting the engine */
	void Initialize();
	/** Create and start the engine, called constructor */
	void Start();
	/** Request the Engine to Stop running, (will happened at the end of the frame) */
	void Stop();
	/** Whether or not the engine should stop running */
	virtual bool EngineShouldStop() { return (m_State == EngineState::Stopping); }

	/** Called when the engine is in starting state */
	virtual void OnInitialize() = 0;
	/** Called after start, the engine will cleanup automatically when this function die */
	virtual void EngineLoop() = 0;
#pragma endregion

#pragma region Accessor - Get
public:
	EngineState::Type GetState() const { return (m_State); }
#pragma endregion

#pragma region Properties
protected:
	EngineState::Type m_State = EngineState::Type::Stopped;
#pragma endregion
};

inline Engine* g_Engine;
