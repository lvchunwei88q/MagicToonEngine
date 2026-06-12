#pragma once
#include <Object.h>
#include <test1.generate.h>

#define XXX_API

MCLASS(MSERIALIZATION);
class XXX_API CLASS_F : public Core::Object
{
	GENERATE_BODY(CLASS_F, ENGINE,"TEST");
public:

	MMEMBER();
	float acsa;
};