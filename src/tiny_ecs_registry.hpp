#pragma once
#include <vector>

#include "tiny_ecs.hpp"
#include "components.hpp"

class ECSRegistry
{
	// Callbacks to remove a particular or all entities in the system
	std::vector<ContainerInterface*> registry_list;

public:
	// Manually created list of all components this game has
	ComponentContainer<DeathTimer> deathTimers;
	ComponentContainer<ToolTip> tips;
	ComponentContainer<Motion> motions;
	ComponentContainer<Collision> collisions;

	ComponentContainer<Player> players;
	ComponentContainer<SpeedPowerup> speedPowerup;
	ComponentContainer<HealthPowerup> healthPowerup;


	ComponentContainer<PlayerKnockbackEffect> playerKnockbackEffects;
	ComponentContainer<PlayerInaccuracyEffect> playerInaccuracyEffects;
	ComponentContainer<Weapon> weapons;
	ComponentContainer<Projectile> projectiles;
	ComponentContainer<Inventory> inventories;

	ComponentContainer<Mob> mobs;
	ComponentContainer<SpaceshipHome> spaceshipHomes; 
	ComponentContainer<MobSlowEffect> mobSlowEffects;
	ComponentContainer<Path> paths;
	ComponentContainer<Item> items;

	ComponentContainer<ParticleTrail> particleTrails;
	ComponentContainer<Particle> particles;

	ComponentContainer<vec2> screenUI;
	ComponentContainer<QuestItemIndicator> questItemIndicators;

	// Rendering related
	ComponentContainer<Mesh*> meshPtrs;
	ComponentContainer<RenderRequest> renderRequests;
	ComponentContainer<Camera> cameras;

	ComponentContainer<ScreenState> screenStates;
	ComponentContainer<DebugComponent> debugComponents;
	ComponentContainer<vec4> colors;
	ComponentContainer<Collider> colliders;


	// Terrain-related
	ComponentContainer<TerrainCell> terrainCells;

	// constructor that adds all containers for looping over them
	// IMPORTANT: Don't forget to add any newly added containers!
	ECSRegistry()
	{
		registry_list.push_back(&deathTimers);
		registry_list.push_back(&motions);
		registry_list.push_back(&collisions);

		registry_list.push_back(&players);
		registry_list.push_back(&speedPowerup);
		registry_list.push_back(&healthPowerup);

		registry_list.push_back(&playerKnockbackEffects);
		registry_list.push_back(&playerInaccuracyEffects);
		registry_list.push_back(&weapons);
		registry_list.push_back(&projectiles);

		registry_list.push_back(&mobs);
		registry_list.push_back(&spaceshipHomes);
		registry_list.push_back(&mobSlowEffects);
		registry_list.push_back(&paths);
		registry_list.push_back(&items);

		registry_list.push_back(&particleTrails);
		registry_list.push_back(&particles);

		registry_list.push_back(&screenUI);

		registry_list.push_back(&meshPtrs);
		registry_list.push_back(&renderRequests);
		registry_list.push_back(&screenStates);
		
		registry_list.push_back(&debugComponents);
		registry_list.push_back(&colors);
		registry_list.push_back(&colliders);


		registry_list.push_back(&cameras);
		registry_list.push_back(&terrainCells);
	}

	void clear_all_components() {
		for (ContainerInterface* reg : registry_list)
			reg->clear();
	}

	void list_all_components() {
		printf("Debug info on all registry entries:\n");
		for (ContainerInterface* reg : registry_list)
			if (reg->size() > 0)
				printf("%4d components of type %s\n", (int)reg->size(), typeid(*reg).name());
	}

	void list_all_components_of(Entity e) {
		printf("Debug info on components of entity %u:\n", (unsigned int)e);
		for (ContainerInterface* reg : registry_list)
			if (reg->has(e))
				printf("type %s\n", typeid(*reg).name());
	}

	void remove_all_components_of(Entity e) {
		for (ContainerInterface* reg : registry_list)
			reg->remove(e);
	}

	Entity get_main_camera() {
		return main_camera;
	}

	void set_main_camera(Entity camera) {
		main_camera = camera;
	}

private:
	Entity main_camera;
};

extern ECSRegistry registry;