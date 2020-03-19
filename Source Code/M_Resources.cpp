#include "M_Resources.h"
#include "Application.h"

//Loaders
#include "I_Meshes.h"
#include "I_Materials.h"
#include "I_Scenes.h"
#include "I_Animations.h"
#include "I_ParticleSystems.h"
#include "I_Shaders.h"
#include "I_Folders.h"

//Resources
#include "R_Mesh.h"
#include "R_Texture.h"
#include "R_Material.h"
#include "R_Prefab.h"
#include "R_Animation.h"
#include "R_AnimatorController.h"
#include "R_ParticleSystem.h"
#include "R_Shader.h"
#include "R_Folder.h"

#include "M_FileSystem.h"
#include "PathNode.h"

#include "Config.h"
#include "M_Scene.h" //Called only in LoadPrefab method
#include "M_Editor.h" //Called only in ImportFileFromExplorer
#include "W_Explorer.h" //Called only in ImportFileFromExplorer

#include "Assimp/include/scene.h"

M_Resources::M_Resources(bool start_enabled) : Module("Resources", start_enabled)
{

}

M_Resources::~M_Resources()
{

}

bool M_Resources::Init(Config& config)
{
	ClearMetaData();
	Importer::Scenes::Init();

	return true;
}

bool M_Resources::Start()
{
	LoadResourcesData();
	UpdateAssetsImport();
	updateAssets_timer.Start();
	saveChangedResources_timer.Stop();
	return true;
}

update_status M_Resources::Update(float dt)
{
	//Little dirty trick to offset both updates
	if (saveChangedResources_timer.IsRunning() == false && updateAssets_timer.ReadSec() > 2.5)
	{
		saveChangedResources_timer.Start();
	}

	if (updateAssets_timer.ReadSec() > 5)
	{
		//UpdateAssetsImport(); //TODO: Add to update in
		updateAssets_timer.Start();
	}

	if (saveChangedResources_timer.ReadSec() > 5)
	{
		SaveChangedResources();
		saveChangedResources_timer.Start();
	}

	return UPDATE_CONTINUE;
}

bool M_Resources::CleanUp()
{
	SaveChangedResources();
	for (std::map<unsigned long long, Resource*>::iterator it = resources.begin(); it != resources.end(); )
	{
		it->second->FreeMemory();
		RELEASE(it->second);
		it = resources.erase(it);
	}

	return true;
}

void M_Resources::ImportFileFromExplorer(const char* path)
{
	std::string normalizedPath = App->fileSystem->NormalizePath(path);
	std::string finalPath;
	if (App->fileSystem->DuplicateFile(normalizedPath.c_str(), App->moduleEditor->w_explorer->GetCurrentFolder()->GetOriginalFile(), finalPath))
	{
		ImportFileFromAssets(finalPath.c_str());
	}
}

uint64 M_Resources::ImportFolderFromAssets(const char* path)
{
	Resource* resource = CreateResourceBase(path, Resource::FOLDER);
	if (ResourceInfo* meta = FindResourceInLibrary(path, resource->name.c_str(), Resource::FOLDER))
	{
		resource->ID = meta->id;
		resource->instances = DeleteResource(resource->ID);
	}

	resources[resource->ID] = resource;
	existingResources[resource->ID] = GetMetaInfo(resource);
	return resource->ID;
}

