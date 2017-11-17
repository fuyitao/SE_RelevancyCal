#include "tool_function.h"

// 字符串分割
void splitString(const string& src, string sep_char, vector<string>& sep_vec){
	sep_vec.clear();
	int sep_charLen = sep_char.size();
	int lastPos = 0, index = -1;
	string word;
	while (-1 != (index = src.find(sep_char, lastPos))){
		word = src.substr(lastPos, index-lastPos);
		if ( !word.empty() ) sep_vec.push_back(word);
		lastPos = index + sep_charLen;
	}
	string lastString = src.substr(lastPos);  // 截取最后一个分隔符后的内容
	if ( !lastString.empty() ) sep_vec.push_back(lastString);
	return;
} // splitString

// 排序(调用快速排序)
// 排序比较
bool cmp(string a, string b){
	return a.compare(b) >= 0;
}
// 替换
void swap(vector< pair<string, string> >& arry, int localA, int localB){
	pair<string, string> tmp;
	tmp = arry[localA];
	arry[localA] = arry[localB];
	arry[localB] = tmp;
	return;
}

// 堆排序
void HeapAdjust(vector< pair<string, string> >& arry, int s, int m){
	pair<string, string> temp = arry[s];
	for (int i = s*2; i <= m; i *= 2){
		if ( i < m && cmp(arry[i+1].second, arry[i].second) ) ++i;
		if ( cmp(temp.second, arry[i].second) ) break;
		arry[s] = arry[i];
		s = i;
	}
	arry[s] = temp;
} // HeapAdjust

void HeapSort(vector< pair<string, string> >& arry, int topn){
	int length = arry.size();
	if (topn <= 0) return;
	for (int i = (length-1)/2; i >= 0; --i) HeapAdjust(arry, i, length-1);

	for (int j = length-1; j >= (length-topn) && j >= 0; --j){
		swap(arry, 0, j);
		HeapAdjust(arry, 0, j-1);
	}
	int eraseLen = (int)arry.size()>topn ? topn : (int)arry.size();
	arry.erase(arry.begin(), arry.end()-eraseLen);
	reverse(arry.begin(), arry.end());
	return;
}

// 排序(调用快速排序)
//// 每次排序获取位置
//int Partition(vector< pair<string, string> >& arry, int low, int high){
//	pair<string, string> key = arry[low];
//	while (low < high){
//		while (low < high && cmp(key.second, arry[high].second)) --high;
//		swap(arry, low, high);
//		while (low < high && cmp(arry[low].second, key.second)) ++low;
//		swap(arry, low, high);
//	}
//	return low;
//}
//// 快速排序
//void QuickSort(vector< pair<string, string> >& arry, int low, int high){
//	if (low < high){
//		int q = Partition(arry, low, high);
//		QuickSort(arry, low, q-1);
//		QuickSort(arry, q+1, high);
//	}
//	return;
//} // QuickSort

//void Sort(vector< pair<string, string> >& arry){
//	QuickSort(arry, 0, arry.size()-1);
//	return;
//} // Sort

// 十进制转十六进制
string Decimal2Hex(string decimal){
	unsigned int n = 0;
	// 将字符串转换成int类型
	for (int i = decimal.length() - 1, j = 1; i >= 0; --i){
		n += (decimal[i] - '0') * j;
		j *= 10;
	}
	// 运用短除法不断除以16取余数，并将其加入字符串结果中
	string hex = "";
	char _16[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};
	// 16进制，除以16
	const int radix = 16;
	while (n){
		int i = n % radix;  // 余数
		hex = _16[i] + hex;  // 将余数对应的十六进制数字加入结果
		n /= radix;
	}

	return hex;
} // Decimal2Hex
