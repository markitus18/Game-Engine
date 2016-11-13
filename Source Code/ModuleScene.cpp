#include "Globals.h"
#include "Application.h"
#include "ModuleScene.h"
#include "Primitive.h"
#include "ModuleCamera3D.h"
#include "ModuleInput.h"
#include "ModuleImport.h"
#include "ModuleEditor.h"
#include "ModuleRenderer3D.h"
#include "ModuleFileSystem.h"

#include "GameObject.h"
#include <gl/GL.h>
#include <gl/GLU.h>
//#include <stdio.h>
#include "C_Camera.h"
#include "Intersections.h"
#include "Config.h"
#include "ModuleMeshes.h"
#include "ModuleMaterials.h"

#include "Quadtree.h"

#include <windows.h>
#include <shobjidl.h> 

//#include <GLFW/glfw3.h>

ModuleScene::ModuleScene(Application* app, bool start_enabled) : Module("Scene", start_enabled)
{
	root = new GameObject(nullptr, "root");
	root->uid = 0;

	//TODO: This should nto be here
	//TMP camera for testing purposes
	//camera = new GameObject(root, "Camera");
	//camera->GetComponent<C_Transform>()->SetPosition(float3(10, 10, 0));
	//camera->CreateComponent(Component::Type::Camera);
	//camera->GetComponent<C_Camera>()->Look(float3(0, 5, 0));
	//camera->uid = random.Int();
}

ModuleScene::~ModuleScene()
{
}

bool ModuleScene::Init(Config& config)
{
	std::string newScene = config.GetString("Current Scene");

	if (newScene == "" || !App->fileSystem->Exists(newScene.c_str()))
	{
		CreateDefaultScene();
	}
	else
	{
		current_scene = newScene;
		App->LoadScene(current_scene.c_str());
	}

	quadtree = new Quadtree(AABB(vec(-100, -50, -100), vec(100, 50, 100)));
	for (uint i = 0; i < 4; i++)
		quadtree->AddGameObject(root);
	return true;
}

// Load assets
bool ModuleScene::Start()
{
	LOG("Loading Intro assets");
	bool ret = true;

	cullingTimer_library = App->moduleEditor->AddTimer("Math library culling", "Culling");
	cullingTimer_normal = App->moduleEditor->AddTimer("Normal culling", "Culling");
	cullingTimer_optimized = App->moduleEditor->AddTimer("Optimized culling", "Culling");

	return ret;
}

// Load assets
bool ModuleScene::CleanUp()
{
	LOG("Unloading scene");

	RELEASE(root);

	return true;
}

// Update
update_status ModuleScene::Update(float dt)
{
	if (App->input->GetKey(SDL_SCANCODE_Z) == KEY_DOWN)
	{
		//Import "external" file into project: creates all own-format files (and then loads them by now)
		std::string file = "Models/Street_environment_V01.FBX";
		App->moduleImport->ImportFile("Models/Street_environment_V01.FBX");

		std::string fileName;
		App->fileSystem->SplitFilePath(file.c_str(), nullptr, &fileName);

		GameObject* gameObject = App->moduleImport->LoadGameObject(fileName.c_str());
		gameObject->parent = root;
		root->childs.push_back(gameObject);
	}

#pragma region WindowTest
	if (App->input->GetKey(SDL_SCANCODE_K) == KEY_DOWN)
	{

		HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED |
			COINIT_DISABLE_OLE1DDE);
		if (SUCCEEDED(hr))
		{
			IFileOpenDialog *pFileOpen;
			// Create the FileOpenDialog object.
			hr = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_ALL,
				IID_IFileOpenDialog, reinterpret_cast<void**>(&pFileOpen));

			if (SUCCEEDED(hr))
			{
				// Show the Open dialog box.
				hr = pFileOpen->Show(NULL);
			}
		}
	}
#pragma endregion

	if (drawGrid)
	{
		//TODO: Move this into a mesh "prefab" or a renderer method
		glLineWidth(1.0f);
		
		glBegin(GL_LINES);
		
		float d = 20.0f;
		
		for(float i = -d; i <= d; i += 1.0f)
		{
			glVertex3f(i, 0.0f, -d);
			glVertex3f(i, 0.0f, d);
			glVertex3f(-d, 0.0f, i);
			glVertex3f(d, 0.0f, i);
		}
		
		glEnd();
	}

	if (App->input->GetKey(SDL_SCANCODE_G) == KEY_DOWN)
	{
		drawGrid = !drawGrid;
	}

	root->Update();

	if (App->renderer3D->culling_camera)
	{
		std::vector<GameObject*> gameObjects;

		//Temporal culling testing, just for performance calculation ------
		App->moduleEditor->StartTimer(cullingTimer_library);
		TestGameObjectsCulling(gameObjects, root, true);
		App->moduleEditor->ReadTimer(cullingTimer_library);
		gameObjects.clear();

		App->moduleEditor->StartTimer(cullingTimer_normal);
		TestGameObjectsCulling(gameObjects, root, false, false);
		App->moduleEditor->ReadTimer(cullingTimer_normal);
		gameObjects.clear();
		//----------------------------------------------------------------

		App->moduleEditor->StartTimer(cullingTimer_optimized);
		TestGameObjectsCulling(gameObjects, root, false, true);
		App->moduleEditor->ReadTimer(cullingTimer_optimized);

		for (uint i = 0; i < gameObjects.size(); i++)
		{
			if (gameObjects[i]->name != "root");
				gameObjects[i]->Draw(App->moduleEditor->shaded, App->moduleEditor->wireframe);
		}
		gameObjects.clear();
		camera->Draw(App->moduleEditor->shaded, App->moduleEditor->wireframe);
	}
	else
	{
		DrawAllGameObjects(root);
	}

	quadtree->Draw();

	return UPDATE_CONTINUE;
}

