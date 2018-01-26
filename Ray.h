#pragma once

namespace Tmpl8 {
	class Ray
	{
	public:
		Ray();
		Ray(vec3 origin, vec3 direction);

		vec3 origin;
		vec3 direction, invertedDirection;
		float t;
		int intersectedObjectId;
		bool lightIntersected;

		void create(vec3 origin, vec3 direction);
	};
}

