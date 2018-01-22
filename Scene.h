#pragma once
namespace Tmpl8 {
	class Scene
	{
	public:
		Scene(Surface* screen);
		Camera* camera;

		void render(int row);
		void increaseAccumulator();
		void resetAccumulator();

		int addPrimitive(Primitive* primitive);
		void addLightSource(LightSource* lightSource);

		int loadModel(const char *filename, Material* material, vec3 translationVector = vec3(0));
		void translateModel(int id, vec3 vector);

		void loadSkydome(const char* fileName);

		int getPrimitivesCount();
		void clear();
	private:
		Surface* screen;

		vec4 accumulator[SCRHEIGHT * SCRWIDTH];
		int accumulatorCounter;
		float inversedAccumulatorCounter;
		std::mt19937 randomNumbersGenerator;

		TopBVH* topBHV;
		std::vector<BVH*> BVHs;
		bool topBVHExists;

		std::vector<Primitive*> primitives;
		std::vector<LightSource*> lightSources;

		HDRBitmap* skydome;
		bool skydomeLoaded;

		struct Model
		{
			Model::Model(int id, int startIndex, int endIndex)
			{
				this->id = id;
				this->startIndex = startIndex;
				this->endIndex = endIndex;
			}
			int id, startIndex, endIndex;
		};
		std::vector<Model*> models;

		vec4 sampleNEE(Ray* ray, int depth, bool isLastIntersectedPrimitiveSpecular = true);
		vec4 sample(Ray* ray, int depth, bool isLastIntersectedPrimitiveSpecular = true);
		vec4 sampleSkydome(Ray* ray);
		vec4 illuminate(Ray* ray, int depth);
		Ray* computeDiffuseReflectionRay(Ray* ray);
		Ray* computeReflectionRay(Ray* ray);
		Ray* computeRefractionRay(Ray* ray);
		void intersectPrimitives(Ray* ray, bool isShadowRay = false);
		void intersectLightSources(Ray* ray);

		Pixel convertColorToPixel(vec4 color);

		void buildTopBVH();
		int buildBVH(int startIndex, int endIndex);
	};
}