uint64 M_Resources::ImportFileFromAssets(const char* path)
{
	Resource::Type type = GetTypeFromPathExtension(path);
	Resource* resource = CreateResourceBase(path, type);
	uint64 resourceID = 0;

	//Checking for an already existing resource. In that case, re-import.
	if (ResourceInfo* meta = FindResourceInLibrary(path, resource->name.c_str(), type))
	{
		resource->ID = meta->id;
		resource->instances = DeleteResource(resource->ID);
	}	

	char* buffer = nullptr;
	if (uint size = App->fileSystem->Load(path, &buffer))
	{
		switch (type)
		{
			case (Resource::TEXTURE):				Importer::Textures::Import(buffer, size, (R_Texture*)resource); break;
			case (Resource::PREFAB):				ImportScene(buffer, size, (R_Prefab*)resource); break;
			case (Resource::SHADER):				Importer::Shaders::Import(buffer, (R_Shader*)resource); break;
			case (Resource::PARTICLESYSTEM):		Importer::Particles::Import(buffer, size, (R_ParticleSystem*)resource); break;
			case (Resource::ANIMATOR_CONTROLLER):	Importer::Animations::Load(buffer, (R_AnimatorController*)resource); break;
		}
		//TODO: Change ImportRTexture functions for a call to the importer, and add SaveResource(resource) at the end. Remove all specific import functions
		//		Prefabs work a little different
		SaveResource(resource);
		resourceID = resource->ID;
		existingResources[resourceID] = GetMetaInfo(resource);
		SaveMetaInfo(resource);

		RELEASE_ARRAY(buffer);
		RELEASE(resource);
	}

	return resourceID;
}

uint64 M_Resources::ImportResourceFromScene(const char* file, const void* data, const char* name, Resource::Type type)
{
	Resource* resource = CreateResourceBase(file, type, name);

	if (ResourceInfo* meta = FindResourceInLibrary(file, name, type))
		resource->ID = meta->id;

	switch (type)
	{
		case (Resource::MESH):		Importer::Meshes::Import((aiMesh*)data, (R_Mesh*)resource); break;
		case (Resource::MATERIAL):	Importer::Materials::Import((aiMaterial*)data, (R_Material*)resource); break;
		case (Resource::ANIMATION): Importer::Animations::Import((aiAnimation*)data, (R_Animation*)resource); break;
	}

	SaveResource(resource);

	uint64 resourceID = resource->GetID();
	existingResources[resource->GetID()] = GetMetaInfo(resource);
	RELEASE(resource);

	return resourceID;
}

void M_Resources::ImportScene(const char* buffer, uint size, R_Prefab* prefab)
{
	const aiScene* scene = Importer::Scenes::ProcessAssimpScene(buffer, size);
	prefab->root = Importer::Scenes::Import(scene, prefab->name.c_str());
	std::vector<uint64> meshes, materials, animations;

	for (uint i = 0; i < scene->mNumMeshes; ++i)
	{
		//TODO: building a temporal mesh name. Assimp meshes have no name so they have conflict when importing multiple meshes
		//Unity loads a mesh per each node, and assign the node's name to it
		std::string meshName = prefab->name;
		meshName.append("_mesh").append(std::to_string(i));
		scene->mMeshes[i]->mName = meshName;
		meshes.push_back(ImportResourceFromScene(prefab->original_file.c_str(), scene->mMeshes[i], meshName.c_str(), Resource::Type::MESH));
		prefab->containingResources.push_back(meshes.back());
	}
	for (uint i = 0; i < scene->mNumMaterials; ++i)
	{
		aiString matName;
		scene->mMaterials[i]->Get(AI_MATKEY_NAME, matName);
		materials.push_back(ImportResourceFromScene(prefab->original_file.c_str(), scene->mMaterials[i], matName.C_Str(), Resource::Type::MATERIAL));
		prefab->containingResources.push_back(materials.back());
	}

	for (uint i = 0; i < scene->mNumAnimations; ++i)
	{
		aiAnimation* anim = scene->mAnimations[i];
		animations.push_back(ImportResourceFromScene(prefab->original_file.c_str(), anim, anim->mName.C_Str(), Resource::Type::ANIMATION));
		prefab->containingResources.push_back(animations.back());
	}

	//Link all loaded meshes and materials to the existing gameObjects
	Importer::Scenes::LinkSceneResources(prefab->root, meshes, materials);
}

