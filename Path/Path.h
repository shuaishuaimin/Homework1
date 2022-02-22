#pragma once
#include "../stdafx.h"

class Path
{
public:
	Path(){}
	Path(const Path& pt) = delete;
	Path& operator= (const Path& pt) = delete;
	virtual ~Path(){}

	static std::string ProjectDir();
};