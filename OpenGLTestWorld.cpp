#include "OpenGLTestWorld.h"

#include "Material.h"
#include <iostream>
#include "Vertices.h"
#include "ScreenClearer.h"
#include "RenderingState.h"
#include "RenderDataHandler.h"
#include "TextureSettings.h"

#include "BasicGenerators.h"
#include "Terrain.h"
#include "NoiseFilterer.h"
#include "Interpolator.h"
#include "Perlin.h"
#include "LayeredOctave.h"

#include <assert.h>


namespace OGLTestPrints
{
	bool PrintRenderError(const char * errorIntro)
{
	const char * error = GetCurrentRenderingError();
	if (strcmp(error, "") != 0)
	{
		std::cout << errorIntro << ": " << error << "\n";
		ClearAllRenderingErrors();
		return false;
	}

	return true;
}

	void Pause(void)
	{
		char dummy;
		std::cin >> dummy;
	}
}
using namespace OGLTestPrints;


const Vector2i windowSize(1000, 700);

const RenderingState worldRenderState(true, true, true);


sf::Image img;
BufferObjHandle imgObj;

const int terrainSize = 300;
const float terrainBreadth = 2.0f, terrainHeight = 60.0f;
const float terrainTexScale = 0.65f;
void GenerateTerrainNoise(Noise2D & outNoise)
{
	Generator * finalG = 0;

	//WhiteNoise whiteNoise;
	//Interpolator interp(&whiteNoise, terrainSize, terrainSize, 5.0f);
	//finalG = &interp;

	Perlin per(60.0f, Perlin::Smoothness::Quintic);
	Perlin per2(30.0f, Perlin::Smoothness::Cubic);
	Perlin per3(5.0f, Perlin::Smoothness::Linear);

	float weights[3] = { 0.75f, 0.25f, 0.0425f };
	Generator * gens[3] = { &per, &per2, &per3 };
	LayeredOctave lay(3, weights, gens);
	finalG = &lay;



	if (finalG != 0) finalG->Generate(outNoise);
}


bool ShouldUseFramebuffer(void) { return sf::Keyboard::isKeyPressed(sf::Keyboard::Key::LShift); }


