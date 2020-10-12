#ifndef __R_MODEL_H__
#define __R_MODEL_H__

#include "Resource.h"

#include "MathGeoLib/src/Math/Quat.h"
#include "MathGeoLib/src/Math/float3.h"
#include "MathGeoLib/src/Math/float4x4.h"

#include <vector>

class GameObject;

struct ModelNode
{
	ModelNode() {}
	ModelNode(uint64 ID, const char* name = "No name", const float3& translation = float3::zero, const float3& scale = float3::one, const Quat& rotation = Quat::identity, uint64 parentID = 0) :
	ID(ID), name(name), parentID(parentID), materialID(-1), meshID(-1)
	{
		transform = float4x4::FromTRS(translation, rotation, scale);
	}

	std::string name;
	uint ID;
	float4x4 transform;

	uint parentID;
	int meshID;
	int materialID;
};

class R_Model : public Resource
{
public:
	R_Model();
	~R_Model();

	uint64 thumbnailID = 0;
	std::vector<ModelNode> nodes;
};

#endif //__R_MODEL_H__