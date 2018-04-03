#pragma once


#include "../sparse-vector.hpp"

#include <Eigen/Dense>


namespace salgo {









namespace internal {
namespace Mesh {

	using H_VERT = int;
	using H_POLY = int;
	using H_SUBVERT = int;

	struct H_POLY_VERT {
		H_POLY poly;
		char ith; // 0,1,2
	};

	struct H_POLY_EDGE {
		H_POLY poly;
		char ith; // 0,1,2
	};

	template<class PROPS, bool> struct Add_props { PROPS props; };
	template<class PROPS> struct Add_props<PROPS,false> {};


	template<bool> struct Add_subvert_link { H_SUBVERT subvert; };
	template<> struct Add_subvert_link<false> {};



	template<
		class SCALAR,
		class VERT_PROPS,
		class SUBVERT_PROPS,
		class POLY_PROPS,
		class POLY_VERT_PROPS,
		class POLY_EDGE_PROPS
	>
	struct Context {



		class Mesh {
		public:
			using Scalar = SCALAR;

			using Vert_Props = VERT_PROPS;
			using Subvert_Props = SUBVERT_PROPS;
			using Poly_Props = POLY_PROPS;
			using Poly_Vert_Props = POLY_VERT_PROPS;
			using Poly_Edge_Props = POLY_EDGE_PROPS;


			using H_Vert = H_VERT;
			using H_Poly = H_POLY;
			using H_Poly_Vert = H_POLY_VERT;
			using H_Poly_Edge = H_POLY_EDGE;
			using H_Subvert = H_SUBVERT;


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


		private:
			struct Vert : Add_props<Vert_Props, Has_Vert_Props> {
				Eigen::Matrix<SCALAR,3,1> pos = {0,0,0};

				template<class... ARGS>
				Vert(ARGS&&... args) : pos( std::forward<ARGS>(args)... ) {}
			};

			struct Poly_Vert :
					Add_props<Poly_Vert_Props, Has_Poly_Vert_Props>,
					Add_subvert_link<Has_Subvert_Props> {
				H_Vert vert = H_Vert();
			};

			struct Poly_Edge : Add_props<Poly_Edge_Props, Has_Poly_Edge_Props> {
				H_Poly_Edge link = H_Poly_Edge();
			};

			struct Poly : Add_props<Poly_Props, Has_Poly_Props> {
				std::array<Poly_Vert,3> verts;
				std::array<Poly_Edge,3> edges;

				Poly(H_Vert a, H_Vert b, H_Vert c) {
					verts[0].vert = a;
					verts[1].vert = b;
					verts[2].vert = c;
				}
			};

		private:
			Sparse_Vector<Vert> vs;
			Sparse_Vector<Poly> ps;


			//
			// fast interface - can be accessed via the accessors
			//
		public:
			inline auto& vert_pos(H_Vert key)       {  return vs.at(key).pos;  }
			inline auto& vert_pos(H_Vert key) const {  return vs.at(key).pos;  }


			inline auto& vert_props(H_Vert key) {
				static_assert(Has_Vert_Props, "accessing non-existing vert_props");
				return vs.at(key).props;
			}

			inline auto& vert_props(H_Vert key) const {
				static_assert(Has_Vert_Props, "accessing non-existing vert_props");
				return vs.at(key).props;
			}


			inline auto poly_vert(H_Poly_Vert handle)   const {  return ps.at( handle.poly ).verts[ handle.ith ].vert;  }
			inline auto poly_vert(H_Poly poly, int ith) const {  return ps.at(        poly ).verts[        ith ].vert;  }


			template<class... ARGS>
			auto& poly_vert_pos(ARGS&&... args)       {  return vert_pos( poly_vert(std::forward<ARGS>(args)...) );  }

			template<class... ARGS>
			auto& poly_vert_pos(ARGS&&... args) const {  return vert_pos( poly_vert(std::forward<ARGS>(args)...) );  }



			auto& poly_vert_props(H_Poly_Vert handle) {
				static_assert(Has_Poly_Vert_Props, "accessing non-existing poly_vert_props");
				return ps.at( handle.poly ).verts[ handle.ith ].props;
			}

