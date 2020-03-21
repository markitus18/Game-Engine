#include "R_Material.h"

//TODO: temporal, just to get default shader
#include "Application.h"
#include "M_Resources.h"

R_Material::R_Material() : Resource(Resource::MATERIAL)
{
	shaderID = App->moduleResources->GetResourceInfo("Engine/Assets/Defaults/Default Shader.shader").ID;
	isInternal = true;
}

R_Material::~R_Material()
{

}
