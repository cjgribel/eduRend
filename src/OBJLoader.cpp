
//
//  OBJ/MTL loader
//
//  Carl Johan Gribel 2016-2021, cjgribel@gmail.com
//

#include <fstream>
#include <algorithm>
#include "OBJLoader.h"
#include "vec/Vec.h"
#include "parseutil.h"

using namespace linalg;

//
// Auxiliary structs for raw file data
//
struct UnweldedTriangle_t { int vi[9]; };
struct UnweldedQuad_t { int vi[12]; };
struct UnweldedDrawcall_t
{
	std::string materialName;
	std::string groupName;
	std::vector<UnweldedTriangle_t> tris;
	std::vector<UnweldedQuad_t> quads;
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
	const std::vector<Vec3f>& v, 
	std::vector<Vec3f>& vn, 
	std::vector<UnweldedDrawcall_t>& drawcalls)
{
	std::vector<Vec3f>* v_bin = new std::vector<Vec3f>[v.size()];

	// bin normals from all faces to vertex bins
	for (UnweldedDrawcall_t& dc : drawcalls)
		for (UnweldedTriangle_t& tri : dc.tris)
		{
			int a = tri.vi[0], b = tri.vi[1], c = tri.vi[2];
			Vec3f v0 = v[a], v1 = v[b], v2 = v[c];
			Vec3f n = linalg::Normalize((v1 - v0) % (v2 - v0));

			v_bin[a].push_back(n);
			v_bin[b].push_back(n);
			v_bin[c].push_back(n);

			memcpy(tri.vi + 3, tri.vi, 3 * sizeof(int));
		}

	// average binned normals and add to array
	for (size_t i = 0; i < v.size(); i++)
	{
		Vec3f n = vec3f_zero;
		for (size_t j = 0; j < v_bin[i].size(); j++)
		{
			n += v_bin[i][j];
		}
		n = linalg::Normalize(n);

		vn.push_back(n);
	}

	delete[] v_bin;
}

void OBJLoader::LoadMaterials(
	std::string path, 
	std::string fileName, 
	MaterialHash &materialHash)
{
    std::string fullPath = path+fileName;
    
    std::ifstream in(fullPath.c_str());
    if (!in)
        throw std::runtime_error(std::string("Failed to open ") + fullPath);
    std::cout << "Opened " << fullPath << "\n";
    
    std::string line;
    Material *currentMaterial = NULL;
    
    while (getline(in, line, '\n'))
    {
		const int maxChars = 1024;
		char str0[maxChars] = { 0 }; // , str1[MaxChars];
        float a,b,c;
        
		lrtrim(line);
        if (sscanf_s(line.c_str(), "newmtl %s", str0, maxChars) == 1)
        {
            // check for duplicate
            if (materialHash.find(str0) != materialHash.end() ) printf("Warning: duplicate material '%s'\n", str0);
            
            materialHash[str0] = Material();
            currentMaterial = &materialHash[str0];
            currentMaterial->name = str0;
        }
        else if (!currentMaterial)
        {
            // no parsed material so can't add any content
            continue;
        }
        else if (sscanf_s(line.c_str(), "map_Kd %[^\n]", str0, maxChars) == 1)
        {
            // search for the image file and ignore the rest
            std::string mapfile;
			if (findFileNameFromSuffixes(str0, ALLOWED_TEXTURE_SUFFIXES, mapfile))
                currentMaterial->diffuseTextureFileName = path + mapfile;
            else
                throw std::runtime_error(std::string("Error: no allowed format found for 'map_Kd' in material ") + currentMaterial->name);
        }
        else if (sscanf_s(line.c_str(), "map_bump %[^\n]", str0, maxChars) == 1)
        {
            // search for the image file and ignore the rest
            std::string mapfile;
			if (findFileNameFromSuffixes(str0, ALLOWED_TEXTURE_SUFFIXES, mapfile))
                currentMaterial->normalTextureFileName = path+mapfile;
            else
                throw std::runtime_error(std::string("Error: no allowed format found for 'map_bump' in material ") + currentMaterial->name);
        }
        else if (sscanf_s(line.c_str(), "bump %[^\n]", str0, maxChars) == 1)
        {
            // search for the image file and ignore the rest
            std::string mapfile;
			if (findFileNameFromSuffixes(str0, ALLOWED_TEXTURE_SUFFIXES, mapfile))
                currentMaterial->normalTextureFileName = path+mapfile;
            else
                throw std::runtime_error(std::string("Error: no allowed format found for 'bump' in material ") + currentMaterial->name);
        }
        else if (sscanf_s(line.c_str(), "Ka %f %f %f", &a, &b, &c) == 3)
        {
            currentMaterial->Ka = Vec3f(a, b, c);
        }
        else if (sscanf_s(line.c_str(), "Kd %f %f %f", &a, &b, &c) == 3)
        {
            currentMaterial->Kd = Vec3f(a, b, c);
        }
        else if (sscanf_s(line.c_str(), "Ks %f %f %f", &a, &b, &c) == 3)
        {
            currentMaterial->Ks = Vec3f(a, b, c);
        }
    }
    in.close();
}

