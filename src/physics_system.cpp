// internal
#include "physics_system.hpp"
#include "world_init.hpp"
#include <vector>
// Returns the local bounding coordinates scaled by the current size of the entity
vec2 get_bounding_box(const Motion& motion)
{
	// abs is to avoid negative scale due to the facing direction.
	return { abs(motion.scale.x), abs(motion.scale.y) };
}

// This is a SUPER APPROXIMATE check that puts a circle around the bounding boxes and sees
// if the center point of either object is inside the other's bounding-box-circle. You can
// surely implement a more accurate detection
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
// citation: https://stackoverflow.com/questions/1560492/how-to-tell-whether-a-point-is-to-the-right-or-left-side-of-a-line
// determine if a point is on left side of a given line using cross product of 2d matrix
// returns true if target point is on left side or on top of line formed by point A, B.
bool isLeftForPoint(vec2 linePointA, vec2 linePointB, vec2 targetPoint) {
	float ans = (linePointB.x - linePointA.x) * (targetPoint.y - linePointA.y) - (linePointB.y - linePointA.y) * (targetPoint.x - linePointA.x);
	return ans >= 0;

	}

bool collidesV2(const Motion& motion1, const Motion& motion2)
	{
	// inside test

	// compute four line for motion 1 bounding box
	vec2 bb1 = get_bounding_box(motion1);
	vec2 bb2 = get_bounding_box(motion2);

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

	
	int count = 0;
	bool ans = false;
	
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			if (isLeftForPoint(box1_points[j], box1_points[(j + 1) % 4], box2_points[i]))
				count += 1;
			}

		if (count == 4) {
			ans = true;
		}
		count = 0;

	}
	
	return ans;
	// check if any point is on same side for all four line, if it is that means the point is inside the box (assuming no non-convex bounding box)

	//vec2 b1_topLeft = { (motion1.position.x - (motion1.scale.x / 2.f)), (motion1.position.y - (motion1.scale.y / 2.f)) };
	//vec2 b1_bottomRight = { (motion1.position.x + (motion1.scale.x / 2.f)), (motion1.position.y + (motion1.scale.y / 2.f))};

	//vec2 b2_topLeft = { (motion2.position.x - (motion2.scale.x / 2.f)), (motion2.position.y - (motion2.scale.y / 2.f)) };
	//vec2 b2_bottomRight = { (motion2.position.x + (motion2.scale.x / 2.f)), (motion2.position.y + (motion2.scale.y / 2.f)) };

	/*if ((b2_bottomRight.x >= b1_topLeft.x) && (b1_bottomRight.x >= b2_topLeft.x) && (b2_bottomRight.y <= b1_topLeft.y)
		&& (b1_bottomRight.y <= b2_topLeft.y)) {
		return true;*/
	


	

}

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

	// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	// TODO A2: HANDLE PEBBLE collisions HERE
	// DON'T WORRY ABOUT THIS UNTIL ASSIGNMENT 2
	// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
}