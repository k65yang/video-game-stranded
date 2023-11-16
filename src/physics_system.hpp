#pragma once

#include "common.hpp"
#include "tiny_ecs.hpp"
#include "components.hpp"
#include "tiny_ecs_registry.hpp"

// AABB BVH node
struct BVHNode 
{
	// 8 byte each, so 16 in total
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

	// assuming we can consistently reference primitives using index
	// node is reference with index

	// Since we know max number of nodes, given N primitives, in BVHtree is no more than 2N - 1
	// we can preallocate all nodes and then reference it using index

	//NOTES
	// during traverse, make sure in leaf level, check that the primitives identifier is different from target to avoid collision with itself since the BVH tree build from all colliders.

	// aim for all colliders, and update tree for moving colliders


	// alternatives is build BVH with onlys static colliders like terains, so first include one identifier for static colliders, then check collisions for movable against 
};

// A simple physics system that moves rigid bodies and checks for collision
class PhysicsSystem
{
public:
	
	std::vector<BVHNode> bvhTree;
	int numberOfColliders;
	std::vector<Collider> colliders;

	void init(size_t numberOfColliders);
	void step(float elapsed_ms);
	void buildBVH();
	void PhysicsSystem::deleteCollider(Entity entity);
	void PhysicsSystem::intersectBVH(Entity entity, const int nodeIndex);

	PhysicsSystem()
	{
		this->numberOfColliders = 0;
		
	}
private:
	int rootNodeIndex = 0;
	int nodeUsed = 1;
	std::vector<int> colliderMapping;
	void PhysicsSystem::updateNodeBounds(int nodeIndex);
	void PhysicsSystem::subDivide(int nodeIndex);



};




	