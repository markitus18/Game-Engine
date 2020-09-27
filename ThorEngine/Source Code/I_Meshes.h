#ifndef __MODULE_MESHES_H__
#define __MODULE_MESHES_H__

#include "Globals.h"
#include <vector>

class C_Mesh;
class R_Mesh;

struct aiMesh;

namespace Importer
{
	namespace Meshes
	{		
		//Creates an empty material resource using default constructor
		R_Mesh* Create();
		
		//Processes aiMesh data into a ready-to-use R_Mesh to be saved later.
		//Returns nullptr if any errors occured during the process.
		void Import(const aiMesh* mesh, R_Mesh* resMesh);

		//Process R_Mesh data into a buffer ready to save
		//Returns the size of the buffer file (0 if any errors)
		//Warning: buffer memory needs to be released after the function call
		uint64 Save(const R_Mesh* mesh, char** buffer);

		//Process buffer data into a ready-to-use R_Mesh.
		//Returns nullptr if any errors occured during the process.
		void Load(const char* buffer, R_Mesh* mesh);


		namespace Private
		{
			void ImportBones(const aiMesh* mesh, R_Mesh* rMesh);

			void SaveBones(const R_Mesh* rMesh, char** cursor);

			void LoadBones(const char** cursor, R_Mesh* rMesh);
		}
	}
}
#endif
