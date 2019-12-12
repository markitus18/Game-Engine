#ifndef __RESOURCE_H__
#define __RESOURCE_H__

#include <string>
#include "Globals.h"
#include "TreeNode.h"

class Resource : public TreeNode
{

//Huh... can it be done differently?
friend class M_Resources;
friend class M_Meshes;
friend class M_Materials;
friend struct ResourceMeta;
friend class M_Import;
friend class M_Animations;
friend class M_ParticleSystems;

public:
	enum Type
	{
		FOLDER,
		MESH,
		TEXTURE,
		MATERIAL,
		ANIMATION,
		BONE,
		PREFAB,
		PARTICLESYSTEM,
		SHADER,
		UNKNOWN,
	};

	Resource(Type type);
	~Resource();

	Type GetType() const;
	unsigned long long GetID() const;
	TreeNode* GetParentNode() const;

	const char* GetOriginalFile() const;
	const char* GetResourceFile() const;
	const char* GetName() const;
	virtual std::vector<TreeNode*> GetChilds() const { std::vector<TreeNode*> ret; return ret; };
	bool IsNodeActive() const { return true; };
	bool DrawTreeNode() const;

	virtual void LoadOnMemory() {};
	virtual void FreeMemory() {};

public:
	uint instances = 0;
	bool needs_save = false;

protected:
	unsigned long long ID = 0;
	Type type = UNKNOWN;

	std::string resource_file = "";
	std::string original_file = "";
	std::string name = "";

	Resource* parent;
};

#endif // !__RESOURCE_H__