uint64 M_Resources::ImportPrefabImage(char* buffer, const char* source_file, uint sizeX, uint sizeY)
{
	uint64 ret = 0;
	uint64 newID = 0;
	R_Texture* resource = nullptr;
	uint64 instances = 0;

	std::string name = "", extension = "";
	ResourceInfo* meta = FindResourceInLibrary(source_file, name.c_str(), Resource::TEXTURE);

	if (meta != nullptr)
	{
		newID = meta->id;
		instances = DeleteResource(newID);
	}
	else
	{
		newID = GetNewID();
	}

	//Importing resource
	//resource = App->moduleMaterials->ImportPrefabImage(buffer, newID, source_file, sizeX, sizeY);
	if (resource)
	{
		resource->instances = instances;
		existingResources[resource->ID] = GetMetaInfo(resource);
		SaveMetaInfo(resource);

		RELEASE(resource);
	}
	return newID;
}

Resource* M_Resources::CreateResourceBase(const char* assetsPath, Resource::Type type, const char* name, uint64 forceID)
{
	Resource* resource = nullptr;
	std::string resourcePath;

	static_assert(static_cast<int>(Resource::UNKNOWN) == 10, "Code Needs Update");

	switch (type)
	{
		case Resource::FOLDER:
			resource = new R_Folder();
			resource->resource_file = FOLDERS_PATH;
			break;
		case Resource::MESH:
			resource = new R_Mesh();
			resource->resource_file = MESHES_PATH;
			break;
		case Resource::TEXTURE:
			resource = new R_Texture();
			resource->resource_file = TEXTURES_PATH;
			break;
		case Resource::MATERIAL:
			resource = new R_Material();
			resource->resource_file = MATERIALS_PATH;
			break;
		case Resource::ANIMATION:
			resource = new R_Animation();
			resource->resource_file = ANIMATIONS_PATH;
			break;
		case Resource::ANIMATOR_CONTROLLER:
			resource = new R_AnimatorController();
			resource->resource_file = ANIMATIONS_PATH;
			break;
		case Resource::PREFAB:
			resource = new R_Prefab();
			resource->resource_file = GAMEOBJECTS_PATH;
			break;
		case Resource::PARTICLESYSTEM:
			resource = new R_ParticleSystem();
			resource->resource_file = PARTICLES_PATH;
			break;
		case Resource::SHADER:
			resource = new R_Shader();
			resource->resource_file = SHADERS_PATH;
			break;
		case Resource::SCENE:
			resource = new Resource(Resource::SCENE);
	}
	
	if (resource == nullptr) return nullptr;

	if (name == nullptr)
	{
		std::string extension;
		App->fileSystem->SplitFilePath(assetsPath, nullptr, &resource->name, &extension);
		if (type == Resource::TEXTURE) resource->name.append(".").append(extension);
	}
	else
		resource->name = name;

	resource->ID = forceID ? forceID : GetNewID();
	resource->original_file = assetsPath;
	resource->resource_file.append(std::to_string(resource->ID));

	return resource;
}

uint64 M_Resources::CreateNewCopyResource(const char* directory, const char* defaultPath, Resource::Type type)
{
	uint64 resourceID = 0;

	//First we get the resource name and generate a unique one
	std::string path, name, extension;
	App->fileSystem->SplitFilePath(defaultPath, &path, &name, &extension);
	
	name = App->fileSystem->GetUniqueName(directory, name.c_str());
	std::string dstPath = std::string(directory).append(name);

	//Duplicate the file in assets, import it as usual
	std::string finalPath;
	if (App->fileSystem->DuplicateFile(defaultPath, directory, finalPath))
	{
		resourceID = ImportFileFromAssets(finalPath.c_str());
	}

	return resourceID;
}

Resource* M_Resources::LoadResourceBase(uint64 ID)
{
	Resource* resource = nullptr;
	std::map<uint64, ResourceInfo>::iterator metasIt = existingResources.find(ID);
	if (metasIt != existingResources.end())
	{
 		ResourceInfo& resInfo = metasIt->second;
		resource = CreateResourceBase(resInfo.file.c_str(), resInfo.type, resInfo.name.c_str(), ID);
	}
	return resource;
}

