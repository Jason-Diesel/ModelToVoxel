#pragma once
#include <stdint.h>

class Component {
public:
	Component();
	virtual ~Component();
	bool deleteInObject = true;
};