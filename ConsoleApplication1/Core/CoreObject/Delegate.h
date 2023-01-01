#pragma once
#include "Core/CoreObject/SCObject.h"
#include <functional>

template<typename RetVal, typename...Args>
class SCDelegate
{

	std::function<RetVal(Args...)> caller;

	struct SCMemberFunctionCallerBase
	{
		virtual bool is_valid() = 0;
		virtual RetVal invoke(Args...args) = 0;
	};
	template<typename T>
	struct SCMemberFunctionCaller : public SCMemberFunctionCallerBase
	{
		typedef RetVal(T::* InnerCaller)(Args...);
		SSPtr<T> t;
		InnerCaller inner_caller = nullptr;

		SCMemberFunctionCaller(SSPtr<T> in_t, InnerCaller in_caller) : t(in_t), inner_caller(in_caller)
		{}
		virtual bool is_valid() override
		{
			return t && inner_caller;
		}
		RetVal invoke(Args...args) override
		{
			auto ptr = t.get_raw_ptr();
			return (ptr->*inner_caller)(args...);
		}
	};

	std::unique_ptr<SCMemberFunctionCallerBase> member_caller;


public:
	operator bool() const
	{
		return (member_caller.get() && member_caller->is_valid()) || caller;
	}

	template<typename Call>
	void bind(const Call& in_call)
	{
		caller = in_call;
		member_caller.reset();
	}

	template<typename ObjType>
	void bind(SSPtr<ObjType> in_obj, RetVal(ObjType::* in_func)(Args...))
	{
		member_caller.reset();
		caller = nullptr;

		member_caller = std::unique_ptr<SCMemberFunctionCallerBase>(new SCMemberFunctionCaller<ObjType>(in_obj, in_func));
	}

	RetVal operator()(Args...args)
	{
		if(member_caller.get())
		{
			return member_caller->invoke(std::forward<Args>(args)...);
		}
		return caller(std::forward<Args>(args)...);
	}

};



//class GG : public SCObject
//{
//public:
//	int tew(double, float)
//	{
//		SCDelegate<int, double, float> tw;
//		SSPtr<GG> test = SSPtr<GG>::construct<GG>();
//		tw.bind(test , &GG::tew);
//		tw.bind([](double, float)->int {return 0; });
//		tw(1, 2);
//	}
//
//
//
//};