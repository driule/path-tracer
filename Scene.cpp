#include "precomp.h"

Scene::Scene(Surface* screen)
{
	// create camera
	this->screen = screen;
	this->camera = new Camera();

	this->topBVHExists = false;
	this->skydomeLoaded = false;

	this->resetAccumulator();

	// set up random numbers generator
	this->randomNumbersGenerator = std::mt19937(
		std::chrono::system_clock::now().time_since_epoch().count()
	);
}

void Scene::render(int row)
{
	for (int x = 0; x < SCRWIDTH; x++)
	{
		// generate and trace ray
		Ray* ray = this->camera->generateRay(x, row);

		vec4 color;// = this->sample(ray, 0);
		if (x < SCRWIDTH / 2)
		{
			color = this->sampleNEE(ray, 0);
		}
		else
		{
			color = this->sample(ray, 0);
		}

		int pixelId = row * SCRWIDTH + x;
		this->accumulator[pixelId] += color;

		// plot pixel with color
		this->screen->Plot(x, row, this->convertColorToPixel(this->accumulator[pixelId] * this->inversedAccumulatorCounter));

		// clear garbages
		delete ray;
	}
}

void Scene::increaseAccumulator()
{
	this->accumulatorCounter++;
	this->inversedAccumulatorCounter = 1.0f / this->accumulatorCounter;
}

void Scene::resetAccumulator()
{
	for (int i = 0; i < SCRWIDTH * SCRHEIGHT; i++)
	{
		this->accumulator[i] = vec4(0);
	}

	this->accumulatorCounter = 0;
}

vec4 Scene::sampleNEE(Ray* ray, int depth, bool isLastIntersectedPrimitiveSpecular)
{
	depth += 1;
	if (depth > 10) return BGCOLOR;

	this->intersectPrimitives(ray);
	this->intersectLightSources(ray);

	if (ray->intersectedObjectId == -1) // no primitive intersected
	{
		return this->skydomeLoaded ? this->sampleSkydome(ray) : BGCOLOR;
	}

	if (ray->lightIntersected)
	{
		if (!isLastIntersectedPrimitiveSpecular)
		{
			return this->lightSources[ray->intersectedObjectId]->color;
		}

		return BGCOLOR;
	}

	// primitive intersected
	Material* material = this->primitives[ray->intersectedObjectId]->material;
	if (material->type == diffuse)
	{
		vec3 hitPoint = ray->origin + ray->t * ray->direction;

		Primitive* intersectedPrimitive = this->primitives[ray->intersectedObjectId];
		vec3 primitiveNormal = intersectedPrimitive->getNormal(hitPoint);

		int randomLightIndex = rand() % this->lightSources.size();
		LightSource* randomLight = this->lightSources[randomLightIndex];

		vec3 lightDirection = randomLight->getRandomPointOnLight(this->randomNumbersGenerator) - hitPoint;
		float distanceToLightSquared = lightDirection.sqrLentgh();
		lightDirection = normalize(lightDirection);

		float lightNormalDotLightDirection = dot(randomLight->getNormal(hitPoint), -lightDirection);
		float primitiveNormalDotLightDirection = dot(primitiveNormal, lightDirection);

		vec4 lightStrength = vec4(0, 0, 0, 1);
		vec4 BRDF = intersectedPrimitive->material->color * INVERSEPI;
		if (lightNormalDotLightDirection > 0 && primitiveNormalDotLightDirection > 0)
		{
			// light is not behind surface point, trace shadow ray
			Ray* shadowRay = new Ray(hitPoint + EPSILON * lightDirection, lightDirection);
			shadowRay->t = sqrt(distanceToLightSquared) - 2 * EPSILON;
			this->intersectPrimitives(shadowRay, true);

			if (shadowRay->intersectedObjectId == -1)
			{
				float solidAngle = CLAMP((lightNormalDotLightDirection * randomLight->getArea()) / distanceToLightSquared, 0, 1);
				lightStrength = randomLight->color * randomLight->intensity * solidAngle * BRDF * primitiveNormalDotLightDirection;
			}
			delete shadowRay;
		}

		//Ray* diffuseReflectionRay = this->computeDiffuseReflectionRay(ray);
		//
		//vec3 hitPoint = ray->origin + ray->t * ray->direction;

		std::uniform_real_distribution<double> uniformGenerator01(0.0, 1.0);
		float random1 = uniformGenerator01(this->randomNumbersGenerator);
		float random2 = uniformGenerator01(this->randomNumbersGenerator);

		float angle = 2 * PI * random2;
		float r = sqrt(1 - random1 * random1);
		vec3 direction = vec3(cosf(angle) * r, sinf(angle) * r, random1);

		// Importance Sampling
		//float r = sqrt(random1);
		//vec3 direction = vec3(cosf(angle) * r, sinf(angle) * r, sqrt(1 - random1));

		if (dot(this->primitives[ray->intersectedObjectId]->getNormal(hitPoint), direction) < 0)
		{
			direction *= -1.0f;
		}

		Ray* diffuseReflectionRay = new Ray(hitPoint + direction * EPSILON, direction);
		//
		//float PDF = PI / dot(primitiveNormal, diffuseReflectionRay->direction);  // Importance Sampling
		float PDF = (2 * PI);

		vec4 bounceRayColor = (this->sampleNEE(diffuseReflectionRay, depth, false) * dot(primitiveNormal, diffuseReflectionRay->direction));
		delete diffuseReflectionRay;

		return (bounceRayColor * PDF) * BRDF + lightStrength;
	}
	if (material->type == mirror)
	{
		Ray* reflectionRay = computeReflectionRay(ray);
		vec4 reflectionColor = this->sampleNEE(reflectionRay, depth, true);
		delete reflectionRay;

		return material->color * reflectionColor;
	}
	if (material->type == dielectric)
	{
		vec4 color = material->color * 0.2;

		Ray* refractionRay = this->computeRefractionRay(ray);
		if (refractionRay->intersectedObjectId == -2)
		{
			delete refractionRay;
			return color;
		}
		else
		{
			color += this->sampleNEE(refractionRay, depth, true);
		}

		Ray* reflectionRay = this->computeReflectionRay(ray);
		vec4 reflectionColor = this->sampleNEE(reflectionRay, depth, true);

		delete refractionRay;
		delete reflectionRay;

		return material->reflection * reflectionColor + material->color.w * color;
	}

	return BGCOLOR;
}

