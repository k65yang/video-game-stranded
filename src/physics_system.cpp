// internal
#include "physics_system.hpp"
#include "world_init.hpp"
#include <vector>
// Returns the local bounding coordinates of collider size
vec2 get_bounding_box(const Collider& collider)
{
	// abs is to avoid negative scale due to the facing direction.
	return { abs(collider.size.x), abs(collider.size.y) };
}

int invertEdgeHelper(int collided_edge)
	{
	if (collided_edge == 0)
		{
		collided_edge = 3;
		}
	else if (collided_edge == 1)
		{
		collided_edge = 2;
		}
	else if (collided_edge == 2)
		{
		collided_edge = 1;
		}
	else
		{
		collided_edge = 0;
		}

	return collided_edge;
	}

// This is a SUPER APPROXIMATE check that puts a circle around the bounding boxes and sees
// if the center point of either object is inside the other's bounding-box-circle. You can
// surely implement a more accurate detection
/*
bool collides(const Motion& motion1, const Motion& motion2)
{
	vec2 dp = motion1.position - motion2.position;
	float dist_squared = dot(dp,dp);
	const vec2 other_bonding_box = get_bounding_box(motion1)/2.f;
	const float other_r_squared = dot(other_bonding_box, other_bonding_box);
	const vec2 my_bonding_box = get_bounding_box(motion2)/2.f;
	const float my_r_squared = dot(my_bonding_box, my_bonding_box);
	const float r_squared = max(other_r_squared, my_r_squared);
	if (dist_squared < r_squared)
		return true;
	return false;
}

*/

// citation: https://stackoverflow.com/questions/1560492/how-to-tell-whether-a-point-is-to-the-right-or-left-side-of-a-line
// determine if a point is on left side of a given line using cross product of 2d matrix
// returns true if target point is on left side or on top of line formed by point A, B.
bool isLeftForPoint(vec2 linePointA, vec2 linePointB, vec2 targetPoint) {
	float ans = (linePointB.x - linePointA.x) * (targetPoint.y - linePointA.y) - (linePointB.y - linePointA.y) * (targetPoint.x - linePointA.x);
	return ans >= 0;

	}
// point inside box test
/*
bool collidesV2(const Motion& motion1, const Motion& motion2)
	{
	
	// get abs(scale)
	vec2 bb1 = get_bounding_box(motion1);
	vec2 bb2 = get_bounding_box(motion2);

	// compute four points for motion 1 bounding box
	vec2 b1_topLeft = { (motion1.position.x - (bb1.x / 2.f)), (motion1.position.y - (bb1.y / 2.f)) };
	vec2 b1_topRight = { b1_topLeft.x + bb1.x, b1_topLeft.y };
	vec2 b1_bottomRight = { (motion1.position.x + (bb1.x / 2.f)), (motion1.position.y + (bb1.y / 2.f)) };
	vec2 b1_bottomLeft = { b1_bottomRight.x - bb1.x, b1_bottomRight.y };

	vec2 box1_points[4] = { b1_topLeft,b1_topRight,b1_bottomRight,b1_bottomLeft };

	// compute four points for motion 2 bounding box
	vec2 b2_topLeft = { (motion2.position.x - (bb2.x / 2.f)), (motion2.position.y - (bb2.y / 2.f)) };
	vec2 b2_topRight = { b2_topLeft.x + bb2.x, b2_topLeft.y };
	vec2 b2_bottomRight = { (motion2.position.x + (bb2.x / 2.f)), (motion2.position.y + (bb2.y / 2.f)) };
	vec2 b2_bottomLeft = { b2_bottomRight.x - bb2.x, b2_bottomRight.y };

	vec2 box2_points[4] = { b2_topLeft,b2_topRight,b2_bottomRight,b2_bottomLeft };

	
	int edge_count = 0;
	bool isIn = false;
	
	// check if any of the 4 edge point of is within the box (on the same side of all edges)
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			if (isLeftForPoint(box1_points[j], box1_points[(j + 1) % 4], box2_points[i]))
				edge_count += 1;
			}

		if (edge_count == 4) {
			isIn = true;
		}
		edge_count = 0;

	}

	// does the same check for the other way around
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			if (isLeftForPoint(box2_points[j], box2_points[(j + 1) % 4], box1_points[i]))
				edge_count += 1;
		}

		if (edge_count == 4) {
			isIn = true;
		}
		edge_count = 0;

	}

	
	return isIn;
}
*/