void OBJLoader::Load(
	const std::string& fileName,
	bool autoGenerateNormals,
	bool triangulate)
{
	std::string parentDir = getParentDir(fileName);

	std::ifstream in(fileName.c_str());
	if (!in) throw std::runtime_error(std::string("Failed to open ") + fileName);
	std::cout << "Opened " << fileName << "\n";

	// raw data from obj
	std::vector<Vec3f> fileVertices, fileNormals;
	std::vector<Vec2f> fileTexcoords;
	std::vector<UnweldedDrawcall_t> fileDrawcalls;
	MaterialHash fileMaterials;

	std::string currentGroupName;
	UnweldedDrawcall_t defaultDrawcall;
	UnweldedDrawcall_t* currentDrawcall = &defaultDrawcall;
	int lastOfs = 0; bool faceSection = false; // info for skin weight mapping

	std::string line;
	while (getline(in, line))
	{
		const int maxChars = 1024;
		float x, y, z;
		int a[3], b[3], c[3], d[3];
		char str[maxChars];

		// material file
		//
		if (sscanf_s(line.c_str(), "mtllib %s", str, maxChars) == 1)
		{
			LoadMaterials(parentDir, str, fileMaterials);
		}
		// active material
		//
		else if (sscanf_s(line.c_str(), "usemtl %s", str, maxChars) == 1)
		{
			UnweldedDrawcall_t udc;
			udc.materialName = str;
			udc.groupName = currentGroupName;
			udc.v_ofs = lastOfs; faceSection = true; // skinning: set current vertex offset and mark beginning of a face-section
			fileDrawcalls.push_back(udc);
			currentDrawcall = &fileDrawcalls.back();
		}
		else if (sscanf_s(line.c_str(), "g %s", str, maxChars) == 1)
		{
			currentGroupName = str;
		}
		// 3D vertex
		//
		else if (sscanf_s(line.c_str(), "v %f %f %f", &x, &y, &z) == 3)
		{
			// update vertex offset and mark end to a face section
			if (faceSection) {
				lastOfs = (int)fileVertices.size();
				faceSection = false;
			}

			fileVertices.push_back(Vec3f(x, y, z));
		}
		// 2D vertex
		//
		else if (sscanf_s(line.c_str(), "v %f %f", &x, &y) == 2)
		{
			fileVertices.push_back(Vec3f(x, y, 0.0f));
		}
		// 3D texel (not supported: ignore last component)
		//
		else if (sscanf_s(line.c_str(), "vt %f %f %f", &x, &y, &z) == 3)
		{
			fileTexcoords.push_back(Vec2f(x, y));
		}
		// 2D texel
		//
		else if (sscanf_s(line.c_str(), "vt %f %f", &x, &y) == 2)
		{
			fileTexcoords.push_back(Vec2f(x, y));
		}
		// normal
		//
		else if (sscanf_s(line.c_str(), "vn %f %f %f", &x, &y, &z) == 3)
		{
			fileNormals.push_back(Vec3f(x, y, z));
		}
		// face: 4x vertex
		//
		else if (sscanf_s(line.c_str(), "f %d %d %d %d", &a[0], &b[0], &c[0], &d[0]) == 4)
		{
			if (triangulate) {
				currentDrawcall->tris.push_back({ a[0] - 1, b[0] - 1, c[0] - 1, -1, -1, -1, -1, -1, -1 });
				currentDrawcall->tris.push_back({ a[0] - 1, c[0] - 1, d[0] - 1, -1, -1, -1, -1, -1, -1 });
			}
			else
				currentDrawcall->quads.push_back({ a[0] - 1, b[0] - 1, c[0] - 1, d[0] - 1, -1, -1, -1, -1, -1, -1, -1, -1 });
		}
		// face: 3x vertex
		//
		else if (sscanf_s(line.c_str(), "f %d %d %d", &a[0], &b[0], &c[0]) == 3)
		{
			currentDrawcall->tris.push_back({ a[0] - 1, b[0] - 1, c[0] - 1, -1, -1, -1, -1, -1, -1 });
		}
		// face: 4x vertex/texel (triangulate)
		//
		else if (sscanf_s(line.c_str(), "f %d/%d %d/%d %d/%d %d/%d", &a[0], &a[1], &b[0], &b[1], &c[0], &c[1], &d[0], &d[1]) == 8)
		{
			if (triangulate) {
				currentDrawcall->tris.push_back({ a[0] - 1, b[0] - 1, c[0] - 1, -1, -1, -1, a[1] - 1, b[1] - 1, c[1] - 1 });
				currentDrawcall->tris.push_back({ a[0] - 1, c[0] - 1, d[0] - 1, -1, -1, -1, a[1] - 1, c[1] - 1, d[1] - 1 });
			}
			else
				currentDrawcall->quads.push_back({ a[0] - 1, b[0] - 1, c[0] - 1, d[0] - 1, -1, -1, -1, -1, a[1] - 1, b[1] - 1, c[1] - 1, d[1] - 1 });
		}
		// face: 3x vertex/texel
		//
		else if (sscanf_s(line.c_str(), "f %d/%d %d/%d %d/%d", &a[0], &a[1], &b[0], &b[1], &c[0], &c[1]) == 6)
		{
			currentDrawcall->tris.push_back({ a[0] - 1, b[0] - 1, c[0] - 1, -1, -1, -1, a[1] - 1, b[1] - 1, c[1] - 1 });
		}
		// face: 4x vertex//normal (triangulate)
		//
		else if (sscanf_s(line.c_str(), "f %d//%d %d//%d %d//%d %d//%d", &a[0], &a[1], &b[0], &b[1], &c[0], &c[1], &d[0], &d[1]) == 8)
		{
			if (triangulate) {
				currentDrawcall->tris.push_back({ a[0] - 1, b[0] - 1, c[0] - 1, a[1] - 1, b[1] - 1, c[1] - 1, -1, -1, -1 });
				currentDrawcall->tris.push_back({ a[0] - 1, c[0] - 1, d[0] - 1, a[1] - 1, c[1] - 1, d[1] - 1, -1, -1, -1 });
			}
			else
				currentDrawcall->quads.push_back({ a[0] - 1, b[0] - 1, c[0] - 1, d[0] - 1, a[2] - 1, b[2] - 1, c[2] - 1, d[2] - 1, -1, -1, -1, -1 });
		}
		// face: 3x vertex//normal
		//
		else if (sscanf_s(line.c_str(), "f %d//%d %d//%d %d//%d", &a[0], &a[1], &b[0], &b[1], &c[0], &c[1]) == 6)
		{
			currentDrawcall->tris.push_back({ a[0] - 1, b[0] - 1, c[0] - 1, a[1] - 1, b[1] - 1, c[1] - 1, -1, -1, -1 });
		}
		// face: 4x vertex/texel/normal (triangulate)
		//
		else if (sscanf_s(line.c_str(), "f %d/%d/%d %d/%d/%d %d/%d/%d %d/%d/%d", &a[0], &a[1], &a[2], &b[0], &b[1], &b[2], &c[0], &c[1], &c[2], &d[0], &d[1], &d[2]) == 12)
		{
			if (triangulate) {
				currentDrawcall->tris.push_back({ a[0] - 1, b[0] - 1, c[0] - 1, a[2] - 1, b[2] - 1, c[2] - 1, a[1] - 1, b[1] - 1, c[1] - 1 });
				currentDrawcall->tris.push_back({ a[0] - 1, c[0] - 1, d[0] - 1, a[2] - 1, c[2] - 1, d[2] - 1, a[1] - 1, c[1] - 1, d[1] - 1 });
			}
			else
				currentDrawcall->quads.push_back({ a[0] - 1, b[0] - 1, c[0] - 1, d[0] - 1, a[2] - 1, b[2] - 1, c[2] - 1, d[2] - 1, a[1] - 1, b[1] - 1, c[1] - 1, d[1] - 1 });
		}
		// face: 3x vertex/texel/normal
		//
		else if (sscanf_s(line.c_str(), "f %d/%d/%d %d/%d/%d %d/%d/%d", &a[0], &a[1], &a[2], &b[0], &b[1], &b[2], &c[0], &c[1], &c[2]) == 9)
		{
			currentDrawcall->tris.push_back({ a[0] - 1, b[0] - 1, c[0] - 1, a[2] - 1, b[2] - 1, c[2] - 1, a[1] - 1, b[1] - 1, c[1] - 1 });
		}
		// unknown obj syntax
		//
		else
		{

		}
	}
	in.close();

	// use defualt drawcall if no instance of usemtl
	if (!fileDrawcalls.size())
		fileDrawcalls.push_back(defaultDrawcall);

	hasNormals = (bool)fileNormals.size();
	hasTexcoords = (bool)fileTexcoords.size();

	printf("Loaded:\n\t%d vertices\n\t%d texels\n\t%d normals\n\t%d drawcalls\n",
		(int)fileVertices.size(), (int)fileTexcoords.size(), (int)fileNormals.size(), (int)fileDrawcalls.size());

#if 1
	// auto-generate normals
	if (!hasNormals && autoGenerateNormals)
	{
		GenerateNormals(fileVertices, fileNormals, fileDrawcalls);
		hasNormals = true;
		printf("Auto-generated %d normals\n", (int)fileNormals.size());
	}
#endif

#if 1
	printf("Welding vertex array...");

	std::unordered_map<std::string, unsigned> materialToIndexHash;

	// hash function for int3
	struct int3_hashfunction {
		std::size_t operator () (const int3& i3) const {
			return i3.x;
		}
	};

	for (auto &dc : fileDrawcalls)
	{
		Drawcall wDrawcall;
		wDrawcall.groupName = dc.groupName;

		std::unordered_map<int3, unsigned, int3_hashfunction> index3ToIndexHash;

		// material
		//
		if (dc.materialName.size())
		{
			//
			// is material added to main vector?
			auto materialIndex = materialToIndexHash.find(dc.materialName);
			if (materialIndex == materialToIndexHash.end())
			{
				auto material = fileMaterials.find(dc.materialName);

				if (material == fileMaterials.end())
					throw std::runtime_error(std::string("Error: used material ") + dc.materialName + " not found\n");

				wDrawcall.materialIndex = (unsigned)materials.size();
				materialToIndexHash[dc.materialName] = (unsigned)materials.size();

				materials.push_back(material->second);
			}
			else
				wDrawcall.materialIndex = materialIndex->second;;
		}
		else
			// mtl string is empty, use empty index
			wDrawcall.materialIndex = -1;

		// weld vertices from triangles
		//
		for (auto &tri : dc.tris)
		{
			Triangle wTri;

			for (int i = 0; i < 3; i++)
			{
				int3 i3 = { tri.vi[0 + i], tri.vi[3 + i], tri.vi[6 + i] };

				auto s = index3ToIndexHash.find(i3);
				if (s == index3ToIndexHash.end())
				{
					// index-combo does not exist, create it
					Vertex v;
					v.pos = fileVertices[i3.x];
					if (i3.y > -1) v.normal = fileNormals[i3.y];
					if (i3.z > -1) v.texCoord = fileTexcoords[i3.z];

					wTri.vi[i] = (unsigned)vertices.size();
					index3ToIndexHash[i3] = (unsigned)(vertices.size());

					vertices.push_back(v);
				}
				else
				{
					// use existing index-combo
					wTri.vi[i] = s->second;
				}
			}
			wDrawcall.tris.push_back(wTri);
		}

#if 1
		// weld vertices from quads
		//
		for (auto &quad : dc.quads)
		{
			Quad wQuad;

			for (int i = 0; i < 4; i++)
			{
				int3 i3 = { quad.vi[0 + i], quad.vi[3 + i], quad.vi[6 + i] };

				auto s = index3ToIndexHash.find(i3);
				if (s == index3ToIndexHash.end())
				{
					// index-combo does not exist, create it
					Vertex v;
					v.pos = fileVertices[i3.x];
					if (i3.y > -1) v.normal = fileNormals[i3.y];
					if (i3.z > -1) v.texCoord = fileTexcoords[i3.z];

					wQuad.vi[i] = (unsigned)vertices.size();
					index3ToIndexHash[i3] = (unsigned)(vertices.size());

					vertices.push_back(v);
				}
				else
				{
					// use existing index-combo
					wQuad.vi[i] = s->second;
				}
			}
			wDrawcall.quads.push_back(wQuad);
		}
#endif

		drawcalls.push_back(wDrawcall);
	}
	printf("Done\n");

	// Produce and print some stats
	//
	int tris = 0, quads = 0;
	for (auto &drawcall : drawcalls){
		tris += (int)drawcall.tris.size();
		quads += (int)drawcall.quads.size();
	}
	printf("\t%d vertices\n\t%d drawcalls\n\t%d triangles\n\t%d quads\n",
		(int)vertices.size(), (int)drawcalls.size(), tris, quads);
	printf("Loaded materials:\n");
	for (auto &material : materials)
		printf("\t%s\n", material.name.c_str());

#ifdef MESH_FORCE_CCW
    // Force counter-clockwise: 
	// flip triangle if geometric normal points away from vertex normal (at index=0)
    for (auto& drawcall : drawcalls)
        for (auto& tri : drawcall.tris)
        {
            int a = tri.vi[0], b = tri.vi[1], c = tri.vi[2];
            Vec3f v0 = vertices[a].pos, v1 = vertices[b].pos, v2 = vertices[c].pos;

            Vec3f geoN = linalg::Normalize((v1-v0)%(v2-v0));
            Vec3f vertN = vertices[a].normal;
            
            if (linalg::Dot(geoN, vertN) < 0)
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
