// internal
#include "physics_system.hpp"

#pragma region Collider creation related function

void PhysicsSystem::createBoundingBox(std::vector<vec2>& points, vec2 scale) {

	// calculate top left x position by center(0) - width/2, similar on y position etc..
	vec2 b1_topLeft = { (0 - (scale.x / 2.f)), (0 - (scale.y / 2.f)) };
	vec2 b1_topRight = { (0 + (scale.x / 2.f)), (0 - (scale.y / 2.f)) };
	vec2 b1_bottomRight = { (0 + (scale.x / 2.f)), (0 + (scale.y / 2.f)) };
	vec2 b1_bottomLeft = { (0 - (scale.x / 2.f)), (0 + (scale.y / 2.f)) };

	points.push_back(b1_topLeft);
	points.push_back(b1_topRight);
	points.push_back(b1_bottomRight);
	points.push_back(b1_bottomLeft);

}

void PhysicsSystem::createDefaultCollider(Entity entity) {

	auto& motion = registry.motions.get(entity);
	auto& collider = registry.colliders.emplace(entity);

	// world position, used in collision detection. This needs to be updated by physics::step
	collider.position = motion.position;

	// generating points in local coord(center at 0,0)
	std::vector<glm::vec2> points;

	// HARDCODED TO BOX NOW. Assuming all entity will use a box collider
	createBoundingBox(points, motion.scale);

	// generating normals
	vec2 edge;
	for (int i = 0; i < points.size(); i++) {
		edge = points[(i + 1) % points.size()] - points[i];

		// We can obtain the normal by swapping <x,y> with <-y, x>. Normalizing vector to ease later calculation.
		collider.normals.push_back(glm::normalize(vec2(-edge.y, edge.x)));
	}

	// move all points over to collider
	collider.points = std::move(points);

	// rotation
	collider.rotation = mat2(cos(motion.angle), -sin(motion.angle), sin(motion.angle), cos(motion.angle));

	// scale
	collider.scale = motion.scale;

	// flags
	collider.flag = 0;

}

void PhysicsSystem::createMeshCollider(Entity entity, GEOMETRY_BUFFER_ID geom_id, RenderSystem* renderer) {

	auto& motion = registry.motions.get(entity);
	auto& collider = registry.colliders.emplace(entity);

	// world position, used in collision detection. This needs to be updated by physics::step
	collider.position = motion.position;

	std::vector<vec2> points;

	// grabing vertice from corresponding mesh
	auto& mesh = renderer->getMesh(geom_id);

	for (int i = 0; i < mesh.vertices.size(); i++) {
		points.push_back(vec2{ mesh.vertices[i].position.x, mesh.vertices[i].position.y });

	}

	// generating normals
	vec2 edge;
	for (int i = 0; i < points.size(); i++) {
		edge = points[(i + 1) % points.size()] - points[i];

		// We can obtain the normal by swapping <x,y> with <-y, x>. Normalizing vector to ease later calculation.
		collider.normals.push_back(glm::normalize(vec2(-edge.y, edge.x)));

	}

	// move all points over to collider
	collider.points = std::move(points);

	// DISABLE rotation FOR NOW SINCE MESH HITBOX DOESNT ROTATE

	//collider.rotation = mat2(cos(motion.angle), -sin(motion.angle), sin(motion.angle), cos(motion.angle));
	collider.rotation = mat2(cos(0.f), -sin(0.f), sin(0.f), cos(0.f));

	// scale
	collider.scale = motion.scale;

	// flags
	collider.flag = 0;

}

void PhysicsSystem::createCustomsizeBoxCollider(Entity entity, vec2 scale) {

	auto& motion = registry.motions.get(entity);
	auto& collider = registry.colliders.emplace(entity);

	// world position, used in collision detection. This needs to be updated by physics::step
	collider.position = motion.position;

	// generating points in local coord(center at 0,0)
	std::vector<glm::vec2> points;

	// HARDCODED TO BOX NOW. Assuming all entity will use a box collider
	createBoundingBox(points, scale);

	// generating normals
	vec2 edge;
	for (int i = 0; i < points.size(); i++) {
		edge = points[(i + 1) % points.size()] - points[i];

		// We can obtain the normal by swapping <x,y> with <-y, x>. Normalizing vector to ease later calculation.
		collider.normals.push_back(glm::normalize(vec2(-edge.y, edge.x)));
	}

	// move all points over to collider
	collider.points = std::move(points);

	// rotation
	collider.rotation = mat2(cos(motion.angle), -sin(motion.angle), sin(motion.angle), cos(motion.angle));

	// scale
	collider.scale = scale;

	// flags
	collider.flag = 0;

}