			auto& poly_vert_props(H_Poly_Vert handle) const {
				static_assert(Has_Poly_Vert_Props, "accessing non-existing poly_vert_props");
				return ps.at( handle.poly ).verts[ handle.ith ].props;
			}



			auto& poly_edge_props(H_Poly_Edge handle) {
				static_assert(Has_Poly_Edge_Props, "accessing non-existing poly_edge_props");
				return ps.at( handle.poly ).edges[ handle.ith ].props;
			}

			auto& poly_edge_props(H_Poly_Edge handle) const {
				static_assert(Has_Poly_Edge_Props, "accessing non-existing poly_edge_props");
				return ps.at( handle.poly ).edges[ handle.ith ].props;
			}



			template<class... ARGS>
			inline void verts_add(ARGS&&... args) {
				vs.emplace_back( std::forward<ARGS>(args)... );
			}

			inline void polys_add(H_Vert a, H_Vert b, H_Vert c) {
				ps.emplace_back(a,b,c);
			}



			//
			// accessors
			//
		public:
			template<Const_Flag C> class A_Vert;
			template<Const_Flag C> class A_Poly;
			template<Const_Flag C> class A_Poly_Vert;
			template<Const_Flag C> class A_Poly_Edge;

			template<Const_Flag C> class A_Verts;
			template<Const_Flag C> class A_Polys;

			template<Const_Flag C> class A_Poly_Verts;
			template<Const_Flag C> class A_Poly_Poly_Verts;
			template<Const_Flag C> class A_Poly_Poly_Edges;


			//
			// iterators
			//
		public:
			template<Const_Flag C> class I_Vert;
			template<Const_Flag C> class I_Poly;
			template<Const_Flag C> class I_Poly_Vert;
			template<Const_Flag C> class I_Poly_Poly_Vert;
			template<Const_Flag C> class I_Poly_Poly_Edge;



		public:
			auto verts()       {  return A_Verts<MUTAB>(*this);  }
			auto verts() const {  return A_Verts<CONST>(*this);  }

			auto polys()       {  return A_Polys<MUTAB>(*this);  }
			auto polys() const {  return A_Polys<CONST>(*this);  }

			int verts_domain() const {  return vs.domain_end();  }
			int polys_domain() const {  return ps.domain_end();  }



		public:
			template<Const_Flag C>
			class A_Vert {
			public:
				inline auto  key() const {  return _key;  }

				inline auto& pos()       {  return _mesh.vert_pos( _key );  }
				inline auto& pos() const {  return _mesh.vert_pos( _key );  }

				inline auto& props()       {  return _mesh.vert_props( _key );  }
				inline auto& props() const {  return _mesh.vert_props( _key );  }

			private:
				A_Vert(Const<Mesh,C>& mesh, H_Vert key) : _mesh(mesh), _key(key) {}
				friend Mesh;

			private:
				Const<Mesh,C>& _mesh;
				const H_Vert _key;
			};


		public:
			auto vert(H_Vert key)       {  return A_Vert<MUTAB>(*this, key);  }
			auto vert(H_Vert key) const {  return A_Vert<CONST>(*this, key);  }




		public:
			template<Const_Flag C>
			class A_Poly {
			public:
				inline auto key() const {  return _key;  }



				inline auto vert(int ith) {
					DCHECK_GE(ith, 0) << "out of bounds";
					DCHECK_LT(ith, 3) << "out of bounds";
					return A_Vert<C>(_mesh, _mesh.poly_vert(_key, ith));
				};

				inline auto vert(int ith) const {
					DCHECK_GE(ith, 0) << "out of bounds";
					DCHECK_LT(ith, 3) << "out of bounds";
					return A_Vert<CONST>(_mesh, _mesh.poly_vert(_key, ith));
				};



				inline auto poly_vert(int ith) {
					DCHECK_GE(ith, 0) << "out of bounds";
					DCHECK_LT(ith, 3) << "out of bounds";
					return A_Poly_Vert<C>(_mesh, _key, ith);
				};

