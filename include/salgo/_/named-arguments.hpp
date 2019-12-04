#pragma once


#include <utility>

namespace salgo {





struct Empty_TList {};

template<class HEAD, class TAIL>
struct TList {
	using Head = HEAD;
	using Tail = TAIL;

	HEAD head;
	TAIL tail;
};

template<class A, class B>
TList(A,B) -> TList<A,B>;

constexpr auto create_tList() { return Empty_TList{}; }

template<class ARG, class... ARGS>
constexpr auto create_tList(ARG&& arg, ARGS&&... args) {
	return TList {
		std::forward<ARG>(arg),
		create_tList( std::forward<ARGS>(args)... )
	};
}

// // use with perfect-forwarding
// // stores lvalue-ref when T=Type&
// // stores rvalue-ref when T=Type
// template<class T>
// class Reference {
// 	T&& _ref;

// public:
// 	Reference(T&& x) : _ref( std::x) {}
// };











// use with perfect-forwarding
// stores lvalue-ref when VALUE=Type&
// stores rvalue-ref when VALUE=Type
template<class NAME, class VALUE>
struct Named_Argument_Value {
	using Name = NAME;
	using Value = VALUE;
	Value&& value;

	constexpr auto&& get() { return std::forward<Value>(value); }

	constexpr Named_Argument_Value(VALUE&& x) : value( std::forward<VALUE>(x) ) {}

	constexpr auto lvalue_reference() const {
		return Named_Argument_Value<NAME, std::remove_reference_t<VALUE>&> { value };
	}

	constexpr auto const_lvalue_reference() const {
		return Named_Argument_Value<NAME, const std::remove_reference_t<VALUE>&> { value };
	}

	constexpr auto rvalue_reference() const {
		return Named_Argument_Value<NAME, VALUE> { std::forward<VALUE>(value) };
	}

	constexpr auto const_rvalue_reference() const {
		return Named_Argument_Value<NAME, const VALUE> { std::forward<VALUE>(value) };
	}

	//constexpr Named_Argument_Value(Named_Argument_Value&& o) : value(std::forward<VALUE>(o.value)) {}
};

// empty named argument value (result of forwarding non-existing argument)
template<>
struct Named_Argument_Value<void, void> {
	using Name = void;
};












namespace _::named_arguments {

	constexpr auto create_args_list() { return Empty_TList{}; }

	template<class ARG, class... ARGS>
	constexpr auto create_args_list(ARG&& arg, ARGS&&... args) {
		//if constexpr( std::is_same_v<typename ARG::Name, void>) return create_args_list( std::forward<ARGS>(args)... );
		//else
		return TList {
			std::forward<ARG>(arg),
			create_args_list( std::forward<ARGS>(args)... )
		};
	}

	template<class ARG, class TL>
	constexpr auto& get(ARG, TL& tlist) {
		static_assert(!std::is_same_v<TL, Empty_TList>, "named argument not found");
		if constexpr(std::is_same_v<typename TL::Head::Name, typename ARG::Name>) return tlist.head;
		else return get(ARG(), tlist.tail);
	}

	template<class ARG>
	constexpr bool has() {
		return false;
	}

	template<class ARG, class T, class... Ts>
	constexpr bool has() {
		if constexpr(std::is_same_v<typename T::Name, typename ARG::Name>) return true;
		else return has<ARG, Ts...>();
	}
}


template<class... ARGS>
struct Named_Arguments {
	using TLIST = decltype( _::named_arguments::create_args_list( std::declval<ARGS>()... ) );
	TLIST _args;

	constexpr Named_Arguments(ARGS&&... args) : _args(
		_::named_arguments::create_args_list( std::forward<ARGS>(args)... )
	) {}

	template<class ARG>
	constexpr auto&& operator()(ARG arg) { return _::named_arguments::get(arg, _args).get(); }

	template<class ARG, class DEF>
	constexpr auto&& operator()(ARG arg, DEF&& def) {
		namespace det = _::named_arguments;
		if constexpr(has(arg)) return det::get(arg, _args).value;
		else return std::forward<DEF>(def);
	}

	template<class ARG>
	static constexpr bool has(ARG) { return _::named_arguments::has<ARG, ARGS...>(); }


	constexpr auto&& forward() { return std::move(*this); }
};

// deduction guide for perfect forwarding
template<class... ARGS>
Named_Arguments(ARGS&&...) -> Named_Arguments<ARGS...>;









template<class NAME>
struct Named_Argument {
	using Name = NAME;

	template<class X>
	constexpr auto operator=(X&& x) const {
		return Named_Argument_Value<NAME, const X>( std::forward<X>(x) );
	}

	template<class X>
	constexpr auto operator<<(X&& x) const {
		return Named_Argument_Value<NAME, X>( std::forward<X>(x) );
	}


	template<class... ARGS>
	constexpr auto operator=(Named_Arguments<ARGS...>&& args) const {
		if constexpr(_::named_arguments::has<Named_Argument,ARGS...>()) {
			return _::named_arguments::get(*this, args._args).const_rvalue_reference();
		}
		else {
			return Named_Argument_Value<void,void>();
		}
	}

	template<class... ARGS>
	constexpr auto operator<<(Named_Arguments<ARGS...>&& args) const {
		if constexpr(_::named_arguments::has<Named_Argument,ARGS...>()) {
			return _::named_arguments::get(*this, args._args).rvalue_reference();
		}
		else {
			return Named_Argument_Value<void,void>();
		}
	}


	template<class... ARGS>
	constexpr auto operator=(Named_Arguments<ARGS...>& args) const {
		if constexpr(_::named_arguments::has<Named_Argument,ARGS...>()) {
			return _::named_arguments::get(*this, args._args).const_lvalue_reference();
		}
		else {
			return Named_Argument_Value<void,void>();
		}
	}

	template<class... ARGS>
	constexpr auto operator<<(Named_Arguments<ARGS...>& args) const {
		if constexpr(_::named_arguments::has<Named_Argument,ARGS...>()) {
			return _::named_arguments::get(*this, args._args).lvalue_reference();
		}
		else {
			return Named_Argument_Value<void,void>();
		}
	}

};


#define NAMED_ARGUMENT(NAME) \
struct _Named_Argument_##NAME {}; \
static constexpr Named_Argument<_Named_Argument_##NAME> NAME;







}