const ResourceInfo& M_Resources::GetResourceInfo(const char* path) const
{
	//TODO: Search process should start at Root Folder and iterate down the path
	//		It will make it much faster than comparing all strings
	std::map<uint64, ResourceInfo>::const_iterator it;
	
	for (it = existingResources.begin(); it != existingResources.end(); ++it)
	{
		if (it->second.file == path)
			return it->second;
	}

	//TODO: Can this cause errors? Is this a scoped variable that will get removed?
	return ResourceInfo();
}

Resource* M_Resources::GetResource(uint64 ID)
{
	Resource* resource = nullptr;
	//First find if the wanted resource is loaded
	std::map<uint64, Resource*>::iterator resourcesIt = resources.find(ID);
	if (resourcesIt != resources.end()) return resourcesIt->second;
	//If the resource is not loaded, search in the library and load it
	if (resource = LoadResourceBase(ID))
	{
		char* buffer = nullptr;
		uint size = App->fileSystem->Load(resource->resource_file.c_str(), &buffer);
		if (size == 0)
		{
			RELEASE(resource);
			return nullptr;
		}

		switch (resource->type)
		{
			case (Resource::FOLDER):				{ Importer::Folders::Load(buffer, (R_Folder*)resource); break; }
			case (Resource::MESH):					{ Importer::Meshes::Load(buffer, (R_Mesh*)resource); break; }
			case (Resource::TEXTURE):				{ Importer::Textures::Load(buffer, size, (R_Texture*)resource); break; }
			case (Resource::MATERIAL):				{ Importer::Materials::Load(buffer, size, (R_Material*)resource); break; }
			case (Resource::PREFAB):
			{
				char* metaBuffer = nullptr;
				std::string metaPath = resource->original_file + std::string(".meta");
				if (uint size = App->fileSystem->Load(metaPath.c_str(), &metaBuffer))
				{
					Config metaFile(metaBuffer);
					Importer::Scenes::LoadContainedResources(metaFile, (R_Prefab*)resource);
					RELEASE_ARRAY(metaBuffer);
				}
				break;
			}
			case (Resource::ANIMATION):				{ Importer::Animations::Load(buffer, (R_Animation*)resource); break; }
			case (Resource::ANIMATOR_CONTROLLER):	{ Importer::Animations::Load(buffer, (R_AnimatorController*)resource); break; }
			case (Resource::PARTICLESYSTEM):		{ Importer::Particles::Load(buffer, size, (R_ParticleSystem*)resource); break; }
			case (Resource::SHADER):				{ Importer::Shaders::Load(buffer, size, (R_Shader*)resource); break; }
		}
		RELEASE_ARRAY(buffer);

		resources[resource->ID] = resource;
		resource->LoadOnMemory();
	}
	return resource;
}

Resource::Type M_Resources::GetTypeFromPathExtension(const char* path) const
{
	std::string ext = "";
	App->fileSystem->SplitFilePath(path, nullptr, nullptr, &ext);

	static_assert(static_cast<int>(Resource::UNKNOWN) == 10, "Code Needs Update");

	if (ext == "FBX" || ext == "fbx")
		return Resource::PREFAB;
	if (ext == "tga" || ext == "png" || ext == "jpg" || ext == "TGA" || ext == "PNG" || ext == "JPG")
		return Resource::TEXTURE;
	if (ext == "shader")
		return Resource::SHADER;
	if (ext == "particles")
		return Resource::PARTICLESYSTEM;
	if (ext == "shader")
		return Resource::SHADER;
	if (ext == "scene")
		return Resource::SCENE;
	if (ext == "anim")
		return Resource::ANIMATION;
	if (ext == "animator")
		return Resource::ANIMATOR_CONTROLLER;
	return Resource::UNKNOWN;
}

bool M_Resources::GetAllMetaFromType(Resource::Type type, std::vector<const ResourceInfo*>& metas) const
{
	std::map<uint64, ResourceInfo>::const_iterator it;
	for (it = existingResources.begin(); it != existingResources.end(); ++it)
	{
		if ((*it).second.type == type)
			metas.push_back(&(*it).second);
	}
	return metas.size() > 0;
}