				inline auto poly_vert(int ith) const {
					DCHECK_GE(ith, 0) << "out of bounds";
					DCHECK_LT(ith, 3) << "out of bounds";
					return A_Poly_Vert<CONST>(_mesh, _key, ith);
				};


				inline auto poly_edge(int ith) {
					DCHECK_GE(ith, 0) << "out of bounds";
					DCHECK_LT(ith, 3) << "out of bounds";
					return A_Poly_Edge<C>(_mesh, _key, ith);
				};

				inline auto poly_edge(int ith) const {
					DCHECK_GE(ith, 0) << "out of bounds";
					DCHECK_LT(ith, 3) << "out of bounds";
					return A_Poly_Edge<CONST>(_mesh, _key, ith);
				};

				inline auto verts()       {  return A_Poly_Verts<C>    (_mesh, _key);  }
				inline auto verts() const {  return A_Poly_Verts<CONST>(_mesh, _key);  }

				inline auto poly_verts()       {  return A_Poly_Poly_Verts<C>    (_mesh, _key);  }
				inline auto poly_verts() const {  return A_Poly_Poly_Verts<CONST>(_mesh, _key);  }

				inline auto poly_edges()       {  return A_Poly_Poly_Edges<C>    (_mesh, _key);  }
				inline auto poly_edges() const {  return A_Poly_Poly_Edges<CONST>(_mesh, _key);  }


				inline auto& props()       {  return _mesh.poly_props( _key );  }
				inline auto& props() const {  return _mesh.poly_props( _key );  }

			private:
				A_Poly(Const<Mesh,C>& mesh, H_Poly key) : _mesh(mesh), _key(key) {}
				friend Mesh;

			private:
				Const<Mesh,C>& _mesh;
				const H_Poly _key;
			};



		public:
			auto poly(H_Poly key)       {  return A_Poly<MUTAB>(*this, key);  }
			auto poly(H_Poly key) const {  return A_Poly<CONST>(*this, key);  }








		public:
			template<Const_Flag C>
			class A_Poly_Vert {
			public:
				inline auto poly()       {  return A_Poly<C>    (_mesh, _poly);  }
				inline auto poly() const {  return A_Poly<CONST>(_mesh, _poly);  }

				inline auto vert()       {  return A_Vert<C>    (_mesh, _mesh.poly_vert(_poly, _ith));  }
				inline auto vert() const {  return A_Vert<CONST>(_mesh, _mesh.poly_vert(_poly, _ith));  }

				inline auto& pos()       {  return _mesh.poly_vert_pos(_poly, _ith);  }
				inline auto& pos() const {  return _mesh.poly_vert_pos(_poly, _ith);  }

				inline auto next()       {  return A_Poly_Vert<C>    (_mesh, _poly, (_ith +1) % 3);  }
				inline auto next() const {  return A_Poly_Vert<CONST>(_mesh, _poly, (_ith +1) % 3);  }

				inline auto prev()       {  return A_Poly_Vert<C>    (_mesh, _poly, (_ith+3 -1) % 3);  }
				inline auto prev() const {  return A_Poly_Vert<CONST>(_mesh, _poly, (_ith+3 -1) % 3);  }


				inline auto next_edge()       {  return A_Poly_Edge<C>    (_mesh, _poly, _ith);  }
				inline auto next_edge() const {  return A_Poly_Edge<CONST>(_mesh, _poly, _ith);  }

				inline auto prev_edge()       {  return A_Poly_Edge<C>    (_mesh, _poly, (_ith+3 -1) % 3);  }
				inline auto prev_edge() const {  return A_Poly_Edge<CONST>(_mesh, _poly, (_ith+3 -1) % 3);  }


				inline auto& props()       {  return _mesh.poly_vert_props( _poly, _ith );  }
				inline auto& props() const {  return _mesh.poly_vert_props( _poly, _ith );  }


