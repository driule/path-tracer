#include "precomp.h" // include (only) this in every .cpp file

int frame, sceneId, movingModelId;
float cameraSpeed = 0.2;
timer _timer;

RayTracerJob** rayTracerJobs;
JobManager* jobManager;

Scene* scene;

void RayTracerJob::Main()
{
	for (uint i = start; i < end; i++)
	{
		scene->render(i);
	}
}

// -----------------------------------------------------------
// Initialize the application
// -----------------------------------------------------------
void Game::Init()
{
	printf("--------------------------------------------------\n");
	printf("Application controls:\n");
	printf("Arrows, '+', '-' to move camera around\n");
	printf("WASD to rotate camera\n");
	printf("Numpad '+' and '-' to change FOV\n");
	printf("R to reset camera\n");
	printf("C to print camera configuration\n");
	printf("Numpad 1, 2, 3 to toggle between scenes\n");
	printf("--------------------------------------------------\n");

	// initialize threads
	rayTracerJobs = new RayTracerJob*[SCRHEIGHT / 32];
	for (int i = 0; i < SCRHEIGHT / 32; i++)
	{
		rayTracerJobs[i] = new RayTracerJob(i * 32, (i + 1) * 32);
	}

	JobManager::CreateJobManager(4);
	jobManager = JobManager::GetJobManager();

	//create scene
	scene = new Scene(screen);
	//this->loadTeddy();
	this->loadNiceScene();
}

// -----------------------------------------------------------
// Close down application
// -----------------------------------------------------------
void Game::Shutdown()
{
}

// -----------------------------------------------------------
// Main application tick function
// -----------------------------------------------------------
void Game::Tick( float deltaTime )
{
	_timer.reset();
	screen->Clear(0);

	this->handleInput();

	// path tracer accumulator
	scene->increaseAccumulator();

	if (MULTITHREADING_ENABLED)
	{
		for (int i = 0; i < SCRHEIGHT / 32; i++)
		{
			jobManager->AddJob2(rayTracerJobs[i]);
		}
		jobManager->RunJobs();
	}
	else
	{
		for (uint i = 0; i < SCRHEIGHT; i++)
		{
			scene->render(i);
		}
	}

	// calculate frame
	frame++;

	if (frame == 100)
	{
		frame = 0;
	}

	// measure FPS
	char buffer[20];
	sprintf(buffer, "FPS: %f", 1000 / _timer.elapsed());
	screen->Print(buffer, 2, 2, 0xffffff);

	sprintf(buffer, "Primitives: %i", scene->getPrimitivesCount());
	screen->Print(buffer, 2, 12, 0xffffff);
}

void Game::handleInput()
{
	// move the camera
	bool cameraChanged = false;

	if (GetAsyncKeyState(VK_DOWN))
	{
		scene->camera->position -= scene->camera->up * cameraSpeed;
		scene->camera->viewDirection -= scene->camera->up * cameraSpeed;
		cameraChanged = true;
	}
	if (GetAsyncKeyState(VK_RIGHT))
	{
		scene->camera->position += scene->camera->right * cameraSpeed;
		scene->camera->viewDirection += scene->camera->right * cameraSpeed;
		cameraChanged = true;
	}
	if (GetAsyncKeyState(VK_UP))
	{
		scene->camera->position += scene->camera->up * cameraSpeed;
		scene->camera->viewDirection += scene->camera->up * cameraSpeed;
		cameraChanged = true;
	}
	if (GetAsyncKeyState(VK_LEFT))
	{
		scene->camera->position -= scene->camera->right * cameraSpeed;
		scene->camera->viewDirection -= scene->camera->right * cameraSpeed;
		cameraChanged = true;
	}
	if (GetAsyncKeyState(VK_OEM_PLUS))
	{
		scene->camera->position += scene->camera->viewDirectionNormalized * cameraSpeed;
		scene->camera->viewDirection += scene->camera->viewDirectionNormalized * cameraSpeed;
		cameraChanged = true;
	}
	if (GetAsyncKeyState(VK_OEM_MINUS))
	{
		scene->camera->position -= scene->camera->viewDirectionNormalized * cameraSpeed;
		scene->camera->viewDirection -= scene->camera->viewDirectionNormalized * cameraSpeed;
		cameraChanged = true;
	}

	// rotate the camera
	if (GetAsyncKeyState('A'))
	{
		scene->camera->viewDirection -= scene->camera->right * cameraSpeed;
		cameraChanged = true;
	}
	if (GetAsyncKeyState('D'))
	{
		scene->camera->viewDirection += scene->camera->right * cameraSpeed;
		cameraChanged = true;
	}
	if (GetAsyncKeyState('W'))
	{
		scene->camera->viewDirection += scene->camera->up * cameraSpeed;
		cameraChanged = true;
	}
	if (GetAsyncKeyState('S'))
	{
		scene->camera->viewDirection -= scene->camera->up * cameraSpeed;
		cameraChanged = true;
	}

	// change FOV
	if (GetAsyncKeyState(VK_ADD))
	{
		scene->camera->fieldOfView += 0.01;
		cameraChanged = true;
	}
	if (GetAsyncKeyState(VK_SUBTRACT))
	{
		scene->camera->fieldOfView -= 0.01;
		cameraChanged = true;
	}
	if (cameraChanged)
	{ 
		scene->resetAccumulator();
		scene->camera->calculateScreen();
	}

	// reset camera
	if (GetAsyncKeyState('R'))
	{
		scene->camera->reset();
	}

	// toggle scenes
	if (GetAsyncKeyState(VK_NUMPAD1))
	{
		frame = 0;
		this->loadNiceScene();
	}
	if (GetAsyncKeyState(VK_NUMPAD2))
	{
		frame = 0;
		this->loadTeddy();
	}
	if (GetAsyncKeyState(VK_NUMPAD3))
	{
		frame = 15;
		this->loadTeapot();
	}

	// print camera configuration
	if (GetAsyncKeyState('C'))
	{
		printf("Camera position: (%f, %f, %f) \n", scene->camera->position.x, scene->camera->position.y, scene->camera->position.z);
		printf("FOV: %f \n", scene->camera->fieldOfView);
		printf("UP: (%f, %f, %f) \n", scene->camera->up.x, scene->camera->up.y, scene->camera->up.z);
		printf("RIGHT: (%f, %f, %f) \n", scene->camera->right.x, scene->camera->right.y, scene->camera->right.z);
	}
}

