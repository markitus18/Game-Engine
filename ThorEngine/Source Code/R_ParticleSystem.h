#ifndef __R_PARTICLE_SYSTEM_H__
#define __R_PARTICLE_SYSTEM_H__

#include "Resource.h"

#include "Emitter.h"
class Config;

class R_ParticleSystem : public Resource
{
public:
	R_ParticleSystem();
	~R_ParticleSystem();
	
	void InitDefaultSystem();
	void AddDefaultEmitter();

	void Serialize(Config& config);

public:
	std::vector<Emitter> emitters;
};

#endif
