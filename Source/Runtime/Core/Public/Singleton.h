#pragma once

/**
 * A singleton class that will instanciate one and unique instance and hold on to an object until he is reseted.
 */
template<typename T>
class Singleton final
{
private:
	Singleton()
	{
		m_Object = new T();
	}

	/** Auto destroy */
	~Singleton()
	{
		Reset();
	}

public:
	inline static T& Get()
	{
		return GetSingleton().GetValue();
	}

private:
	/** Get the singleton instance */
	static Singleton<T>& GetSingleton()
	{
		static Singleton singleton;
		return singleton;
	}

	/** Get the object hold by the singleton */
	inline T& GetValue()
	{
		return (*m_Object);
	}

	/** Reset the singleton */
	void Reset()
	{
		if (m_Object)
		{
			delete m_Object;
			m_Object = nullptr;
		}
	}

private:
	/** The ptr to the singleton */
	T* m_Object;
};