#pragma endregion

#pragma region Collision Detection

vec2 invertHelper(vec2 MTV)
{
	return MTV *= -1.0f;
}

// Broad phase collision detection in Axis Aligned Bounding Box (AABB)
bool AABBCollides(const Collider& collider1, const Collider& collider2)
{
	float xPos1 = collider1.position.x;
	float yPos1 = collider1.position.y;
	float width1 = collider1.scale.x;
	float height1 = collider1.scale.y;

	float xPos2 = collider2.position.x;
	float yPos2 = collider2.position.y;
	float width2 = collider2.scale.x;
	float height2 = collider2.scale.y;

	float minX1 = xPos1 - (width1 / 2.f);
	float maxX1 = xPos1 + (width1 / 2.f);
	float minY1 = yPos1 - (height1 / 2.f);
	float maxY1 = yPos1 + (height1 / 2.f);

	float minX2 = xPos2 - (width2 / 2.f);
	float maxX2 = xPos2 + (width2 / 2.f);
	float minY2 = yPos2 - (height2 / 2.f);
	float maxY2 = yPos2 + (height2 / 2.f);

	if (maxX1 < minX2 || minX1 > maxX2 || maxY1 < minY2 || minY1 > maxY2) {
		return false;
	}
	else {
		return true;
	}

}

bool AABBCollides(const Collider& collider1, vec2 aabbMin, vec2 aabbMax)
{
	float xPos1 = collider1.position.x;
	float yPos1 = collider1.position.y;
	float width1 = collider1.scale.x;
	float height1 = collider1.scale.y;

	float minX1 = xPos1 - (width1 / 2.f);
	float maxX1 = xPos1 + (width1 / 2.f);
	float minY1 = yPos1 - (height1 / 2.f);
	float maxY1 = yPos1 + (height1 / 2.f);

	float minX2 = aabbMin.x;
	float maxX2 = aabbMax.x;
	float minY2 = aabbMin.y;
	float maxY2 = aabbMax.y;

	if (maxX1 < minX2 || minX1 > maxX2 || maxY1 < minY2 || minY1 > maxY2) {
		return false;
	}
	else {
		return true;
	}

}

