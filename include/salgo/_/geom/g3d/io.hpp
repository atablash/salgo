#pragma once

#include "../../has-member.hpp"

#include <tinyply.h>

#include <glog/logging.h>

#include <fstream>
#include <chrono>

namespace salgo::geom::g3d {















//
// todo: save texcoords, normals and all other props that mesh contains
//
template<class MESH, class FILE_NAME>
inline void save_ply(const MESH& mesh, FILE_NAME&& filename, bool binary = true) {

	// Tinyply does not perform any file i/o _ly
	std::filebuf fb;
	fb.open(filename, std::ios::out | std::ios::binary);
	std::ostream outputStream(&fb);

	::tinyply::PlyFile myFile;

	std::vector<int> v_remap(mesh.verts().domain());

	std::vector<float> verts;
	verts.reserve( mesh.verts().domain() * 3);
	{
		int ith_vert = 0;
		for(auto& v : mesh.verts()) {
			verts.push_back( v.pos()[0] );
			verts.push_back( v.pos()[1] );
			verts.push_back( v.pos()[2] );

			v_remap[v.handle()] = ith_vert++;
		}
	}

	std::vector<int32_t> vertexIndicies;
	vertexIndicies.reserve( mesh.polys().domain() * 3);
	for(auto& p : mesh.polys()) {
		vertexIndicies.push_back( v_remap[ p.vert(0).handle() ] );
		vertexIndicies.push_back( v_remap[ p.vert(1).handle() ] );
		vertexIndicies.push_back( v_remap[ p.vert(2).handle() ] );
	}

	//std::vector<float> faceTexcoords;
	//for(auto& p : mesh.polys) {
	//	for(int i=0; i<3; ++i) {
	//		for(int j=0; j<2; ++j) {
	//			faceTexcoords.push_back(p.texcoords[i][j]);
	//		}
	//	}
	//}

	//myFile.comments.push_back("TextureFile " + texture_path);

	myFile.add_properties_to_element("vertex", { "x", "y", "z" }, verts);

	myFile.add_properties_to_element("face", { "vertex_indices" }, vertexIndicies, 3, ::tinyply::PlyProperty::Type::UINT8);
	//myFile.add_properties_to_element("face", { "texcoord" }, faceTexcoords, 6, tinyply::PlyProperty::Type::UINT8);

	myFile.write(outputStream, binary);

	fb.close();
}



SALGO_GENERATE_HAS_MEMBER(normal);
SALGO_GENERATE_HAS_MEMBER(color);
SALGO_GENERATE_HAS_MEMBER(texcoords);


template<class MESH>
MESH load_ply(const std::string& file_name) {
	using namespace ::tinyply;
	using namespace std;
	using namespace std::chrono;

	//std::cout << "opening " << file_name << std::endl;
	std::ifstream ss(file_name.c_str(), std::ios::binary);

	if(!ss.good()) {
		std::cerr << "Failed to open " << file_name << ": " << strerror(errno) << std::endl;
		return MESH();
	}

	// Parse the ASCII header fields
	PlyFile file(ss);

	if(!ss.good()) {
		std::cerr << "Failed to read " << file_name << " ASCII header: " << strerror(errno) << std::endl;
		return MESH();
	}

	// debug print
	for (auto e : file.get_elements())
	{
		DLOG(INFO) << "element - " << e.name << " (" << e.size << ")";
		for (auto p : e.properties)
		{
			DLOG(INFO) << "\tproperty - " << p.name << " (" << PropertyTable[p.propertyType].str << ")";
		}
	}
	DLOG(INFO) << std::endl;

	for (auto c : file.comments)
	{
		DLOG(INFO) << "Comment: " << c;
	}

	// Define containers to hold the extracted data. The type must match
	// the property type given in the header. Tinyply will interally allocate the
	// the appropriate amount of memory.
	std::vector<float> verts;
	std::vector<float> v_normals;
	std::vector<uint8_t> v_colors;

	std::vector<uint32_t> polys;
	std::vector<float> p_texcoords;

	// The count returns the number of instances of the property group. The vectors
	// above will be resized into a multiple of the property group size as
	// they are "flattened"... i.e. verts = {x, y, z, x, y, z, ...}
	int verts_count = file.request_properties_from_element("vertex", { "x", "y", "z" }, verts);
	int v_normals_count = file.request_properties_from_element("vertex", { "nx", "ny", "nz" }, v_normals);
	int v_colors_count = file.request_properties_from_element("vertex", { "red", "green", "blue", "alpha" }, v_colors);

	// For properties that are list types, it is possibly to specify the expected count (ideal if a
	// consumer of this library knows the layout of their format a-priori). Otherwise, tinyply
	// defers allocation of memory until the first instance of the property has been found
	// as implemented in file.read(ss)
	int polys_count = file.request_properties_from_element("face", { "vertex_indices" }, polys, 3);
	int p_texcoords_count = file.request_properties_from_element("face", { "texcoord" }, p_texcoords, 6);

	// Now populate the vectors...
	auto before = steady_clock::now();
	file.read(ss);
	auto after = steady_clock::now();

	if(!ss.good()) {
		std::cerr << "Failed to read " << file_name << " past the ASCII header: " << strerror(errno) << std::endl;
		return MESH();
	}

	// Good place to put a breakpoint!
	DLOG(INFO) << "Parsing took " << duration_cast<milliseconds>(after-before).count() << "ms:";
	DLOG(INFO) << "\tRead " << verts.size() << " total vertices (" << verts_count << " properties).";
	DLOG(INFO) << "\tRead " << v_normals.size() << " total normals (" << v_normals_count << " properties).";
	DLOG(INFO) << "\tRead " << v_colors.size() << " total vertex colors (" << v_colors_count << " properties).";
	DLOG(INFO) << "\tRead " << polys.size() << " total faces (triangles) (" << polys_count << " properties).";
	DLOG(INFO) << "\tRead " << p_texcoords.size() << " total texcoords (" << p_texcoords_count << " properties).";




	//
	// copy data to resultant MESH
	//

	MESH mesh;

	mesh.verts().reserve(verts_count);
	for(int i=0; i<(int)verts_count; ++i){
		auto v = mesh.verts().add(verts[i*3 + 0], verts[i*3 + 1], verts[i*3 + 2]);
		(void)v;

		if constexpr(has_member__normal<typename MESH::Vert_Data>) if(v_normals_count) {
			v.data().normal = { v_normals[i*3 + 0], v_normals[i*3 + 1], v_normals[i*3 + 2] };
		}

		if constexpr(has_member__color<typename MESH::Vert_Data>) if(v_colors_count) {
			v.data().color = { v_colors[i*4 + 0], v_colors[i*4 + 1], v_colors[i*4 + 2], v_colors[i*4 + 3] };
		}
	}

	mesh.polys().reserve(polys_count);
	for(int i=0; i<(int)polys_count; ++i) {
		auto p = mesh.polys().add(
			polys[i*3 + 0],
			polys[i*3 + 1],
			polys[i*3 + 2]
		);
		(void)p;

		if constexpr(has_member__texcoords<typename MESH::PolyVert_Data>) if(p_texcoords_count) {
			p.poly_vert(0).data().texcoords = { p_texcoords[i*6 + 0], p_texcoords[i*6 + 1] };
			p.poly_vert(1).data().texcoords = { p_texcoords[i*6 + 2], p_texcoords[i*6 + 3] };
			p.poly_vert(2).data().texcoords = { p_texcoords[i*6 + 4], p_texcoords[i*6 + 5] };
		}
	}

	LOG(INFO) << "loaded " << file_name << "   verts: " << mesh.verts().domain() << "   polys: " << mesh.polys().domain();

	return mesh;
}




} // namespace smesh