OpenGLTestWorld::OpenGLTestWorld(void)
	: SFMLOpenGLWorld(windowSize.x, windowSize.y), testMat(0), testMesh(PrimitiveTypes::Triangles), foliage(0)
{
	dirLight.Dir = Vector3f(1.0f, 1.0f, -1.0f).Normalized();
	dirLight.Col = Vector3f(1.0f, 1.0f, 1.0f);

	dirLight.Ambient = 0.1f;
	dirLight.Diffuse = 0.8f;
	dirLight.Specular = 0.0f;
	
	dirLight.SpecularIntensity = 128.0f;
}
void OpenGLTestWorld::InitializeWorld(void)
{
	SFMLOpenGLWorld::InitializeWorld();
	if (IsGameOver()) return;
	
	GetWindow()->setVerticalSyncEnabled(true);
	GetWindow()->setMouseCursorVisible(true);


	if (!img.loadFromFile("Grass.png"))
	{
		std::cout << "Failed to load grass texture.\n";
		Pause();
		EndWorld();
		return;
	}
	RenderDataHandler::CreateTexture2D(imgObj, img);
	
	TextureSettings(TextureSettings::TextureFiltering::TF_LINEAR, TextureSettings::TextureWrapping::TW_WRAP).SetData(imgObj);
	if (!PrintRenderError("Error setting up grass texture"))
	{
		Pause();
		EndWorld();
		return;
	}

	sf::Image otherImg;
	BufferObjHandle otherImgH;
	if (!otherImg.loadFromFile("Brick.png"))
	{
		std::cout << "Failed to load brick texture.\n";
		Pause();
		EndWorld();
		return;
	}
	RenderDataHandler::CreateTexture2D(otherImgH, otherImg);

	TextureSettings(TextureSettings::TextureFiltering::TF_LINEAR, TextureSettings::TextureWrapping::TW_WRAP).SetData(otherImgH);
	if (!PrintRenderError("Error setting up brick texture"))
	{
		Pause();
		EndWorld();
		return;
	}



	worldRenderState.EnableState();
	if (!PrintRenderError("Error setting up render state"))
	{
		Pause();
		EndWorld();
		return;
	}
	

	//Camera.
	Vector3f pos(-100, -100, terrainHeight);
	cam = MovingCamera(pos, 80.0f, 0.05f, -pos);
	cam.Info.FOV = ToRadian(55.0f);
	cam.Info.zFar = 10000.0f;
	cam.Info.zNear = 1.0f;
	cam.Info.Width = windowSize.x;
	cam.Info.Height = windowSize.y;
	cam.Window = GetWindow();



	testMat = new Material(Materials::LitTexture.VertexShader, Materials::LitTexture.FragmentShader);
	if (testMat->HasError())
	{
		std::cout << "Lit Texture material error: " << testMat->GetErrorMessage() << "\n";
		Pause();
		EndWorld();
		return;
	}
	testMat->TextureSamplers[0] = imgObj;
	if (!Materials::LitTexture_GetUniforms(*testMat))
	{
		std::cout << "Lit texture directional light uniform get location error.\n";
	}
	if (!Materials::LitTexture_SetUniforms(*testMat, dirLight))
	{
		std::cout << "Lit texture directional light uniform set error.\n";
	}


	//Create vertices.

	BufferObjHandle vs, is;
	VertexIndexData vid;

	Noise2D noise(terrainSize, terrainSize);
	GenerateTerrainNoise(noise);

	Terrain terr(terrainSize);
	terr.SetHeightmap(noise);

	const int size = terrainSize * terrainSize;
	assert(size == terr.GetVerticesCount());
	Vertex * vertices = new Vertex[size];
	Vector3f * vertexPoses = new Vector3f[size], * vertexNormals = new Vector3f[size];
	Vector2f * vertexTexCoords = new Vector2f[size];
	int * indices = new int[terr.GetIndicesCount()];

	terr.CreateVertexPositions(vertexPoses, Vector2i(0, 0), Vector2i(terrainSize - 1, terrainSize - 1));
	for (int i = 0; i < size; ++i)
	{
		vertexPoses[i].x *= terrainBreadth;
		vertexPoses[i].y *= terrainBreadth;
		vertexPoses[i].z *= terrainHeight;
	}
	terr.CreateVertexNormals(vertexNormals, vertexPoses, Vector3f(1.0f, 1.0f, 1.0f), Vector2i(0, 0), Vector2i(terrainSize - 1, terrainSize - 1));
	terr.CreateVertexTexCoords(vertexTexCoords, Vector2f(terrainTexScale, terrainTexScale), Vector2i(0, 0), Vector2i(terrainSize - 1, terrainSize - 1));
	terr.CreateVertexIndices(indices, Vector2i(0, 0), Vector2i(terrainSize - 1, terrainSize - 1));

	for (int i = 0; i < size; ++i)
	{
		vertices[i] = Vertex(vertexPoses[i], vertexTexCoords[i], vertexNormals[i]);
	}

	RenderDataHandler::CreateVertexBuffer(vs, vertices, size, RenderDataHandler::BufferPurpose::UPDATE_ONCE_AND_DRAW);
	RenderDataHandler::CreateIndexBuffer(is, indices, terr.GetIndicesCount(), RenderDataHandler::BufferPurpose::UPDATE_ONCE_AND_DRAW);
	vid = VertexIndexData(size, vs, terr.GetIndicesCount(), is);


	if (!PrintRenderError("Error creating vertex/index buffer for terrain"))
	{
		Pause();
		EndWorld();
		return;
	}

	delete[] vertices, vertexNormals, vertexTexCoords, indices;

	//Create the foliage before deleting the vertex data.
	std::vector<Vector3f> poses;
	for (int i = 0; i < size; i += size)
	{
		poses.insert(poses.end(), vertexPoses[i]);
	}
	foliage = new Foliage(poses, 2.0f);
	foliage->SetFoliageTextureUnit(1);
	foliage->SetFoliageTexture(otherImgH);
	if (foliage->HasRenderError())
	{
		std::cout << "Error creating foliage: " << foliage->GetRenderError();
		Pause();
		EndWorld();
		return;
	}
	
	delete[] vertexPoses;


	//Create mesh.
	testMesh = Mesh(PrimitiveTypes::Triangles, 1, &vid);


	//Create render target.
	ClearAllRenderingErrors();
	rend = new RenderTarget(windowSize.x, windowSize.y);
	if (!rend->IsValid())
	{
		std::cout << "Error creating render target: " << rend->GetErrorMessage() << "\n";
		Pause();
		EndWorld();
		return;
	}
	rend->TextureSlot = 0;
}