void M_Resources::LoadPrefab(uint64 ID)
{
	if (Resource* resource = GetResource(ID))
	{
		char* buffer = nullptr;
		uint size = App->fileSystem->Load(resource->resource_file.c_str(), &buffer);
		if (size > 0)
		{
			Config file(buffer);
			App->scene->LoadGameObject(file);
		}

		resource->instances++;
	}
}

void M_Resources::LoadResourcesData()
{
	std::vector<std::string> filter_ext;
	filter_ext.push_back("meta");

	PathNode engineAssets = App->fileSystem->GetAllFiles("Engine/Assets", &filter_ext);
	LoadMetaFromFolder(engineAssets);

	PathNode assets = App->fileSystem->GetAllFiles("Assets", &filter_ext);
	LoadMetaFromFolder(assets);
}

void M_Resources::LoadMetaFromFolder(PathNode node)
{
	if (node.isFile)
	{
		LoadResourceInfo(node.path.c_str());
	}

	//If node folder has something inside
	else if (!node.isLeaf)
	{
		for (uint i = 0; i < node.children.size(); i++)
		{
			LoadMetaFromFolder(node.children[i]);
		}
	}
}

ResourceInfo* M_Resources::FindResourceInLibrary(const char* original_file, const char* name, Resource::Type type)
{
	for (std::map<uint64, ResourceInfo>::iterator it = existingResources.begin(); it != existingResources.end(); it++)
	{
		if (it->second.Compare(original_file, name, type))
		{
			return &(it->second);
		}
	}
	return nullptr;
}

ResourceInfo M_Resources::GetMetaInfo(const Resource* resource) const
{
	return ResourceInfo(resource->GetType(), resource->original_file.c_str(), resource->name.c_str(), resource->GetID());
}

void M_Resources::LoadResourceInfo(const char* path)
{
	char* buffer = nullptr;
	std::string metaPath = std::string(path).append(".meta");
	uint size = App->fileSystem->Load(metaPath.c_str(), &buffer);

	if (size > 0)
	{
		Config metaFile(buffer);
		RELEASE_ARRAY(buffer);

		ResourceInfo resourceInfo;
		resourceInfo.Assign((Resource::Type)(int)metaFile.GetNumber("Type"), path, metaFile.GetString("Name").c_str(), metaFile.GetNumber("ID"));

		//If the resource is a prefab, add all recources contained in it
		if (resourceInfo.type == Resource::PREFAB)
		{
			Config_Array containingResources = Config(buffer).GetArray("Containing Resources");
			for (uint i = 0; i < containingResources.GetSize(); ++i)
			{
				Config fileNode = containingResources.GetNode(i);
				ResourceInfo containedInfo((Resource::Type)(int)(fileNode.GetNumber("Type")), path, fileNode.GetString("Name").c_str(), fileNode.GetNumber("ID"));
				existingResources[containedInfo.id] = containedInfo;
			}
		}
	}
}

ResourceInfo M_Resources::GetMetaFromNode(const char* file, const Config& node) const
{
	return ResourceInfo(static_cast<Resource::Type>((int)(node.GetNumber("Type"))), file, node.GetString("Name").c_str(), node.GetNumber("ID"));
}

//Save .meta file in assets
void M_Resources::SaveMetaInfo(const Resource* resource)
{
	Config config;

	config.SetNumber("ID", resource->ID);
	config.SetString("Name", resource->name.c_str());
	config.SetNumber("Type", static_cast<int>(resource->GetType()));

	//Getting file modification date
	uint64 modDate = App->fileSystem->GetLastModTime(resource->original_file.c_str());
	config.SetNumber("Date", modDate);

	//TODO: Probably a bit dirty to add it here with generic meta files... check later
	if (resource->GetType() == Resource::PREFAB)
	{
		R_Prefab* prefab = (R_Prefab*)resource;
		Importer::Scenes::SaveContainedResources(prefab, config);
	}

	char* buffer = nullptr;
	uint size = config.Serialize(&buffer);
	if (size > 0)
	{
		std::string path = resource->original_file + ".meta";
		App->fileSystem->Save(path.c_str(), buffer, size);
		RELEASE_ARRAY(buffer);
	}
}

