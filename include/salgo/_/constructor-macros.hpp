#pragma once


#define FORWARDING_CONSTRUCTOR_VAR(SELF,VAR) \
	template<class... _ARGS, class = decltype(decltype(VAR)(std::declval<_ARGS>()...))> \
	SELF(_ARGS&&... args) : VAR( std::forward<_ARGS>(args)... )


#define FORWARDING_CONSTRUCTOR(SELF,BASE) \
	template<class... _ARGS, class = decltype(BASE(std::declval<_ARGS>()...))> \
	SELF(_ARGS&&... args) : BASE( std::forward<_ARGS>(args)... )

#define FORWARDING_ASSIGNMENT(BASE) template<class X> auto& operator=(X&& x) { return BASE::operator=( std::forward<X>(x) ); }

