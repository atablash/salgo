#pragma once

#include "memory-block.hpp"

namespace salgo {





// TODO:
// compact()
// force_compact()




namespace internal {
namespace Memory_Block_Storage {

template<
	class _VAL
>
struct Context {

	using Val = _VAL;

	using Memory_Block = salgo::Memory_Block<Val>;
	using Handle = typename Memory_Block::Handle;


	// forward
	class Memory_Block_Storage;





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
		Accessor(Const<Memory_Block_Storage,C>& owner, Handle handle)
			: _owner(owner), _handle(handle) {}

		friend Memory_Block_Storage;


	private:
		Const<Memory_Block_Storage,C>& _owner;
		const Handle _handle;
	};






	class Memory_Block_Storage {
	private:
		Memory_Block v;

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



	struct With_Builder : Memory_Block_Storage {

		template<class NEW_VAL>
		using VAL = typename
			Context<NEW_VAL> :: With_Builder;

	};


}; // struct Context
}  // namespace Storage
}  // namespace internal







template< class VAL >
using Memory_Block_Storage = typename internal::Memory_Block_Storage::Context<VAL>::With_Builder;








} // namespace salgo




