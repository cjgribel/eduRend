
//
//  OBJ/MTL loader
//
//  Carl Johan Gribel 2016-2021, cjgribel@gmail.com
//

#include <fstream>
#include <algorithm>
#include "OBJLoader.h"
#include "vec/vec.h"
#include "parseutil.h"

using namespace linalg;

//
// Auxiliary structs for raw file data
//
struct unwelded_triangle_t { int vi[9]; };
struct unwelded_quad_t { int vi[12]; };
struct unwelded_drawcall_t
{
	std::string mtl_name;
	std::string group_name;
	std::vector<unwelded_triangle_t> tris;
	std::vector<unwelded_quad_t> quads;
	int v_ofs = 0;
};

//
// Creates normals to a set of vertices by averaging the 
// geometric normals of the faces they belong to
//
// If a model lacks normals, this function can be used 
// to create them. Works best for relatively smooth models.
//
void GenerateNormals(
	const std::vector<vec3f>& v, 
	std::vector<vec3f>& vn, 
	std::vector<unwelded_drawcall_t>& drawcalls)
{
	std::vector<vec3f>* v_bin = new std::vector<vec3f>[v.size()];

	// bin normals from all faces to vertex bins
	for (unwelded_drawcall_t& dc : drawcalls)
		for (unwelded_triangle_t& tri : dc.tris)
		{
			int a = tri.vi[0], b = tri.vi[1], c = tri.vi[2];
			vec3f v0 = v[a], v1 = v[b], v2 = v[c];
			vec3f n = linalg::normalize((v1 - v0) % (v2 - v0));

			v_bin[a].push_back(n);
			v_bin[b].push_back(n);
			v_bin[c].push_back(n);

			memcpy(tri.vi + 3, tri.vi, 3 * sizeof(int));
		}

	// average binned normals and add to array
	for (size_t i = 0; i < v.size(); i++)
	{
		vec3f n = vec3f_zero;
		for (size_t j = 0; j < v_bin[i].size(); j++)
		{
			n += v_bin[i][j];
		}
		n = linalg::normalize(n);

		vn.push_back(n);
	}

	delete[] v_bin;
}

void OBJLoader::LoadMaterials(
	std::string path, 
	std::string filename, 
	MaterialHash &mtl_hash)
{
    std::string fullpath = path+filename;
    
    std::ifstream in(fullpath.c_str());
    if (!in)
        throw std::runtime_error(std::string("Failed to open ") + fullpath);
    std::cout << "Opened " << fullpath << "\n";
    
    std::string line;
    Material *current_mtl = NULL;
    
    while (getline(in, line, '\n'))
    {
		const int MaxChars = 1024;
		char str0[MaxChars] = { 0 }; // , str1[MaxChars];
        float a,b,c;
        
		lrtrim(line);
        if (sscanf_s(line.c_str(), "newmtl %s", str0, MaxChars) == 1)
        {
            // check for duplicate
            if (mtl_hash.find(str0) != mtl_hash.end() ) printf("Warning: duplicate material '%s'\n", str0);
            
            mtl_hash[str0] = Material();
            current_mtl = &mtl_hash[str0];
            current_mtl->name = str0;
        }
        else if (!current_mtl)
        {
            // no parsed material so can't add any content
            continue;
        }
        else if (sscanf_s(line.c_str(), "map_Kd %[^\n]", str0, MaxChars) == 1)
        {
            // search for the image file and ignore the rest
            std::string mapfile;
			if (find_filename_from_suffixes(str0, ALLOWED_TEXTURE_SUFFIXES, mapfile))
                current_mtl->Kd_texture_filename = path + mapfile;
            else
                throw std::runtime_error(std::string("Error: no allowed format found for 'map_Kd' in material ") + current_mtl->name);
        }
        else if (sscanf_s(line.c_str(), "map_bump %[^\n]", str0, MaxChars) == 1)
        {
            // search for the image file and ignore the rest
            std::string mapfile;
			if (find_filename_from_suffixes(str0, ALLOWED_TEXTURE_SUFFIXES, mapfile))
                current_mtl->normal_texture_filename = path+mapfile;
            else
                throw std::runtime_error(std::string("Error: no allowed format found for 'map_bump' in material ") + current_mtl->name);
        }
        else if (sscanf_s(line.c_str(), "bump %[^\n]", str0, MaxChars) == 1)
        {
            // search for the image file and ignore the rest
            std::string mapfile;
			if (find_filename_from_suffixes(str0, ALLOWED_TEXTURE_SUFFIXES, mapfile))
                current_mtl->normal_texture_filename = path+mapfile;
            else
                throw std::runtime_error(std::string("Error: no allowed format found for 'bump' in material ") + current_mtl->name);
        }
        else if (sscanf_s(line.c_str(), "Ka %f %f %f", &a, &b, &c) == 3)
        {
            current_mtl->Ka = vec3f(a, b, c);
        }
        else if (sscanf_s(line.c_str(), "Kd %f %f %f", &a, &b, &c) == 3)
        {
            current_mtl->Kd = vec3f(a, b, c);
        }
        else if (sscanf_s(line.c_str(), "Ks %f %f %f", &a, &b, &c) == 3)
        {
            current_mtl->Ks = vec3f(a, b, c);
        }
    }
    in.close();
}

