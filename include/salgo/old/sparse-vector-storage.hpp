#pragma once

#include "vector.hpp"

namespace salgo {





// TODO:
// compact()
// force_compact()




namespace internal {
namespace Sparse_Vector_Storage {

template<
	class _VAL
>
struct Context {

	using Val = _VAL;

	using Sparse_Vector = std::conditional_t<
		std::is_trivially_copy_constructible_v<Val>,
		typename salgo::Vector<Val> :: SPARSE,
		typename salgo::Vector<Val> :: SPARSE :: EXISTS
	>;

	using Handle = typename Sparse_Vector::Handle;


	// forward
	class Sparse_Vector_Storage;





	//
	// accessor
	//
	template<Const_Flag C>
	class Accessor {
	public:
		inline Handle handle() const {
			return _handle;
		}

		inline auto& val() {
			return _owner[ _handle ];
		}

		inline auto& val() const {
			return _owner[ _handle ];
		}

		inline void destruct() {
			static_assert(C == MUTAB, "called erase() on CONST accessor");
			_owner.destruct( _handle );
		}


	private:
		Accessor(Const<Sparse_Vector_Storage,C>& owner, Handle handle)
			: _owner(owner), _handle(handle) {}

		friend Sparse_Vector_Storage;


	private:
		Const<Sparse_Vector_Storage,C>& _owner;
		const Handle _handle;
	};






	class Sparse_Vector_Storage {
	private:
		Sparse_Vector v;

	public:
		using Val = Context::Val;
		using Handle = Context::Handle;
		

		template<class... ARGS>
		auto construct(ARGS&&... args) {
			return Accessor<MUTAB>( *this, v.emplace_back( std::forward<ARGS>(args)... ).handle() );
		}

		void destruct(Handle handle) {
			v.destruct(handle);
		}

		auto& operator[]( Handle handle )       { return v[handle]; }
		auto& operator[]( Handle handle ) const { return v[handle]; }

		auto operator()( Handle handle )       { return Accessor<MUTAB>(*this, handle); }
		auto operator()( Handle handle ) const { return Accessor<CONST>(*this, handle); }
	};



	struct With_Builder : Sparse_Vector_Storage {

		template<class NEW_VAL>
		using VAL = typename
			Context<NEW_VAL> :: With_Builder;

		// hack to avoid circular dependency in List
		template<class>
		using HANDLE_FOR_VAL = internal::Vector::Handle;
	};


}; // struct Context
}  // namespace Storage
}  // namespace internal







template< class VAL >
using Sparse_Vector_Storage = typename internal::Sparse_Vector_Storage::Context<VAL>::With_Builder;








} // namespace salgo




