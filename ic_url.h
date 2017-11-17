//
#ifndef _IC_URL_H
#define _IC_URL_H
////////////////////////////
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <curl/curl.h>

#ifdef __cplusplus
extern "C"
{
#endif
//
///////////////////////////////////////////////////////
struct MemoryStruct {
	char * mem;
	size_t size;
};
/////////////////////////
//
char * get_url_contents(char * url);
//
#ifdef __cplusplus
}
#endif

#endif
