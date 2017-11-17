#ifndef _TOOLFUNCTION__h
#define _TOOLFUNCTION__h

#include <string>
#include <iostream>
#include <vector>
#include <algorithm>
using namespace std;

struct CmpByStrValue{
	bool operator()(const pair<string, double> &lhs, const pair<string, double> &rhs){
		return lhs.second > rhs.second;
	}
};

struct CmpByIntValue{
	bool operator()(const pair<int, double> &lhs, const pair<int, double> &rhs){
		return lhs.second > rhs.second;
	}
};

void splitString(const string& src, string seg_char, vector<string>& sep_vec);  // 字符串按sep_char分割
void Sort(vector< pair<string, string> >& arry);  // 排序(调用快速排序)
void HeapSort(vector< pair<string, string> >& arry, int topn);  // 堆排序
string Decimal2Hex(string decimal);               // 十进制转十六进制

#endif
