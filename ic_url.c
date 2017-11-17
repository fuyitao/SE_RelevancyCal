#include "ic_url.h"
////////////////////////////////////////////////////
//callback
static size_t WriteMemoryCallBack(void * cont, size_t size, size_t nmemb, void * userp) {
	size_t realsize = size * nmemb;
	struct MemoryStruct * mem = (struct MemoryStruct *)userp;
	//////////
	mem->mem = (char *)realloc(mem->mem, mem->size + realsize + 1);
	if(mem->mem == NULL) {
		return 0;
	}
	//
	memcpy(&(mem->mem[mem->size]), cont, realsize);
	mem->size += realsize;
	mem->mem[mem->size] = 0;
	//
	return realsize;
}

char * get_url_contents(char * url) {
	CURL * curl_handler;
	CURLcode res;
	///////////
	struct MemoryStruct chunk;
	chunk.mem = (char *)malloc(1);
	chunk.size = 0;
	/////////////////////////////////////////////
	//curl_global_init(CURL_GLOBAL_ALL);
	curl_handler = curl_easy_init();
	curl_easy_setopt(curl_handler, CURLOPT_URL, url);
	curl_easy_setopt(curl_handler, CURLOPT_WRITEFUNCTION, WriteMemoryCallBack);
	curl_easy_setopt(curl_handler, CURLOPT_WRITEDATA, (void *)&chunk);
	curl_easy_setopt(curl_handler, CURLOPT_USERAGENT, "libcurl-agent/1.0");
	curl_easy_setopt(curl_handler, CURLOPT_NOSIGNAL, 1);
	//
	res = curl_easy_perform(curl_handler);
	//
	curl_easy_cleanup(curl_handler);
	//curl_global_cleanup();
	//
	if(res != CURLE_OK) {
		if(chunk.mem) {
			free(chunk.mem);
		}
		return 0;
	}
	//
	return chunk.mem;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/*
int main(int argc, char * argv[]) {
	char * url = argv[1];
	char * cont = get_url_contents(url);
	/////////
	printf("%d\n%s\n", strlen(cont), cont);
	//
	free(cont);
}
*/
