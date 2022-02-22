#pragma once
#include "d3dApp.h"
#include "MathHelper.h"
#include "UploadBuffer.h"
#include "DataStruct.h"

class DrawMatrix : public D3DApp
{
public:
	DrawMatrix(HINSTANCE hInstance);
	// ban copy constructor and assignment
	DrawMatrix(const DrawMatrix& dm) = delete;
	DrawMatrix operator= (const DrawMatrix& dm) = delete;
	~DrawMatrix();

	virtual bool Initialize()override;

private:
	virtual void OnResize()override;
	virtual void Update(const GameTimer& gt)override;
	virtual void Draw(const GameTimer& gt)override;

	// transform
	virtual void OnMouseDown(WPARAM btnState, int x, int y)override;
	virtual void OnMouseUp(WPARAM btnState, int x, int y)override;
	virtual void OnMouseMove(WPARAM btnState, int x, int y)override;

	// Help function
	void ConstructProjectionMatrix();

	// render pipeline
	void BuildDescriptorHeaps();
	void BulidConstantBuffers();
	void BuildRootSignature();
	void BuildShadersAndInputLayOut();
	void BuildBoxGeometry(const std::string& GeometryName = "", const FTransform& Transform = FTransform());
	void BuildEnviroument(const std::string& MapName = "");
	void BuildPSO();

private:

	Microsoft::WRL::ComPtr<ID3D12RootSignature> mRootSignature = nullptr;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> mCbvHeap = nullptr;

	std::unique_ptr<UploadBuffer<ObjectConstants>> mObjectCB = nullptr;

	std::unique_ptr<MeshGeometry> mMeshGeo = nullptr;
	std::vector<FMeshsInEnviroument> mEnviroument;
	
	Microsoft::WRL::ComPtr<ID3DBlob> mvsByteCode = nullptr;
	Microsoft::WRL::ComPtr<ID3DBlob> mpsByteCode = nullptr;

	std::vector<D3D12_INPUT_ELEMENT_DESC> mInputLayout;

	Microsoft::WRL::ComPtr<ID3D12PipelineState> mPSO = nullptr;

	DirectX::XMFLOAT4X4 mWorld = MathHelper::Identity4x4();
	DirectX::XMFLOAT4X4 mView = MathHelper::Identity4x4();
	DirectX::XMFLOAT4X4 mProj = MathHelper::Identity4x4();

	// this num we can change
	float mTheta = 1.5f * DirectX::XM_PI;	//??
	float mPhi = DirectX::XM_PIDIV4;
	float mRadius = 5.0f;

	float mSensitivity = 0.25f;

	std::vector<Vertex> vertices;
	std::vector<int16_t> indices;
	POINT mLastMousePos;
};