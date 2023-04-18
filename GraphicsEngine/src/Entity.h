#pragma once

#include "Platform.h"

class Entity
{
public:
	Entity();
	~Entity();

	void Render();

public:
	u32 modelID;
};