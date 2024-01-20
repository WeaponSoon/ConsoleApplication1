#pragma once
#include <istream>
#include <memory>
#include <mutex>
#include <vector>

#ifndef SM_DEBUG
#ifdef _DEBUG
#define SM_DEBUG 1
#else
#define SM_DEBUG 0
#endif
#endif

typedef std::uint32_t SSEnumFlag;

#define TRANSPARENT_MEMBER_TYPE(TypeName)\
public:\
TypeName() = default;\
TypeName(const TypeName&) {}\
TypeName(TypeName&&) noexcept {}\
TypeName& operator=(const TypeName&) { return *this; }\
TypeName& operator=( TypeName &&) noexcept { return *this; }\
bool operator==(const TypeName&) const { return true; }\
bool operator!=(const TypeName&) const { return false; }\
~TypeName() = default;\
private:


#define NO_COPY_MEMBER_TYPE(TypeName)\
public:\
TypeName(const TypeName&) = delete;\
TypeName(TypeName&&) noexcept  = delete;\
TypeName& operator=(const TypeName&)  = delete;\
TypeName& operator=( TypeName &&) noexcept = delete;\
private:

#define TRANSPARENT_MEMBER_TYPE_NO_DESTRUCTOR( TypeName )\
public:\
TypeName() = default;\
TypeName(const TypeName&) {} \
TypeName(TypeName&&) noexcept {} \
TypeName& operator=(const TypeName&) { return *this; } \
TypeName& operator=( TypeName &&) noexcept { return *this; } \
bool operator==(const TypeName&) const { return true; } \
bool operator!=(const TypeName&) const { return false; } \
private:


template<typename T, typename Ret, typename...Args>
struct has_init
{
	template<typename C, Ret(C::*)(Args...) = &C::init>
	static constexpr bool Check(C*) { return true; }
	static constexpr bool Check(...) { return false; }

	enum
	{
		value = Check((T*)nullptr),
	};
};

template<typename T, typename Ret>
struct has_release
{
	template<typename C, Ret(C::*)() = &C::release>
	static constexpr bool Check(C*) { return true; }
	static constexpr bool Check(...) { return false; }

	enum
	{
		value = Check((T*)nullptr),
	};
};


struct SSMutexLock
{
private:
	mutable std::mutex mutex_;

	TRANSPARENT_MEMBER_TYPE(SSMutexLock)

public:
	void lock() const
	{
		mutex_.lock();
	}
	void unlock() const
	{
		mutex_.unlock();
	}
};

struct SSSpinLock
{
private:
	mutable std::atomic_flag flag = ATOMIC_FLAG_INIT;

	TRANSPARENT_MEMBER_TYPE(SSSpinLock)
public:
	void lock() const
	{
		while (flag.test_and_set()){}
	}
	void unlock() const
	{
		flag.clear();
	}
};
template<typename T>
struct SSScopeSpinLock
{
private:
	const T* t;

	NO_COPY_MEMBER_TYPE(SSScopeSpinLock)
public:
	SSScopeSpinLock(const T& inLock) : t(&inLock)
	{
		inLock.lock();
	}

	~SSScopeSpinLock()
	{
		t->unlock();
	}
};

//template<typename T>
class SCBase : public std::enable_shared_from_this<SCBase>
{
public:
	//typedef SCBase _SC_INTERNAL_TUTISCBASE;

	virtual ~SCBase() = default;
};

//template<typename T>
//using fake_void = void;
//
//template<typename T, typename U = void>
//class is_a_sc
//{
//public:
//	enum  {value = 0  };
//};
//
//template<typename T>
//class is_a_sc<T, fake_void<typename T::_SC_INTERNAL_TUTISCBASE>> : public std::is_convertible<typename std::decay<T>::type*, typename T::_SC_INTERNAL_TUTISCBASE*>
//{
//
//};

template<typename T>
class is_a_sc : public std::is_convertible<typename std::decay<T>::type*, SCBase*>
{

};

enum class SEObjectLiftStatus
{
	NotInited,
	Inited,
	Released
};

class SCObject : public virtual SCBase
{
	SEObjectLiftStatus ObjectStatus = SEObjectLiftStatus::NotInited;
public:
	virtual ~SCObject() = default;
};

template<typename InterfaceType>
class IInterface : public virtual SCBase
{
	
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

	operator bool() const
	{
		return impl != nullptr;
	}

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

	
	void make_ssptr_by_impl_internal_use_only(const SSPtr_Impl& in_impl) { impl = in_impl; }
	const SSPtr_Impl& use_internal_only_ssptr_get_impl() const { return impl; }

	~SSPtr() = default;

	//can be called with complete class define

	SSDirectAccessor accessor() const
	{
		return impl.get();
	}

	template<typename Convertible, typename...ParamType>
	static typename std::enable_if<has_init<Convertible,void, ParamType...>::value, SSPtr<T>>::type construct(ParamType...args)
	{
		static_assert(is_a_sc<T>::value && is_a_sc<Convertible>::value, "T must be a SCBase");
		static_assert(std::is_base_of<T, Convertible>::value || std::is_same<T, Convertible>::value, "connot convert");
		SSPtr<T> ret;
		std::shared_ptr<Convertible> cvtb = std::make_shared<Convertible>();
		cvtb->init(args...);
		ret.impl = std::dynamic_pointer_cast<T>(cvtb);
		return ret;
	}



