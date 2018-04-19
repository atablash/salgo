#pragma once


#include "../common.hpp"
#include "../accessors.hpp"
#include "../vector.hpp"
#include "../segment.hpp"
#include "../int-handle.hpp"
#include "../hash-table.hpp"

#include <Eigen/Dense>

//#include <unordered_set>




#define MESH_FRIENDS \
	friend Mesh; \
	friend A_Vert<MUTAB>; \
	friend A_Vert<CONST>; \
	friend A_Poly<MUTAB>; \
	friend A_Poly<CONST>; \
	friend A_Poly_Vert<MUTAB>; \
	friend A_Poly_Vert<CONST>; \
	friend A_Poly_Edge<MUTAB>; \
	friend A_Poly_Edge<CONST>; \
	friend I_Vert<MUTAB>; \
	friend I_Vert<CONST>; \
	friend I_Poly<MUTAB>; \
	friend I_Poly<CONST>; \
	friend I_Poly_Poly_Vert<MUTAB>; \
	friend I_Poly_Poly_Vert<CONST>; \
	friend I_Poly_Poly_Edge<MUTAB>; \
	friend I_Poly_Poly_Edge<CONST>; \
	friend I_Vert_Poly_Vert<MUTAB>; \
	friend I_Vert_Poly_Vert<CONST>; \
	friend A_Verts<MUTAB>; \
	friend A_Verts<CONST>; \
	friend A_Polys<MUTAB>; \
	friend A_Polys<CONST>; \
	friend A_Poly_Verts<MUTAB>; \
	friend A_Poly_Verts<CONST>; \
	friend A_Poly_Poly_Verts<MUTAB>; \
	friend A_Poly_Poly_Verts<CONST>; \
	friend A_Poly_Poly_Edges<MUTAB>; \
	friend A_Poly_Poly_Edges<CONST>; \
	friend A_Vert_Polys<MUTAB>; \
	friend A_Vert_Polys<CONST>; \
	friend typename Vert_Polys_Context::template Accessor<MUTAB>; \
	friend typename Vert_Polys_Context::template Accessor<CONST>; \












namespace salgo {
namespace internal {
namespace Mesh {









	//
	// add members
	//
	template<class PROPS, bool> struct Add_props { PROPS props; };
	template<class PROPS> struct Add_props<PROPS,false> {};


	template<class H, bool> struct Add_subvert_link { H subvert; };
	template<class H> struct Add_subvert_link<H,false> {};


	template<class T, bool> struct Add_link { T link = T(); };
	template<class T> struct Add_link<T,false> {};

	template<class T, bool> struct Add_poly_links { T poly_links; };
	template<class T> struct Add_poly_links<T,false> {};










	template<
		class _SCALAR,
		class _VERT_PROPS,
		class _SUBVERT_PROPS,
		class _POLY_PROPS,
		class _POLY_VERT_PROPS,
		class _POLY_EDGE_PROPS,
		bool _VERTS_ERASABLE,
		bool _POLYS_ERASABLE,
		bool _EDGE_LINKS,
		bool _VERT_POLY_LINKS
	>
	struct Context {

		using Scalar          = _SCALAR;

		using Vert_Props      = _VERT_PROPS;
		using Subvert_Props   = _SUBVERT_PROPS;
		using Poly_Props      = _POLY_PROPS;
		using Poly_Vert_Props = _POLY_VERT_PROPS;
		using Poly_Edge_Props = _POLY_EDGE_PROPS;

		static constexpr bool Verts_Erasable = _VERTS_ERASABLE;
		static constexpr bool Polys_Erasable = _POLYS_ERASABLE;

		static constexpr bool Has_Edge_Links      = _EDGE_LINKS;
		static constexpr bool Has_Vert_Poly_Links = _VERT_POLY_LINKS;



		static constexpr bool Has_Vert_Props =
			!std::is_same_v<Vert_Props, void>;

		static constexpr bool Has_Subvert_Props =
			!std::is_same_v<Subvert_Props, void>;

		static constexpr bool Has_Poly_Props =
			!std::is_same_v<Poly_Props, void>;

		static constexpr bool Has_Poly_Vert_Props =
			!std::is_same_v<Poly_Vert_Props, void>;

		static constexpr bool Has_Poly_Edge_Props =
			!std::is_same_v<Poly_Edge_Props, void>;











		//
		// handles - forward declarations
		//
		struct H_Vert;
		struct H_Poly;
		struct H_Subvert;
		struct H_Poly_Vert;
		struct H_Poly_Edge;



		struct H_Vert    : Int_Handle_Base<H_Vert>    { FORWARDING_CONSTRUCTOR(H_Vert,    Int_Handle_Base<H_Vert>) {}    };
		struct H_Poly    : Int_Handle_Base<H_Poly>    { FORWARDING_CONSTRUCTOR(H_Poly,    Int_Handle_Base<H_Poly>) {}    };
		struct H_Subvert : Int_Handle_Base<H_Subvert> { FORWARDING_CONSTRUCTOR(H_Subvert, Int_Handle_Base<H_Subvert>) {} };


		struct H_Poly_Vert {
			H_Poly poly = H_Poly();
			int ith = 3; // 0,1,2 - char would be enough

			H_Poly_Vert() = default;
			H_Poly_Vert(H_Poly new_poly, char new_ith) : poly(new_poly), ith(new_ith) {}

			auto next(int i = 1) const { return H_Poly_Vert( poly, (            ith +i) % 3 ); }
			auto prev(int i = 1) const { return H_Poly_Vert( poly, (333'333'333+ith -i) % 3 ); }

			inline auto next_edge() const { return H_Poly_Edge( poly, (  ith   )     ); }
			inline auto prev_edge() const { return H_Poly_Edge( poly, (3+ith -1) % 3 ); }

			bool valid() const { return poly.valid(); }
			void reset() { poly.reset(); }

			bool operator==(const H_Poly_Vert& o) const { return poly == o.poly && ith == o.ith; }
			bool operator!=(const H_Poly_Vert& o) const { return poly != o.poly || ith != o.ith; }

