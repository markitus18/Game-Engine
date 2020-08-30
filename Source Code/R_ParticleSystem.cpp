#include "R_ParticleSystem.h"

#include "Config.h"

#include "ParticleModule.h"

R_ParticleSystem::R_ParticleSystem() : Resource(ResourceType::PARTICLESYSTEM)
{
	emitters.reserve(4);
}

R_ParticleSystem::~R_ParticleSystem()
{

}

void R_ParticleSystem::InitDefaultSystem()
{
	emitters.clear();
	emitters.reserve(4);
	AddDefaultEmitter();
}

void R_ParticleSystem::AddDefaultEmitter()
{
	emitters.push_back(Emitter());

	emitters.back().modules.push_back(new EmitterBase());
	emitters.back().modules.push_back(new EmitterSpawn());
	emitters.back().modules.push_back(new ParticleLifetime());
	emitters.back().modules.push_back(new ParticleVelocity());
	emitters.back().modules.push_back(new ParticleSize());
	emitters.back().modules.push_back(new ParticleColor());
}

void R_ParticleSystem::SaveAsset(Config& config) const
{
	Config_Array emittersArray = config.SetArray("Emitters");

	for (uint i = 0; i < emitters.size(); ++i)
	{
		emitters[i].SaveAsset(emittersArray.AddNode());
	}
}

void R_ParticleSystem::SaveResource(char* buffer)
{

}

void R_ParticleSystem::Load(Config& config)
{
	Config_Array emittersArray = config.GetArray("Emitters");

	for (uint i = 0; i < emittersArray.GetSize(); ++i)
	{
		Config emitterNode = emittersArray.GetNode(i);
		emitters.push_back(Emitter());
		emitters[i].Load(emitterNode);
	}

}