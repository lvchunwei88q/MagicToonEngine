#pragma once
#include <Object.h>
#include <test1.object.generate.h>

#define XXX_API

MCLASS(MSERIALIZATION);
class XXX_API CLASS_F : public MObject
{
	GENERATE_BODY(CLASS_F, ENGINE,"TEST");
public:

	MMEMBER();
	float acsa;
};