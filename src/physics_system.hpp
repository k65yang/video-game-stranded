#pragma once

#include "common.hpp"
#include "tiny_ecs.hpp"
#include "components.hpp"
#include "tiny_ecs_registry.hpp"

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
	void initStaticBVH(size_t numberOfColliders);
	void buildBVH();
	void intersectBVH(Entity entity, const int nodeIndex);
	void collides(Entity entity1, Entity entity2);

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
	void updateNodeBounds(int nodeIndex);
	void subDivide(int nodeIndex);
};




	