			private:
				A_Poly_Vert(Const<Mesh,C>& mesh, H_Poly poly, int ith) : _mesh(mesh), _poly(poly), _ith(ith) {}
				friend A_Poly<MUTAB>;
				friend A_Poly<CONST>;
				friend I_Poly_Poly_Vert<MUTAB>;
				friend I_Poly_Poly_Vert<CONST>;
				friend A_Poly_Vert<MUTAB>;
				friend A_Poly_Vert<CONST>;

			private:
				Const<Mesh,C>& _mesh;
				const H_Poly _poly;
				const int _ith;
			};






		public:
			template<Const_Flag C>
			class A_Poly_Edge {
			public:
				inline auto poly()       {  return A_Poly<C>    (_mesh, _poly);  }
				inline auto poly() const {  return A_Poly<CONST>(_mesh, _poly);  }


				inline auto next()       {  return A_Poly_Edge<C>    (_mesh, _poly, (_ith +1) % 3);  }
				inline auto next() const {  return A_Poly_Edge<CONST>(_mesh, _poly, (_ith +1) % 3);  }

				inline auto prev()       {  return A_Poly_Edge<C>    (_mesh, _poly, (_ith+3 -1) % 3);  }
				inline auto prev() const {  return A_Poly_Edge<CONST>(_mesh, _poly, (_ith+3 -1) % 3);  }


				inline auto next_vert()       {  return A_Poly_Vert<C>    (_mesh, _poly, (_ith +1) % 3);  }
				inline auto next_vert() const {  return A_Poly_Vert<CONST>(_mesh, _poly, (_ith +1) % 3);  }

				inline auto prev_vert()       {  return A_Poly_Vert<C>    (_mesh, _poly, _ith);  }
				inline auto prev_vert() const {  return A_Poly_Vert<CONST>(_mesh, _poly, _ith);  }


				inline auto& props()       {  return _mesh.poly_edge_props( _poly, _ith );  }
				inline auto& props() const {  return _mesh.poly_edge_props( _poly, _ith );  }


			private:
				A_Poly_Edge(Const<Mesh,C>& mesh, H_Poly poly, int ith) : _mesh(mesh), _poly(poly), _ith(ith) {}
				friend A_Poly<MUTAB>;
				friend A_Poly<CONST>;

			private:
				Const<Mesh,C>& _mesh;
				const H_Poly _poly;
				const int _ith;
			};






		public:
			template<Const_Flag C>
			class I_Vert {
			public:
				inline auto& operator++() {
					_increment();
					return *this; }

				inline auto operator++(int) {
					auto old = *this;
					_increment();
					return old; }

				inline auto& operator--() {
					_decrement();
					return *this; }

				inline auto operator--(int) {
					auto old = *this;
					_decrement();
					return old; }


			public:
				template<Const_Flag CC>
				inline bool operator==(const I_Vert<CC>& o) const {
					DCHECK_EQ(&_owner, &o._owner);
					return _key == o._key;
				}

				template<Const_Flag CC>
				inline bool operator!=(const I_Vert<CC>& o) const {
					DCHECK_EQ(&_owner, &o._owner);
					return _key != o._key;
				}

				template<Const_Flag CC>
				inline bool operator<(const I_Vert<CC>& o) const {
					DCHECK_EQ(&_owner, &o._owner);
					return _key < o._key;
				}

				template<Const_Flag CC>
				inline bool operator>(const I_Vert<CC>& o) const {
					DCHECK_EQ(&_owner, &o._owner);
					return _key > o._key;
				}

				template<Const_Flag CC>
				inline bool operator<=(const I_Vert<CC>& o) const {
					DCHECK_EQ(&_owner, &o._owner);
					return _key <= o._key;
				}

				template<Const_Flag CC>
				inline bool operator>=(const I_Vert<CC>& o) const {
					DCHECK_EQ(&_owner, &o._owner);
					return _key >= o._key;
				}




			public:
				// mutable version not needed - const iterators don't pass const'ness onto the object
				inline auto operator*() const {  return A_Vert<C>(_owner, _key);  }

				// unable to implement if using accessors:
				// auto operator->()       {  return &container[idx];  }




			private:
				inline void _increment() {
					do {
						++_key;
					} while(_key != _owner.vs.domain_end() && !_owner.vs.exists( _key ));
				}