void Game::loadNiceScene()
{
	// https://groups.csail.mit.edu/graphics/classes/6.837/F03/models/
	//movingModelId = scene->loadModel("assets/cube.obj", brownMaterial);

	// reset scene
	sceneId = 0;
	cameraSpeed = 1;
	scene->clear();
	scene->camera->reset();

	scene->camera->position = vec3(0, 10, -60);
	scene->camera->up = vec3(0, 0.9, 0.15);
	scene->camera->right = vec3(1, 0, 0);
	scene->camera->calculateScreen();

	//scene->loadSkydome("assets/skydome/city.hdr");

	// scene lights
	scene->addLightSource(new SphericalLight(vec3(-5, 30, -20), 6, vec4(1, 1, 1, 1), 1000));
	scene->addLightSource(new SphericalLight(vec3(15, 30, -20), 10, vec4(1, 1, 1, 1), 1000));

	// materials
	Material* floorMaterial = new Material(vec4(0.5, 0.5, 0.5, 1.0), diffuse);
	Material* whiteMaterial = new Material(vec4(1, 1, 1, 1), diffuse);
	Material* wallMaterial = new Material(vec4(0.09, 0.63, 0.52, 1), diffuse);

	Material* glassMaterial = new Material(vec4(0, 0, 0, 1), dielectric);
	glassMaterial->refraction = 1.33;
	glassMaterial->reflection = 0.1;

	Material* redGlassMaterial = new Material(vec4(1, 0, 0, 1), dielectric);
	redGlassMaterial->refraction = 1.1;
	redGlassMaterial->reflection = 0.8;

	Material* mirrorMaterial = new Material(vec4(0.75, 0.8, 0.7, 1), mirror);
	Material* orangeMaterial = new Material(vec4(0.95, 0.61, 0.07, 1), diffuse);
	Material* redMaterial = new Material(vec4(0.8, 0.21, 0.19, 1), diffuse);

	int roomWidth = 50;
	int roomHeigth = 50;

	// floor
	//scene->addPrimitive(new Triangle(floorMaterial, vec3(-roomWidth, -10, -100), vec3(-roomWidth, -10, 10), vec3(roomWidth, -10, 10)));
	//scene->addPrimitive(new Triangle(floorMaterial, vec3(roomWidth, -10, -100), vec3(-roomWidth, -10, -100), vec3(roomWidth, -10, 10)));
	scene->addPrimitive(new Plane(floorMaterial, vec3(roomWidth, -10, 10), vec3(0, 1, 0), 100));
	
	// walls
	scene->addPrimitive(new Triangle(mirrorMaterial, vec3(roomWidth, -roomHeigth, 10), vec3(-roomWidth, -roomHeigth, 10), vec3(roomWidth, roomHeigth, 10)));
	scene->addPrimitive(new Triangle(mirrorMaterial, vec3(-roomWidth, -roomHeigth, 10), vec3(-roomWidth, roomHeigth, 10), vec3(roomWidth, roomHeigth, 10)));

	/*scene->addPrimitive(new Triangle(wallMaterial, vec3(roomWidth, -roomHeigth, -100), vec3(roomWidth, -roomHeigth, 10), vec3(roomWidth, roomHeigth, 10)));
	scene->addPrimitive(new Triangle(wallMaterial, vec3(roomWidth, roomHeigth, -100), vec3(roomWidth, -roomHeigth, -100), vec3(roomWidth, roomHeigth, 10)));

	scene->addPrimitive(new Triangle(wallMaterial, vec3(-roomWidth, -roomHeigth, 10), vec3(-roomWidth, -roomHeigth, -100), vec3(-roomWidth, roomHeigth, 10)));
	scene->addPrimitive(new Triangle(wallMaterial, vec3(-roomWidth, -roomHeigth, -100), vec3(-roomWidth, roomHeigth, -100), vec3(-roomWidth, roomHeigth, 10)));

	scene->addPrimitive(new Triangle(wallMaterial, vec3(-roomWidth, -roomHeigth, -100), vec3(roomWidth, -roomHeigth, -100), vec3(roomWidth, roomHeigth, -100)));
	scene->addPrimitive(new Triangle(wallMaterial, vec3(-roomWidth, roomHeigth, -100), vec3(-roomWidth, -roomHeigth, -100), vec3(roomWidth, roomHeigth, -100)));
	//*/

	// teapots
	Material* brownMaterial = new Material(vec4(1, 0.8, 0.5, 0), diffuse);

	scene->loadModel("assets/teapot.obj", brownMaterial, vec3(-10, -7, -30));
	scene->addPrimitive(new Sphere(whiteMaterial, vec3(-10, -12, -30), 5));

	scene->loadModel("assets/teapot.obj", redGlassMaterial, vec3(EPSILON, -10, -30));

	scene->loadModel("assets/teapot.obj", brownMaterial, vec3(10, -7, -30));
	scene->addPrimitive(new Sphere(whiteMaterial, vec3(10, -12, -30), 5));

	// sphere with torus
	scene->addPrimitive(new Sphere(glassMaterial, vec3(0, -5, -10), 5));
	scene->addPrimitive(new Torus(orangeMaterial, 7, 1, vec3(0, -5, -10), vec3(-1, -1.5, 0)));

	// cylinders
	scene->addPrimitive(new Cylinder(redMaterial, vec3(-10, -5, -20), vec3(0, 1, 0), 0.5, 10));
	scene->addPrimitive(new Cylinder(glassMaterial, vec3(-13, -5, -22), vec3(0, 1, 0), 0.5, 10));
	scene->addPrimitive(new Cylinder(redMaterial, vec3(-16, -5, -24), vec3(0, 1, 0), 0.5, 10));
	scene->addPrimitive(new Cylinder(glassMaterial, vec3(-19, -5, -26), vec3(0, 1, 0), 0.5, 10));
	scene->addPrimitive(new Cylinder(redMaterial, vec3(-22, -5, -28), vec3(0, 1, 0), 0.5, 10));
	scene->addPrimitive(new Cylinder(glassMaterial, vec3(-25, -5, -30), vec3(0, 1, 0), 0.5, 10));

	scene->addPrimitive(new Cylinder(redMaterial, vec3(10, -5, -20), vec3(0, 1, 0), 0.5, 10));
	scene->addPrimitive(new Cylinder(glassMaterial, vec3(13, -5, -22), vec3(0, 1, 0), 0.5, 10));
	scene->addPrimitive(new Cylinder(redMaterial, vec3(16, -5, -24), vec3(0, 1, 0), 0.5, 10));
	scene->addPrimitive(new Cylinder(glassMaterial, vec3(19, -5, -26), vec3(0, 1, 0), 0.5, 10));
	scene->addPrimitive(new Cylinder(redMaterial, vec3(22, -5, -28), vec3(0, 1, 0), 0.5, 10));
	scene->addPrimitive(new Cylinder(glassMaterial, vec3(25, -5, -30), vec3(0, 1, 0), 0.5, 10));
}

