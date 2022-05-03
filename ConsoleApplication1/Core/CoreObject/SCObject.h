#pragma once
#include <istream>
#include <memory>

#ifndef SM_DEBUG
#ifdef _DEBUG
#define SM_DEBUG 1
#else
#define SM_DEBUG 0
#endif
#endif


class SCObject : public std::enable_shared_from_this<SCObject>
{
public:
	virtual ~SCObject() = default;
};

//SSPtr is a type of pointer which can hold a reference to a SCObject, when there are
//no reference to a specific SCObject, the SCObject will be
//deleted automatically.
template<typename T>
struct SSPtr final
{
public:
	typedef std::shared_ptr<T> SSPtr_Impl;
private:
	SSPtr_Impl impl;

public:
	//fast accessor used in function body to avoid double-addressing
	//e.g:
	//void func()
	//{
	//	SSPtr<SomeClass> Ptr;///
	//	Ptr->some_op();//double-addressing may occur
	//	SSPtr<SomeClass>::SSDirectAccessor&& accessor = Ptr.accessor();
	//	accessor->some_op();//no double-addressing
	//}
	//it will take come performance advantage if there are lots of un-ref operations in a function body
	//you cannot store any SSDirectAccessor
	struct SSDirectAccessor
	{
		friend struct SSPtr<T>;
		T* Accessor = nullptr;

		inline T* operator->() const { return Accessor; }
		~SSDirectAccessor() = default;
	private:
		SSDirectAccessor(T* Ptr) : Accessor(Ptr)
		{
		}
		SSDirectAccessor(const SSDirectAccessor& Other) : Accessor(Other.Accessor)
		{
		}
		SSDirectAccessor(SSDirectAccessor&& Other) noexcept : Accessor(Other.Accessor)
		{
			Other.Accessor = nullptr;
		}
		SSDirectAccessor& operator=(const SSDirectAccessor& Other)
		{
			return *this;
		}
		SSDirectAccessor& operator=(SSDirectAccessor&& Other) noexcept
		{
			return *this;
		}
	};

	//can be called with only forward-declaration
	SSPtr() : impl(nullptr) {}
	SSPtr(decltype(nullptr)) : impl(nullptr){}
	SSPtr(const SSPtr<T>& Other) : impl(Other.impl) {}
	SSPtr(SSPtr<T>&& Other) noexcept : impl(std::move(Other.impl)) {}
	SSPtr<T>& operator=(const SSPtr<T>& Other)
	{
		impl = Other.impl;
		return *this;
	}
	SSPtr<T>& operator=(SSPtr<T>&& Other) noexcept
	{
		impl = std::move(Other.impl);
		return *this;
	}

	const SSPtr_Impl& use_internal_only_ssptr_get_impl() const { return impl; }

	~SSPtr() = default;

	//can be called with complete class define

	SSDirectAccessor accessor() const
	{
		return impl.get();
	}

	template<typename Convertible, typename...ParamType>
	static SSPtr<T> construct(ParamType...args)
	{
		return SSPtr<T>(new Convertible(args...));
	}

	template<typename DynConvertible>
	typename std::enable_if<std::is_same<T, DynConvertible>::value, SSPtr<DynConvertible>>::type as() const
	{
		return *this;
	}

	template<typename DynConvertible>
	typename std::enable_if<!std::is_same<T, DynConvertible>::value && std::is_base_of<DynConvertible,T>::value, SSPtr<DynConvertible>>::type as() const
	{
		return SSPtr<DynConvertible>(*this);
	}

	template<typename DynConvertible>
	typename std::enable_if<!std::is_same<T, DynConvertible>::value && !std::is_base_of<DynConvertible, T>::value, SSPtr<DynConvertible>>::type as() const
	{
		SSPtr<DynConvertible> try_ret;
		typename SSPtr<DynConvertible>::SSPtr_Impl& other_impl = const_cast<typename SSPtr<DynConvertible>::SSPtr_Impl&>(try_ret.use_internal_only_ssptr_get_impl());
		other_impl = std::dynamic_pointer_cast<DynConvertible>(impl);
		return try_ret;
	}

	template<typename Convertable>
	SSPtr(const SSPtr<Convertable>& Other,  typename std::enable_if<!std::is_same<T, Convertable>::value, bool>::type dummy = true) : impl(Other.use_internal_only_ssptr_get_impl())
	{
		static_assert(std::is_base_of<T, Convertable>::value || std::is_same<T, Convertable>::value, "T must be the base class of convertible");
	}

	template<typename Convertable>
	typename std::enable_if<!std::is_same<T, Convertable>::value && std::is_base_of<T, Convertable>::value, SSPtr<T>&>::type operator=(const SSPtr<Convertable>& Other)
	{
		impl = Other.use_internal_only_ssptr_get_impl();
		return *this;
	}

	T* operator->() const
	{
		return impl.get();
	}
private:
	template<typename Convertable>
	SSPtr(Convertable* Obj) : impl(Obj)
	{
		static_assert(std::is_base_of<T, Convertable>::value || std::is_same<T, Convertable>::value, "T must be the base class of convertible");
	}
};