				inline void _decrement() {
					do {
						--_key;
					} while(!_owner.vs.exists( _key ));
				}




			private:
				inline I_Vert(Const<Mesh,C>& owner, int key) : _owner(owner), _key(key) {
					if(key != owner.vs.domain_end() && !owner.vs.exists(key)) _increment();
				}
				friend A_Verts<MUTAB>;
				friend A_Verts<CONST>;

			private:
				Const<Mesh,C>& _owner;
				int _key;
			};






		public:
			template<Const_Flag C>
			class I_Poly {
			public:
				inline auto& operator++() {
					_increment();
					return *this; }

				inline auto operator++(int) {
					auto old = *this;
					_increment();
					return old; }

				inline auto& operator--() {
					_decrement();
					return *this; }

				inline auto operator--(int) {
					auto old = *this;
					_decrement();
					return old; }


			public:
				template<Const_Flag CC>
				inline bool operator==(const I_Poly<CC>& o) const {
					DCHECK_EQ(&_owner, &o._owner);
					return _key == o._key;
				}

				template<Const_Flag CC>
				inline bool operator!=(const I_Poly<CC>& o) const {
					DCHECK_EQ(&_owner, &o._owner);
					return _key != o._key;
				}

				template<Const_Flag CC>
				inline bool operator<(const I_Poly<CC>& o) const {
					DCHECK_EQ(&_owner, &o._owner);
					return _key < o._key;
				}

				template<Const_Flag CC>
				inline bool operator>(const I_Poly<CC>& o) const {
					DCHECK_EQ(&_owner, &o._owner);
					return _key > o._key;
				}

				template<Const_Flag CC>
				inline bool operator<=(const I_Poly<CC>& o) const {
					DCHECK_EQ(&_owner, &o._owner);
					return _key <= o._key;
				}

				template<Const_Flag CC>
				inline bool operator>=(const I_Poly<CC>& o) const {
					DCHECK_EQ(&_owner, &o._owner);
					return _key >= o._key;
				}




			public:
				// mutable version not needed - const iterators don't pass const'ness onto the object
				inline auto operator*() const {  return A_Poly<C>(_owner, _key);  }

				// unable to implement if using accessors:
				// auto operator->()       {  return &container[idx];  }




			private:
				inline void _increment() {
					do {
						++_key;
					} while(_key != _owner.ps.domain_end() && !_owner.ps.exists( _key ));
				}

				inline void _decrement() {
					do {
						--_key;
					} while(!_owner.ps.exists( _key ));
				}




			private:
				inline I_Poly(Const<Mesh,C>& owner, int key) : _owner(owner), _key(key) {
					if(key != owner.ps.domain_end() && !owner.ps.exists(key)) _increment();
				}
				friend A_Polys<MUTAB>;
				friend A_Polys<CONST>;

			private:
				Const<Mesh,C>& _owner;
				int _key;
			};









		public:
			template<Const_Flag C>
			class I_Poly_Vert {
			public:
				inline auto& operator++() {  ++_ith; return *this;  }
				inline auto& operator--() {  --_ith; return *this;  }

				inline auto operator++(int) {  auto old = *this; ++_ith; return old;  }
				inline auto operator--(int) {  auto old = *this; --_ith; return old;  }


			public:
				template<Const_Flag CC>
				inline bool operator==(const I_Poly_Vert<CC>& o) const {
					DCHECK_EQ(&_owner, &o._owner);
					DCHECK_EQ(&_key, &o._key);
					return _ith == o._ith;
				}

				template<Const_Flag CC>
				inline bool operator!=(const I_Poly_Vert<CC>& o) const {
					DCHECK_EQ(&_owner, &o._owner);
					DCHECK_EQ(&_key, &o._key);
					return _ith != o._ith;
				}

				template<Const_Flag CC>
				inline bool operator<(const I_Poly_Vert<CC>& o) const {
					DCHECK_EQ(&_owner, &o._owner);
					DCHECK_EQ(&_key, &o._key);
					return _ith < o._ith;
				}