			auto hash() const { return (poly << 2) | ith; }
		};


		struct H_Poly_Edge {
			H_Poly poly = H_Poly();
			int ith = 3; // 0,1,2 - char would be enough

			H_Poly_Edge() = default;
			H_Poly_Edge(H_Poly new_poly, char new_ith) : poly(new_poly), ith(new_ith) {}

			auto next(int i = 1) const { return H_Poly_Edge( poly, (            ith +i) % 3 ); }
			auto prev(int i = 1) const { return H_Poly_Edge( poly, (333'333'333+ith -i) % 3 ); }

			inline auto next_vert() const { return H_Poly_Vert( poly, (ith +1) % 3 ); }
			inline auto prev_vert() const { return H_Poly_Vert( poly, (ith   )     ); }

			bool valid() const { return poly.valid(); }
			void reset() { poly.reset(); }

			bool operator==(const H_Poly_Edge& o) const { return poly == o.poly && ith == o.ith; }
			bool operator!=(const H_Poly_Edge& o) const { return poly != o.poly || ith != o.ith; }

			auto hash() const { return (poly << 2) | ith; }
		};











		using Vert_Poly_Links = salgo::Hash_Table<H_Poly_Vert>;

		struct H_Vert_Poly : Pair_Handle_Base<H_Vert_Poly, Int_Handle<>, typename Vert_Poly_Links::Handle> {
			using BASE = Pair_Handle_Base<H_Vert_Poly, Int_Handle<>, typename Vert_Poly_Links::Handle>;
			FORWARDING_CONSTRUCTOR(H_Vert_Poly, BASE) {}
		};





		class Mesh;

		struct Verts_Context;
		struct Vert_Polys_Context;

		struct Polys_Context;


		//
		// accessors
		//
		template<Const_Flag C> using A_Vert = typename Verts_Context::template Accessor<C>;
		template<Const_Flag C> using A_Vert_Poly = typename Vert_Polys_Context::template Accessor<C>;
		template<Const_Flag C> class A_Poly;
		template<Const_Flag C> class A_Poly_Vert;
		template<Const_Flag C> class A_Poly_Edge;

		template<Const_Flag C> class A_Verts;
		template<Const_Flag C> class A_Polys;
		template<Const_Flag C> class A_Vert_Polys;
		template<Const_Flag C> class A_Poly_Verts;
		template<Const_Flag C> class A_Poly_Poly_Verts;
		template<Const_Flag C> class A_Poly_Poly_Edges;


		//
		// iterators
		//
		template<Const_Flag C> using I_Vert = typename Verts_Context::template Iterator<C>;
		template<Const_Flag C> using I_Vert_Poly = typename Vert_Polys_Context::template Iterator<C>;
		template<Const_Flag C> class I_Poly;
		template<Const_Flag C> class I_Poly_Vert;
		template<Const_Flag C> class I_Poly_Poly_Vert;
		template<Const_Flag C> class I_Poly_Poly_Edge;
		template<Const_Flag C> class I_Vert_Poly_Vert;









		//
		// raw data types
		//
		struct Vert :
				Add_props<Vert_Props, Has_Vert_Props>,
				Add_poly_links<Vert_Poly_Links, Has_Vert_Poly_Links> {

			Eigen::Matrix<Scalar,3,1> pos = {0,0,0};

			template<class... ARGS>
			Vert(ARGS&&... args) : pos( std::forward<ARGS>(args)... ) {}
		};

		struct Poly_Vert :
				Add_props<Poly_Vert_Props, Has_Poly_Vert_Props>,
				Add_subvert_link<H_Subvert, Has_Subvert_Props> {
			H_Vert vert = H_Vert();
		};

		struct Poly_Edge :
			Add_props<Poly_Edge_Props, Has_Poly_Edge_Props>,
			Add_link<H_Poly_Edge, Has_Edge_Links> {};

		struct Poly : Add_props<Poly_Props, Has_Poly_Props> {
			std::array<Poly_Vert,3> verts;
			std::array<Poly_Edge,3> edges;

			Poly(H_Vert a, H_Vert b, H_Vert c) {
				verts[0].vert = a;
				verts[1].vert = b;
				verts[2].vert = c;
			}
		};





		//
		//
		// accessors
		//
		//



		template<Const_Flag C>
		class A_Poly {
		public:
			using Mesh = Context::Mesh;
		
		public:
			inline operator auto() const {  return _handle;  }
			inline auto handle()   const {  return _handle;  }

			inline auto& mesh()       {  return _mesh;  }
			inline auto& mesh() const {  return _mesh;  }

			inline auto vert(int ith) {
				DCHECK_GE(ith, 0) << "out of bounds";
				DCHECK_LT(ith, 3) << "out of bounds";
				return _mesh( H_Poly_Vert( _handle, ith ) );
			};

			inline auto vert(int ith) const {
				DCHECK_GE(ith, 0) << "out of bounds";
				DCHECK_LT(ith, 3) << "out of bounds";
				return _mesh( H_Poly_Vert( _handle, ith ) );
			};



			inline auto poly_vert(int ith) {
				DCHECK_GE(ith, 0) << "out of bounds";
				DCHECK_LT(ith, 3) << "out of bounds";
				return A_Poly_Vert<C>( _mesh, H_Poly_Vert(_handle, ith) );
			};

			inline auto poly_vert(int ith) const {
				DCHECK_GE(ith, 0) << "out of bounds";
				DCHECK_LT(ith, 3) << "out of bounds";
				return A_Poly_Vert<CONST>( _mesh, H_Poly_Vert(_handle, ith) );
			};


			inline auto poly_edge(int ith) {
				DCHECK_GE(ith, 0) << "out of bounds";
				DCHECK_LT(ith, 3) << "out of bounds";
				return A_Poly_Edge<C>( _mesh, H_Poly_Edge(_handle, ith) );
			};

			inline auto poly_edge(int ith) const {
				DCHECK_GE(ith, 0) << "out of bounds";
				DCHECK_LT(ith, 3) << "out of bounds";
				return A_Poly_Edge<CONST>(_mesh, _handle, ith);
			};

