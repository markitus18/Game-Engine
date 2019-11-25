#include "C_Mesh.h"

#include "Application.h"
#include "M_Resources.h"

#include "GameObject.h"

#include "R_Mesh.h"
#include "R_Bone.h"

#include "C_Transform.h"
#include "C_Bone.h"

#include "OpenGL.h"

C_Mesh::C_Mesh() : Component(Type::Mesh, nullptr, true)
{
}

C_Mesh::C_Mesh(GameObject* new_GameObject) : Component(Type::Mesh, new_GameObject, true)
{
}

C_Mesh::~C_Mesh()
{

}

void C_Mesh::AddBone(C_Bone* bone)
{
	for (uint i = 0; i < bones.size(); i++)
		if (bones[i] == bone)
			return;

	bones.push_back(bone);
}

const AABB& C_Mesh::GetAABB() const
{
	if (resourceID != 0)
		return ((R_Mesh*)App->moduleResources->GetResource(resourceID))->aabb;
	else
		return AABB(float3(-.5f, -.5f, .5f), float3(.5f, .5f, .5f));
}

void C_Mesh::StartBoneDeformation()
{
	if (animMesh == nullptr)
	{
		animMesh = new R_Mesh();
		R_Mesh* rMesh = (R_Mesh*)GetResource();
		animMesh->buffersSize[R_Mesh::b_vertices] = rMesh->buffersSize[R_Mesh::b_vertices];
		animMesh->buffersSize[R_Mesh::b_normals] = rMesh->buffersSize[R_Mesh::b_normals];
		animMesh->buffersSize[R_Mesh::b_indices] = rMesh->buffersSize[R_Mesh::b_indices];

		animMesh->vertices = new float[rMesh->buffersSize[R_Mesh::b_vertices] * 3];
		animMesh->normals = new float[rMesh->buffersSize[R_Mesh::b_normals] * 3];
		animMesh->indices = new uint [rMesh->buffersSize[R_Mesh::b_indices]];
		memcpy(animMesh->indices, rMesh->indices, rMesh->buffersSize[R_Mesh::b_indices] * sizeof(uint));
	}

	R_Mesh* rMesh = (R_Mesh*)GetResource();
	memset(animMesh->vertices, 0, rMesh->buffersSize[R_Mesh::b_vertices] * sizeof(float) * 3);

	if (rMesh->buffersSize[R_Mesh::b_normals] > 0)
	{
		memset(animMesh->normals, 0, rMesh->buffersSize[R_Mesh::b_normals] * sizeof(float) * 3);
	}
}

void C_Mesh::DeformAnimMesh()
{
	//Just for security
	if (animMesh == nullptr)
		StartBoneDeformation();

	for (uint i = 0; i < bones.size(); i++)
	{
		R_Bone* rBone = (R_Bone*)bones[i]->GetResource();
		R_Mesh* rMesh = (R_Mesh*)GetResource();
		C_Bone* rootBone = bones[i]->GetRoot();

		float4x4 matrix = bones[i]->GetSystemTransform();
		matrix = gameObject->GetComponent<C_Transform>()->GetTransform().Inverted() * matrix;
		matrix = matrix * rBone->offset;

		for (uint i = 0; i < rBone->numWeights; i++)
		{
			uint index = rBone->weightsIndex[i];
			float3 originalV(&rMesh->vertices[index * 3]);

			float3 toAdd = matrix.TransformPos(originalV);

			animMesh->vertices[index * 3] += toAdd.x  * rBone->weights[i];
			animMesh->vertices[index * 3 + 1] += toAdd.y * rBone->weights[i];
			animMesh->vertices[index * 3 + 2] += toAdd.z * rBone->weights[i];

			if (rMesh->buffersSize[R_Mesh::b_normals] > 0)
			{
				toAdd = matrix.TransformPos(float3(&rMesh->normals[index * 3]));
				animMesh->normals[index * 3] += toAdd.x * rBone->weights[i];
				animMesh->normals[index * 3 + 1] += toAdd.y * rBone->weights[i];
				animMesh->normals[index * 3 + 2] += toAdd.z * rBone->weights[i];
			}
		}
	}
}