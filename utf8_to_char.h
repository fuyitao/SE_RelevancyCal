#ifndef _UTF8TOCHAR__h
#define _UTF8TOCHAR__h

#include <string>
#include <vector>
#include <ctype.h>
#include <string.h>
using namespace std;

#define CHAR_TYPE_ERR 1
#define CHAR_TYPE_OK  2

int clean_utf8(char * s);                                    // 字符串清洗
int char_type(char* s, int len, int* char_len, char* type);  // 判断正常字符
int utf8_to_charvec(const string& input, vector<string>& output);    // utf8编码切成单字
int utf8_to_len(const string& input);                               // utf8编码单字长度

# endif