			inline auto verts()       {  return A_Poly_Verts<C>    (_mesh, _handle);  }
			inline auto verts() const {  return A_Poly_Verts<CONST>(_mesh, _handle);  }

			inline auto poly_verts()       {  return A_Poly_Poly_Verts<C>    (_mesh, _handle);  }
			inline auto poly_verts() const {  return A_Poly_Poly_Verts<CONST>(_mesh, _handle);  }

			inline auto poly_edges()       {  return A_Poly_Poly_Edges<C>    (_mesh, _handle);  }
			inline auto poly_edges() const {  return A_Poly_Poly_Edges<CONST>(_mesh, _handle);  }


			void erase() {
				static_assert(C==MUTAB, "called erase() on CONST A_Poly accessor");
				_mesh.erase( _handle );
			}


			inline auto& props()       {  return _mesh.poly_props( _handle );  }
			inline auto& props() const {  return _mesh.poly_props( _handle );  }

			template<Const_Flag CC>
			bool operator==(const A_Poly<CC>& o) const {
				DCHECK_EQ(&_mesh, &o._mesh);
				return _handle == o._handle;
			}

			template<Const_Flag CC>
			bool operator!=(const A_Poly<CC>& o) const {
				DCHECK_EQ(&_mesh, &o._mesh);
				return _handle != o._handle;
			}


		private:
			A_Poly(Const<Mesh,C>& mesh, H_Poly key) : _mesh(mesh), _handle(key) {}
			MESH_FRIENDS;

		private:
			Const<Mesh,C>& _mesh;
			const H_Poly _handle;
		};









		template<Const_Flag C>
		class A_Poly_Vert {
		public:
			using Mesh = Context::Mesh;
		
		public:
			operator auto() const {  return _handle;  }
			auto handle()   const {  return _handle;  }

			auto& mesh()       {  return _mesh;  }
			auto& mesh() const {  return _mesh;  }

			auto poly()       {  return A_Poly<C>    (_mesh, _handle.poly);  }
			auto poly() const {  return A_Poly<CONST>(_mesh, _handle.poly);  }

			auto vert()       {  return A_Vert<C>    (&_mesh, _mesh._poly_vert( _handle ).vert);  }
			auto vert() const {  return A_Vert<CONST>(&_mesh, _mesh._poly_vert( _handle ).vert);  }

			template<class VERT>
			void change_vert(VERT&& v) {
				static_assert(C==MUTAB, "called change_vert() on CONST A_Poly_Vert accessor");
				_mesh.poly_vert_change( _handle, std::forward<VERT>(v) );
			}

			auto& pos()       {  return _mesh.poly_vert_pos( _handle );  }
			auto& pos() const {  return _mesh.poly_vert_pos( _handle );  }

			auto next()       {  return A_Poly_Vert<C>    (_mesh, _handle.next());  }
			auto next() const {  return A_Poly_Vert<CONST>(_mesh, _handle.next());  }

			auto prev()       {  return A_Poly_Vert<C>    (_mesh, _handle.prev());  }
			auto prev() const {  return A_Poly_Vert<CONST>(_mesh, _handle.prev());  }


			auto next_poly_edge()       {  return A_Poly_Edge<C>    (_mesh, _handle.next_edge());  }
			auto next_poly_edge() const {  return A_Poly_Edge<CONST>(_mesh, _handle.next_edge());  }

			auto prev_poly_edge()       {  return A_Poly_Edge<C>    (_mesh, _handle.prev_edge());  }
			auto prev_poly_edge() const {  return A_Poly_Edge<CONST>(_mesh, _handle.prev_edge());  }


			auto& props()       {  return _mesh.poly_vert_props( _handle );  }
			auto& props() const {  return _mesh.poly_vert_props( _handle );  }


		public:
			template<Const_Flag CC>
			bool operator==(const A_Poly_Vert<CC>& o) const {
				DCHECK_EQ(&_mesh, &o._mesh);
				return _handle == o._handle;
			}

			template<Const_Flag CC>
			bool operator!=(const A_Poly_Vert<CC>& o) const {
				DCHECK_EQ(&_mesh, &o._mesh);
				return _handle != o._handle;
			}


		private:
			A_Poly_Vert(Const<Mesh,C>& mesh, H_Poly_Vert handle) : _mesh(mesh), _handle(handle) {}
			MESH_FRIENDS;

		private:
			Const<Mesh,C>& _mesh;
			const H_Poly_Vert _handle;
		};






		template<Const_Flag C>
		class A_Poly_Edge {
		public:
			using Mesh = Context::Mesh;
		
		public:
			inline operator auto() const {  return _handle;  }
			inline auto handle()   const {  return _handle;  }

			inline auto& mesh()       {  return _mesh;  }
			inline auto& mesh() const {  return _mesh;  }


			inline auto poly()       {  return A_Poly<C>    (_mesh, _handle.poly);  }
			inline auto poly() const {  return A_Poly<CONST>(_mesh, _handle.poly);  }


			inline auto next()       {  return A_Poly_Edge<C>    (_mesh, _handle.next());  }
			inline auto next() const {  return A_Poly_Edge<CONST>(_mesh, _handle.next());  }

			inline auto prev()       {  return A_Poly_Edge<C>    (_mesh, _handle.prev());  }
			inline auto prev() const {  return A_Poly_Edge<CONST>(_mesh, _handle.prev());  }


			inline auto next_vert()       {  return _mesh( _mesh.poly_vert_vert( _handle.next_vert() ) ); }
			inline auto next_vert() const {  return _mesh( _mesh.poly_vert_vert( _handle.next_vert() ) ); }

			inline auto prev_vert()       {  return _mesh( _mesh.poly_vert_vert( _handle.prev_vert() ) ); }
			inline auto prev_vert() const {  return _mesh( _mesh.poly_vert_vert( _handle.prev_vert() ) ); }


			inline auto next_poly_vert()       {  return A_Poly_Vert<C>    (_mesh, _handle.next_vert());  }
			inline auto next_poly_vert() const {  return A_Poly_Vert<CONST>(_mesh, _handle.next_vert());  }

