#include "Application.h"
#include "ModuleUI.h"
#include "ModuleWindow.h"
#include "UI_Console.h"

#include "ImGui\imgui.h"
#include "ImGui\imgui_impl_sdl_gl3.h"

ModuleUI::ModuleUI(Application* app, bool start_enabled) : Module(app, start_enabled)
{

}

ModuleUI::~ModuleUI()
{}

bool ModuleUI::Init()
{
	ImGui_ImplSdlGL3_Init(App->window->window);

	ImGuiIO& io = ImGui::GetIO();
	io.IniFilename = "imgui.ini";

	buf = new char[20];
	buf = "hey";

	console = new UI_Console();

	test_color = ImColor(114, 144, 154);
	return true;
}

update_status ModuleUI::Update(float dt)
{
	ImGui_ImplSdlGL3_NewFrame(App->window->window);
	ImGuiIO& io = ImGui::GetIO();

	ImGui::Text("Hello, world!");

	//Showing all windows ----------
	if (show_About_window)
		ShowAboutWindow();
	if (show_Demo_window)
		ImGui::ShowTestWindow();
	if (show_Test_window)
		ShowTestWindow();
	if (show_Settings_window)
		ShowSettingsWindow();

	// -----------------------------
	if (ImGui::BeginMainMenuBar())
	{
		//ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImColor::HSV(1.0f, 0.8f, 0.8f));
		if (ImGui::BeginMenu("File"))
		{
			//ImGui::PopStyleColor();
			if (ImGui::MenuItem("Exit          ", "Esc"))
			{
				ImGui::EndMenu();
				return UPDATE_STOP;	
			}
			ImGui::EndMenu();

		}
		//ImGui::PopStyleColor();
		if (ImGui::BeginMenu("Window"))
		{
			if (ImGui::MenuItem("Console          ", "1", &console->active))
			{
			}
			if (ImGui::MenuItem("Settings         ", "2", &show_Settings_window))
			{
			}
			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Help"))
		{
			ImGui::MenuItem("About Thor Engine   ", NULL, &show_About_window);
			ImGui::Separator();
			if (ImGui::MenuItem("Documentation       "))
			{
				App->RequestBrowser("https://github.com/markitus18/Game-Engine/wiki");
			}
			if (ImGui::MenuItem("Download latest     "))
			{
				App->RequestBrowser("https://github.com/markitus18/Game-Engine/releases");
			}
			if (ImGui::MenuItem("Report a bug        "))
			{
				App->RequestBrowser("https://github.com/markitus18/Game-Engine/issues");
			}

			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Debugging Tools"))
		{
			ImGui::MenuItem("ImGui Demo", NULL, &show_Demo_window);
			ImGui::EndMenu();
		}

		ImGui::EndMainMenuBar();
	}

	if (ImGui::Button("Quit", ImVec2(100, 50)))
	{
		return UPDATE_STOP;
	}

	char* button_label = "Open Window";
	if (show_Test_window)
	{
		button_label = "Close Window";
	}

	if (ImGui::Button(button_label, ImVec2(100, 50)))
	{
		show_Test_window ^= 1;
	}

	if (console->active)
	{
		console->Draw("Console", &console->active);
	}

	//Change background color, we use "test_color", controllable variable from UI
	glClearColor(test_color.x, test_color.y, test_color.z, test_color.w);

	ImGui::Render();
	return UPDATE_CONTINUE;
}

bool ModuleUI::CleanUp()
{
	if (console)
	{
		delete console;
		console = NULL;
	}

	ImGui_ImplSdlGL3_Shutdown();
	return true;
}

void ModuleUI::Log(const char* input)
{
	if (console != NULL)
		console->AddLog(input);
}

void ModuleUI::GetEvent(SDL_Event* event)
{
	ImGui_ImplSdlGL3_ProcessEvent(event);
}

void ModuleUI::ShowAboutWindow()
{
	ImGui::Begin("About Thor Engine", &show_About_window, ImVec2(400, 100), 1.0f);
	ImGui::Text("v0.1-alpha");
	ImGui::Separator();
	ImGui::Text("By Marc Garrigo for educational purposes.");
	ImGui::Text("Thor Engine is licensed under Public Domain, see LICENSE for more information.");
	ImGui::End();	
}

void ModuleUI::ShowTestWindow()
{
	//Setting up window flags
	ImGuiWindowFlags window_flags = 0;
	//Allowing menu bar on the window
	window_flags |= ImGuiWindowFlags_MenuBar;
	ImGui::Begin("Testing windows", &show_Test_window, ImVec2(500, 300), 1.0f, window_flags);
	ImGui::Text("Some random text in here!");

	//Menu Bar
	if (ImGui::BeginMenuBar())
	{
		//Menu Bar -- Menu Button
		if (ImGui::BeginMenu("Menu"))
		{
			ImGui::MenuItem("Disabled button", NULL, false, false);
			ImGui::MenuItem("Menu option 1");
			ImGui::MenuItem("Menu option 2");
			ImGui::MenuItem("Menu option 3");
			ImGui::MenuItem("Menu option 4");
			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Edit"))
		{
			ImGui::MenuItem("Nothing in here, sorry!", NULL, false, false);
			ImGui::EndMenu();
		}
		ImGui::EndMenuBar();
	}


	ImGui::ColorEdit3("Change Bg color", (float*)&test_color);
	ImGui::End();
}

void ModuleUI::ShowSettingsWindow()
{
	ImGui::Begin("Settings", &show_Settings_window, ImVec2(500, 600), 1.0f);
	if (ImGui::BeginMenu("Options"))
	{
		ImGui::MenuItem("Default", NULL, false, false);
		ImGui::MenuItem("Save", NULL, false, false);
		ImGui::MenuItem("Load", NULL, false, false);
		ImGui::EndMenu();
	}

	if (ImGui::CollapsingHeader("Application"))
	{
		ImGui::InputText("Application name", buf, IM_ARRAYSIZE(buf));
	}

	if (ImGui::CollapsingHeader("Window"))
	{

	}

	if (ImGui::CollapsingHeader("File System"))
	{

	}

	if (ImGui::CollapsingHeader("Input"))
	{

	}

	if (ImGui::CollapsingHeader("Hardware"))
	{

	}

	ImGui::End();
}
