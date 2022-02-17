#pragma once
#include "d3dApp.h"
#include "MathHelper.h"
#include "UploadBuffer.h"


struct Vertex
{
	DirectX::XMFLOAT3 Pos;
	DirectX::XMFLOAT4 Color;
};

struct ObjectConstants
{
	DirectX::XMFLOAT4X4 WorldViewProj = MathHelper::Identity4x4();
};

class DrawMatrix : D3DApp
{
public:
	DrawMatrix(HINSTANCE hInstance);
	~DrawMatrix();

	virtual bool Initialize()override;

private:
	virtual void OnResize()override;
	virtual void Update(const GameTimer& gt)override;
	virtual void Draw(const GameTimer& gt)override;
};