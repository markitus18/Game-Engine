#include "C_ParticleSystem.h"

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

void C_ParticleSystem::SetResource(Resource* resource)
{
	Component::SetResource(resource);

}

void C_ParticleSystem::SetResource(unsigned long long id)
{
	Component::SetResource(id);

}