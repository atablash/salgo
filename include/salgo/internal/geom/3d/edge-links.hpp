#pragma once

#include <unordered_map>



namespace salgo::geom::geom_3d {









//
// check if edge links are valid and 2-way
//
template< class MESH >
bool has_valid_edge_links( const MESH& mesh ) {

	for( auto& p : mesh.polys() ) {
		for( auto& pe : p.polyEdges() ) {
			if( !pe.is_linked() ) continue;

			if( !pe.linked_polyEdge().is_linked() ) return false;

			if( pe.linked_polyEdge().linked_polyEdge().handle() != pe.handle() ) return false;
		}
	}

	return true;
}


//
// check if all edges have links
// (assumes links are valid)
//
template< class MESH >
bool has_all_edge_links( const MESH& mesh ) {

	for( auto p : mesh.polys() ) {
		for( auto pe : p.polyEdges() ) {
			if( !pe.is_linked() ) return false;
		}
	}

	return true;
}

















//
// hash pairs:
//

// namespace ab {
// 	template<class T>inline T cycle_bits_left(T x,int i){return (x<<i) ^ (x>>(sizeof(T)*8-i));} // cycle bits left
// }

// namespace std {
// 	template<class A, class B> struct hash<pair<A,B> > {
// 		size_t operator()(const pair<A,B>& p) const {
// 			return hash<A>()(p.first) ^ ab::cycle_bits_left(hash<B>()(p.second),sizeof(size_t)*8/2);
// 	    }
// 	};
// }



//
// if one edge is shared among more than 2 polys, pairing order is undefined
//
template<class MESH>
auto fast_compute_edge_links(MESH& mesh) {
	struct Result {
		int num_matched_edges = 0;
		int num_open_edges = 0;
	};
	Result result;

	salgo::Hash_Table<
		std::pair<typename MESH::SH_Vert, typename MESH::SH_Vert>,
		typename MESH::SH_PolyEdge
	> open_edges;

	for(auto& p : mesh.polys()) {
		//std::cout << "process poly " << p.handle() << std::endl;

		for(auto& pe : p.polyEdges()) {
			//std::cout << "process poly-edge " << pe.handle() << std::endl;

			auto edge_key = std::pair(pe.prev_vert().handle(), pe.next_vert().handle());

			auto rev_edge_key = edge_key;
			std::swap(rev_edge_key.first, rev_edge_key.second);

			auto open_edge = open_edges(rev_edge_key);

			if(open_edge.not_found()) {
				//std::cout << "rev-edge " << rev_edge_key.first << "," << rev_edge_key.second << " not found" << std::endl;
				//std::cout << "emplacing this edge " << edge_key.first << "," << edge_key.second << std::endl;
				open_edges.emplace(edge_key, pe.handle());
			}
			else {
				//std::cout << "found rev-edge " << rev_edge_key.first << "," << rev_edge_key.second << std::endl;

				auto other_pe = mesh( open_edge.val() );
				open_edge.erase();

				//std::cout << "link " << pe.handle() << " with " << other_pe.handle() << std::endl;
				pe.link(other_pe);
				++result.num_matched_edges;
			}
		}
	}

	result.num_open_edges = open_edges.count();

	return result;
}






} // namespace salgo::geom::geom_3d