				template<Const_Flag CC>
				inline bool operator>(const I_Poly_Vert<CC>& o) const {
					DCHECK_EQ(&_owner, &o._owner);
					DCHECK_EQ(&_key, &o._key);
					return _ith > o._ith;
				}

				template<Const_Flag CC>
				inline bool operator<=(const I_Poly_Vert<CC>& o) const {
					DCHECK_EQ(&_owner, &o._owner);
					DCHECK_EQ(&_key, &o._key);
					return _ith <= o._ith;
				}

				template<Const_Flag CC>
				inline bool operator>=(const I_Poly_Vert<CC>& o) const {
					DCHECK_EQ(&_owner, &o._owner);
					DCHECK_EQ(&_key, &o._key);
					return _ith >= o._ith;
				}



			public:
				// mutable version not needed - const iterators don't pass const'ness onto the object
				inline auto operator*() const {  return A_Vert<C>(_owner, _owner.poly_vert(_key, _ith));  }

				// unable to implement if using accessors:
				// auto operator->()       {  return &container[idx];  }



			private:
				inline I_Poly_Vert(Const<Mesh,C>& owner, H_Poly key, int ith) : _owner(owner), _key(key), _ith(ith) {}
				friend A_Poly_Verts<MUTAB>;
				friend A_Poly_Verts<CONST>;

			private:
				Const<Mesh,C>& _owner;
				const H_Poly _key;
				int _ith;
			};








		public:
			template<Const_Flag C>
			class I_Poly_Poly_Vert {
			public:
				inline auto& operator++() {  ++_ith; return *this;  }
				inline auto& operator--() {  --_ith; return *this;  }

				inline auto operator++(int) {  auto old = *this; ++_ith; return old;  }
				inline auto operator--(int) {  auto old = *this; --_ith; return old;  }


			public:
				template<Const_Flag CC>
				inline bool operator==(const I_Poly_Poly_Vert<CC>& o) const {
					DCHECK_EQ(&_owner, &o._owner);
					DCHECK_EQ(&_key, &o._key);
					return _ith == o._ith;
				}

				template<Const_Flag CC>
				inline bool operator!=(const I_Poly_Poly_Vert<CC>& o) const {
					DCHECK_EQ(&_owner, &o._owner);
					DCHECK_EQ(&_key, &o._key);
					return _ith != o._ith;
				}

				template<Const_Flag CC>
				inline bool operator<(const I_Poly_Poly_Vert<CC>& o) const {
					DCHECK_EQ(&_owner, &o._owner);
					DCHECK_EQ(&_key, &o._key);
					return _ith < o._ith;
				}

				template<Const_Flag CC>
				inline bool operator>(const I_Poly_Poly_Vert<CC>& o) const {
					DCHECK_EQ(&_owner, &o._owner);
					DCHECK_EQ(&_key, &o._key);
					return _ith > o._ith;
				}

				template<Const_Flag CC>
				inline bool operator<=(const I_Poly_Poly_Vert<CC>& o) const {
					DCHECK_EQ(&_owner, &o._owner);
					DCHECK_EQ(&_key, &o._key);
					return _ith <= o._ith;
				}

				template<Const_Flag CC>
				inline bool operator>=(const I_Poly_Poly_Vert<CC>& o) const {
					DCHECK_EQ(&_owner, &o._owner);
					DCHECK_EQ(&_key, &o._key);
					return _ith >= o._ith;
				}



			public:
				// mutable version not needed - const iterators don't pass const'ness onto the object
				inline auto operator*() const {  return A_Poly_Vert<C>(_owner, _key, _ith);  }

				// unable to implement if using accessors:
				// auto operator->()       {  return &container[idx];  }



			private:
				inline I_Poly_Poly_Vert(Const<Mesh,C>& owner, H_Poly key, int ith) : _owner(owner), _key(key), _ith(ith) {}
				friend A_Poly_Poly_Verts<MUTAB>;
				friend A_Poly_Poly_Verts<CONST>;

			private:
				Const<Mesh,C>& _owner;
				const H_Poly _key;
				int _ith;
			};