	template<typename Convertible, typename...ParamType>
	static typename std::enable_if<!has_init<Convertible, void, ParamType...>::value, SSPtr<T>>::type construct(ParamType...args)
	{
		static_assert(is_a_sc<T>::value && is_a_sc<Convertible>::value, "T must be a SCBase");
		static_assert(std::is_base_of<T, Convertible>::value || std::is_same<T, Convertible>::value, "connot convert");
		SSPtr<T> ret;
		std::shared_ptr<Convertible> cvtb = std::make_shared<Convertible>();
		ret.impl = std::dynamic_pointer_cast<T>(cvtb);
		return ret;
	}

	template<typename DynConvertible>
	typename std::enable_if<std::is_same<T, DynConvertible>::value, SSPtr<DynConvertible>>::type as() const
	{
		static_assert(is_a_sc<T>::value && is_a_sc<DynConvertible>::value, "T must be a SCBase");
		return *this;
	}

	template<typename DynConvertible>
	typename std::enable_if<!std::is_same<T, DynConvertible>::value && std::is_base_of<DynConvertible,T>::value, SSPtr<DynConvertible>>::type as() const
	{
		static_assert(is_a_sc<T>::value && is_a_sc<DynConvertible>::value, "T must be a SCBase");
		SSPtr<DynConvertible> try_ret;
		typename SSPtr<DynConvertible>::SSPtr_Impl& other_impl = const_cast<typename SSPtr<DynConvertible>::SSPtr_Impl&>(try_ret.use_internal_only_ssptr_get_impl());
		other_impl = std::static_pointer_cast<DynConvertible>(impl);
		return try_ret;
	}

	template<typename DynConvertible>
	typename std::enable_if<!std::is_same<T, DynConvertible>::value && !std::is_base_of<DynConvertible, T>::value, SSPtr<DynConvertible>>::type as() const
	{
		static_assert(is_a_sc<T>::value && is_a_sc<DynConvertible>::value, "T must be a SCBase");
		SSPtr<DynConvertible> try_ret;
		typename SSPtr<DynConvertible>::SSPtr_Impl& other_impl = const_cast<typename SSPtr<DynConvertible>::SSPtr_Impl&>(try_ret.use_internal_only_ssptr_get_impl());
		other_impl = std::dynamic_pointer_cast<DynConvertible>(impl);
		return try_ret;
	}

	template<typename Convertable>
	SSPtr(const SSPtr<Convertable>& Other,  typename std::enable_if<!std::is_same<T, Convertable>::value, bool>::type dummy = true) : impl(Other.use_internal_only_ssptr_get_impl())
	{
		static_assert(is_a_sc<T>::value && is_a_sc<Convertable>::value, "T must be a SCBase");
		static_assert(std::is_base_of<T, Convertable>::value || std::is_same<T, Convertable>::value, "T must be the base class of convertible");
	}

	template<typename Convertable>
	typename std::enable_if<!std::is_same<T, Convertable>::value && std::is_base_of<T, Convertable>::value, SSPtr<T>&>::type operator=(const SSPtr<Convertable>& Other)
	{
		static_assert(is_a_sc<T>::value && is_a_sc<Convertable>::value, "T must be a SCBase");
		impl = Other.use_internal_only_ssptr_get_impl();
		return *this;
	}

	T* operator->() const
	{
		static_assert(is_a_sc<T>::value, "T must be a SCBase");
		return impl.get();
	}

	T* get_raw_ptr() const { return impl.get(); }

	template<typename Convertable>
	SSPtr(Convertable* Obj) : impl(std::dynamic_pointer_cast<T>(Obj->shared_from_this()))
	{
		static_assert(is_a_sc<T>::value && is_a_sc<Convertable>::value, "T must be a SCBase");
		static_assert(std::is_base_of<T, Convertable>::value || std::is_same<T, Convertable>::value, "T must be the base class of convertible");
	}
};

class A : public SCBase {};
class B : public A{};

template<typename T>
struct SSWeakPtr final
{
private:
	std::weak_ptr<T> impl;

public:

	SSPtr<T> lock() const
	{
		SSPtr<T> ret;
		ret.make_ssptr_by_impl_internal_use_only(impl.lock());
		return ret;
	}

	SSWeakPtr() noexcept : impl() {}
	SSWeakPtr(decltype(nullptr)) noexcept: impl() {}
	SSWeakPtr(const SSPtr<T>& Other) : impl(Other.use_internal_only_ssptr_get_impl()) {}

	template<typename Convertable>
	SSWeakPtr(const SSPtr<Convertable> Obj) : impl(std::static_pointer_cast<T>(Obj.use_internal_only_ssptr_get_impl()))
	{
		static_assert(is_a_sc<T>::value && is_a_sc<Convertable>::value, "T must be a SCBase");
		static_assert(std::is_base_of<T, Convertable>::value || std::is_same<T, Convertable>::value, "T must be the base class of convertible");
	}

	SSWeakPtr(const SSWeakPtr<T>& other) : impl(other.impl) {}
	SSWeakPtr(SSWeakPtr<T>&& Other) noexcept : impl(std::move(Other.impl)) {}



	SSWeakPtr<T>& operator=(const SSPtr<T>& Other)
	{
		impl = Other.use_internal_only_ssptr_get_impl();
		return *this;
	}

	template<typename Convertable>
	SSWeakPtr<T>& operator=(const SSPtr<Convertable>& Other)
	{
		static_assert(std::is_base_of<T, Convertable>::value || std::is_same<T, Convertable>::value, "T must be the base class of convertible");
		impl = std::static_pointer_cast<T>(Other.use_internal_only_ssptr_get_impl());
		return *this;
	}

	SSWeakPtr<T>& operator=(const SSWeakPtr<T>& Other) noexcept
	{
		impl = (Other.impl);
		return *this;
	}
	SSWeakPtr<T>& operator=(SSWeakPtr<T>&& Other) noexcept
	{
		impl = std::move(Other.impl);
		return *this;
	}



};