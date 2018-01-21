#include "precomp.h"


LightSource::LightSource(vec3 position, vec4 color, int intensity)
{
	this->position = position;
	this->color = color;
	this->intensity = intensity;
}

// -------------------- DIRECT LIGHT ------------------------------------

DirectLight::DirectLight(vec3 position, vec4 color, int intensity) : LightSource(position, color, intensity)
{
}

void DirectLight::intersect(Ray* ray)
{
	vec3 c = this->position - ray->origin;
	float t = dot(c, ray->direction);
	if (t < 0) return;
	vec3 q = c - t * ray->direction;
	if (q.x == 0 && q.y == 0 && q.z == 0)
	{
		ray->t = t;
		ray->intersectedObjectId = this->id;
	}
}

vec3 DirectLight::getRandomPointOnLight(std::mt19937 randomNumbersGenerator)
{
	return this->position;
}

vec3 DirectLight::getNormal(vec3 point)
{
	return normalize(point - this->position);
}

float DirectLight::getArea()
{
	return EPSILON;
}

// -------------------- SPHERICAL LIGHT ------------------------------------

SphericalLight::SphericalLight(vec3 position, float radius, vec4 color, int intensity) : LightSource(position, color, intensity)
{
	this->radius = radius;
	this->radius2 = radius * radius;

	this->area = 4 * PI * this->radius2;
}

void SphericalLight::intersect(Ray* ray)
{
	vec3 c = this->position - ray->origin;
	float t = dot(c, ray->direction);
	if (t < 0) return;
	vec3 q = c - t * ray->direction;
	float p2 = dot(q, q);

	if (p2 > this->radius2) return;

	t -= sqrt(this->radius2 - p2);
	if (t < ray->t && t >= EPSILON)
	{
		ray->t = t;
		ray->intersectedObjectId = this->id;
		ray->lightIntersected = true;
	}
}

vec3 SphericalLight::getRandomPointOnLight(std::mt19937 randomNumbersGenerator)
{
	std::uniform_real_distribution<double> uniformGenerator01(0.0, 1.0);

	double theta = 2 * M_PI * uniformGenerator01(randomNumbersGenerator);
	double phi = acos(1 - 2 * uniformGenerator01(randomNumbersGenerator));

	double x = sin(phi) * cos(theta);
	double y = sin(phi) * sin(theta);
	double z = cos(phi);

	return this->position + vec3(x, y, z);
}

vec3 SphericalLight::getNormal(vec3 point)
{
	return normalize(point - this->position);
}

float SphericalLight::getArea()
{
	return this->area;
}
