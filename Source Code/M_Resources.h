#ifndef __M_RESOURCES_H__
#define __M_RESOURCES_H__

#include "Module.h"
#include "Resource.h"
#include "Component.h"
#include "Timer.h"
#include "MathGeoLib\src\Algorithm\Random\LCG.h"

#include <map>
#include <vector>

struct aiMesh;
struct aiMaterial;
struct aiAnimation;
struct aiBone;

class R_Mesh;
class R_Texture;
class R_Material;
class R_Prefab;
class R_Shader;
class R_ParticleSystem;

struct PathNode;

struct ResourceMeta
{
	Resource::Type type = Resource::UNKNOWN;
	std::string original_file = "";
	std::string resource_name = "";
	uint64 id = 0;

	bool Compare(const char* file, const char* name, Resource::Type type)
	{
		return (original_file == file && (name ? resource_name == name : true) && type == this->type);
	}

};

class M_Resources : public Module
{
public:
	M_Resources(bool start_enabled = true);
	~M_Resources();

	bool Init(Config& config);
	bool Start();
	update_status Update(float dt);
	bool CleanUp();

	//Import a file from outside the project folder
	void ImportFileFromExplorer(const char* path);

	//Import a file existing in assets creating the resources
	Resource* ImportFileFromAssets(const char* path);
	
	void ImportScene(const char* buffer, uint size, R_Prefab* prefab);
	uint64 ImportRMesh(const aiMesh* mesh, const char* source_file);
	void ImportRTexture(const char* buffer, uint size, R_Texture* texture);
	//Same as ImportRTexture, but we need different functions
	uint64 ImportPrefabImage(char* buffer, const char* source_file, uint sizeX, uint sizeY);
	uint64 ImportRMaterial(const aiMaterial* mat, const char* source_file);
	uint64 ImportRAnimation(const aiAnimation* anim, const char* source_file);

	//Called when a particle system is modified externally (due to copy-paste or commit update)
	void ImportRParticleSystem(const char* buffer, uint size, R_ParticleSystem* rParticles);
	
	//Called when a shader is modified externally (due to copy-paste or commit update)
	void ImportRShader(const char* buffer, uint size, R_Shader* shader);
	
	//Generates the base data for a resource
	Resource* CreateResourceBase(const char* assetsPath, Resource::Type type, const char* name = nullptr, uint64 forceID = 0);

	//Used for internal resources (external referring to fbx, textures,...)
	Resource* CreateNewCopyResource(const char* directory, const char* defaultPath, Resource::Type type);
	
	Resource* LoadResourceBase(uint64 ID);

	//Getting a resource by ID
	//Resource PREFAB creates a new GameObject in the scene
	Resource* GetResource(uint64 ID);
	Resource::Type GetTypeFromPath(const char* path) const;
	bool GetAllMetaFromType(Resource::Type type, std::vector<const ResourceMeta*>& metas) const;

	void LoadPrefab(uint64 ID);

	PathNode CollectImportedScenes();
	Component::Type M_Resources::ResourceToComponentType(Resource::Type type);

	//TMP: move into private? usage in P_Explorer.cpp
	uint64 GetIDFromMeta(const char* path);
	Resource::Type GetTypeFromMeta(const char* path);
	inline uint64 GetNewID() { return random.Int(); } ;

private:
	void LoadResourcesData();
	void LoadMetaFromFolder(PathNode node);

	ResourceMeta	GetMetaInfo(const Resource* resource);
	ResourceMeta*	FindResourceInLibrary(const char* original_file, const char* name, Resource::Type type);

	//.meta file generation
	void SaveMetaInfo(const Resource* resource);

	bool LoadMetaInfo(const char* file);
	bool LoadSceneMeta(const char* file, const char* source_file);

	//Search through all assets and imports / re-imports
	void UpdateAssetsImport();
	void UpdateAssetsFolder(const PathNode& node);

	//Remove all .meta files in assets TODO: fix fileSystem removing error
	void ClearMetaData();
	//Remove all .meta files in a folder
	void RemoveMetaFromFolder(PathNode node);

	bool IsFileModified(const char* path);

	void SaveChangedResources();

	//Adds a new resource (importion previous to this)
	void AddResource(Resource* resource);
	//Loads an existing resource. Loading is previous to this, this is just for data management
	void LoadResource(Resource* resource);
	//Completely deletes a resource, including its file (not yet though)
	//Return number of instances previous to deletion
	uint DeleteResource(uint64 ID);
	//Removes a resource from memory
	void UnLoadResource(uint64 ID);

public:
	//Just for quick info display
	std::map<uint64, Resource*> meshes;
	std::map<uint64, Resource*> materials;
	std::map<uint64, Resource*> textures;
	std::map<uint64, Resource*> scenes;
	std::map<uint64, Resource*> animations;
	//TODO: is this really used anywhere?

private:
	//Resources loaded in memory
	std::map<uint64, Resource*> resources;

	//All resources imported
	std::map<uint64, ResourceMeta> existingResources;

	Timer updateAssets_timer;
	Timer saveChangedResources_timer;
	LCG random;
};

#endif