			inline auto prev_poly_vert()       {  return A_Poly_Vert<C>    (_mesh, _handle.prev_vert());  }
			inline auto prev_poly_vert() const {  return A_Poly_Vert<CONST>(_mesh, _handle.prev_vert());  }


			inline auto& props()       {  return _mesh.poly_edge_props( _handle );  }
			inline auto& props() const {  return _mesh.poly_edge_props( _handle );  }

			auto segment() const {
				return Segment<Scalar, 3>{
					_mesh.poly_vert_pos( _handle.prev_vert() ),
					_mesh.poly_vert_pos( _handle.next_vert() )
				};
			}

			void unlink() {
				static_assert(C==MUTAB, "unlink() called on CONST accessor");
				_check_link();

				auto other = _mesh.poly_edge_link( _handle );
				_mesh.poly_edge_link( _handle ).reset();
				_mesh.poly_edge_link( other ).reset();
			}

			void link(A_Poly_Edge other) {
				static_assert(C==MUTAB, "link() called on CONST accessor");
				DCHECK( !_mesh.poly_edge_link( _handle ).valid() );
				DCHECK( !_mesh.poly_edge_link( other._handle ).valid() );

				_mesh.poly_edge_link( _handle ) = other._handle;
				_mesh.poly_edge_link( other._handle ) = _handle;

				_check_link();
			}

			auto linked_edge() {
				auto other = _mesh.poly_edge_link( _handle );
				DCHECK( other.valid() ) << "following broken edge link";
				return A_Poly_Edge(_mesh, other);
			}

			bool has_link() const {
				return _mesh.poly_edge_link( _handle ).valid();
			}



		private:
			void _check_link() const {
				auto other = _mesh.poly_edge_link( _handle );
				DCHECK( other.valid() );
				DCHECK( _mesh.poly_edge_link( other ).valid() );
				DCHECK( _mesh.poly_edge_link( other ) == _handle );
			}



		public:
			template<Const_Flag CC>
			bool operator==(const A_Poly_Edge<CC>& o) const {
				DCHECK_EQ(&_mesh, &o._mesh);
				return _handle == o._handle;
			}

			template<Const_Flag CC>
			bool operator!=(const A_Poly_Edge<CC>& o) const {
				DCHECK_EQ(&_mesh, &o._mesh);
				return _handle != o._handle;
			}




		private:
			A_Poly_Edge(Const<Mesh,C>& mesh, H_Poly_Edge handle) : _mesh(mesh), _handle(handle) {}
			MESH_FRIENDS;

		private:
			Const<Mesh,C>& _mesh;
			const H_Poly_Edge _handle;
		};



		//
		// access/iterate verts
		//
		struct Verts_Context {
			using Container = Mesh;
			using Handle = H_Vert;

			template<Const_Flag C>
			class Accessor : public Accessor_Base<C,Verts_Context> {
				using BASE = Accessor_Base<C,Verts_Context>;
				using BASE::_container;
				using BASE::_handle;

			public:
				FORWARDING_CONSTRUCTOR(Accessor, BASE) {}

			public:
				using Mesh = Container;

				auto& mesh()       {  return _container;  }
				auto& mesh() const {  return _container;  }

				auto& pos()       {  return _container->vert_pos( _handle );  }
				auto& pos() const {  return _container->vert_pos( _handle );  }

				auto& props()       {  return _container->vert_props( _handle );  }
				auto& props() const {  return _container->vert_props( _handle );  }

				auto vert_polys()       { return A_Vert_Polys<C>    ( *_container, _handle ); }
				auto vert_polys() const { return A_Vert_Polys<CONST>( *_container, _handle ); }


				void erase() {
					static_assert(C==MUTAB, "called erase() on CONST A_Vert accessor");
					_container.erase( _handle );
				}


				template<Const_Flag CC>
				bool operator==(const Accessor<CC>& o) const {
					DCHECK_EQ(_container, o._container);
					return _handle == o._handle;
				}

				template<Const_Flag CC>
				bool operator!=(const Accessor<CC>& o) const {
					return !(*this == o);
				}
			}; // Accessor


			template<Const_Flag C>
			class Iterator : public Iterator_Base<C,Verts_Context> {
				using BASE = Iterator_Base<C,Verts_Context>;
				using BASE::_container;
				using BASE::_handle;

			public:
				FORWARDING_CONSTRUCTOR(Iterator, BASE) {}

			private:
				friend BASE;

				void _increment() {
					do ++_handle; while((int)_handle != _container->vs.domain() && !_container->vs.exists( _handle ));
				}

				void _decrement() {
					do --_handle; while(!_container->vs.exists( _handle ));
				}

				auto _get_comparable() const {
					return _handle;
				}
			}; // Iterator

		}; // Verts_Context






		struct Vert_Polys_Context {
			using Container = Mesh;
			using Handle = H_Vert_Poly;

			template<Const_Flag C>
			class Accessor : public Accessor_Base<C, Vert_Polys_Context> {
				using BASE = Accessor_Base<C, Vert_Polys_Context>;
				using BASE::_container;
				using BASE::_handle;

			public:
				FORWARDING_CONSTRUCTOR(Accessor, BASE) {}

			public:
				auto poly_vert()       {
					return A_Poly_Vert<C>    (*_container, _container->vs[ _handle.a ].poly_links[_handle.b]);
				}

				auto poly_vert() const {
					return A_Poly_Vert<CONST>(*_container, _container->vs[ _handle.a ].poly_links[_handle.b]);
				}

			};

			template<Const_Flag C>
			class Iterator : public Iterator_Base<C, Vert_Polys_Context> {
				using BASE = Iterator_Base<C, Vert_Polys_Context>;
				using BASE::_container;
				using BASE::_handle;

			public:
				FORWARDING_CONSTRUCTOR(Iterator, BASE) {}

			private:
				friend BASE;

				inline void _increment() {
					_handle = _container(_handle).next();
				}

				inline void _decrement() {
					_handle = _container(_handle).prev();
				}

				auto _get_comparable() const {
					return _handle;
				}
			};
		};









