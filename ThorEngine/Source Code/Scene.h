#ifndef __SCENE_H__
#define __SCENE_H__

#include "Globals.h"
#include <string>
#include <vector>

#include "MathGeoLib/src/Algorithm/Random/LCG.h"
#include "MathGeoLib/src/Geometry/LineSegment.h" //TODO: Fwd declaration

class Quadtree;
class C_Camera;
class GameObject;
struct ResourceBase;
class M_SceneManager;

class Scene
{
	friend class M_SceneManager;

public:
	Scene();
	~Scene();

	void InitFromResourceData(const ResourceBase* data);
	
	void AddGameObject(GameObject* newGameObject, GameObject* parent = nullptr);
	GameObject* CreateNewGameObject(const char* name, GameObject* parent = nullptr);

	std::string GetNewGameObjectName(const char* name, const GameObject* parent = nullptr);
	int GetGameObjectNameCount(const char* name, const GameObject* parent = nullptr);

	void OnDestroyGameObject(GameObject* gameObject);
	void OnGameObjectStaticChanged(GameObject* gameObject);

	void PerformMousePick(const LineSegment& segment);

	GameObject* CreateCamera(); //TODO: Keeping old structure for now. Consider moving somewhere else
	const C_Camera* GetMainCamera() const;

	//TODO: Keeping old structure for now. Consider moving somewhere else
	//Tick handling should be done in a separate structure
	void Play();
	void Stop();

	//Deletes all GameObjects, clears quadtree and dynamic vector.
	void ClearScene();

private:
	void UpdateAllGameObjects(float dt);
	void DrawAllChildren(GameObject* gameObject);
	void DrawCulledGameObjects(C_Camera* targetCamera);

	void CollectCullingCandidates(std::vector<const GameObject*>& vector, std::vector<const GameObject*>& candidates);

	void ClearGameObjectsForRemoval();
public:
	std::string file_path;
	std::string name;
	uint64 ID;

	GameObject* root = nullptr; //TODO: Move to private. Access in M_Editor save scene
private:
	C_Camera* mainCamera = nullptr;

	M_SceneManager* managerOwner = nullptr;

	std::vector<const GameObject*> dynamicGameObjects;
	Quadtree* quadtree = nullptr;

	std::vector<GameObject*> gameObjectsPendingRemoval;

	LCG random; //TODO: Avoid including everywhere
};

#endif
