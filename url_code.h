#ifndef _URLCODE__h
#define _URLCODE__h

#include <string.h>
#include <stdlib.h>

char hex2dec(char ch);
char dec2hex(char ch);
char *urlencode(char* src);  // url 编码
char *urldecode(char* src);  // url 解码

#endif
