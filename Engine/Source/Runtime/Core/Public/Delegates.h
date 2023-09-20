#pragma once

#include <functional>
#include <vector>

/* FUNCTOR */

/**
 * @brief Interface for functors
 *
 * @tparam RetValue Return value of the functor
 * @tparam ...Args Arguments of the functor
 */
template<typename RetValue, typename ...Args>
class Functor
{
public:
	virtual RetValue Execute(Args... args) const = 0;
};

/**
 * @brief Functor for calling a class member function
 *
 * @tparam OwnerClass Class type of the owner of the member function
 * @tparam RetValue Return value of the member function
 * @tparam ...Args Arguments of the member function
 */
template<typename OwnerClass, typename RetValue, typename ...Args>
class ClassMemberFunctor : public Functor<RetValue, Args...>
{
public:
	using ClassMemberType = RetValue(OwnerClass::*)(Args...);

public:
	ClassMemberFunctor(OwnerClass* owner, ClassMemberType member)
		: m_Owner(owner),
		m_Member(member)
	{}

	RetValue Execute(Args... args) const override
	{
		return ((m_Owner->*m_Member)(std::forward<Args>(args)...));
	}

public:
	OwnerClass* GetOwner() const { return (m_Owner); }
	ClassMemberType GetMember() const { return (m_Member); }

private:
	OwnerClass* m_Owner;
	RetValue(OwnerClass::*m_Member)(Args...);
};

/**
 * @brief Functor for calling a lambda function
 *
 * @tparam RetValue Return value of the lambda function
 * @tparam ...Args Arguments of the lambda function
 */
template<typename RetValue, typename ...Args>
class LambdaFunctor : public Functor<RetValue, Args...>
{

public:
	LambdaFunctor(const std::function<RetValue(Args...)>& lambda)
		: m_Lambda(lambda)
	{}

	RetValue Execute(Args... args) const override
	{
		return (m_Lambda(std::forward(Args...)));
	}

public:
	std::function<RetValue(Args...)> GetLambda() const { return (m_Lambda); }

private:
	std::function<RetValue(Args...)> m_Lambda;
};

/* DELEGATES */

/**
 * @brief  A delegate is a class that holds a pointer to a function, that can bind then called later (Useful for callbacks, events, etc.)
 *
 * @tparam RetValue Return value of the function
 * @tparam ...Args Arguments of the function
 */
template<typename RetValue, typename ...Args>
class OVDelegate
{
public:
	using FunctorType = Functor<RetValue, Args...>;
	using DelegateType = OVDelegate<RetValue, Args...>;

public:
	OVDelegate() = default;
	OVDelegate(DelegateType&& other) noexcept
		: m_Functor(std::move(other.m_Functor))
	{
		other.m_Functor = nullptr; // Do not use SetFunctor, because it will delete the functor
	}
	OVDelegate(FunctorType* functor)
		: m_Functor(functor)
	{}
	DelegateType& operator=(DelegateType&& rhs) noexcept
	{
		SetFunctor(std::move(rhs.m_Functor));
		rhs.m_Functor = nullptr; // Do not use SetFunctor, because it will delete the functor
		return (*this);
	}

public:
	__forceinline RetValue operator()(Args... args) const
	{
		return (Execute(std::forward<Args>(args)...));
	}
	__forceinline RetValue Execute(Args... args) const
	{
		return (m_Functor->Execute(std::forward<Args>(args)...));
	}
	__forceinline bool ExecuteIfBound(Args... args) const
	{
		if (IsValid())
		{
			Execute(std::forward<Args>(args)...);
			return (true);
		}
		return (false);
	}

	void Bind(FunctorType* functor) { SetFunctor(functor); }
	template<typename OwnerClass>
	void BindClassMember(OwnerClass* owner, RetValue(OwnerClass::*member)(Args...)) { SetFunctor(new ClassMemberFunctor<OwnerClass, RetValue, Args...>(owner, member)); }
	void BindLambda(const std::function<RetValue(Args...)>& function) { SetFunctor(new LambdaFunctor<RetValue, Args...>(function)); }
	void Unbind() { SetFunctor(nullptr); }

public:
	bool IsValid() const { return (m_Functor != nullptr); }

private:
	void SetFunctor(FunctorType* functor)
	{
		if (m_Functor)
			delete (m_Functor);
		m_Functor = functor;
	}

private:
	FunctorType* m_Functor;

	template <typename ...Args>
	friend class OVMulticastDelegate;
};
/**
 * @brief Helper macro to create a delegate.
 * @details @see DECLARE_DELEGATE
 *
 * @param DelegateName Name of the new type
 * @param Ret Return type of the function
 * @param ... Arguments of the function (e.g. int, float, const std::string&)
 */
#define DECLARE_RET_DELEGATE(DelgateName, Ret, ...) using DelegateName = OVDelegate<Ret, __VA_ARGS__>;
/**
 * @brief Helper macro to create a delegate.
 * @details This macro will create a new type (named whatever your first param is) that is a OVDelegate with the specified arguments.
 * @note This macros will set the return to void. if you want to change it, use @see DECLARE_RET_DELEGATE
 *
 * @param DelegateName Name of the new type
 * @param ... Arguments of the function (e.g. int, float, const std::string&)
 */