		template<Const_Flag C>
		class I_Poly : public Iterator_Base_Old<C, I_Poly> {

			// member functions accessed by BASE:
		private:
			friend Iterator_Base_Old<C, I_Poly>;

			inline void _increment() {
				do ++_handle; while(_handle != _container.ps.domain() && !_container.ps.exists( _handle ));
			}

			inline void _decrement() {
				do --_handle; while(!_container.ps.exists( _handle ));
			}

			auto _get_comparable() const {
				return _handle;
			}

			template<Const_Flag CC>
			auto _will_compare_with(const I_Poly<CC>& o) const {
				DCHECK_EQ(&_container, &o._container);
			}


		public:
			// mutable version not needed - const iterators don't pass const'ness onto the object
			inline auto operator*() const {  return A_Poly<C>(_container, _handle);  }

			// unable to implement if using accessors:
			// auto operator->()       {  return &container[idx];  }


		private:
			inline I_Poly(Const<Mesh,C>& owner, int key) : _container(owner), _handle(key) {
				if(key != owner.ps.domain() && !owner.ps.exists(key)) _increment();
			}
			MESH_FRIENDS;

		private:
			Const<Mesh,C>& _container;
			int _handle;
		};









		template<Const_Flag C>
		class I_Poly_Vert : public Iterator_Base_Old<C, I_Poly_Vert> {

			// member functions accessed by BASE:
		private:
			friend Iterator_Base_Old<C, I_Poly_Vert>;

			inline void _increment() {
				++_ith;
			}

			inline void _decrement() {
				--_ith;
			}

			auto _get_comparable() const {
				return _ith;
			}

			template<Const_Flag CC>
			auto _will_compare_with(const I_Poly_Vert<CC>& o) const {
				DCHECK_EQ(&_container, &o._container);
				DCHECK_EQ(_handle, o._handle);
			}


		public:
			// mutable version not needed - const iterators don't pass const'ness onto the object
			inline auto operator*() const {  return A_Vert<C>(_container, _container.poly_vert(_handle, _ith));  }

			// unable to implement if using accessors:
			// auto operator->()       {  return &container[idx];  }



		private:
			I_Poly_Vert(Const<Mesh,C>& owner, H_Poly key, int ith) : _container(owner), _handle(key), _ith(ith) {}
			MESH_FRIENDS;

		private:
			Const<Mesh,C>& _container;
			const H_Poly _handle;
			int _ith;
		};








		template<Const_Flag C>
		class I_Poly_Poly_Vert : public Iterator_Base_Old<C, I_Poly_Poly_Vert> {

			// member functions accessed by BASE:
		private:
			friend Iterator_Base_Old<C, I_Poly_Poly_Vert>;

			inline void _increment() {
				++_handle.ith;
			}

			inline void _decrement() {
				--_handle.ith;
			}

			auto _get_comparable() const {
				return _handle.ith;
			}

			template<Const_Flag CC>
			auto _will_compare_with(const I_Poly_Poly_Vert<CC>& o) const {
				DCHECK_EQ(&_container, &o._container);
				DCHECK_EQ(_handle.poly, o._handle.poly);
			}


		public:
			// mutable version not needed - const iterators don't pass const'ness onto the object
			inline auto operator*() const {  return A_Poly_Vert<C>(_container, _handle);  }

			// unable to implement if using accessors:
			// auto operator->()       {  return &container[idx];  }


		private:
			inline I_Poly_Poly_Vert(Const<Mesh,C>& owner, H_Poly_Vert handle) : _container(owner), _handle(handle) {}
			MESH_FRIENDS;

		private:
			Const<Mesh,C>& _container;
			H_Poly_Vert _handle;
		};










		template<Const_Flag C>
		class I_Poly_Poly_Edge : public Iterator_Base_Old<C, I_Poly_Poly_Edge> {
		
			// member functions accessed by BASE:
		private:
			friend Iterator_Base_Old<C, I_Poly_Poly_Edge>;

			inline void _increment() {
				++_handle.ith;
			}

			inline void _decrement() {
				--_handle.ith;
			}

			auto _get_comparable() const {
				return _handle.ith;
			}

			template<Const_Flag CC>
			auto _will_compare_with(const I_Poly_Poly_Edge<CC>& o) const {
				DCHECK_EQ(&_container, &o._container);
				DCHECK_EQ(_handle.poly, o._handle.poly);
			}

		public:
			// mutable version not needed - const iterators don't pass const'ness onto the object
			inline auto operator*() const {  return A_Poly_Edge<C>(_container, _handle);  }

			// unable to implement if using accessors:
			// auto operator->()       {  return &container[idx];  }


		private:
			inline I_Poly_Poly_Edge(Const<Mesh,C>& owner, H_Poly_Edge handle) : _container(owner), _handle(handle) {}
			MESH_FRIENDS;

		private:
			Const<Mesh,C>& _container;
			H_Poly_Edge _handle;
		};













		//
		// plural
		//
		template<Const_Flag C>
		class A_Verts {
		public:
			inline int domain() const {  return _mesh.verts_domain();  }

			template<class... ARGS>
			inline void add(ARGS&&... args) {  _mesh.verts_add( std::forward<ARGS>(args)... );  }

			inline auto begin()       {  return typename Verts_Context::template Iterator<C>    (&_mesh, 0);  }
			inline auto begin() const {  return typename Verts_Context::template Iterator<CONST>(&_mesh, 0);  }

			inline auto end()       {  return typename Verts_Context::template Iterator<C>    (&_mesh, _mesh.verts_domain()); }
			inline auto end() const {  return typename Verts_Context::template Iterator<CONST>(&_mesh, _mesh.verts_domain()); }

		private:
			A_Verts(Const<Mesh,C>& mesh) : _mesh(mesh) {}
			MESH_FRIENDS;

		private:
			Const<Mesh,C>& _mesh;
		};






		template<Const_Flag C>
		class A_Polys {
		public:
			inline int domain() const {  return _mesh.polys_domain();  }

