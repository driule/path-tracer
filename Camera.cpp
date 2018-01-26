#include "precomp.h"

Camera::Camera()
{
	this->reset();

	int poolSize = SCRWIDTH * SCRHEIGHT;
	this->rayPool = new Ray*[poolSize];
	for (int i = 0; i < poolSize; i++)
	{
		this->rayPool[i] = new Ray();
	}
}

void Camera::reset()
{
	this->position = CAMERA_ORIGIN;
	this->viewDirection = vec3(0, 0, 1);
	this->fieldOfView = 1;
	this->up = vec3(0, -1, 0);
	this->right = vec3(1, 0, 0);

	this->calculateScreen();
}

void Camera::calculateScreen()
{
	this->viewDirectionNormalized = normalize(this->viewDirection - this->position);
	this->right = cross(this->up, this->viewDirectionNormalized);
	this->up = cross(this->viewDirectionNormalized, this->right);

	vec3 center = this->position + this->fieldOfView * this->viewDirectionNormalized;
	//vec3 screenCenter = this->position + this->fieldOfView * this->viewDirection;

	this->topLeft = center -  this->right + this->up * ASPECT_RATIO;
	this->topRight = center + this->right + this->up * ASPECT_RATIO;
	this->bottomLeft = center - this->right - this->up * ASPECT_RATIO;
 
	//this->topLeft = screenCenter + vec3(-1, -ASPECT_RATIO, 1);
	//this->topRight = screenCenter + vec3(1, -ASPECT_RATIO, 1);
	//this->bottomLeft = screenCenter + vec3(-1, ASPECT_RATIO, 1);
}

Ray* Camera::generateRay(float x, float y)
{
	vec3 direction = normalize(
		(this->topLeft + (x / SCRWIDTHf) * (this->topRight - this->topLeft) + (y / SCRHEIGHTf) * (this->bottomLeft - this->topLeft)) - this->position
	);

	int rayIndex = (int)x + (int)y * SCRWIDTH;
	this->rayPool[rayIndex]->create(this->position, direction);

	return this->rayPool[rayIndex];
}
