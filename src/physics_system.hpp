#pragma once

#include "common.hpp"
#include "tiny_ecs.hpp"
#include "components.hpp"
#include "tiny_ecs_registry.hpp"
#include "world_init.hpp"
#include "render_system.hpp"
#include <vector>
#include <iostream>
#include <cmath>

// AABB BVH node
struct BVHNode 
{
	vec2 aabbMin;
	vec2 aabbMax;

	// Index for left node or first collider it stores depending if the node is internal/leaf
	int leftFirst;

	// if 0 then node is an internal node, else if leaf node then it represent the number of primitive under the node.
	int primitiveCount; 

	// normally a BVH AABB node consist of aabb min/max, left/right node, and 
	// number of primitives (collider in our case) stored under each nodes. 
	// we can save space by using the following convention.
	// 
	// depending on primitiveCount, we know whether then node is internal or leaf node
	// 
	// primitiveCount = 0 means the node is internal/root node.
	// in this case leftFirst stored the index to its leftNode, and we get index of rightNode by 
	// leftFirst + 1

	// primitiveCount > 0 means the node is a leaf node.
	// in this case leftFirst stored the index to its first collider because 
	// there is no need for left/right node variable
	// we can get all primitives under such using primitiveCount

};

// A simple physics system that moves rigid bodies and checks for collision
class PhysicsSystem
{
public:
	
	int numberOfColliders;
	float detectRange;
	void step(float elapsed_ms);

	/// <summary>
	/// Initializes the static BVH with N:number of colliders (all terrain colliders). Total number of tree nodes is 2N - 1.
	/// ColliderMapping is used for indirect reference to avoid tempering with actual collider containers.
	/// </summary>
	/// <param name="numberOfColliders">Number of colliders</param>
	void initStaticBVH(size_t numberOfColliders);

	/// <summary>
	/// building a static BVH tree for all terrain colliders in a top down approach.
	/// This must be call after creating terrain colliders and before all other colliders such as items, mobs....
	/// citation for reference1: https://jacco.ompf2.com/2022/04/13/how-to-build-a-bvh-part-1-basics/
	/// reference2: https://box2d.org/files/ErinCatto_DynamicBVH_Full.pdf
	/// </summary>
	void buildBVH();

	/// <summary>
	//  Collision detection for entity against terrain collider using the static BVH. Creates corresponding collision component for the entities
	/// </summary>
	/// <param name="entity"></param>
	/// <param name="nodeIndex">This is always the rootnode index</param>
	void intersectBVH(Entity entity, const int nodeIndex);
	void collides(Entity entity1, Entity entity2);

	/// <summary>
	/// create coord of a box given a scale(width/height). Points are in local coord and center is 0,0. 
	/// points are added in clockwise direction starting from top left point
	/// </summary>
	/// <param name="points">destination buffer to store the points</param>
	/// <param name="scale">width and height of the box</param>
	/// <returns>void</returns>
	void createBoundingBox(std::vector<vec2>& points, vec2 scale);


	/// <summary>
	/// Create a box shape collider of size based on entity's scale at entity's motion location. 
	/// Reference:: https://gamedev.stackexchange.com/questions/105296/calculation-correct-position-of-object-after-collision-2d
	/// </summary>
	/// <param name="entity">entity to attach this collider</param>
	/// <returns>void</returns>
	void createDefaultCollider(Entity entity);

	/// <summary>
	/// Create a convex hull collider using corresponding mesh and attach to the entity.
	/// </summary>
	/// <param name="entity">entity to attach this collider</param>
	/// <returns>void</returns>
	void createMeshCollider(Entity entity, GEOMETRY_BUFFER_ID geom_id, RenderSystem* renderer);


	/// <summary>
	/// Create a box collider with given scale and attach to the entity
	/// </summary>
	/// <param name="entity">entity to attach this collider</param>
	/// <param name="entity">scale of the collider in vec2</param>
	/// <returns>void</returns>
	void createCustomsizeBoxCollider(Entity entity, vec2 scale);

	PhysicsSystem()
	{
		this->numberOfColliders = 0; 
		this->detectRange = 2.f;
	}
private:

	int rootNodeIndex = 0;
	int nodeUsed = 1;
	std::vector<int> colliderMapping;
	std::vector<BVHNode> bvhTree;

	// internal functions: update the AABB of the BVH node based on all colliders it holds
	void updateNodeBounds(int nodeIndex);

	// internal functions: recursively divide nodes during tree construction. 
	void subDivide(int nodeIndex);

};




	