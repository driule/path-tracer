#pragma once
namespace Tmpl8 {
	class Camera
	{
	public:
		Camera();

		vec3 position;
		vec3 viewDirection;
		vec3 up;
		vec3 right;
		vec3 viewDirectionNormalized;
		float fieldOfView;
		vec3 topLeft, topRight, bottomLeft;

		Ray** rayPool;

		void reset();
		void calculateScreen();
		Ray* generateRay(float x, float y);
	};
}