			template<class... ARGS>
			inline void add(ARGS&&... args) {  _mesh.polys_add( std::forward<ARGS>(args)... );  }

			inline auto begin()       {  return I_Poly<C>    (_mesh, 0);  }
			inline auto begin() const {  return I_Poly<CONST>(_mesh, 0);  }

			inline auto end()       {  return I_Poly<C>    (_mesh, _mesh.polys_domain());  }
			inline auto end() const {  return I_Poly<CONST>(_mesh, _mesh.polys_domain());  }

		private:
			A_Polys(Const<Mesh,C>& mesh) : _mesh(mesh) {}
			MESH_FRIENDS;

		private:
			Const<Mesh,C>& _mesh;
		};




		template<Const_Flag C>
		class A_Vert_Polys {
		public:
			auto begin()       { return I_Vert_Poly<C>    (&_mesh, H_Vert_Poly(_vert, _mesh.vs[_vert].poly_links.begin())); }
			auto begin() const { return I_Vert_Poly<CONST>(&_mesh, _mesh.vs[_vert].poly_links.begin()); }

			auto end()         { return I_Vert_Poly<C>    (&_mesh, _mesh.vs[_vert].poly_links.end()); }
			auto end()   const { return I_Vert_Poly<CONST>(&_mesh, _mesh.vs[_vert].poly_links.end()); }

		private:
			A_Vert_Polys(Const<Mesh,C>& mesh, H_Vert poly) : _mesh(mesh), _vert(poly) {}
			MESH_FRIENDS;

		private:
			Const<Mesh,C>& _mesh;
			const H_Vert _vert;
		};





		template<Const_Flag C>
		class A_Poly_Verts {
		public:
			auto begin()       {  return I_Poly_Vert<C>    (_mesh, _poly, 0);  }
			auto begin() const {  return I_Poly_Vert<CONST>(_mesh, _poly, 0);  }

			auto end()       {  return I_Poly_Vert<C>    (_mesh, _poly, 3);  }
			auto end() const {  return I_Poly_Vert<CONST>(_mesh, _poly, 3);  }

		private:
			A_Poly_Verts(Const<Mesh,C>& mesh, H_Poly poly) : _mesh(mesh), _poly(poly) {}
			MESH_FRIENDS;

		private:
			Const<Mesh,C>& _mesh;
			const H_Poly _poly;
		};




		template<Const_Flag C>
		class A_Poly_Poly_Verts {
		public:
			auto begin()       {  return I_Poly_Poly_Vert<C>    (_mesh, H_Poly_Vert(_poly, 0) );  }
			auto begin() const {  return I_Poly_Poly_Vert<CONST>(_mesh, H_Poly_Vert(_poly, 0) );  }

			auto end()       {  return I_Poly_Poly_Vert<C>    (_mesh, H_Poly_Vert(_poly, 3));  }
			auto end() const {  return I_Poly_Poly_Vert<CONST>(_mesh, H_Poly_Vert(_poly, 3));  }

		private:
			A_Poly_Poly_Verts(Const<Mesh,C>& mesh, H_Poly poly) : _mesh(mesh), _poly(poly) {}
			MESH_FRIENDS;

		private:
			Const<Mesh,C>& _mesh;
			const H_Poly _poly;
		};





		template<Const_Flag C>
		class A_Poly_Poly_Edges {
		public:
			auto begin()       {  return I_Poly_Poly_Edge<C>    (_mesh, H_Poly_Edge(_poly, 0));  }
			auto begin() const {  return I_Poly_Poly_Edge<CONST>(_mesh, H_Poly_Edge(_poly, 0));  }

			auto end()       {  return I_Poly_Poly_Edge<C>    (_mesh, H_Poly_Edge(_poly, 3));  }
			auto end() const {  return I_Poly_Poly_Edge<CONST>(_mesh, H_Poly_Edge(_poly, 3));  }

		private:
			A_Poly_Poly_Edges(Const<Mesh,C>& mesh, H_Poly poly) : _mesh(mesh), _poly(poly) {}
			MESH_FRIENDS;

		private:
			Const<Mesh,C>& _mesh;
			const H_Poly _poly;
		};















		//
		//
		//
		class Mesh {
		public:
			using Scalar          = Context::Scalar;

			using Vert_Props      = Context::Vert_Props;
			using Subvert_Props   = Context::Subvert_Props;
			using Poly_Props      = Context::Poly_Props;
			using Poly_Vert_Props = Context::Poly_Vert_Props;
			using Poly_Edge_Props = Context::Poly_Edge_Props;

			static constexpr bool Verts_Erasable = Context::Verts_Erasable;
			static constexpr bool Polys_Erasable = Context::Polys_Erasable;


			using H_Vert      = Context::H_Vert;
			using H_Poly      = Context::H_Poly;
			using H_Poly_Vert = Context::H_Poly_Vert;
			using H_Poly_Edge = Context::H_Poly_Edge;
			using H_Subvert   = Context::H_Subvert;


			static constexpr bool Has_Vert_Props      = Context::Has_Vert_Props;
			static constexpr bool Has_Subvert_Props   = Context::Has_Subvert_Props;
			static constexpr bool Has_Poly_Props      = Context::Has_Poly_Props;
			static constexpr bool Has_Poly_Vert_Props = Context::Has_Poly_Vert_Props;
			static constexpr bool Has_Poly_Edge_Props = Context::Has_Poly_Edge_Props;


			static constexpr bool Has_Edge_Links      = Context::Has_Edge_Links;
			static constexpr bool Has_Vert_Poly_Links = Context::Has_Vert_Poly_Links;

		private:
			MESH_FRIENDS


		private:
			using Vs = std::conditional_t<Verts_Erasable, typename salgo::Vector<Vert>::SPARSE, salgo::Vector<Vert>>;
			using Ps = std::conditional_t<Polys_Erasable, typename salgo::Vector<Poly>::SPARSE, salgo::Vector<Poly>>;
			Vs vs;
			Ps ps;




			//
			// accessors from handles
			//
		public:
			auto vert(H_Vert handle)       {  return A_Vert<MUTAB>(this, handle);  }
			auto vert(H_Vert handle) const {  return A_Vert<CONST>(this, handle);  }

