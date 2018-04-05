#pragma once


namespace salgo {





// TODO:
// compact()
// force_compact()




namespace internal {
namespace Vector_Storage {

template<
	class _VAL
>
struct Context {

	using Val = _VAL;

	using Vector = Sparse_Vector< Val >;


	class Vector_Storage {
	private:
		Vector v;

	public:
		using Val = Context::Val;
		
		using Handle = Vector::Handle;

		template<class... ARGS>
		Handle construct(ARGS&&... args) {
			return v.emplace_back( std::forward<ARGS>(args)... ).handle();
		}

		void destruct(Handle handle) {
			v.erase(handle);
		}

		auto& operator[]( Handle handle )       { return v.at(handle); }
		auto& operator[]( Handle handle ) const { return v.at(handle); }
	};



}; // struct Context
}  // namespace Storage
}  // namespace internal







template< class VAL >
using Vector_Storage = internal::Vector_Storage<VAL>::Vector_Storage;








} // namespace salgo




