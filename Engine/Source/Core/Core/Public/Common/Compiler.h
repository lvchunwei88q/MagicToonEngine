#pragma once

/*
* This is used to control the compiler (MSVC) compilation settings.
* Note that you should not use the tools here casually unless you know what you are doing.
*/

#define BEGIN_PIMPL __pragma(warning(push)) __pragma(warning(disable: 4251))
#define END_PIMPL __pragma(warning(pop))