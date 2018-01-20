#pragma once
namespace Tmpl8 {
	class LightSource
	{
	public:
		LightSource(vec3 position, vec4 color, int intensity);

		int id;
		vec3 position;
		vec4 color;
		int intensity;

		virtual void intersect(Ray* ray) = 0;
		virtual vec3 getRandomPointOnLight() = 0;
		virtual vec3 getNormal(vec3 point) = 0;
		virtual float getArea() = 0;
	};

	class DirectLight : public LightSource
	{
	public:
		DirectLight(vec3 position, vec4 color, int intensity);

		void intersect(Ray* ray);
		vec3 getRandomPointOnLight();
		vec3 getNormal(vec3 point);
		float getArea();
	};

	class SphericalLight : public LightSource
	{
	public:
		SphericalLight(vec3 position, float radius, vec4 color, int intensity);

		void intersect(Ray* ray);
		vec3 getRandomPointOnLight();
		vec3 getNormal(vec3 point);
		float getArea();
	private:
		float radius, radius2, area;
	};
}