			auto poly(H_Poly handle)       {  return A_Poly<MUTAB>(*this, handle);  }
			auto poly(H_Poly handle) const {  return A_Poly<CONST>(*this, handle);  }


			auto operator()(H_Vert handle)       {  return A_Vert<MUTAB>(this, handle);  }
			auto operator()(H_Vert handle) const {  return A_Vert<CONST>(this, handle);  }

			auto operator()(H_Poly handle)       {  return A_Poly<MUTAB>(*this, handle);  }
			auto operator()(H_Poly handle) const {  return A_Poly<CONST>(*this, handle);  }


			auto operator()(H_Poly_Vert handle)       {  return A_Poly_Vert<MUTAB>(*this, handle);  }
			auto operator()(H_Poly_Vert handle) const {  return A_Poly_Vert<CONST>(*this, handle);  }

			auto operator()(H_Poly_Edge handle)       {  return A_Poly_Edge<MUTAB>(*this, handle);  }
			auto operator()(H_Poly_Edge handle) const {  return A_Poly_Edge<CONST>(*this, handle);  }

			void erase(H_Vert handle) {
				static_assert(Verts_Erasable, "erase() called on non-erasable vert");

				if constexpr(Has_Vert_Poly_Links) {
					auto& poly_links = vs[handle].poly_links;

					for(auto it = poly_links.begin(); it != poly_links.end();) {
						auto next = std::next(it);
						erase(it->poly);
						it = next;
					}
				}
				else {
					// no vert-pv links
					DLOG(WARNING) << "erasing vertex without vert-pv links - make sure it's isolated";
				}

				vs.destruct( handle );
			}


			void erase(H_Poly handle) {
				static_assert(Polys_Erasable, "erase() called on non-erasable poly");

				// unlink vert->pv links
				if constexpr(Has_Vert_Poly_Links) {
					for(int ith=0; ith < 3; ++ith) {
						auto& pv = ps[handle].verts[ith];

						auto& v = vs[pv.vert];
						DCHECK( v.poly_links.find( H_Poly_Vert(handle, ith) ) != v.poly_links.end() );
						v.poly_links.erase( H_Poly_Vert(handle, ith) );
					}
				}

				ps.destruct( handle );
			}

			// erase and unlink edge links
			void safe_erase(H_Poly handle) {
				auto& p = ps[handle];

				// unlink edge links
				if constexpr(Has_Edge_Links) {
					for(const auto& pe : p.edges) {
						if(pe.link.valid()) {
							_poly_edge(pe.link).link.reset();
						}
					}
				}

				erase(handle);
			}


			//
			// fast interface - can be accessed via the accessors
			//
		public:
			inline auto& vert_pos(H_Vert handle)       {  return vs[handle].pos;  }
			inline auto& vert_pos(H_Vert handle) const {  return vs[handle].pos;  }


			inline auto& vert_props(H_Vert handle) {
				static_assert(Has_Vert_Props, "accessing non-existing vert_props");
				return vs[handle].props;
			}

			inline auto& vert_props(H_Vert handle) const {
				static_assert(Has_Vert_Props, "accessing non-existing vert_props");
				return vs[handle].props;
			}

			auto poly_vert_vert(H_Poly_Vert handle)       { return _poly_vert(handle).vert; }
			auto poly_vert_vert(H_Poly_Vert handle) const { return _poly_vert(handle).vert; }

			auto& poly_vert_pos(H_Poly_Vert handle)       { return vert_pos( _poly_vert(handle).vert ); }
			auto& poly_vert_pos(H_Poly_Vert handle) const { return vert_pos( _poly_vert(handle).vert ); }



			auto& poly_vert_props(H_Poly_Vert handle) {
				static_assert(Has_Poly_Vert_Props, "accessing non-existing poly_vert_props");
				return _poly_vert(handle).props;
			}

			auto& poly_vert_props(H_Poly_Vert handle) const {
				static_assert(Has_Poly_Vert_Props, "accessing non-existing poly_vert_props");
				return _poly_vert(handle).props;
			}


			void poly_vert_change(H_Poly_Vert pv, H_Vert v) {
				if constexpr(Has_Vert_Poly_Links) {
					auto& old_poly_links = vs[ _poly_vert(pv).vert ].poly_links;

					DCHECK(old_poly_links.find( pv ) != old_poly_links.end())
						<< "old vert-pv link should be present before poly_vert_change()";

					old_poly_links.erase(pv);

					auto& new_poly_links = vs[v].poly_links;

					DCHECK(new_poly_links.find( pv ) == new_poly_links.end())
						<< "new vert-pv link should NOT be present before poly_vert_change()";

					new_poly_links.insert(pv);
				}

				_poly_vert(pv).vert = v;
			}



			template<class... ARGS>
			auto& poly_edge_props(ARGS&&... args) {
				static_assert(Has_Poly_Edge_Props, "accessing non-existing poly_edge_props");
				return _poly_edge( std::forward<ARGS>(args)... ).props;
			}

			template<class... ARGS>
			auto& poly_edge_props(ARGS&&... args) const {
				static_assert(Has_Poly_Edge_Props, "accessing non-existing poly_edge_props");
				return _poly_edge( std::forward<ARGS>(args)... ).props;
			}



			template<class... ARGS>
			auto& poly_edge_link(ARGS&&... args)       {
				static_assert(Has_Edge_Links, "accessing non-existing edge link");
				return _poly_edge( std::forward<ARGS>(args)... ).link;
			}

			template<class... ARGS>
			auto& poly_edge_link(ARGS&&... args) const {
				static_assert(Has_Edge_Links, "accessing non-existing edge link");
				return _poly_edge( std::forward<ARGS>(args)... ).link;
			}



			template<class... ARGS>
			inline auto verts_add(ARGS&&... args) {
				vs.emplace_back( std::forward<ARGS>(args)... );
				return typename Verts_Context::template Accessor<MUTAB>(this, vs.domain()-1);
			}

