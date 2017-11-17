#include "utf8_to_char.h"

// 字符串清洗
int clean_utf8(char * s){
	int i = 0, j = 0;
	int len = strlen(s);
	int char_len = 0;
	char type = 0;
	int unicode = 0;
	for (j = 0; j < len;){
		unicode = char_type(s+j, len-j, &char_len, &type);
		if ( type == (char)CHAR_TYPE_OK ){
			for (int k = 0; k < char_len; ++k){
				s[i++] = s[j+k];
			}
		}else{
			s[i++] = ' ';
		}
		char_len = char_len > 0 ? char_len : 1;
		j += char_len;
	}
	s[i] = 0;
	return i;
} // clean_utf8

// 判断正常字符
int char_type(char * s, int len, int * char_len, char * type){
	int unicode = 0;
	char c = *s;
	*type = CHAR_TYPE_OK;
	if ( len < 1 ){
		*char_len = 0;
		*type = CHAR_TYPE_ERR;
		return 0;
	}
	if( (c&0x80) == 0x00 ){ /* ascii char */
		*char_len = 1;
		unicode = c;
		if( c<32 || c==127 ){ /* control char */
			*type = CHAR_TYPE_ERR;
		}
	}else if( (c&0xf0) == 0xe0 ){ /* maybe cn char, len 3 */
		if (len < 3){ /* incomplete code */
			*char_len = 1;
			*type = CHAR_TYPE_ERR;
		}else{
			if( ((s[1]&0xc0)!=0x80) || ((s[2]&0xc0)!=0x80) ){ /* damaged code */
				*char_len = 1;
				*type = CHAR_TYPE_ERR;
			}else{
				/* utf-8 to unicode */
				unicode  = (int)(s[0]&0x0f) << 12;
				unicode += (int)(s[1]&0x3f) << 6;
				unicode += (int)(s[2]&0x3f);
				*char_len = 3;
				/*
				3400: 13312
				4db5: 19893
				4dff: 19967

				4e00: 19968
				9fcb: 40907
				9fff: 40959
				*/
				if(((unicode>=13312)&&(unicode<=19893))||((unicode>=19968)&&(unicode<=40907))){ /* ch char, len 3 */
					*char_len = 3;
					*type = CHAR_TYPE_OK;
				}else{ /* other language such as Japanese or Korean */
					*char_len = 3;
					*type = CHAR_TYPE_ERR;
				}
			}
		}
	}else if( (c&0xf8) == 0xf0 ){ /* maybe cn char, len 4 */
		if(len < 4){ /* incomplete code */
			*char_len = 1;
			*type = CHAR_TYPE_ERR;
		}else{
			if( ((s[1]&0xc0)!=0x80) || ((s[2]&0xc0)!=0x80) || ((s[3]&0xc0)!=0x80) ){ /* damaged code */
				*char_len=1;
				*type = CHAR_TYPE_ERR;
			}else{
				/* utf-8 to unicode */
				unicode  = (int)(s[0]&0x07) << 15;
				unicode += (int)(s[1]&0x3f) << 12;
				unicode += (int)(s[2]&0x3f) << 6;
				unicode += (int)(s[3]&0x3f);
				*char_len=4;
				/*
				20000: 131072
				2a6d6: 173782
				2a6df: 173791
				*/
				if((unicode>=131072)&&(unicode<=173782)){ /* ch char, len 4 */
					*char_len = 4;
					*type = CHAR_TYPE_OK;
				}else{ /* other language such as Japanese or Korean */
					*char_len = 4;
					*type = CHAR_TYPE_ERR;
				}
			}
		}
	}else{ /* error code or chars I do not care, but char_len should be comfirmed */
		*type=CHAR_TYPE_ERR;
		if( (c&0xc0) == 0x80 ){ /* illegal char */
			*char_len = 1;
		}else if( (c&0xe0) == 0xc0 ){
			if( len < 2 ){ /* incomplete code */
				*char_len = 1;
			}else if( (s[1]&0xc0)==0x80 ){
				*char_len = 2;
			}else{
				*char_len = 1;
			}
		}else if( (c&0xfc) == 0xf8 ){
			if( len < 5 ){ /* incomplete code */
				*char_len = 1;
			}else if( ((s[1]&0xc0)==0x80) && ((s[2]&0xc0)==0x80) && ((s[3]&0xc0)==0x80) && ((s[4]&0xc0)==0x80) ){
				*char_len = 5;
			}else{ /* damaged code */
				*char_len = 1;
			}
		}else if( (c&0xfe) == 0xfc ){
			if( len < 6 ){ /* incomplete code */
				*char_len = 1;
			}else if( ((s[1]&0xc0)==0x80) && ((s[2]&0xc0)==0x80) && ((s[3]&0xc0)==0x80) && ((s[4]&0xc0)==0x80) && ((s[5]&0xc0)==0x80) ){
				*char_len=6;
			}else{ /* damaged code */
				*char_len = 1;
			}
		}else{
			*char_len = 1;
		}
	}
	return unicode;
}

// utf8编码切成单字
int utf8_to_charvec(const string& input, vector<string>& output){
	output.clear();
	string ch;
	for (int i = 0, len = 0; i != (int)input.length(); i += len){
		unsigned char byte = (unsigned)input[i];
		if (byte >= 0xFC) len = 6;
		else if (byte >= 0xF8) len = 5;
		else if (byte >= 0xF0) len = 4;
		else if (byte >= 0xE0) len = 3;
		else if (byte >= 0xC0) len = 2;
		else len = 1;
		ch = input.substr(i, len);
		output.push_back(ch);
	}
	return output.size();
}

// utf8编码单字长度
int utf8_to_len(const string& input){
	string ch;
	int strLen = 0;
	for (int i = 0, len = 0; i != (int)input.length(); i += len){
		unsigned char byte = (unsigned)input[i];
		if (byte >= 0xFC) len = 6;
		else if (byte >= 0xF8) len = 5;
		else if (byte >= 0xF0) len = 4;
		else if (byte >= 0xE0) len = 3;
		else if (byte >= 0xC0) len = 2;
		else len = 1;
		strLen += 1;
	}
	return strLen;
}