// Separating axis theorem(SAT): if there are no axises(all normals of two collider's edges) that seperates two collider,
// then they must intercept each other.
// reference: https://gamedev.stackexchange.com/questions/105296/calculation-correct-position-of-object-after-collision-2d
std::tuple <bool, float, vec2> SATcollides(const Collider& collider1, const Collider& collider2)
{
	vec2 minimalTranslationVector;
	float overlap = 10000;


	// obtain collider points in world coord through world position and local coord of all points for both collider
	std::vector<vec2> collider1_worldPoints;
	int c1_numOfPoints = collider1.points.size();

	for (int i = 0; i < c1_numOfPoints; i++) {
		collider1_worldPoints.push_back(collider1.position + collider1.rotation * collider1.points[i]);
	}

	std::vector<vec2> collider2_worldPoints;
	int c2_numOfPoints = collider2.points.size();

	for (int i = 0; i < c2_numOfPoints; i++) {
		collider2_worldPoints.push_back(collider2.position + collider2.rotation * collider2.points[i]);
	}

	// obtain rotated normals for both collider
	std::vector<vec2> collider1_worldNormals;
	int c1_numOfNormals = collider1.normals.size();

	for (int i = 0; i < c1_numOfNormals; i++) {
		collider1_worldNormals.push_back(collider1.rotation * collider1.normals[i]);
	}

	std::vector<vec2> collider2_worldNormals;
	int c2_numOfNormals = collider2.normals.size();

	for (int i = 0; i < c2_numOfNormals; i++) {
		collider2_worldNormals.push_back(collider2.rotation * collider2.normals[i]);
	}



	// for every normal, we project all points from collider onto this axis and find the max and min 
	// basically we want to find out the interval (min and max point) after projection all point to the normal
	// if the intervals on the same normal from both collider intercept, then it mean they are colliding

	// since we just care about the min max point of the interval on the axis , we dont need the projected vector 
	// but just the "distance" they landed on the line. 
	// To project vector x onto vector y: Proj_y(x) = (x . y)/(y . y) * y
	// since our normal vector is normalized it becomes (x . y) * y
	// in a case comparing v1 and v2 against normal
	// if (v1 . normal) * normal  > (v2 . normal) * normal then (v1 . normal) > (v2 . normal)
	// This means v1 is more align with normal than v2 alignment with normal
	// in another word, v2 is farther away from the edge since the alignment is against normal
	// we see that we can just compare (v1 . normal) instead of (v1 . normal) * normal


	// check all normals from collider 1

	for (int i = 0; i < c1_numOfNormals; i++) {

		// finding collider 1 interval

		float min1, max1;

		// initialize min max from the first point
		min1 = max1 = glm::dot(collider1_worldPoints[0], collider1_worldNormals[i]);

		for (int j = 1; j < c1_numOfPoints; j++) {

			float current = glm::dot(collider1_worldPoints[j], collider1_worldNormals[i]);
			if (current < min1) {
				min1 = current;
			}
			else if (current > max1) {
				max1 = current;
			}

		}

		// finding collider 2 interval
		float min2, max2;

		min2 = max2 = glm::dot(collider2_worldPoints[0], collider1_worldNormals[i]);

		for (int j = 1; j < c2_numOfPoints; j++) {

			float current = glm::dot(collider2_worldPoints[j], collider1_worldNormals[i]);
			if (current < min2) {
				min2 = current;
			}
			else if (current > max2) {
				max2 = current;
			}

		}

		// checking if two interval overlaps
		if (min1 < max2 && max1 > min2) {

			// compute the two overlap
			float overlap1 = max2 - min1;
			float overlap2 = max1 - min2;

			// find the smaller overlap
			overlap1 = overlap1 < overlap2 ? overlap1 : overlap2;

			// if smaller than current than we update overlap and minimal translation vector
			if (i == 0 || overlap1 < overlap) {
				overlap = overlap1;
				minimalTranslationVector = collider1_worldNormals[i];
			}
		}
		else {
			// if no interval overlaps, then we found an axis that separates the two collider, hence no collision
			return std::make_tuple(false, 0.f, vec2{ 0.f,0.f });

		}

	}

	// check all normals from collider 2

	for (int i = 0; i < c2_numOfNormals; i++) {

		// finding collider 1 interval
		float min1, max1;

		// initialize min max from the first point
		min1 = max1 = glm::dot(collider1_worldPoints[0], collider2_worldNormals[i]);

		for (int j = 1; j < c1_numOfPoints; j++) {

			float current = glm::dot(collider1_worldPoints[j], collider2_worldNormals[i]);
			if (current < min1) {
				min1 = current;
			}
			else if (current > max1) {
				max1 = current;
			}

		}

		// finding collider 2 interval
		float min2, max2;

		min2 = max2 = glm::dot(collider2_worldPoints[0], collider2_worldNormals[i]);

		for (int j = 1; j < c2_numOfPoints; j++) {

			float current = glm::dot(collider2_worldPoints[j], collider2_worldNormals[i]);
			if (current < min2) {
				min2 = current;
			}
			else if (current > max2) {
				max2 = current;
			}

		}

		// checking if two interval overlaps
		if (min1 < max2 && max1 > min2) {

			// compute the two overlap
			float overlap1 = max2 - min1;
			float overlap2 = max1 - min2;

			// find the smaller overlap
			overlap1 = overlap1 < overlap2 ? overlap1 : overlap2;

			// if smaller than current than we update overlap and minimal translation vector
			if (overlap1 < overlap) {
				overlap = overlap1;
				minimalTranslationVector = collider2_worldNormals[i];
			}
		}
		else {
			// if no interval overlaps, then we found an axis that separates the two collider, hence no collision
			return std::make_tuple(false, 0.f, vec2{ 0.f,0.f });

		}

	}

	// Reach here if there is collision

	// make the minimal translation vector always points to first collider
	vec2 dirPointedToCollider1 = collider1.position - collider2.position;

	// negative dot product means two vector are facing opposite direction
	if (glm::dot(dirPointedToCollider1, minimalTranslationVector) < 0.f) {
		minimalTranslationVector *= -1.0f;
	}


	return std::make_tuple(true, overlap, minimalTranslationVector);
}

