#pragma once

#include "SFMLOpenGLWorld.h"
#include "Math/HigherMath.hpp"
#include "MovingCamera.h"
#include "Mesh.h"
#include "RenderTarget.h"
#include "Math/Higher Math/Lighting.h"
#include "Rendering/Helper Classes/DrawingQuad.h"


class OpenGLTestWorld : public SFMLOpenGLWorld
{
public:

	OpenGLTestWorld(void);
	~OpenGLTestWorld(void);

protected:

	virtual void InitializeWorld(void) override;

	virtual void UpdateWorld(float elapsedSeconds) override;

	virtual void RenderOpenGL(float elapsedSeconds) override;

	virtual void OnInitializeError(std::string errorMsg) override;

	virtual void OnWindowResized(unsigned int newWidth, unsigned int newHeight) override;

	virtual void OnWorldEnd(void) override;

private:

    void InitializeTextures(void);
    void InitializeMaterials(void);
    void InitializeObjects(void);

	void RenderWorldGeometry(const RenderInfo & info);

    Terrain * terr;
    Vector3f * terrPoses;
    Mesh terrMesh;

    Material * mat;

	MovingCamera cam;
	DirectionalLight dirLight;

    sf::Texture diffuseTex;
};