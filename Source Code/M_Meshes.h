#ifndef __MODULE_MESHES_H__
#define __MODULE_MESHES_H__

#include "Module.h"
#include "R_Mesh.h"
#include <vector>

class C_Mesh;
class R_Mesh;

struct aiMesh;

class M_Meshes : public Module
{
	public:
		M_Meshes(bool start_enabled = true);
		~M_Meshes();

		bool Init();
		bool CleanUp();


		R_Mesh*	ImportMeshResource(const aiMesh* mesh, unsigned long long ID, const char* file, const char* name);

		bool	SaveMeshResource(const R_Mesh*);
		R_Mesh* LoadMeshResource(u64 ID);
};

namespace Importer
{
	namespace Meshes
	{
		//Processes aiMesh data into a ready-to-use R_Mesh to be saved later.
		//Returns nullptr if any errors occured during the process.
		R_Mesh* Import(const aiMesh* mesh, R_Mesh* resMesh);

		//Process R_Mesh data into a buffer ready to save
		//Returns the size of the buffer file (0 if any errors)
		//Warning: buffer memory needs to be released after the function call
		uint64 Save(const R_Mesh* mesh, char** buffer);

		//Process buffer data into a ready-to-use R_Mesh.
		//Returns nullptr if any errors occured during the process.
		R_Mesh* Load(const char* buffer);
	}
}
#endif