		public:
			template<Const_Flag C>
			class I_Poly_Edge {
			public:
				inline auto& operator++() {  ++_ith; return *this;  }
				inline auto& operator--() {  --_ith; return *this;  }

				inline auto operator++(int) {  auto old = *this; ++_ith; return old;  }
				inline auto operator--(int) {  auto old = *this; --_ith; return old;  }


			public:
				template<Const_Flag CC>
				inline bool operator==(const I_Poly_Edge<CC>& o) const {
					DCHECK_EQ(&_owner, &o._owner);
					DCHECK_EQ(&_key, &o._key);
					return _ith == o._ith;
				}

				template<Const_Flag CC>
				inline bool operator!=(const I_Poly_Edge<CC>& o) const {
					DCHECK_EQ(&_owner, &o._owner);
					DCHECK_EQ(&_key, &o._key);
					return _ith != o._ith;
				}

				template<Const_Flag CC>
				inline bool operator<(const I_Poly_Edge<CC>& o) const {
					DCHECK_EQ(&_owner, &o._owner);
					DCHECK_EQ(&_key, &o._key);
					return _ith < o._ith;
				}

				template<Const_Flag CC>
				inline bool operator>(const I_Poly_Edge<CC>& o) const {
					DCHECK_EQ(&_owner, &o._owner);
					DCHECK_EQ(&_key, &o._key);
					return _ith > o._ith;
				}

				template<Const_Flag CC>
				inline bool operator<=(const I_Poly_Edge<CC>& o) const {
					DCHECK_EQ(&_owner, &o._owner);
					DCHECK_EQ(&_key, &o._key);
					return _ith <= o._ith;
				}

				template<Const_Flag CC>
				inline bool operator>=(const I_Poly_Edge<CC>& o) const {
					DCHECK_EQ(&_owner, &o._owner);
					DCHECK_EQ(&_key, &o._key);
					return _ith >= o._ith;
				}



			public:
				// mutable version not needed - const iterators don't pass const'ness onto the object
				inline auto operator*() const {  return A_Poly_Edge<C>(_owner, _key, _ith);  }

				// unable to implement if using accessors:
				// auto operator->()       {  return &container[idx];  }



			private:
				inline I_Poly_Edge(Const<Mesh,C>& owner, H_Poly key, int ith) : _owner(owner), _key(key), _ith(ith) {}

			private:
				Const<Mesh,C>& _owner;
				const H_Poly _key;
				int _ith;
			};














		public:
			template<Const_Flag C>
			class A_Verts {
			public:
				inline int domain() const {  return _mesh.verts_domain();  }

				template<class... ARGS>
				inline void add(ARGS&&... args) {  _mesh.verts_add( std::forward<ARGS>(args)... );  }

				inline auto begin()       {  return I_Vert<C>    (_mesh, 0);  }
				inline auto begin() const {  return I_Vert<CONST>(_mesh, 0);  }

				inline auto end()       {  return I_Vert<C>    (_mesh, _mesh.verts_domain());  }
				inline auto end() const {  return I_Vert<CONST>(_mesh, _mesh.verts_domain());  }

			private:
				A_Verts(Const<Mesh,C>& mesh) : _mesh(mesh) {}
				friend Mesh;

			private:
				Const<Mesh,C>& _mesh;
			};






		public:
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
				friend Mesh;

			private:
				Const<Mesh,C>& _mesh;
			};









		public:
			template<Const_Flag C>
			class A_Poly_Verts {
			public:
				auto begin()       {  return I_Poly_Vert<C>    (_mesh, _poly, 0);  }
				auto begin() const {  return I_Poly_Vert<CONST>(_mesh, _poly, 0);  }

				auto end()       {  return I_Poly_Vert<C>    (_mesh, _poly, 3);  }
				auto end() const {  return I_Poly_Vert<CONST>(_mesh, _poly, 3);  }

			private:
				A_Poly_Verts(Const<Mesh,C>& mesh, H_Poly poly) : _mesh(mesh), _poly(poly) {}
				friend Mesh;

			private:
				Const<Mesh,C>& _mesh;
				H_Poly _poly;
			};