// Return the edge of collision as int: -1 no collision, 0 top, 1 right, 2 bottom, 3 lef 
bool collidesV3(const Collider& collider1, const Collider& collider2)
{

	// get width/height of bounding box
	vec2 bb1 = get_bounding_box(collider1);
	vec2 bb2 = get_bounding_box(collider2);

	// compute four points for motion 1 bounding box

	// calculate top left x position by center x - width/2, similar on y position etc..
	vec2 b1_topLeft = { (collider1.center.x - (bb1.x / 2.f)), (collider1.center.y - (bb1.y / 2.f)) };
	vec2 b1_topRight = { b1_topLeft.x + bb1.x, b1_topLeft.y };
	vec2 b1_bottomRight = { (collider1.center.x + (bb1.x / 2.f)), (collider1.center.y + (bb1.y / 2.f)) };
	vec2 b1_bottomLeft = { b1_bottomRight.x - bb1.x, b1_bottomRight.y };

	vec2 box1_points[4] = { b1_topLeft,b1_topRight,b1_bottomRight,b1_bottomLeft };

	// compute four points for motion 2 bounding box
	vec2 b2_topLeft = { (collider2.center.x - (bb2.x / 2.f)), (collider2.center.y - (bb2.y / 2.f)) };
	vec2 b2_topRight = { b2_topLeft.x + bb2.x, b2_topLeft.y };
	vec2 b2_bottomRight = { (collider2.center.x + (bb2.x / 2.f)), (collider2.center.y + (bb2.y / 2.f)) };
	vec2 b2_bottomLeft = { b2_bottomRight.x - bb2.x, b2_bottomRight.y };

	vec2 box2_points[4] = { b2_topLeft,b2_topRight,b2_bottomRight,b2_bottomLeft };


	int edge_count = 0;
	bool isIn = false;
	int closest_edge = -1;
	int dis = 1000; // setting default to a high number 

	// check if any of the 4 edge point of is within the box (on the same side of all edges)
	// also check the smallest distance between point with 4 edges
	
	for (uint i = 0; i < 4; i++) {
		for (uint j = 0; j < 4; j++) {
			if (isLeftForPoint(box1_points[j], box1_points[(j + 1) % 4], box2_points[i])) {
				edge_count += 1;
				
			}
				
		}

		// if a point is detected to be inside the hit box
		if (edge_count == 4) {
			isIn = true;
			
			// compute the edge that is clostest to the point
			if (abs(box1_points[0].y - box2_points[i].y) <= dis) {
				dis = abs(box1_points[0].y - box2_points[i].y);
				closest_edge = 0;
			}

			
			if (abs(box1_points[1].x - box2_points[i].x) <= dis) {
				dis = abs(box1_points[1].x - box2_points[i].x);
				closest_edge = 1;
			}

			if (abs(box1_points[2].y - box2_points[i].y) <= dis) {
				dis = abs(box1_points[2].y - box2_points[i].y);
				closest_edge = 2;
			}

			if (abs(box1_points[3].x - box2_points[i].x) <= dis) {
				dis = abs(box1_points[3].x - box2_points[i].x);
				closest_edge = 3;
			}

		}

		edge_count = 0;



	}

	// does the same check for the other way around
	for (uint i = 0; i < 4; i++) {
		for (uint j = 0; j < 4; j++) {
			if (isLeftForPoint(box2_points[j], box2_points[(j + 1) % 4], box1_points[i]))
				edge_count += 1;
		}

		if (edge_count == 4) {
			isIn = true;


			// note that clostest edge is invereted since the returned closest edge is with respect to first bounding box to be consistent
			if (abs(box2_points[0].y - box1_points[i].y) <= dis) {
				dis = abs(box1_points[0].y - box2_points[i].y);
				closest_edge = 3;
			}

			if (abs(box2_points[1].x - box1_points[i].x) <= dis) {
				dis = abs(box1_points[1].x - box2_points[i].x);
				closest_edge = 2;
			}

			if (abs(box2_points[2].y - box1_points[i].y) <= dis) {
				dis = abs(box1_points[2].y - box2_points[i].y);
				closest_edge = 1;
			}

			if (abs(box2_points[3].x - box1_points[i].x) <= dis) {
				dis = abs(box1_points[3].x - box2_points[i].x);
				closest_edge = 0;
			}
		}
		edge_count = 0;

	}

	return closest_edge;
}

