#ifndef __W_EXPLORER_H__
#define __W_EXPLORER_H__

#include "Window.h"

#include "PathNode.h"
#include "Vec2.h"
#include "Timer.h"

#include <map>

class Resource;

class R_Prefab;
class R_Folder;

struct ImVec2;
struct ImGuiWindowClass;

class W_Explorer : public Window
{
public:
	W_Explorer(M_Editor* editor, ImGuiWindowClass* windowClass, ImGuiWindowClass* explorerWindowClass, int ID);
	~W_Explorer() {}

	void Draw() override;
	void OnResize(Vec2 newSize) override;

	const Resource* GetCurrentFolder() const { return (Resource*)currentFolder; };
	static inline const char* GetName() { return "Explorer"; };

private:
	void DrawFolderNode(PathNode& pathNode);

	void DrawSelectedFolderContent();
	void DrawResourceItem(Resource* resource, uint& itemIndex, ImVec2 windowCursorPos);
	void DrawResourceImage(const Resource* resource);

	void UpdateTree();
	std::string GetTextAdjusted(const char* text) const;

	void HandleResourceDoubleClick(Resource* resource);
	uint GetTextureFromResource(const Resource* resource, std::string* dnd_event = nullptr);

public:
	bool explorerActive = true;
	std::map<uint, uint> resourceIcons;
	uint selectedResourceImage = 0;
	
	uint imageSize = 64;
	uint columnsNumber = 0;
	uint imageSpacingX = 50;
	uint imageSpacingY = 50;
	uint textOffset = 6;
	uint topMarginOffset = 10;
	uint nodeButtonOffset = 30;

private:
	ImGuiWindowClass* explorerWindowClass = nullptr;

	PathNode assets;

	R_Folder* assetsFolder = nullptr;
	R_Folder* engineAssetsFolder = nullptr;

	R_Folder* currentFolder = nullptr;
	R_Folder* nextCurrentFolder = nullptr;
	Resource* selectedResource = nullptr;

	Vec2 explorerPosition;
	Vec2 explorerSize;

	uint updateTime = 5;
	Timer updateTimer;

	Vec2 windowSize;

	R_Prefab*  openModel = nullptr;
};

#endif