void M_Resources::UpdateAssetsImport()
{
	//Getting all files in assets
	std::vector<std::string> ignore_extensions;
	ignore_extensions.push_back("meta");

	PathNode engineAssets = App->fileSystem->GetAllFiles("Engine/Assets", nullptr, &ignore_extensions);
	UpdateAssetsFolder(engineAssets);

	PathNode assets = App->fileSystem->GetAllFiles("Assets", nullptr, &ignore_extensions);
	UpdateAssetsFolder(assets);
}

uint64 M_Resources::UpdateAssetsFolder(const PathNode& node, bool ignoreResource)
{
	//If node is a file
	uint64 resourceID = 0;
	if (node.isFile)
	{
		if (!App->fileSystem->Exists(std::string(node.path + ".meta").c_str()))
		{
			return ImportFileFromAssets(node.path.c_str());
		}
		else
		{
			if (IsFileModified(node.path.c_str()))
			{
				//TODO: Force ID when re-importing a modified asset
				LOG("File modified: %s", node.path.c_str());
				//uint64 id = GetResourceInfo(node.path.c_str()).id;
				resourceID = ImportFileFromAssets(node.path.c_str());// , id);
			}
		} 
	}

	//At this point only folders are processed
	R_Folder* folder = nullptr;
	bool isFolderUpdated = false;

	if (true) //TODO: ignoreResource
	{
		if (!App->fileSystem->Exists(std::string(node.path + ".meta").c_str()))
		{
			resourceID = ImportFolderFromAssets(node.path.c_str());
			folder = (R_Folder*)GetResource(resourceID);
			isFolderUpdated = true;
		}
		else
		{
			resourceID = GetResourceInfo(node.path.c_str()).id;
			folder = (R_Folder*)GetResource(resourceID);
		}
	}

	for (uint i = 0; i < node.children.size(); i++)
	{
		uint64 childrenID = UpdateAssetsFolder(node.children[i]);
		if (childrenID != 0) //TODO: ignoreResource
		{
			folder->AddResource(childrenID);
			isFolderUpdated = true;
		}
	}

	if (isFolderUpdated)
	{
		SaveResource(folder);
		SaveMetaInfo(folder);
	}

	return resourceID;
}

void M_Resources::ClearMetaData()
{
	//Getting all .meta in assets
	std::vector<std::string> filter_extensions;
	filter_extensions.push_back("meta");

	PathNode engineAssets = App->fileSystem->GetAllFiles("Engine", &filter_extensions, nullptr);
	RemoveMetaFromFolder(engineAssets);
	
	PathNode projectFolder = App->fileSystem->GetAllFiles("Assets", &filter_extensions, nullptr);
	RemoveMetaFromFolder(projectFolder);

	App->fileSystem->Remove("Assets.meta");
	App->fileSystem->Remove("Library/");
	App->fileSystem->CreateLibraryDirectories();
}

void M_Resources::RemoveMetaFromFolder(PathNode node)
{
	if (node.isFile == true)
	{
		App->fileSystem->Remove(node.path.c_str());
	}

	//If node folder has something inside
	else if (node.isLeaf == false)
	{
		for (uint i = 0; i < node.children.size(); i++)
		{
			RemoveMetaFromFolder(node.children[i]);
		}
	}
}

bool M_Resources::IsFileModified(const char* path)
{
	std::string meta_file = path;
	meta_file.append(".meta");

	char* buffer = nullptr;
	uint size = App->fileSystem->Load(meta_file.c_str(), &buffer);

	if (size > 0)
	{
		uint64 fileMod = App->fileSystem->GetLastModTime(path);
		uint64 configDate = Config(buffer).GetNumber("Date");
		return fileMod != configDate;
	}
	return false;
}

