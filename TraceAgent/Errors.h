#pragma once

#include "jvmti.h"

class Errors {
public:
	void static Check(jvmtiError error);
	static char* GetErrorName(jvmtiError error);
};