vec4 Scene::sample(Ray* ray, int depth)
{
	depth += 1;
	if (depth > 10) return BGCOLOR;

	this->intersectPrimitives(ray);
	this->intersectLightSources(ray);

	if (ray->intersectedObjectId == -1) // no primitive intersected
	{
		return this->skydomeLoaded ? this->sampleSkydome(ray) : BGCOLOR;
	}

	if (ray->lightIntersected)
	{
		return this->lightSources[ray->intersectedObjectId]->color;
	}

	// primitive intersected
	Material* material = this->primitives[ray->intersectedObjectId]->material;
	if (material->type == diffuse)
	{
		return illuminate(ray, depth);
	}
	if (material->type == mirror)
	{
		Ray* reflectionRay = computeReflectionRay(ray);
		vec4 reflectionColor = this->sample(reflectionRay, depth);
		delete reflectionRay;

		return material->color * reflectionColor;
	}
	if (material->type == dielectric)
	{
		vec4 color = material->color * 0.2;

		Ray* refractionRay = this->computeRefractionRay(ray);
		if (refractionRay->intersectedObjectId == -2)
		{
			delete refractionRay;
			return color;
		}
		else
		{
			color += this->sample(refractionRay, depth);
		}

		Ray* reflectionRay = this->computeReflectionRay(ray);
		vec4 reflectionColor = this->sample(reflectionRay, depth);

		delete refractionRay;
		delete reflectionRay;

		return material->reflection * reflectionColor + material->color.w * color;
	}

	return BGCOLOR;
}

vec4 Scene::sampleSkydome(Ray* ray)
{
	float u = fmodf(0.5f * (1.0f + atan2(ray->direction.x, -ray->direction.z) * INVERSEPI), 1.0f);
	float v = acosf(ray->direction.y) * INVERSEPI;
	int pixel = (int)(u * (float)(this->skydome->width - 1)) + ((int)(v * (float)(this->skydome->height - 1)) * this->skydome->width);

	return this->skydome->buffer[pixel];
}

