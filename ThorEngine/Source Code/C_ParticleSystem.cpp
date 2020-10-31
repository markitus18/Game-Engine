#include "C_ParticleSystem.h"

#include "R_ParticleSystem.h"

C_ParticleSystem::C_ParticleSystem() : Component(Component::Type::ParticleSystem, nullptr, true)
{

}

C_ParticleSystem::C_ParticleSystem(GameObject* gameObject) : Component(Component::Type::ParticleSystem, gameObject, true)
{

}

C_ParticleSystem::~C_ParticleSystem()
{

}

void C_ParticleSystem::Update(float dt)
{
	for (unsigned int i = 0; i < emitters.size(); ++i)
	{
		emitters[i].Update(dt);
	}
}

void C_ParticleSystem::Reset()
{
	for (unsigned int i = 0; i < emitters.size(); ++i)
	{
		emitters[i].Reset();
	}
}

void C_ParticleSystem::Draw(RenderingSettings::RenderingFlags flags)
{
	//TODO: Draw 2D Gizmo
	for (unsigned int i = 0; i < emitters.size(); ++i)
	{
		emitters[i].DrawParticles(flags);
	}
}

void C_ParticleSystem::SetResource(Resource* resource)
{
	Reset();
	emitters.clear();

	rParticleSystemHandle.Set((R_ParticleSystem*)resource);

	if (R_ParticleSystem* system = rParticleSystemHandle.Get())
	{
		for (uint i = 0; i < system->emitters.size(); ++i)
		{
			emitters.push_back(EmitterInstance());
			emitters.back().Init(&system->emitters[i], this);
		}
	}
}

void C_ParticleSystem::SetResource(unsigned long long id)
{
	Reset();
	emitters.clear();

	rParticleSystemHandle.Set(id);

	if (R_ParticleSystem* system = rParticleSystemHandle.Get())
	{
		for (uint i = 0; i < system->emitters.size(); ++i)
		{
			emitters.push_back(EmitterInstance());
			emitters.back().Init(&system->emitters[i], this);
		}
	}
}

uint64 C_ParticleSystem::GetResourceID() const
{
	return rParticleSystemHandle.GetID();
}