#ifndef __PANEL_H__
#define __PANEL_H__

#include "ImGui\imgui.h"
#include "Color.h"
#include "Globals.h"
#include <vector>
#include <string>


class Panel
{
public:
	Panel();
	Panel(int, int, int, int);
	virtual ~Panel();

	virtual void Draw(ImGuiWindowFlags flags);

	void SetSize(int, int);
	void SetPosition(int, int);

	virtual void UpdateSize(int, int);
	virtual void UpdatePosition(int, int);

protected:
	ImVec2 position;
	ImVec2 size;

public:
	bool active = true;
};

class SwapButtons
{
public:
	uint active_button = 0;
	ImVec4 active_color[3];
	ImVec4 nonActive_color[3];

	std::vector<std::string> buttons;

public:
	SwapButtons();
	void AddButton(const char* text);
	void Draw();
	const std::string& GetActiveTag() const;

private:
	void PushColors(bool active_button);
	void PopColors();
};
#endif //__PANEL_H__
