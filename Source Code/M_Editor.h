#ifndef __M_EDITOR_H__
#define __M_EDITOR_H__

#include "Module.h"

#include <string>
#include <vector>

class Config;
class GameObject;
class Resource;
class TreeNode;

class WindowFrame;
class Window;

union SDL_Event;
struct ImGuiWindowClass;

class M_Editor : public Module
{
public:
	M_Editor(bool start_enabled = true);
	~M_Editor();

	bool Init(Config& config) override;
	bool Start() override;
	update_status PreUpdate() override;
	bool CleanUp() override;

	void CreateWindows();
	void LoadLayout(WindowFrame* windowFrame, const char* layout = "Default");
	void SaveLayout(WindowFrame* windowFrame, const char* layout = "Default");

	void Draw();

	void Log(const char* input);
	void GetEvent(SDL_Event* event);
	void UpdateFPSData(int fps, int ms);

	void OnResize(int screen_width, int screen_height);

	bool UsingKeyboard() const;
	bool UsingMouse() const;

	void OpenResource(Resource* resource);

	//Timer management -------------------
	uint AddTimer(const char* text, const char* tag);
	void StartTimer(uint index);
	void ReadTimer(uint index);
	void StopTimer(uint index);
	//------------------------------------

	//Selection --------------------
	void SelectSingle(TreeNode* node, bool openTree = true);
	void AddSelect(TreeNode* node, bool openTree = true);
	void AddToSelect(TreeNode* node);
	void UnselectSingle(TreeNode* node);
	void UnselectAll();
	void UnselectGameObjects();
	void UnselectResources();
	void DeleteSelected();

	void FinishDrag(bool drag, bool SelectDrag);
	//Endof Selection------------------------------------

	void SaveConfig(Config& config) const override;
	void LoadConfig(Config& config) override;

	void LoadScene(Config& root, bool tmp = false) override;
	void ResetScene();

	void OnRemoveGameObject(GameObject* gameObject) override;

	void OpenFileNameWindow();
	void ShowFileNameWindow();

	WindowFrame* GetWindowFrame(const char* name);
public:

	std::vector<TreeNode*> selectedGameObjects;
	std::vector<TreeNode*> toSelectGOs;
	std::vector<TreeNode*> toDragGOs;
	std::vector<TreeNode*> toUnselectGOs;

	std::vector<Resource*> selectedResources;

	TreeNode* lastSelected = nullptr;
	bool dragging = false;

	bool showDebugInfo = false;

	bool show_Demo_window = false;
	bool show_fileName_window = false;
	
private:
	//All currently active window frames
	std::vector<WindowFrame*> windowFrames;

	ImGuiWindowClass* frameWindowClass = nullptr;
	ImGuiWindowClass* normalWindowClass = nullptr;

	char fileName[50];

	bool using_keyboard;
	bool using_mouse;

	float mainWindowPositionY = 0;
	float toolbarSizeY = 30;
};

#endif //!__M_EDITOR_H__
