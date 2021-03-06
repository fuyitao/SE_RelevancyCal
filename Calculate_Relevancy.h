#ifndef _CALCULATERELEVANCY__h
#define _CALCULATERELEVANCY__h

#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <pthread.h>
#include <time.h>
#include "tool_function.h"
using namespace std;

#define unmap unordered_map

class CalculateRelevancy;

// 获取权重字典多线程结构体
typedef struct WeightThreadType{
	int local;
	string mode;
	CalculateRelevancy * point;
} WeightThreadType;
// id得分结构体
typedef struct IdScoreType{
	int is_source_name;     // 是否命中来源词
	int title_match_num;    // 题目匹配term数量
	int content_match_num;  // 内容匹配term数量
	double title_score;     // 题目得分
	double content_score;   // 内容得分
	string timestamp;       // 时间戳
} IdScoreType;
// 获取id得分多线程结构体
typedef struct IdScoreThreadType{
	int local;                           // 第几个线程
	vector<string> * idVec;              // id vector
	vector< vector<string> > * termVec;  // term vector
	CalculateRelevancy * point;          // 类指针
	string sourcename;                   // 来源词
	int topN;                            // 排序topN

	vector< pair<string, string> > id_scoreStrVec;  // id得分vector
} IdScoreThreadType;
// 相关性结构体
typedef struct RelevancyType{
	vector< pair<string, string> > id_scoreStrVec;  // id对应相关性得分串
} RelevancyType;

class CalculateRelevancy{
public:
	CalculateRelevancy(string r_titleweightFile, string r_contentweightFile, string r_id_sourcenameFile, string r_timestampFile);
	~CalculateRelevancy();
	void GetScoreList(string r_termline,
					vector<string>& r_idvec,
					RelevancyType& r_relStruct,
					int r_topN,
					string r_sourcename);  // 相关性得分
private:
	int m_thread_num;  // 多线程数量
	void Analysis_Line(string r_termline, vector< vector<string> >& r_termVec);  // 对term串进行解析
	//****字典****//
	vector< unmap<string, unmap<string, double> > > m_titleweight_map_vec;  // 每个id下term的title得分(多线程)
	vector< unmap<string, unmap<string, double> > > m_contentweight_map_vec;  // 每个id下term的content得分(多线程)
	unmap<string, string> m_id_sourcename_map;  // id 来源字典
	unmap<string, string> m_timestamp_map;      // id 时间戳字典
	vector<string> m_line_vec;
	void GetMap(string r_titleweightFile,
				string r_contentweightFile,
				string r_idsourcenameFile,
				string r_timestamp);  // 获取字典
	void GetMapThread(string r_weightFile, string r_mode);  // 使用多线程获取字典
	static void * Get_TermWeightMap_Thread(void * dat);  // 多线程获取term权重字典
	////////////////
	
	//**计算得分**//
	static void * Get_IdScore_Thread(void * dat);  // 多线程计算id得分	
	double Calculate_Score(string r_id,
						vector< vector<string> >& r_termVec,
						vector< unmap<string, unmap<string, double> > >& weight_map_vec,
						int& term_match_num);  // 计算相关性得分
	double GetWeight(string r_word, unmap<string, double>& r_weight_map);  // 获取word的权重值
	string ScoreJoinStr(IdScoreType& scoreType);  // 将相关性得分拼接成串
	////////////////
};

#endif