void OBJLoader::Load(
	const std::string& filename,
	bool auto_generate_normals,
	bool triangulate)
{
	std::string parentdir = get_parentdir(filename);

	std::ifstream in(filename.c_str());
	if (!in) throw std::runtime_error(std::string("Failed to open ") + filename);
	std::cout << "Opened " << filename << "\n";

	// raw data from obj
	std::vector<vec3f> file_vertices, file_normals;
	std::vector<vec2f> file_texcoords;
	std::vector<unwelded_drawcall_t> file_drawcalls;
	MaterialHash file_materials;

	std::string current_group_name;
	unwelded_drawcall_t default_drawcall;
	unwelded_drawcall_t* current_drawcall = &default_drawcall;
	int last_ofs = 0; bool face_section = false; // info for skin weight mapping

	std::string line;
	while (getline(in, line))
	{
		const int MaxChars = 1024;
		float x, y, z;
		int a[3], b[3], c[3], d[3];
		char str[MaxChars];

		// material file
		//
		if (sscanf_s(line.c_str(), "mtllib %s", str, MaxChars) == 1)
		{
			LoadMaterials(parentdir, str, file_materials);
		}
		// active material
		//
		else if (sscanf_s(line.c_str(), "usemtl %s", str, MaxChars) == 1)
		{
			unwelded_drawcall_t udc;
			udc.mtl_name = str;
			udc.group_name = current_group_name;
			udc.v_ofs = last_ofs; face_section = true; // skinning: set current vertex offset and mark beginning of a face-section
			file_drawcalls.push_back(udc);
			current_drawcall = &file_drawcalls.back();
		}
		else if (sscanf_s(line.c_str(), "g %s", str, MaxChars) == 1)
		{
			current_group_name = str;
		}
		// 3D vertex
		//
		else if (sscanf_s(line.c_str(), "v %f %f %f", &x, &y, &z) == 3)
		{
			// update vertex offset and mark end to a face section
			if (face_section) {
				last_ofs = file_vertices.size();
				face_section = false;
			}

			file_vertices.push_back(vec3f(x, y, z));
		}
		// 2D vertex
		//
		else if (sscanf_s(line.c_str(), "v %f %f", &x, &y) == 2)
		{
			file_vertices.push_back(vec3f(x, y, 0.0f));
		}
		// 3D texel (not supported: ignore last component)
		//
		else if (sscanf_s(line.c_str(), "vt %f %f %f", &x, &y, &z) == 3)
		{
			file_texcoords.push_back(vec2f(x, y));
		}
		// 2D texel
		//
		else if (sscanf_s(line.c_str(), "vt %f %f", &x, &y) == 2)
		{
			file_texcoords.push_back(vec2f(x, y));
		}
		// normal
		//
		else if (sscanf_s(line.c_str(), "vn %f %f %f", &x, &y, &z) == 3)
		{
			file_normals.push_back(vec3f(x, y, z));
		}
		// face: 4x vertex
		//
		else if (sscanf_s(line.c_str(), "f %d %d %d %d", &a[0], &b[0], &c[0], &d[0]) == 4)
		{
			if (triangulate) {
				current_drawcall->tris.push_back({ a[0] - 1, b[0] - 1, c[0] - 1, -1, -1, -1, -1, -1, -1 });
				current_drawcall->tris.push_back({ a[0] - 1, c[0] - 1, d[0] - 1, -1, -1, -1, -1, -1, -1 });
			}
			else
				current_drawcall->quads.push_back({ a[0] - 1, b[0] - 1, c[0] - 1, d[0] - 1, -1, -1, -1, -1, -1, -1, -1, -1 });
		}
		// face: 3x vertex
		//
		else if (sscanf_s(line.c_str(), "f %d %d %d", &a[0], &b[0], &c[0]) == 3)
		{
			current_drawcall->tris.push_back({ a[0] - 1, b[0] - 1, c[0] - 1, -1, -1, -1, -1, -1, -1 });
		}
		// face: 4x vertex/texel (triangulate)
		//
		else if (sscanf_s(line.c_str(), "f %d/%d %d/%d %d/%d %d/%d", &a[0], &a[1], &b[0], &b[1], &c[0], &c[1], &d[0], &d[1]) == 8)
		{
			if (triangulate) {
				current_drawcall->tris.push_back({ a[0] - 1, b[0] - 1, c[0] - 1, -1, -1, -1, a[1] - 1, b[1] - 1, c[1] - 1 });
				current_drawcall->tris.push_back({ a[0] - 1, c[0] - 1, d[0] - 1, -1, -1, -1, a[1] - 1, c[1] - 1, d[1] - 1 });
			}
			else
				current_drawcall->quads.push_back({ a[0] - 1, b[0] - 1, c[0] - 1, d[0] - 1, -1, -1, -1, -1, a[1] - 1, b[1] - 1, c[1] - 1, d[1] - 1 });
		}
		// face: 3x vertex/texel
		//
		else if (sscanf_s(line.c_str(), "f %d/%d %d/%d %d/%d", &a[0], &a[1], &b[0], &b[1], &c[0], &c[1]) == 6)
		{
			current_drawcall->tris.push_back({ a[0] - 1, b[0] - 1, c[0] - 1, -1, -1, -1, a[1] - 1, b[1] - 1, c[1] - 1 });
		}
		// face: 4x vertex//normal (triangulate)
		//
		else if (sscanf_s(line.c_str(), "f %d//%d %d//%d %d//%d %d//%d", &a[0], &a[1], &b[0], &b[1], &c[0], &c[1], &d[0], &d[1]) == 8)
		{
			if (triangulate) {
				current_drawcall->tris.push_back({ a[0] - 1, b[0] - 1, c[0] - 1, a[1] - 1, b[1] - 1, c[1] - 1, -1, -1, -1 });
				current_drawcall->tris.push_back({ a[0] - 1, c[0] - 1, d[0] - 1, a[1] - 1, c[1] - 1, d[1] - 1, -1, -1, -1 });
			}
			else
				current_drawcall->quads.push_back({ a[0] - 1, b[0] - 1, c[0] - 1, d[0] - 1, a[2] - 1, b[2] - 1, c[2] - 1, d[2] - 1, -1, -1, -1, -1 });
		}
		// face: 3x vertex//normal
		//
		else if (sscanf_s(line.c_str(), "f %d//%d %d//%d %d//%d", &a[0], &a[1], &b[0], &b[1], &c[0], &c[1]) == 6)
		{
			current_drawcall->tris.push_back({ a[0] - 1, b[0] - 1, c[0] - 1, a[1] - 1, b[1] - 1, c[1] - 1, -1, -1, -1 });
		}
		// face: 4x vertex/texel/normal (triangulate)
		//
		else if (sscanf_s(line.c_str(), "f %d/%d/%d %d/%d/%d %d/%d/%d %d/%d/%d", &a[0], &a[1], &a[2], &b[0], &b[1], &b[2], &c[0], &c[1], &c[2], &d[0], &d[1], &d[2]) == 12)
		{
			if (triangulate) {
				current_drawcall->tris.push_back({ a[0] - 1, b[0] - 1, c[0] - 1, a[2] - 1, b[2] - 1, c[2] - 1, a[1] - 1, b[1] - 1, c[1] - 1 });
				current_drawcall->tris.push_back({ a[0] - 1, c[0] - 1, d[0] - 1, a[2] - 1, c[2] - 1, d[2] - 1, a[1] - 1, c[1] - 1, d[1] - 1 });
			}
			else
				current_drawcall->quads.push_back({ a[0] - 1, b[0] - 1, c[0] - 1, d[0] - 1, a[2] - 1, b[2] - 1, c[2] - 1, d[2] - 1, a[1] - 1, b[1] - 1, c[1] - 1, d[1] - 1 });
		}
		// face: 3x vertex/texel/normal
		//
		else if (sscanf_s(line.c_str(), "f %d/%d/%d %d/%d/%d %d/%d/%d", &a[0], &a[1], &a[2], &b[0], &b[1], &b[2], &c[0], &c[1], &c[2]) == 9)
		{
			current_drawcall->tris.push_back({ a[0] - 1, b[0] - 1, c[0] - 1, a[2] - 1, b[2] - 1, c[2] - 1, a[1] - 1, b[1] - 1, c[1] - 1 });
		}
		// unknown obj syntax
		//
		else
		{

		}
	}
	in.close();

	// use defualt drawcall if no instance of usemtl
	if (!file_drawcalls.size())
		file_drawcalls.push_back(default_drawcall);

	has_normals = (bool)file_normals.size();
	has_texcoords = (bool)file_texcoords.size();

	printf("Loaded:\n\t%d vertices\n\t%d texels\n\t%d normals\n\t%d drawcalls\n",
		(int)file_vertices.size(), (int)file_texcoords.size(), (int)file_normals.size(), (int)file_drawcalls.size());

#if 1
	// auto-generate normals
	if (!has_normals && auto_generate_normals)
	{
		GenerateNormals(file_vertices, file_normals, file_drawcalls);
		has_normals = true;
		printf("Auto-generated %d normals\n", (int)file_normals.size());
	}
#endif

#if 1
	printf("Welding vertex array...");

	std::unordered_map<std::string, unsigned> mtl_to_index_hash;

	// hash function for int3
	struct int3_hashfunction {
		std::size_t operator () (const int3& i3) const {
			return i3.x;
		}
	};

	for (auto &dc : file_drawcalls)
	{
		Drawcall wdc;
		wdc.group_name = dc.group_name;

		std::unordered_map<int3, unsigned, int3_hashfunction> index3_to_index_hash;

		// material
		//
		if (dc.mtl_name.size())
		{
			//
			// is material added to main vector?
			auto mtl_index = mtl_to_index_hash.find(dc.mtl_name);
			if (mtl_index == mtl_to_index_hash.end())
			{
				auto mtl = file_materials.find(dc.mtl_name);

				if (mtl == file_materials.end())
					throw std::runtime_error(std::string("Error: used material ") + dc.mtl_name + " not found\n");

				wdc.mtl_index = (unsigned)materials.size();
				mtl_to_index_hash[dc.mtl_name] = (unsigned)materials.size();

				materials.push_back(mtl->second);
			}
			else
				wdc.mtl_index = mtl_index->second;;
		}
		else
			// mtl string is empty, use empty index
			wdc.mtl_index = -1;

		// weld vertices from triangles
		//
		for (auto &tri : dc.tris)
		{
			triangle wtri;

			for (int i = 0; i < 3; i++)
			{
				int3 i3 = { tri.vi[0 + i], tri.vi[3 + i], tri.vi[6 + i] };

				auto s = index3_to_index_hash.find(i3);
				if (s == index3_to_index_hash.end())
				{
					// index-combo does not exist, create it
					Vertex v;
					v.Pos = file_vertices[i3.x];
					if (i3.y > -1) v.Normal = file_normals[i3.y];
					if (i3.z > -1) v.TexCoord = file_texcoords[i3.z];

					wtri.vi[i] = (unsigned)vertices.size();
					index3_to_index_hash[i3] = (unsigned)(vertices.size());

					vertices.push_back(v);
				}
				else
				{
					// use existing index-combo
					wtri.vi[i] = s->second;
				}
			}
			wdc.tris.push_back(wtri);
		}

#if 1
		// weld vertices from quads
		//
		for (auto &quad : dc.quads)
		{
			quad_t_ wquad;

			for (int i = 0; i < 4; i++)
			{
				int3 i3 = { quad.vi[0 + i], quad.vi[3 + i], quad.vi[6 + i] };

				auto s = index3_to_index_hash.find(i3);
				if (s == index3_to_index_hash.end())
				{
					// index-combo does not exist, create it
					Vertex v;
					v.Pos = file_vertices[i3.x];
					if (i3.y > -1) v.Normal = file_normals[i3.y];
					if (i3.z > -1) v.TexCoord = file_texcoords[i3.z];

					wquad.vi[i] = (unsigned)vertices.size();
					index3_to_index_hash[i3] = (unsigned)(vertices.size());

					vertices.push_back(v);
				}
				else
				{
					// use existing index-combo
					wquad.vi[i] = s->second;
				}
			}
			wdc.quads.push_back(wquad);
		}
#endif

		drawcalls.push_back(wdc);
	}
	printf("Done\n");

	// Produce and print some stats
	//
	int tris = 0, quads = 0;
	for (auto &dc : drawcalls){
		tris += dc.tris.size();
		quads += dc.quads.size();
	}
	printf("\t%d vertices\n\t%d drawcalls\n\t%d triangles\n\t%d quads\n",
		vertices.size(), drawcalls.size(), tris, quads);
	printf("Loaded materials:\n");
	for (auto &mtl : materials)
		printf("\t%s\n", mtl.name.c_str());

#ifdef MESH_FORCE_CCW
    // Force counter-clockwise: 
	// flip triangle if geometric normal points away from vertex normal (at index=0)
    for (auto& dc : drawcalls)
        for (auto& tri : dc.tris)
        {
            int a = tri.vi[0], b = tri.vi[1], c = tri.vi[2];
            vec3f v0 = vertices[a].Pos, v1 = vertices[b].Pos, v2 = vertices[c].Pos;

            vec3f geo_n = linalg::normalize((v1-v0)%(v2-v0));
            vec3f vert_n = vertices[a].Normal;
            
            if (linalg::dot(geo_n, vert_n) < 0)
                std::swap(tri.vi[0], tri.vi[1]);
        }
#endif
    
#ifdef MESH_SORT_DRAWCALLS
	// Sort drawcalls based on material
	// This is a first step towards 'batch-rendering', which means that 
	// drawcalls with the same resources (mainly shader & material) are 
	// rendered back-to-back to make the number of texture binds (which are slow)
	// as low as possible
    std::sort(drawcalls.begin(), drawcalls.end());
	printf("Sorted drawcalls\n");
#endif
    
#endif
}