#define DECLARE_DELEGATE(DelegateName, ...) DECLARE_RET_DELEGATE(DelegateName, void, __VA_ARGS__)

/**
 * @brief A multicast delegate is like a delegate but instead of calling one function, it calls multiple functions at once.
 * @note The return value of the delegates is void.
 *
 * @tparam ...Args Arguments of the function
 */
template <typename ...Args>
class OVMulticastDelegate
{
private:
	using RetValue = void;

	using DelegateType = OVDelegate<RetValue, Args...>;

public:
	OVMulticastDelegate()
		: m_Delegates(0)
	{}

public:
	__forceinline void operator()(Args... args) const { Execute(std::forward<Args>(args)...); }
	/**
	 * @brief Calls all the delegates
	 * @param ...args Arguments that will be passed to the delegates
	 */
	__forceinline void Execute(Args... args) const
	{
		for (auto& delegate : m_Delegates)
			delegate.Execute(std::forward<Args>(args)...);
	}

	/**
	 * @brief Add a delegate to the delegate list
	 * @param delegate The delegate to add
	 */
	void Bind(const DelegateType& delegate) { m_Delegates.emplace_back(delegate); }
	/**
	 * @brief Add any kind of functor to the delegate list
	 * @note The functor need to be allocated on the heap, and will be deleted when the delegate is destroyed
	 * @param functor The functor to add
	 */
	void Bind(Functor<RetValue, Args...>* functor) { m_Delegates.emplace_back(functor); }
	/**
	 * @brief Add a Class member function to the delegate list
	 * @tparam OwnerClass Class type of the owner of the member function
	 * @param owner Owner of the member function (Usually "this")
	 * @param member Member function to add (e.g. "&Class::Function")
	 */
	template<typename OwnerClass>
	void BindClassMember(OwnerClass* owner, RetValue(OwnerClass::*member)(Args...))
	{
		DelegateType newDelegate;
		newDelegate.BindClassMember(owner, member);
		m_Delegates.emplace_back(std::move(newDelegate));
	}
	/**
	 * @brief Add a lambda function to the delegate list
	 * @param function Lambda function to add
	 */
	void BindLambda(const std::function<RetValue(Args...)>& function)
	{
		DelegateType newDelegate;
		newDelegate.BindLambda(function);
		m_Delegates.emplace_back(std::move(newDelegate));
	}

	/**
	 * @brief Remove all the delegates that have the owner as owner
	 * @tparam OwnerClass Class type of the owner of the member function
	 * @param owner Owner of the member function (Usually "this")
	 */
	template<typename OwnerClass>
	void UnbindClass(OwnerClass* owner)
	{
		using ClassMemberFunctorType = ClassMemberFunctor<OwnerClass, RetValue, Args...>;

		auto it = m_Delegates.begin();
		while (it != m_Delegates.end())
		{
			ClassMemberFunctorType* functor = static_cast<ClassMemberFunctorType*>(it->m_Functor);
			if (functor && functor->GetOwner() == owner)
				it = m_Delegates.erase(it);
			else
				it++;
		}
	}
	/**
	 * @brief Remove a specific class member function from the delegates
	 * @tparam OwnerClass Class type of the owner of the member function
	 * @param owner Owner of the member function (Usually "this")
	 * @param member Member function to remove (e.g. "&Class::Function")
	 */
	template<typename OwnerClass>
	void UnbindClassMember(OwnerClass* owner, RetValue(OwnerClass::*member)(Args...))
	{
		using ClassMemberFunctorType = ClassMemberFunctor<OwnerClass, RetValue, Args...>;

		auto it = m_Delegates.begin();
		while (it != m_Delegates.end())
		{
			ClassMemberFunctorType* functor = static_cast<ClassMemberFunctorType*>(it->m_Functor);
			if (functor && functor->GetOwner() == owner && functor->GetMember() == member)
			{
				it = m_Delegates.erase(it);
				break;
			}
			else
				it++;
		}
	}
	/**
	 * @brief Remove a specific lambda function from the delegates
	 * @param function Lambda function to remove
	 */
	void UnbindLambda(const std::function<RetValue(Args...)>& function)
	{
		using LambdaFunctorType = LambdaFunctor<RetValue, Args...>;

		auto it = m_Delegates.begin();
		while (it != m_Delegates.end())
		{
			LambdaFunctorType* functor = static_cast<LambdaFunctorType*>(it->m_Functor);
			if (functor && functor->GetLamda() == function)
			{
				it = m_Delegates.erase(it);
				break;
			}
			else
				it++;
		}
	}

private:
	/* List of all the delegates */
	std::vector<DelegateType> m_Delegates;
};
/**
 * @brief Helper macro to create a multicast delegate.
 * @details This macro will create a new type (named whatever your first param is) that is a OVMulticastDelegate with the specified arguments.
 *
 * @param DelegateName Name of the new type
 * @param ... Arguments of the function (e.g. int, float, const std::string&)
 */
#define DECLARE_MULTICAST_DELEGATE(DelegateName, ...) using DelegateName = OVMulticastDelegate<__VA_ARGS__>;