vec4 Scene::illuminate(Ray* ray, int depth)
{
	vec3 hitPoint = ray->origin + ray->t * ray->direction;

	Primitive* intersectedPrimitive = this->primitives[ray->intersectedObjectId];
	vec3 primitiveNormal = intersectedPrimitive->getNormal(hitPoint);

	int randomLightIndex = rand() % this->lightSources.size();
	LightSource* randomLight = this->lightSources[randomLightIndex];

	vec3 lightDirection = randomLight->getRandomPointOnLight(this->randomNumbersGenerator) - hitPoint;
	float distanceToLightSquared = lightDirection.sqrLentgh();
	lightDirection = normalize(lightDirection);

	float lightNormalDotLightDirection = dot(randomLight->getNormal(hitPoint), -lightDirection);
	float primitiveNormalDotLightDirection = dot(primitiveNormal, lightDirection);

	vec4 directIlluminationColor = vec4(0, 0, 0, 1);
	vec4 BRDF = intersectedPrimitive->material->color * INVERSEPI;
	if (lightNormalDotLightDirection > 0 && primitiveNormalDotLightDirection > 0)
	{
		// light is not behind surface point, trace shadow ray
		Ray* shadowRay = new Ray(hitPoint + EPSILON * lightDirection, lightDirection);
		shadowRay->t = sqrt(distanceToLightSquared) - 2 * EPSILON;
		this->intersectPrimitives(shadowRay, true);

		if (shadowRay->intersectedObjectId == -1)
		{
			float solidAngle = CLAMP((lightNormalDotLightDirection * randomLight->getArea()) / distanceToLightSquared, 0, 1);
			directIlluminationColor = randomLight->color * randomLight->intensity * solidAngle * BRDF * primitiveNormalDotLightDirection;
		}
		delete shadowRay;
	}

	Ray* diffuseReflectionRay = this->computeDiffuseReflectionRay(ray);
	float PDF = PI / dot(primitiveNormal, diffuseReflectionRay->direction);  // Importance Sampling
	//float PDF = (2 * PI);

	vec4 indirectIlluminationColor = this->sample(diffuseReflectionRay, depth) * dot(primitiveNormal, diffuseReflectionRay->direction) * PDF * BRDF;
	delete diffuseReflectionRay;

	return directIlluminationColor + indirectIlluminationColor;
}

Ray* Scene::computeDiffuseReflectionRay(Ray* ray)
{
	vec3 hitPoint = ray->origin + ray->t * ray->direction;

	std::uniform_real_distribution<double> uniformGenerator01(0.0, 1.0);
	float random1 = uniformGenerator01(this->randomNumbersGenerator);
	float random2 = uniformGenerator01(this->randomNumbersGenerator);

	float angle = 2 * PI * random2;
	//float r = sqrt(1 - random1 * random1);
	//vec3 direction = vec3(cosf(angle) * r, sinf(angle) * r, random1);

	// Importance Sampling
	float r = sqrt(random1);
	vec3 direction = vec3(cosf(angle) * r, sinf(angle) * r, sqrt(1 - random1));

	if (dot(this->primitives[ray->intersectedObjectId]->getNormal(hitPoint), direction) < 0)
	{
		direction *= -1.0f;
	}

	return new Ray(hitPoint + direction * EPSILON, direction);
}

Ray* Scene::computeReflectionRay(Ray* ray)
{
	vec3 hitPoint = ray->origin + ray->t * ray->direction;
	vec3 N = this->primitives[ray->intersectedObjectId]->getNormal(hitPoint);

	vec3 direction = ray->direction - 2 * (ray->direction * N) * N;
	vec3 origin = hitPoint + direction * EPSILON;

	return new Ray(origin, direction);
}

Ray* Scene::computeRefractionRay(Ray* ray)
{
	// source: https://www.scratchapixel.com/lessons/3d-basic-rendering/introduction-to-shading/reflection-refraction-fresnel

	vec3 hitPoint = ray->origin + ray->t * ray->direction;
	vec3 N = this->primitives[ray->intersectedObjectId]->getNormal(hitPoint);
	float incommingAngle = dot(N, ray->direction);

	float cosi = CLAMP(-1, 1, incommingAngle);
	float etai = 1, etat = this->primitives[ray->intersectedObjectId]->material->refraction;
	vec3 n = N;
	if (cosi < 0) { cosi = -cosi; }
	else { swap(etai, etat); n = -N; }
	float eta = etai / etat;
	float k = 1 - eta * eta * (1 - cosi * cosi);

	if (k < 0)
	{
		Ray* refractionRay = new Ray(vec3(0), vec3(0));
		refractionRay->intersectedObjectId = -2;
		return refractionRay;
	}
	else
	{
		vec3 direction = eta * ray->direction + (eta * cosi - sqrtf(k)) * n;
		vec3 origin = hitPoint + direction * EPSILON;

		return new Ray(origin, direction);
	}
}

void Scene::intersectPrimitives(Ray* ray, bool isShadowRay)
{
	if (BVH_ENABLED)
	{
		this->topBHV->traverse(this->topBHV->root, ray, isShadowRay);
	}
	else
	{
		for (int i = 0; i < this->primitives.size(); i++)
		{
			this->primitives[i]->intersect(ray);
		}
	}
}

void Scene::intersectLightSources(Ray* ray)
{
	for (int i = 0; i < this->lightSources.size(); i++)
	{
		this->lightSources[i]->intersect(ray);
	}
}

Pixel Scene::convertColorToPixel(vec4 color)
{
	int r = (int)min(256.0f * BRIGHTNESS * sqrt(color.x), 255);
	int g = (int)min(256.0f * BRIGHTNESS * sqrt(color.y), 255);
	int b = (int)min(256.0f * BRIGHTNESS * sqrt(color.z), 255);

	return (r << 16) + (g << 8) + b;
}

