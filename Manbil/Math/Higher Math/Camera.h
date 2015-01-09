#pragma once

#include "../LowerMath.hpp"
#include "ProjectionInfo.h"


//Basic camera functionality: UVN rotation, transform calculation, etc.
//Has two modes, indicated by the "LockUp" flag:
//   - If "LockUp" is true, the camera's Up vector will not be affected by pitching,
//        and the Forward vector can only get so close to it (specified by "ClosestDotVariance").
//        Rolling the camera or rotating it with a Quaternion will still change the Up vector like normal.
//   - If "LockUP" is false, the camera's Up vector is not locked in any way.
//Can specify a "closest dot variance", which means the closest the "forward" vector can get
//     to pointing straight down/up, in terms of the dot product value.
class Camera
{
public:

    //If true, keeps the upward vector along the positive Z axis regardless of rotation.
    bool LockUp;
    //If "LockUp" is true, this field indicates the closest the camera can get to the up/down vector.
    //Should be between 0 (can look all the way up/down) and 1 (can't pitch the camera at all).
    float ClosestDotVariance;

    //The information about this camera's 3D perspective (field of view, z-near/far, and aspect ratio).
	ProjectionInfo PerspectiveInfo;
    //The information about this camera's 2D perspective (visible region, relative to the camera).
    Vector3f MinOrthoBounds, MaxOrthoBounds;


	Camera(Vector3f pos = Vector3f(0.0f, 0.0f, 0.0f),
           Vector3f forward = Vector3f(1.0f, 0.0f, 0.0f),
           Vector3f up = Vector3f(0.0f, 0.0f, 1.0f),
           bool lockUp = true);


	Vector3f GetPosition(void) const { return pos; }
	Vector3f GetForward(void) const { return forward; }
	Vector3f GetUpward(void) const { return up; }
	Vector3f GetSideways(void) const { return up.Cross(forward).Normalized(); }

	void SetPosition(Vector3f newPos) { pos = newPos; }
	void SetPositionX(float newX) { SetPosition(Vector3f(newX, pos.y, pos.z)); }
	void SetPositionY(float newY) { SetPosition(Vector3f(pos.x, newY, pos.z)); }
	void SetPositionZ(float newZ) { SetPosition(Vector3f(pos.x, pos.y, newZ)); }
	void IncrementPosition(Vector3f amount) { SetPosition(pos + amount); }
	void IncrementPositionX(float newX) { IncrementPosition(Vector3f(newX, 0.0f, 0.0f)); }
	void IncrementPositionY(float newY) { IncrementPosition(Vector3f(0.0f, newY, 0.0f)); }
	void IncrementPositionZ(float newZ) { IncrementPosition(Vector3f(0.0f, 0.0f, newZ)); }

	void SetRotation(Vector3f newForward, Vector3f newUp, bool alreadyNormalized);
    void Rotate(Quaternion rotation);
	void AddPitch(float radians);
	void AddYaw(float radians);
	void AddRoll(float radians);


	void GetViewTransform(Matrix4f & outM) const;
    void GetPerspectiveTransform(Matrix4f& outM) const;
    void GetOrthoProjection(Matrix4f & outM) const;
	

private:

	Vector3f pos, forward, up;
};