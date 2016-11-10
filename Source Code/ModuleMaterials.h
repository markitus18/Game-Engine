#ifndef __MODULE_MATERIALS_H__
#define __MODULE_MATERIALS_H__

#include "Module.h"
#include "Color.h"
#include <list>

class C_Material;
struct aiMaterial;

class ModuleMaterials : public Module
{
public:
	ModuleMaterials(Application* app, bool start_enabled = true);
	~ModuleMaterials();

	bool Init();
	bool CleanUp();

	C_Material* Exists(const char* texture_path) const;
	C_Material* ImportMaterial(const aiMaterial* from, const std::string& texture_path);


	//Own file format conversion
	uint ImportTexture(const char* path);

	std::string SaveTexture(const char* buffer, uint size, const char* path);
	uint LoadTexture(const char* path);

private:
	std::list<C_Material*> materials;
	uint materials_count = 0;
};

#endif // __MODULE_MATERIALS_H__