OpenGLTestWorld::~OpenGLTestWorld(void)
{
	DeleteAndSetToNull(rend);
	DeleteAndSetToNull(testMat);
	DeleteAndSetToNull(foliage);
}
void OpenGLTestWorld::OnWorldEnd(void)
{
	DeleteAndSetToNull(rend);
	DeleteAndSetToNull(testMat);
	DeleteAndSetToNull(foliage);
}

void OpenGLTestWorld::OnInitializeError(std::string errorMsg)
{
	EndWorld();

	SFMLOpenGLWorld::OnInitializeError(errorMsg);

	std::cout << "Enter any key to continue:\n";
	Pause();
}


void OpenGLTestWorld::UpdateWorld(float elapsedSeconds)
{
	if (cam.Update(elapsedSeconds))
	{
		EndWorld();
	}
}

void OpenGLTestWorld::RenderWorldGeometry(const RenderInfo & info)
{
	Material::InitializeMaterialDrawing();

	std::vector<const Mesh *> meshes;
	meshes.insert(meshes.begin(), &testMesh);
	if (!testMat->Render(info, meshes))
	{
		std::cout << "Error rendering world: " << testMat->GetErrorMessage() << "\n";
		Pause();
		EndWorld();
	}

	Material::EndMaterialDrawing();


	if (IsGameOver()) return;
	

	//Foliage::StartFoliageRendering(true);

	//if (!foliage->Render(info))
	//{
	//	std::cout << "Error rendering foliage: " << foliage->GetRenderError() << "\n";
	//	Pause();
	//	EndWorld();
	//}

	//Foliage::EndFoliageRendering(true);
}

void OpenGLTestWorld::RenderWorld(float elapsedSeconds)
{
	Matrix4f worldM, viewM, projM;
	TransformObject dummy;

	worldM.SetAsIdentity();
	cam.GetViewTransform(viewM);
	projM.SetAsPerspProj(cam.Info);

	RenderInfo info((SFMLOpenGLWorld*)this, (Camera*)&cam, &dummy, &worldM, &viewM, &projM);
	
	bool should = ShouldUseFramebuffer();
	if (should)
	{
		rend->EnableDrawingInto();
		//if (!rend->IsValid())
		//{
		//	std::cout << "Error setting up render target.\n";
		//	Pause();
		//	EndWorld();
		//	return;
		//}
	}
	
	worldRenderState.EnableState();
	ScreenClearer().ClearScreen();
	if (!PrintRenderError("Error clearing screen of color and depth for render buffer"))
	{
		Pause();
		EndWorld();
		return;
	}
	RenderWorldGeometry(info);

	if (should)
	{
		rend->DisableDrawingInto();
		//if (!rend->IsValid())
		//{
		//	std::cout << "Error setting up render target.\n";
		//	Pause();
		//	EndWorld();
		//	return;
		//}
	
		ScreenClearer().ClearScreen();
		rend->Draw();
		if (!rend->IsValid())
		{
			std::cout << "Error setting up render target.\n";
			Pause();
			EndWorld();
			return;
		}
	}

	GetWindow()->display();
}


void OpenGLTestWorld::OnWindowResized(unsigned int newW, unsigned int newH)
{
	ClearAllRenderingErrors();

	glViewport(0, 0, newW, newH);
	if (!PrintRenderError("Error updating the OpenGL viewport size"))
	{
		return;
	}

	rend->ChangeSize(newW, newH);
	if (!rend->IsValid())
	{
		std::cout << "Error changing render target size: " << rend->GetErrorMessage() << "\n";
		Pause();
		EndWorld();
		return;
	}

	cam.Info.Width = newW;
	cam.Info.Height = newH;
}