void PhysicsSystem::step(float elapsed_ms)
	{
	// Move fish based on how much time has passed, this is to (partially) avoid
	// having entities move at different speed based on the machine.
	auto& motion_container = registry.motions;
	auto& collider_container = registry.colliders;

	for (uint i = 0; i < motion_container.size(); i++)
		{
		Motion& motion = motion_container.components[i];
		Entity entity = motion_container.entities[i];
		motion.position += motion.velocity * elapsed_ms / 1000.f;

		// adjusting collider center
		if (collider_container.has(entity))
			{
			collider_container.get(entity).center = motion.position;
			}

		}

	// Check for collisions between all entities with collider
	for (uint i = 0; i < collider_container.components.size(); i++)
		{
		Collider& collider_i = collider_container.components[i];
		Entity entity_i = collider_container.entities[i];

		// note starting j at i+1 to compare all (i,j) pairs only once (and to not compare with itself)
		for (uint j = i + 1; j < collider_container.components.size(); j++)
			{
			Collider& collider_j = collider_container.components[j];

			int collided_edge = collidesV3(collider_i, collider_j);

			// has collided with other 
			if (collided_edge > -1)
				{
				Entity entity_j = collider_container.entities[j];

				// Create a collisions event
				// We are abusing the ECS system a bit in that we potentially insert muliple collisions for the same entity
				registry.collisions.emplace_with_duplicates(entity_i, entity_j, collided_edge);

				// inverting collided edge for other entites
				registry.collisions.emplace_with_duplicates(entity_j, entity_i, invertEdgeHelper(collided_edge));
				}
			}
		}
	}



		
		
		

/*
void PhysicsSystem::step(float elapsed_ms)
{
	// Move fish based on how much time has passed, this is to (partially) avoid
	// having entities move at different speed based on the machine.
	auto& motion_container = registry.motions;
	for(uint i = 0; i < motion_container.size(); i++)
	{
		Motion& motion = motion_container.components[i];
		Entity entity = motion_container.entities[i];
		
		motion.position += motion.velocity * elapsed_ms / 1000.f;
	}

	// Check for collisions between all moving entities
	for(uint i = 0; i < motion_container.components.size(); i++)
	{
		Motion& motion_i = motion_container.components[i];
		Entity entity_i = motion_container.entities[i];
		
		// note starting j at i+1 to compare all (i,j) pairs only once (and to not compare with itself)
		for(uint j = i+1; j < motion_container.components.size(); j++)
		{
			Motion& motion_j = motion_container.components[j];
			if (collidesV2(motion_i, motion_j))
			{
				Entity entity_j = motion_container.entities[j];
				// Create a collisions event
				// We are abusing the ECS system a bit in that we potentially insert muliple collisions for the same entity
				registry.collisions.emplace_with_duplicates(entity_i, entity_j);
				registry.collisions.emplace_with_duplicates(entity_j, entity_i);
			}
		}
	}
	*/

	// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	// TODO A2: HANDLE PEBBLE collisions HERE
	// DON'T WORRY ABOUT THIS UNTIL ASSIGNMENT 2
	// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