/// <summary>
/// Collision detection in two phase: broad phase with AABB(axis align bounding box) and narrow phase with SAT(separating axis theorem)
/// </summary>
/// <param name="entity1"></param>
/// <param name="entity2"></param>
bool PhysicsSystem::collides(Entity entity1, Entity entity2) {
	auto& c1 = registry.colliders.get(entity1);
	auto& c2 = registry.colliders.get(entity2);
	bool isCollide = false; 
	if (distance(c1.position, c2.position) < detectRange) {
		if (AABBCollides(c1, c2)) {
			auto result = SATcollides(c1, c2);
			
			float overlap;
			vec2 MTV;
			std::tie(isCollide, overlap, MTV) = result;

			if (isCollide)
			{
				// Create a collisions event
				// We are abusing the ECS system a bit in that we potentially insert muliple collisions for the same entity
				registry.collisions.emplace_with_duplicates(entity1, entity2, overlap, MTV);

				// inverting correction vector for other entites
				registry.collisions.emplace_with_duplicates(entity2, entity1, overlap, invertHelper(MTV));
			}

		}
	}
	return isCollide;
}

#pragma endregion

#pragma region Bounding Volume Hierarchy for static collider

void PhysicsSystem::initStaticBVH(size_t numberOfColliders) {
	this->numberOfColliders = numberOfColliders;
	this->bvhTree.clear();
	this->bvhTree.resize(2 * numberOfColliders - 1);
	this->colliderMapping.resize(this->numberOfColliders);

	std::cout << "started building BVH" << std::endl;
	buildBVH();
	std::cout << "done building BVH" << std::endl;
}

void PhysicsSystem::updateNodeBounds(int nodeIndex)
{
	BVHNode& node = bvhTree[nodeIndex];

	// initialize very big number
	node.aabbMin = { 1e30f, 1e30f };
	node.aabbMin = { -1e30f, -1e30f };

	// loop through each collider under the node
	for (int first = node.leftFirst, i = 0; i < node.primitiveCount; i++)
	{
		auto& collider = registry.colliders.components[this->colliderMapping[first + i]];

		node.aabbMin.x = fminf(node.aabbMin.x, collider.position.x - (collider.scale.x / 2));
		node.aabbMin.y = fminf(node.aabbMin.y, collider.position.y - (collider.scale.y / 2));

		node.aabbMax.x = fmaxf(node.aabbMax.x, collider.position.x + (collider.scale.x / 2));
		node.aabbMax.y = fmaxf(node.aabbMax.y, collider.position.y + (collider.scale.y / 2));
	}
}

void PhysicsSystem::subDivide(int nodeIndex)
{
	// leafnode: terminate recursion when only one primitive(collider) left in the node
	BVHNode& node = bvhTree[nodeIndex];
	if (node.primitiveCount <= 1) {
		return;
	}

	// Midpoint split approach, could be optimize with surface area heuristic approach later on.

	// step1. determine split plane axis and position
	vec2 bb = node.aabbMax - node.aabbMin;
	int axis = 0;

	// split in the longer axis
	if (bb.y > bb.x)
		axis = 1;

	float splitPos = node.aabbMin[axis] + bb[axis] * 0.5f;

	// step2. partition the primitive based on the splitPos

	// index for first primitive on the node
	int i = node.leftFirst;

	// index for last primitive on the node
	int j = i + node.primitiveCount - 1;

	while (i <= j)
	{
		// if the collider center is to the left of split axis, it belong to left child
		if (registry.colliders.components[colliderMapping[i]].position[axis] < splitPos) {
			i++;
		}
		else {
			// if collider center is to the right, we move the index of such collider to the end and update j to one before it
			std::swap(colliderMapping[i], colliderMapping[j--]);
		}

	}
	// now index before i are for colliders belong to left child, after (including i) are colliders that belongs to right child


	// check if any of the side is empty, dont need to split if thats the case
	int leftCount = i - node.leftFirst;
	if (leftCount == 0 || leftCount == node.primitiveCount) {
		return;
	}

	//step3. split group of primitives into two halves with the split plane for left/right child

	// since i separates the two group, using i and primitive count splits them easily
	int leftChildIndex = nodeUsed++;
	int rightChildIndex = nodeUsed++;
	bvhTree[leftChildIndex].leftFirst = node.leftFirst;
	bvhTree[leftChildIndex].primitiveCount = leftCount;
	bvhTree[rightChildIndex].leftFirst = i;
	bvhTree[rightChildIndex].primitiveCount = node.primitiveCount - leftCount;

	// after assigning colliders, internal node's leftFirst represents the index of its leftchild.
	node.leftFirst = leftChildIndex;

	// setting primitive count to zero for internal node.
	node.primitiveCount = 0;

	//step4. updating AABB for both child nodes.
	updateNodeBounds(leftChildIndex);
	updateNodeBounds(rightChildIndex);

	// subdivise by recursing into each of child node
	subDivide(leftChildIndex);
	subDivide(rightChildIndex);
}

