#include "precomp.h"

Ray::Ray()
{
}

Ray::Ray(vec3 origin, vec3 direction)
{
	this->create(origin, direction);
}

void Ray::create(vec3 origin, vec3 direction)
{
	this->origin = origin;
	this->direction = direction;

	this->t = INFINITY;
	this->intersectedObjectId = -1;
	this->lightIntersected = false;

	this->invertedDirection = vec3(1.0f / this->direction.x, 1.0f / this->direction.y, 1.0f / this->direction.z);
}
