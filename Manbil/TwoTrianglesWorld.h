#pragma once

#include "sfmlopenglworld.h"
#include "Material.h"
#include "Rendering/Materials/MaterialData.h"
#include "Rendering/Helper Classes/DrawingQuad.h"


//TODO: Add DOF to raymarchers by slightly rotating the ray at every iteration by a random amount.
//TODO: Add ambient occlusion to raymarchers based on the number of steps taken to destination.
//TODO: Move TwoTriangles files into "Toys" folder to match filter.

//A world that draws a quad using a fragment shader specified by the user.
//Refer to "QuadWorld shader constants.txt" for more info.
class TwoTrianglesWorld : public SFMLOpenGLWorld
{
public:

    static const std::string CustomSamplerName, NoiseSamplerName, ShaderElapsedName;

	TwoTrianglesWorld(void);
	~TwoTrianglesWorld(void);

protected:

	virtual void InitializeWorld(void) override;

	virtual void UpdateWorld(float elapsedSeconds) override;
	virtual void RenderOpenGL(float elapsedSeconds) override;

	virtual void OnInitializeError(std::string errorMsg) override;

	virtual void OnWindowResized(unsigned int newWidth, unsigned int newHeight) override;

    virtual void OnWorldEnd(void) override;

	virtual void OnOtherWindowEvent(sf::Event & event) override;
};