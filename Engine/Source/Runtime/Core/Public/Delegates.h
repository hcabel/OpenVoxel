#pragma once

#include <functional>
#include <vector>

template<typename ...Args>
class OVDelegate
{
public:
	OVDelegate() = default;
	OVDelegate(void(*function)(Args...))
		: m_Function(function)
	{}
	OVDelegate(const std::function<void(Args...)>& function)
		: m_Function(function)
	{}

	bool operator==(void(*function)(Args...)) const { return m_Function == function; }

public:
	void operator()(Args... args) const { Execute(std::forward<Args>(args)...); }
	void Execute(Args... args) const
	{
		if (m_Function)
			m_Function(std::forward<Args>(args)...);
	}
	void Bind(void(*function)(Args...)) { m_Function = function; }
	void BindLambda(const std::function<void(Args...)>& function) { m_Function = function; }

private:
	std::function<void(Args...)> m_Function;
};

#define DECLARE_DELEGATE(DelegateName, ...) using DelegateName = OVDelegate<__VA_ARGS__>;

template <typename ...Args>
class OVMulticastDelegate
{
public:
	OVMulticastDelegate() = default;

public:
	void operator()(Args... args) const { Execute(std::forward<Args>(args)...); }
	void Execute(Args... args) const
	{
		for (auto& delegate : m_Delegates)
			delegate.Execute(std::forward<Args>(args)...);
	}
	void Bind(void(*function)(Args...)) { m_Delegates.emplace_back(function); }
	void BindLambda(const std::function<void(Args...)>& function) { m_Delegates.emplace_back(function); }
	void Unbind(void(*function)(Args...))
	{
		for (auto it = m_Delegates.begin(); it != m_Delegates.end(); ++it)
		{
			if (*it == function)
			{
				m_Delegates.erase(it);
				break;
			}
		}
	}

private:
	std::vector<OVDelegate<Args...>> m_Delegates;
};

#define DECLARE_MULTICAST_DELEGATE(DelegateName, ...) using DelegateName = OVMulticastDelegate<__VA_ARGS__>;
