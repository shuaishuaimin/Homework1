#pragma once
#include "d3dapp.h"
#include "DataStruct.h"

class Camera
{
public:
	Camera(const Charalotte::CameraData& Data);
	~Camera();

	void GetMVPTransform(DirectX::XMMATRIX& Matrix);
	void TransformCamera(const Charalotte::CameraTransform& Transform, DirectX::XMMATRIX& NewMvpMatrix);
	void ChangeAspectRatio(float NewAspectRatio);
	void ChangeFovAngle(float NewFovAngle);
	void AddFovAngle(float AngleForAdd);

	void GetCameraData(Charalotte::CameraData& Data);

protected:
	void CalcMVPMatrix();

private:
	DirectX::XMMATRIX MVPMatrix;
	Charalotte::MVPTransformData cameraTransform;
	Charalotte::CameraData MainCameraData;

	DirectX::XMFLOAT4X4 mWorld = MathHelper::Identity4x4();
	DirectX::XMFLOAT4X4 mView = MathHelper::Identity4x4();
	DirectX::XMFLOAT4X4 mProj = MathHelper::Identity4x4();
	DirectX::XMFLOAT4X4 mMVP = MathHelper::Identity4x4();

	float Sensitivity;
};