void Scene::buildTopBVH()
{
	if (this->topBVHExists)
		delete this->topBHV;

	this->topBHV = new TopBVH(this->primitives, this->BVHs);
	this->topBVHExists = true;
}

int Scene::buildBVH(int startIndex, int endIndex)
{
	int id = this->BVHs.size();

	BVH* tree = new BVH(this->primitives);
	tree->build(id, startIndex, endIndex);
	this->BVHs.push_back(tree);
	this->buildTopBVH();

	return id;
}

int Scene::addPrimitive(Primitive* primitive)
{
	primitive->id = this->primitives.size();
	this->primitives.push_back(primitive);

	int modelId = this->buildBVH(primitive->id, primitive->id);

	this->models.push_back(
		new Model(modelId, primitive->id, primitive->id)
	);

	return modelId;
}

void Scene::addLightSource(LightSource* lightSource)
{
	lightSource->id = this->lightSources.size();
	this->lightSources.push_back(lightSource);
}

void Scene::clear()
{
	for (int i = 0; i < this->primitives.size(); i++)
	{
		delete this->primitives[i]->boundingBox;
		delete this->primitives[i];
	}
	this->primitives.clear();

	for (int i = 0; i < this->lightSources.size(); i++)
	{
		delete this->lightSources[i];
	}
	this->lightSources.clear();

	for (int i = 0; i < this->BVHs.size(); i++)
	{
		this->BVHs[i]->root->destroy();
		delete this->BVHs[i]->objectIndices;
		delete this->BVHs[i];
	}
	this->BVHs.clear();

	this->models.clear();

	if (this->skydomeLoaded)
	{
		delete[] this->skydome->buffer;
		delete this->skydome;
	}
	this->skydomeLoaded = false;

	this->resetAccumulator();
}

int Scene::getPrimitivesCount()
{
	return this->primitives.size();
}

int Scene::loadModel(const char *filename, Material* material, vec3 translationVector)
{
	// obj file content
	std::vector<vec3> vertices;
	std::vector<int> faceIndexes;
	std::vector<vec3> meshVertices;

	std::ifstream stream(filename, std::ios::in);
	if (!stream)
	{
		printf("Cannot load %s file!", filename);
	}

	std::string line;
	while (std::getline(stream, line))
	{
		if (line.substr(0, 2) == "v ")
		{
			std::istringstream v(line.substr(2));
			float x, y, z;
			v >> x; v >> y; v >> z;

			vertices.push_back(vec3(x, y, z));
		}
		else if (line.substr(0, 2) == "vt")
		{
			// TODO: implement textures support
		}
		else if (line.substr(0, 2) == "f ")
		{
			std::istringstream v(line.substr(2));
			int a, b, c;
			v >> a; v >> b; v >> c;

			faceIndexes.push_back(--a);
			faceIndexes.push_back(--b);
			faceIndexes.push_back(--c);
		}
	}

	// calculate mesh vertices
	for (unsigned int i = 0; i < faceIndexes.size(); i++)
	{
		meshVertices.push_back(
			vec3(vertices[faceIndexes[i]].x, vertices[faceIndexes[i]].y, vertices[faceIndexes[i]].z) + translationVector
		);
	}

	// add triangles to the scene
	int primitivesCount = meshVertices.size() / 3;
	int startIndex = this->primitives.size();
	for (int i = 0; i < primitivesCount; i++)
	{
		vec3 a = meshVertices[i * 3];
		vec3 b = meshVertices[i * 3 + 1];
		vec3 c = meshVertices[i * 3 + 2];

		Triangle* triangle = new Triangle(material, a, b, c);
		triangle->id = this->primitives.size();
		this->primitives.push_back(triangle);
	}
	int endIndex = this->primitives.size() - 1;

	int modelId = this->buildBVH(startIndex, endIndex);
	this->models.push_back(
		new Model(modelId, startIndex, endIndex)
	);

	return modelId;
}

void Scene::translateModel(int id, vec3 vector)
{
	// find model by id
	Model* model;
	for (int i = 0; i < this->models.size(); i++)
	{
		if (this->models[i]->id == id) model = this->models[i];
	}

	// find BVH by id
	BVH* bvh;
	for (int i = 0; i < this->BVHs.size(); i++)
	{
		if (this->BVHs[i]->id == id) bvh = this->BVHs[i];
	}

	if (model == NULL || bvh == NULL) return;

	// translate model
	for (int i = model->startIndex; i <= model->endIndex; i++)
	{
		this->primitives[i]->translate(vector);
	}

	// translate bvh
	bvh->root->translate(vector);

	this->buildTopBVH();
}

void Scene::loadSkydome(const char* fileName)
{
	this->skydome = new HDRBitmap(fileName);
	this->skydomeLoaded = true;
}