void PhysicsSystem::buildBVH()
{
	auto& collider_container = registry.colliders.components;

	// populate the mapping array for an indirect reference
	for (int i = 0; i < colliderMapping.size(); i++)
	{
		colliderMapping[i] = i;
	}

	// set up root node's child node
	BVHNode& root = bvhTree[rootNodeIndex];
	root.leftFirst = 0;

	// adding all colliders under root node
	root.primitiveCount = numberOfColliders;

	// update AABB for the root node
	updateNodeBounds(rootNodeIndex);

	// recursively divide node into smaller nodes
	subDivide(rootNodeIndex);
}

void PhysicsSystem::intersectBVH(Entity entity, const int nodeIndex) {
	BVHNode& node = bvhTree[nodeIndex];
	auto& collider = registry.colliders.get(entity);

	// if does not collide with bounding box of this node, abort
	if (!AABBCollides(collider, node.aabbMin, node.aabbMax))
		return;

	// if node is a leaf node
	if (node.primitiveCount > 0)
	{
		// check target collider against all collider under this node
		for (int i = 0; i < node.primitiveCount; i++)
		{
			Entity entity_j = registry.colliders.entities[colliderMapping[node.leftFirst + i]];

			// Checking if the entity is itself 
			if (entity != entity_j)
			{
				if (AABBCollides(collider, registry.colliders.components[colliderMapping[node.leftFirst + i]]))
				{
					auto result = SATcollides(collider, registry.colliders.components[colliderMapping[node.leftFirst + i]]);
					bool isCollide;
					float overlap;
					vec2 MTV;
					std::tie(isCollide, overlap, MTV) = result;

					if (isCollide)
					{
						// Create a collisions event
						// We are abusing the ECS system a bit in that we potentially insert muliple collisions for the same entity
						registry.collisions.emplace_with_duplicates(entity, entity_j, overlap, MTV);

						// inverting correction vector for other entites
						registry.collisions.emplace_with_duplicates(entity_j, entity, overlap, invertHelper(MTV));
					}

				}

			}
		}
	}
	else {
		// node is an internal node
		intersectBVH(entity, node.leftFirst);
		intersectBVH(entity, node.leftFirst + 1);
	}
}

#pragma endregion

void PhysicsSystem::step(float elapsed_ms)
{
	// POSITION UPDATE FROM VELOCITY
	auto& motion_container = registry.motions;
	auto& collider_container = registry.colliders;

	for (uint i = 0; i < motion_container.size(); i++)
	{
		Motion& motion = motion_container.components[i];
		Entity entity = motion_container.entities[i];
		motion.position += motion.velocity * elapsed_ms / 1000.f;

		// adjusting collider center for moving object
		if (collider_container.has(entity))
		{
			collider_container.get(entity).position = motion.position;

			auto& player_container = registry.players;
			if (player_container.has(entity))
			{
				// update collider rotation matrix since player angle changes
				collider_container.get(entity).rotation = mat2(cos(motion.angle), -sin(motion.angle), sin(motion.angle), cos(motion.angle));
			}

		}

	}

	// COLLISION DETECTION 
	 
	// player against terrain - uses static BVH 
	auto& player_entity = registry.players.entities[0];
	intersectBVH(player_entity, rootNodeIndex);
	
	// projectile against terrain - uses static BVH
	auto& projectile_entity_container = registry.projectiles.entities;

	for (int i = 0; i < projectile_entity_container.size(); i++) {
		intersectBVH(projectile_entity_container[i], rootNodeIndex);
	}

	auto& mob_entity_container = registry.mobs.entities;

	

	// player against item - brute force for now since BVH is not dynamic...
	auto& item_entity_container = registry.items.entities;

	for (int i = 0; i < item_entity_container.size(); i++) {
		collides(player_entity, item_entity_container[i]);
	}

	// player against mob - brute force...

	// ignore check against mob if player is invincible. The bool is updated by iframe timer
	if (!isPlayerInvincible) {
		for (int i = 0; i < mob_entity_container.size(); i++) {
			bool c = collides(player_entity, mob_entity_container[i]);


		}
	}
	

	// projectile against mob - brute force...
	for (int i = 0; i < projectile_entity_container.size(); i++) {
		for (int j = 0; j < mob_entity_container.size(); j++) {
			
			collides(projectile_entity_container[i], mob_entity_container[j]);
			
		}
	}

}