void Game::loadTeddy()
{
	sceneId = 1;
	cameraSpeed = 1;
	scene->clear();

	scene->camera->reset();
	scene->camera->position = vec3(-32, 0, 40);
	scene->camera->up = vec3(0, 1, 0);
	scene->camera->right = vec3(-0.77, 0, -0.62);
	scene->camera->calculateScreen();

	scene->loadSkydome("assets/skydome/clouds.hdr");

	scene->addLightSource(new DirectLight(vec3(-10.0f, 0.0f, 20.0), vec4(1, 1, 1, 0), 250));

	Material* redMaterial = new Material(vec4(1, 0, 0, 0), diffuse);
	scene->addPrimitive(
		new Sphere(redMaterial, vec3(-25, 10, 0), 5)
	);

	Material* brownMaterial = new Material(vec4(1, 0.8, 0.5, 0), diffuse);
	for (int i = 0; i < 3; i++)
	{
		scene->loadModel("assets/teddy.obj", brownMaterial, vec3(i * 40, 0, 0));
	}
}

void Game::loadTeapot()
{
	sceneId = 2;
	cameraSpeed = 0.5;
	scene->clear();

	scene->camera->reset();
	scene->camera->position = vec3(-20, -0.013, 20);
	scene->camera->up = vec3(0, 1, 0);
	scene->camera->right = vec3(-0.921, 0, -0.387);
	scene->camera->calculateScreen();

	scene->loadSkydome("assets/skydome/clouds.hdr");

	scene->addLightSource(new DirectLight(vec3(-10.0f, 0.0f, 20.0), vec4(1, 1, 1, 0), 250));

	Material* brownMaterial = new Material(vec4(1, 0.8, 0.5, 0), diffuse);
	for (int i = 0; i < 10; i++)
	{
		scene->loadModel("assets/teapot.obj", brownMaterial, vec3(i * 7, 0, 0));
	}
}