			// does not add edge links!
			template<class... ARGS>
			inline auto polys_add(ARGS&&... args) {
				ps.emplace_back( std::forward<ARGS>(args)... );

				// build vert->pv links
				if constexpr(Has_Vert_Poly_Links) {

					auto handle = ps.domain()-1;
					auto& p = ps[handle];

					for(int ith=0; ith < 3; ++ith) {
						auto& pv = p.verts[ith];

						auto& v = vs[pv.vert];
						DCHECK( ! v.poly_links( H_Poly_Vert(handle, ith) ).exists() );
						v.poly_links.emplace( H_Poly_Vert(handle, ith) );
					}
				}

				return A_Poly<MUTAB>(*this, ps.domain()-1);
			}


			void verts_reserve(int capacity) {
				vs.reserve(capacity);
			}

			void polys_reserve(int capacity) {
				ps.reserve(capacity);
			}


			//
			//
			//
		private:
			auto& _poly_vert(H_Poly_Vert handle)       { return ps[ handle.poly ].verts[ handle.ith ]; }
			auto& _poly_vert(H_Poly_Vert handle) const { return ps[ handle.poly ].verts[ handle.ith ]; }
			// auto _poly_vert(H_Poly poly, int ith) const {  return ps.at(        poly ).verts[        ith ].vert;  }

			auto& _poly_edge(H_Poly_Edge handle)       { return ps[ handle.poly ].edges[ handle.ith ]; }
			auto& _poly_edge(H_Poly_Edge handle) const { return ps[ handle.poly ].edges[ handle.ith ]; }
			// auto& _poly_edge(H_Poly poly, int ith) { return ps.at(        poly ).edges[        ith ]; }
			// auto& _poly_edge(H_Poly poly, int ith) const { return ps.at(        poly ).edges[        ith ]; }



		public:
			auto verts()       {  return A_Verts<MUTAB>(*this);  }
			auto verts() const {  return A_Verts<CONST>(*this);  }

			auto polys()       {  return A_Polys<MUTAB>(*this);  }
			auto polys() const {  return A_Polys<CONST>(*this);  }

			int verts_domain() const {  return vs.domain();  }
			int polys_domain() const {  return ps.domain();  }


		}; // class Mesh










		struct With_Builder : Mesh {

			template<class NEW_VERT_PROPS>
			using VERT_PROPS =
				typename Context<Scalar, NEW_VERT_PROPS, Subvert_Props,
					Poly_Props, Poly_Vert_Props, Poly_Edge_Props, Verts_Erasable, Polys_Erasable,
					Has_Edge_Links, Has_Vert_Poly_Links> :: With_Builder;

			template<class NEW_SUBVERT_PROPS>
			using SUBVERT_PROPS =
				typename Context<Scalar, Vert_Props, NEW_SUBVERT_PROPS,
					Poly_Props, Poly_Vert_Props, Poly_Edge_Props, Verts_Erasable, Polys_Erasable,
					Has_Edge_Links, Has_Vert_Poly_Links> :: With_Builder;

			template<class NEW_POLY_PROPS>
			using POLY_PROPS =
				typename Context<Scalar, Vert_Props, Subvert_Props,
					NEW_POLY_PROPS, Poly_Vert_Props, Poly_Edge_Props, Verts_Erasable, Polys_Erasable,
					Has_Edge_Links, Has_Vert_Poly_Links> :: With_Builder;

			template<class NEW_POLY_VERT_PROPS>
			using POLY_VERT_PROPS =
				typename Context<Scalar, Vert_Props, Subvert_Props,
					Poly_Props, NEW_POLY_VERT_PROPS, Poly_Edge_Props, Verts_Erasable, Polys_Erasable,
					Has_Edge_Links, Has_Vert_Poly_Links> :: With_Builder;

			template<class NEW_POLY_EDGE_PROPS>
			using POLY_EDGE_PROPS =
				typename Context<Scalar, Vert_Props, Subvert_Props,
					Poly_Props, Poly_Vert_Props, NEW_POLY_EDGE_PROPS, Verts_Erasable, Polys_Erasable,
					Has_Edge_Links, Has_Vert_Poly_Links> :: With_Builder;

			using VERTS_ERASABLE =
				typename Context<Scalar, Vert_Props, Subvert_Props,
					Poly_Props, Poly_Vert_Props, Poly_Edge_Props, true, Polys_Erasable,
					Has_Edge_Links, Has_Vert_Poly_Links> :: With_Builder;

			using POLYS_ERASABLE =
				typename Context<Scalar, Vert_Props, Subvert_Props,
					Poly_Props, Poly_Vert_Props, Poly_Edge_Props, Verts_Erasable, true,
					Has_Edge_Links, Has_Vert_Poly_Links> :: With_Builder;

			using EDGE_LINKS =
				typename Context<Scalar, Vert_Props, Subvert_Props,
					Poly_Props, Poly_Vert_Props, Poly_Edge_Props, Verts_Erasable, Polys_Erasable,
					true, Has_Vert_Poly_Links> :: With_Builder;

			using VERT_POLY_LINKS =
				typename Context<Scalar, Vert_Props, Subvert_Props,
					Poly_Props, Poly_Vert_Props, Poly_Edge_Props, Verts_Erasable, Polys_Erasable,
					Has_Edge_Links, true> :: With_Builder;

			using FULL =
				typename Context<Scalar, Vert_Props, Subvert_Props,
					Poly_Props, Poly_Vert_Props, Poly_Edge_Props, true, true,
					true, true> :: With_Builder;

		};



	}; // struct Context


} // namespace Mesh
} // namespace internal












template< class T = double >
using Mesh = typename internal::Mesh::Context<
	T, // SCALAR
	void,   // VERT_PROPS
	void,   // SUBVERT_PROPS
	void,   // POLY_PROPS
	void,   // POLY_VERT_PROPS
	void,   // POLY_EDGE_PROPS
	false,  // VERTS_ERASABLE
	false,  // POLYS_ERASABLE
	false,  // EDGE_LINKS
	false   // POLY_VERT_LINKS
> :: With_Builder;















} // namespace salgo









#undef MESH_FRIENDS