update_status ModuleScene::PostUpdate(float dt)
{
	return UPDATE_CONTINUE;
}

GameObject* ModuleScene::GetRoot()
{
	return root;
}

const GameObject* ModuleScene::GetRoot() const
{
	return root;
}

void ModuleScene::SaveConfig(Config& config) const
{
	config.SetString("Current Scene", current_scene.c_str());
}

void ModuleScene::LoadConfig(Config& config)
{
	std::string newScene = config.GetString("Current Scene");

	if (newScene != "" && newScene != current_scene)
	{
		current_scene = newScene;
		App->LoadScene(current_scene.c_str());
	}
}

void ModuleScene::SaveScene(Config& node) const
{
	//Store all gameObjects in a vector
	std::vector<GameObject*> gameObjects;
	GettAllGameObjects(gameObjects, root);

	App->moduleImport->SaveGameObjectConfig(node, gameObjects);
}

void ModuleScene::LoadScene(Config& node)
{
	DeleteAllGameObjects();

	std::vector<GameObject*> not_parented_GameObjects;

	Config_Array gameObjects_array = node.GetArray("GameObjects");
	for (uint i = 0; i < gameObjects_array.GetSize(); i++)
	{
		//Single GameObject load
		Config gameObject_node = gameObjects_array.GetNode(i);

		float3 position = gameObject_node.GetArray("Translation").GetFloat3(0);
		Quat rotation = gameObject_node.GetArray("Rotation").GetQuat(0);
		float3 scale = gameObject_node.GetArray("Scale").GetFloat3(0);

		//Parent setup
		GameObject* parent = nullptr;
		FindGameObjectByID(gameObject_node.GetNumber("ParentUID"), root, &parent);

		GameObject* gameObject = new GameObject(parent, gameObject_node.GetString("Name").c_str(), position, scale, rotation);
		gameObject->uid = gameObject_node.GetNumber("UID");

		if (parent == nullptr)
			not_parented_GameObjects.push_back(gameObject);

		//Mesh load
		std::string meshPath = gameObject_node.GetString("Mesh");

		if (meshPath != "")
		{
			C_Mesh* mesh = App->moduleMeshes->LoadMesh(meshPath.c_str());
			if (mesh != nullptr)
			{
				gameObject->AddComponent(mesh);
			}
		}	

		//Material load
		std::string matPath = gameObject_node.GetString("Material");

		if (matPath != "")
		{
			C_Material* mat = App->moduleMaterials->LoadMaterial(matPath.c_str());
			if (mat != nullptr)
			{
				gameObject->AddComponent(mat);
			}
		}
	}

	//Security method if any game object is left without a parent
	for (uint i = 0; i < not_parented_GameObjects.size(); i++)
	{
		not_parented_GameObjects[i]->parent = root;
	}
}

void ModuleScene::LoadScene(const char* file)
{
	current_scene = file;
	App->LoadScene(file);
}

GameObject* ModuleScene::CreateGameObject(const char* name)
{
	return new GameObject(root, name);
}

void ModuleScene::DeleteToRemoveGameObjects()
{

}

bool ModuleScene::DeleteGameObject(GameObject* gameObject)
{
	if (gameObject->toRemove == true)
	{
		RELEASE(gameObject);
		return true;
	}
	else
	{
		for (std::vector<GameObject*>::iterator it = gameObject->childs.begin(); it != gameObject->childs.end(); it++)
		{
			if (DeleteGameObject(*it))
			{
				gameObject->childs.erase(it);
			}
		}
	}
}

void ModuleScene::TestGameObjectsCulling(std::vector<GameObject*>& vector, GameObject* gameObject, bool lib, bool optimized)
{
	//Intersection method according to "lib" and "optimized" parameters
	if (lib ? Intersects(camera->GetComponent<C_Camera>()->frustum, gameObject->GetAABB()) : Intersects(camera->GetComponent<C_Camera>()->planes, gameObject->GetAABB(), optimized))
		vector.push_back(gameObject);

	for (uint i = 0; i < gameObject->childs.size(); i++)
	{
		TestGameObjectsCulling(vector, gameObject->childs[i], lib, optimized);
	}
}

void ModuleScene::DrawAllGameObjects(GameObject* gameObject)
{
	if (gameObject->name != "root");
		gameObject->Draw(App->moduleEditor->shaded, App->moduleEditor->wireframe);

	for (uint i = 0; i < gameObject->childs.size(); i++)
	{
		DrawAllGameObjects(gameObject->childs[i]);
	}
}

void ModuleScene::GettAllGameObjects(std::vector<GameObject*>& vector, GameObject* gameObject) const
{
	if (gameObject->name != "root")
		vector.push_back(gameObject);
	for (uint i = 0; i < gameObject->childs.size(); i++)
	{
		GettAllGameObjects(vector, gameObject->childs[i]);
	}
}

void ModuleScene::FindGameObjectByID(uint id, GameObject* gameObject, GameObject** ret)
{
	if (gameObject->uid == id)
	{
		if (*ret != nullptr)
			LOG("[error] Conflict: GameObjects with same UID");
		*ret = gameObject;
	}

	//Optimization vs security: if ret != nullptr we can stop searching
	//If we search for all game objects, we can find uid conflicts if any
	for (uint i = 0; i < gameObject->childs.size(); i++)
	{
		FindGameObjectByID(id, gameObject->childs[i], ret);
	}

}

void ModuleScene::DeleteAllGameObjects()
{
	for (uint i = 0; i < root->childs.size(); i++)
	{
		RELEASE(root->childs[i]);
	}
	root->childs.clear();
}

void ModuleScene::CreateDefaultScene()
{
	App->LoadScene("ProjectSettings/Untitled.scene");
}