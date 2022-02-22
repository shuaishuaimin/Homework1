#include "stdafx.h"
#include "DataHandler.h"
#include "Path/Path.h"

using std::ifstream;
using std::ios;
using std::cout;
using std::string;

DataHandler::DataHandler()
{

}

DataHandler::~DataHandler()
{

}

void DataHandler::LoadMesh(const std::string& FileName, FMeshInfoForPrint& MeshInfo)
{
	FMeshInfoForPrint& TempMeshInfo = MeshInfo;
	ifstream file("BinaryMeshFiles/" + FileName, ios::in|ios::binary);
	if (!file)
	{
		return;
	}
	int MeshNameLength;
	file.read((char*) &MeshNameLength, sizeof(int));

	string MeshName;
	MeshName.resize(MeshNameLength);
	file.read((char*)MeshName.data(), MeshNameLength);
	TempMeshInfo.MeshName = MeshName;

	int LODNum = 0;
	file.read((char*) &LODNum, sizeof(int));
	TempMeshInfo.LODNums = LODNum;

	int LODInfosSize = 0;
	file.read((char*)&LODInfosSize, sizeof(int));

	if (LODNum != LODInfosSize)
	{
		return;
	}

	// Due to the file format that we define,
	// we use this rule to get our file data
	for (int index = 0; index < LODInfosSize; index++)
	{
		FLODInfo lodinfo;
		file.read((char*)&(lodinfo.VerticesNum), sizeof(int));
		file.read((char*)&(lodinfo.TrianglesNum), sizeof(int));
		file.read((char*)&(lodinfo.IndicesNum), sizeof(int));

		int VerticesLocationnum = 0;
		file.read((char*)&VerticesLocationnum, sizeof(int));
		for (int VerIndex = 0; VerIndex < VerticesLocationnum; VerIndex++)
		{
			FVector VertexLocation;
			file.read((char*)&(VertexLocation.x), sizeof(float));
			file.read((char*)&(VertexLocation.y), sizeof(float));
			file.read((char*)&(VertexLocation.z), sizeof(float));
			lodinfo.VerticesLocation.push_back(VertexLocation);
		}
		int IndicesNum = 0;
		file.read((char*)&IndicesNum, sizeof(int));
		for (int InIndex = 0; InIndex < IndicesNum; InIndex++)
		{
			int VertexIndex;
			file.read((char*)&VertexIndex, sizeof(int));
			lodinfo.Indices.push_back(VertexIndex);
		}
		TempMeshInfo.LodInfos.push_back(lodinfo);
	//	TempMeshInfo.LodInfos 
	}
}