void M_Resources::SaveChangedResources()
{
	for (std::map<uint64, Resource*>::iterator it = resources.begin(); it != resources.end(); it++)
	{
		if (it->second->needs_save == true)
		{
			switch (it->second->type)
			{
				//By now the only resource that can be modified is material
				case(Resource::MATERIAL):
				{
					char* buffer = nullptr;
					if (uint size = Importer::Materials::Save((R_Material*)it->second, &buffer))
					{
						App->fileSystem->Save(it->second->resource_file.c_str(), buffer, size);
						RELEASE_ARRAY(buffer);
					}
				}
			}
			it->second->needs_save = false;
		}
	}
}

uint M_Resources::DeleteResource(uint64 ID)
{
	//TODO: it would be simpler if resource meta stored the path
	//TODO: update folder resource and remove this one from the contained list
	Resource::Type type = existingResources[ID].type;
	uint instances = 0;

	if (resources[ID] != nullptr)
	{
		//Could break here
		instances = resources[ID]->instances;
		UnLoadResource(ID);
		resources.erase(ID);
	}
	
	std::string resourcePath = "";
	switch (type)
	{
		case(Resource::MESH):
		{
			resourcePath.append("/Library/Meshes/");
			break;
		}
		case(Resource::MATERIAL):
		{
			resourcePath.append("/Library/Materials/");
			break;
		}
		case(Resource::TEXTURE):
		{
			resourcePath.append("/Library/Textures/");
			break;
		}
		case(Resource::PREFAB):
		{
			resourcePath.append("/Library/GameObjects/");
			break;
		}
	}
	resourcePath.append(std::to_string(ID));
	App->fileSystem->Remove(resourcePath.c_str());

	existingResources.erase(ID);
	return instances;
}

void M_Resources::UnLoadResource(uint64 ID)
{
	std::map<uint64, Resource*>::iterator it = resources.find(ID);
	if (it != resources.end())
	{
		//TODO: at this moment no resource is freeing any memory
		it->second->FreeMemory();

		Resource* resource = it->second;
		resources.erase(it);
		RELEASE(resource);
	}
}

void M_Resources::SaveResource(Resource* resource)
{
	char* buffer = nullptr;
	uint size = 0;

	switch (resource->GetType())
	{
		case(Resource::FOLDER):					{ size = Importer::Folders::Save((R_Folder*)resource, &buffer); break; }
		case(Resource::MESH):					{ size = Importer::Meshes::Save((R_Mesh*)resource, &buffer); break; }
		case(Resource::TEXTURE):				{ size = Importer::Textures::Save((R_Texture*)resource, &buffer); break; }
		case(Resource::MATERIAL):				{ size = Importer::Materials::Save((R_Material*)resource, &buffer); break; }
		case(Resource::ANIMATION):				{ size = Importer::Animations::Save((R_Animation*)resource, &buffer); break;	}
		case(Resource::ANIMATOR_CONTROLLER):	{ size = Importer::Animations::Save((R_AnimatorController*)resource, &buffer);	break; }
		case(Resource::PREFAB):					{ size = Importer::Scenes::SaveScene((R_Prefab*)resource, &buffer); break; }
		case(Resource::PARTICLESYSTEM):			{ size = Importer::Particles::Save((R_ParticleSystem*)resource, &buffer); break; }
		case(Resource::SHADER):					{ size = Importer::Shaders::Save((R_Shader*)resource, &buffer); break; }
		case(Resource::SCENE):					{ /*size = Importer::Scenes::SaveScene((R_Prefab*)resource, &buffer);*/ break; }
	}

	if (size > 0)
	{
		App->fileSystem->Save(resource->resource_file.c_str(), buffer, size);

		if (IsModifiableResource(resource))
		{
			App->fileSystem->Save(resource->original_file.c_str(), buffer, size);
		}
		RELEASE_ARRAY(buffer);
	}
}

bool M_Resources::IsModifiableResource(const Resource* resource) const
{
	return resource->isInternal == false;
}