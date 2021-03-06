#pragma once
#include "d3dApp.h"
#include "DataStruct.h"

class DataHandler
{
public:
	DataHandler();

	DataHandler(const DataHandler& datahandler) = delete;
	DataHandler& operator=(const DataHandler& datahandler) = delete;
	virtual ~DataHandler();

	static void LoadMesh(const std::string& FileName, Charalotte::FMeshInfoForPrint& MeshInfo);

	static void LoadActors(const std::string& FileName, Charalotte::FActorsInfoForPrint& ActorInfos);
};

