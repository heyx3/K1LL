#pragma once

class ProjectionInfo
{
public:

    //FOV is measured in radians, but "SetFOVDegrees" can be used to specify it in degrees.
    float FOV;
    float Width; 
    float Height;
    float zNear;
    float zFar;

	ProjectionInfo(void) : FOV(2.3561945f), Width(10.0f), Height(10.0f), zNear(1.0f), zFar(1000.0f) { }
	ProjectionInfo(float fov, float width, float height, float _zNear, float _zFar) : FOV(fov), Width(width), Height(height), zNear(_zNear), zFar(_zFar) { }
	ProjectionInfo(const ProjectionInfo & cpy) : FOV(cpy.FOV), Width(cpy.Width), Height(cpy.Height), zNear(cpy.zNear), zFar(cpy.zFar) { }

    ProjectionInfo & operator=(const ProjectionInfo & cpy) { FOV = cpy.FOV; Width = cpy.Width; Height = cpy.Height; zNear = cpy.zNear; zFar = cpy.zFar; return *this; }

	float GetAspectRatio(void) const { return Width / Height; }

    void SetFOVDegrees(float degrees) { FOV = degrees * 0.0174532925f; }
    void SetFOVRadians(float radians) { FOV = radians; }
};