#include "W_Resources.h"

#include "ImGui/imgui.h"

#include "Engine.h"
#include "M_Resources.h"

//Resources
#include "R_Mesh.h"
#include "R_Material.h"
#include "R_Texture.h"

W_Resources::W_Resources(M_Editor* editor, ImGuiWindowClass* windowClass, int ID) : Window(editor, GetName(), windowClass, ID)
{


}

void W_Resources::Draw()
{
	ImGui::SetNextWindowClass(windowClass);
	if (!ImGui::Begin(windowStrID.c_str(), &active)) { ImGui::End(); return; }

	ImGui::Text("Resources loaded in memory");

	if (ImGui::CollapsingHeader("Models"))
	{
		for (std::map<uint64, Resource*>::iterator it = Engine->moduleResources->resources.begin(); it != Engine->moduleResources->resources.end(); it++)
		{
			if (it->second->GetType() != Resource::MODEL) continue;

			ImGui::Text("-- %s", it->second->GetName());
			if (ImGui::IsItemHovered())
			{
				DisplayResourceInfo(it->second);
			}
		}
	}

	if (ImGui::CollapsingHeader("Meshes"))
	{
		for (std::map<uint64, Resource*>::iterator it = Engine->moduleResources->resources.begin(); it != Engine->moduleResources->resources.end(); it++)
		{
			if (it->second->GetType() != Resource::MESH) continue;
			
			ImGui::Text("-- %s", it->second->GetName());
			if (ImGui::IsItemHovered())
			{
				DisplayResourceInfo(it->second);
			}
		}
	}

	if (ImGui::CollapsingHeader("Materials"))
	{
		for (std::map<uint64, Resource*>::iterator it = Engine->moduleResources->resources.begin(); it != Engine->moduleResources->resources.end(); it++)
		{
			if (it->second->GetType() != Resource::MATERIAL) continue;

			ImGui::Text("-- %s", it->second->GetName());
			if (ImGui::IsItemHovered())
			{
				DisplayResourceInfo(it->second);
			}
		}
	}

	if (ImGui::CollapsingHeader("Textures"))
	{
		for (std::map<uint64, Resource*>::iterator it = Engine->moduleResources->resources.begin(); it != Engine->moduleResources->resources.end(); it++)
		{
			if (it->second->GetType() != Resource::TEXTURE) continue;

			ImGui::Text("-- %s", it->second->GetName());
			if (ImGui::IsItemHovered())
			{
				DisplayResourceInfo(it->second);
			}
		}
	}

	if (ImGui::CollapsingHeader("Animations"))
	{
		for (std::map<uint64, Resource*>::iterator it = Engine->moduleResources->resources.begin(); it != Engine->moduleResources->resources.end(); it++)
		{
			if (it->second->GetType() != Resource::ANIMATION) continue;

			ImGui::Text("-- %s", it->second->GetName());
			if (ImGui::IsItemHovered())
			{
				DisplayResourceInfo(it->second);
			}
		}
	}

	ImGui::End();
}

void W_Resources::DisplayResourceInfo(Resource* resource)
{
	ImGui::BeginTooltip();
	ImGui::Text("UID: %llu", resource->GetID());
	ImGui::Text("Source file: %s", resource->GetOriginalFile());
	ImGui::Text("Instances: %i", resource->instances);
	ImGui::EndTooltip();
}