		public:
			template<Const_Flag C>
			class A_Poly_Poly_Verts {
			public:
				auto begin()       {  return I_Poly_Poly_Vert<C>    (_mesh, _poly, 0);  }
				auto begin() const {  return I_Poly_Poly_Vert<CONST>(_mesh, _poly, 0);  }

				auto end()       {  return I_Poly_Poly_Vert<C>    (_mesh, _poly, 3);  }
				auto end() const {  return I_Poly_Poly_Vert<CONST>(_mesh, _poly, 3);  }

			private:
				A_Poly_Poly_Verts(Const<Mesh,C>& mesh, H_Poly poly) : _mesh(mesh), _poly(poly) {}
				friend Mesh;

			private:
				Const<Mesh,C>& _mesh;
				H_Poly _poly;
			};





		public:
			template<Const_Flag C>
			class A_Poly_Poly_Edges {
			public:
				auto begin()       {  return I_Poly_Edge<C>    (_mesh, _poly, 0);  }
				auto begin() const {  return I_Poly_Edge<CONST>(_mesh, _poly, 0);  }

				auto end()       {  return I_Poly_Edge<C>    (_mesh, _poly, 3);  }
				auto end() const {  return I_Poly_Edge<CONST>(_mesh, _poly, 3);  }

			private:
				A_Poly_Poly_Edges(Const<Mesh,C>& mesh, H_Poly poly) : _mesh(mesh), _poly(poly) {}
				friend Mesh;

			private:
				Const<Mesh,C>& _mesh;
				H_Poly _poly;
			};









		};








		class Builder {
		public:
			using BUILD = Mesh;

			template<class NEW_VERT_PROPS>
			using Vert_Props =
				typename Context<SCALAR, NEW_VERT_PROPS, SUBVERT_PROPS,
					POLY_PROPS, POLY_VERT_PROPS, POLY_EDGE_PROPS> :: Builder;

			template<class NEW_SUBVERT_PROPS>
			using Subvert_Props =
				typename Context<SCALAR, VERT_PROPS, NEW_SUBVERT_PROPS,
					POLY_PROPS, POLY_VERT_PROPS, POLY_EDGE_PROPS> :: Builder;

			template<class NEW_POLY_PROPS>
			using Poly_Props =
				typename Context<SCALAR, VERT_PROPS, SUBVERT_PROPS,
					NEW_POLY_PROPS, POLY_VERT_PROPS, POLY_EDGE_PROPS> :: Builder;

			template<class NEW_POLY_VERT_PROPS>
			using Poly_Vert_Props =
				typename Context<SCALAR, VERT_PROPS, SUBVERT_PROPS,
					POLY_PROPS, NEW_POLY_VERT_PROPS, POLY_EDGE_PROPS> :: Builder;

			template<class NEW_POLY_EDGE_PROPS>
			using Poly_Edge_Props =
				typename Context<SCALAR, VERT_PROPS, SUBVERT_PROPS,
					POLY_PROPS, POLY_VERT_PROPS, NEW_POLY_EDGE_PROPS> :: Builder;
		};



	}; // struct Context
} // namespace Mesh
} // namespace internal












template<
	class T = void
>
class Mesh : public internal::Mesh::Context<
	double, // SCALAR
	void,   // VERT_PROPS
	void,   // SUBVERT_PROPS
	void,   // POLY_PROPS
	void,   // POLY_VERT_PROPS
	void    // POLY_EDGE_PROPS
> :: Mesh {
private:
	using _BASE = internal::Mesh::Context<
		double, // SCALAR
		void,   // VERT_PROPS
		void,   // SUBVERT_PROPS
		void,   // POLY_PROPS
		void,   // POLY_VERT_PROPS
		void    // POLY_EDGE_PROPS
	> :: Mesh;

public:
	using BUILDER = internal::Mesh::Context<
		double, // SCALAR
		void,   // VERT_PROPS
		void,   // SUBVERT_PROPS
		void,   // POLY_PROPS
		void,   // POLY_VERT_PROPS
		void    // POLY_EDGE_PROPS
	> :: Builder;
};












} // namespace salgo
