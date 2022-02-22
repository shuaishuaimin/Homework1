#pragma once
#include "stdafx.h"

struct Vertex
{
	DirectX::XMFLOAT3 Pos;
	DirectX::XMFLOAT4 Color;
};

struct ObjectConstants
{
	DirectX::XMFLOAT4X4 TransMatrix = MathHelper::Identity4x4();
};

struct FVector
{
public:

	/** Vector's X component. */
	float x = 0.0f;

	/** Vector's Y component. */
	float y = 0.0f;

	/** Vector's Z component. */
	float z = 0.0f;

	friend std::ostream& operator<<(std::ostream& os, const FVector& Vector)
	{
		os << " x: " << Vector.x << " y: " << Vector.y << " z: " << Vector.z;
		return os;
	}
};

struct FLODInfo
{

	int VerticesNum;

	int TrianglesNum;

	int IndicesNum;

	std::vector<FVector> VerticesLocation;

	std::vector<int> Indices;
};

struct FMeshInfoForPrint
{
	std::string MeshName;

	int LODNums;

	std::vector<FLODInfo> LodInfos;
};

struct FVector4
{
	float x = 0.0f;
	float y = 0.0f;
	float z = 0.0f;
	float w = 0.0f;

	friend std::ostream& operator<<(std::ostream& os, const FVector4& Vector4)
	{
		os << " x: " << Vector4.x << " y: " << Vector4.y << " z: " << Vector4.z << " w: " << Vector4.w;
		return os;
	}
};

struct FTransform
{
	FVector4 Rotation;
	FVector Translation;
	FVector Scale3D;
	friend std::ostream& operator << (std::ostream& os, const FTransform& TransForm)
	{
		os << "Rotation: " << TransForm.Rotation << "\n" << "Translation: " << TransForm.Translation << "\n" << "Scale3d: " << TransForm.Scale3D << "\n";
		return os;
	}
};

struct FActorInfo
{
	FTransform Transform;

	std::string AssetName;

	friend std::ostream& operator << (std::ostream& os, const FActorInfo& FActorInfo)
	{
		os << "Translation: " << "\n" << FActorInfo.Transform << "\n";
		{
			os << "Asset name:" << FActorInfo.AssetName << "\n";
		}
		return os;
	}
};

struct FActorsInfoForPrint
{
	std::vector<FActorInfo> ActorsInfo;
};

struct FMeshsInEnviroument
{
	std::string AssetName;
	std::unique_ptr<MeshGeometry> MeshGeometryIns = nullptr;
};