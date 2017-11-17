#include "Calculate_Relevancy_new.h"

pthread_mutex_t pre_mutex;

CalculateRelevancy::CalculateRelevancy(string r_weightFile){
	m_thread_num = 10;  // 线程数量
	GetMap(r_weightFile);
	int article = 0;
	for (int i = 0; i < (int)m_weight_map_vec.size(); ++i) article += m_weight_map_vec[i].size();
	cout << "article:" << article << endl;
} // CalculateRelevancy

CalculateRelevancy::~CalculateRelevancy(){
} // ~CalculateRelevancy

/************************************************
 * Function    : GetScoreList(主函数)
 * Description : 生成相关性的得分列表
 * Date        : 2017.8.14
 ***********************************************/
void CalculateRelevancy::GetScoreList(string r_termline, 
									vector<string>& r_idVec,
									RelevancyType& r_relStruct,
									int r_topN){
	r_relStruct.id_scoreStrVec.clear();  // 初始化
	if (r_idVec.size() == 0 || r_termline == "") return;
	vector< vector<string> > termVec;
	Analysis_Line(r_termline, termVec);  // 解析term字符串

	// 多线程获取id得分
	vector<pthread_t> pid_list(m_thread_num);
	vector<IdScoreThreadType> paraVec(m_thread_num);
	int local;

	// 每个thread获取起始和结束位置
	for (local = 0; local < m_thread_num; ++local){
		paraVec[local].local   = local;         // 第几个线程
		paraVec[local].idVec   = &r_idVec;      // id vector
		paraVec[local].termVec = &termVec;      // term vector
		paraVec[local].point   = this;          // 类指针
	} // end for(local)

	// 创建多线程
	for (local = 0; local < m_thread_num; ++local) pthread_create(&(pid_list[local]), NULL, Get_IdScore_Thread, &paraVec[local]);
	for (local = 0; local < m_thread_num; ++local) pthread_join(pid_list[local], NULL);

	// 结果合并，并排序topN
	for (int i = 0; i < m_thread_num; ++i){
		r_relStruct.id_scoreStrVec.insert(r_relStruct.id_scoreStrVec.end(), paraVec[i].id_scoreStrVec.begin(), paraVec[i].id_scoreStrVec.end());
	} // end for(i)
	HeapSort(r_relStruct.id_scoreStrVec, r_topN);

	return;
} // GetScoreList

/************************************************
 * Function    : Get_IdScore_Thread
 * Description : 多线程计算id得分
 * Date        : 2017.8.21
 ***********************************************/
void * CalculateRelevancy::Get_IdScore_Thread(void * dat){
	IdScoreThreadType * para = (IdScoreThreadType *)dat;
	string id;
	string scoreStr;
	double article_score;

	for (int i = para->local; i < (int)(*para->idVec).size(); i+=para->point->m_thread_num){
		id = (*para->idVec)[i];
		article_score = para->point->Calculate_Score(id, *para->termVec);

		scoreStr = para->point->ScoreJoinStr(article_score);
		para->id_scoreStrVec.push_back(make_pair(id, scoreStr));
	} // end for(i)

	return (void *)0;
} // Get_IdScore_Thread

/************************************************
 * Function    : Calculate_Score
 * Description : 计算相关性得分
 * Date        : 2017.8.15
 ***********************************************/
double CalculateRelevancy::Calculate_Score(string r_id, vector< vector<string> >& r_termVec){
	double score = 0.0;
	// 判断权重字典中是否包含id, 并获取存放该id的map位置
	bool judge = false;
	unmap<string, double> * weight_map;

	for (int i = 0; i < (int)m_weight_map_vec.size(); ++i){  // 判断是否存在id
		if (m_weight_map_vec[i].find(r_id) != m_weight_map_vec[i].end()){
			judge = true;
			weight_map = &(m_weight_map_vec[i][r_id]);
			break;
		}
	} // end for(i)
	if ( !judge ) return score;
	
	// 获取term的加和(若是同义词，以第一个词为主，若找不到第一个词，取后面同义词权重最大的)
	string word;
	double tmpscore;
	double gscore;
	for (int i = 0; i < (int)r_termVec.size(); ++i){
		tmpscore = 0.0;
		for (int j = 0; j < (int)r_termVec[i].size(); ++j){
			word = r_termVec[i][j];
			gscore = GetWeight(word, (*weight_map));
			if (j == 0 && gscore != 0.0){
				tmpscore = gscore;
				break;
			}else if(gscore > tmpscore){
				tmpscore = gscore;
			}
		} // end for(j)
		score += tmpscore;
	} // end for(i)

	return score;
} // Calculate_Score

/************************************************
 * Function    : GetWeight
 * Description : 获取权重值
 * Date        : 2017.8.15
 ***********************************************/
double CalculateRelevancy::GetWeight(string r_word, unmap<string, double>& r_weight_map){
	double weight = 0.0;
	string term;
	vector<string> word_vec;

	splitString(r_word, " ", word_vec);  // 按空格切分
	for (int i = 0; i < (int)word_vec.size(); ++i){
		term = word_vec[i];
		if (r_weight_map.find(term) == r_weight_map.end()){
			return 0.0;
		}
		weight += r_weight_map[term];
	} // end for(i)

	return weight;
} // GetWeight

/************************************************
 * Function    : Id_ScoreJoinStr
 * Description : id的相关性得分拼接成串
 * Date        : 2017.8.31
 ***********************************************/
string CalculateRelevancy::ScoreJoinStr(double score){
	stringstream score_css;
	score = (int)(score * 1000);
	score_css << setfill('0') << setw(6) << score;

	return score_css.str();
}  // ScoreJoinStr

/************************************************
 * Function    : GetMap
 * Description : 获取所有需要的字典
 * Date        : 2017.8.14
 ***********************************************/
void CalculateRelevancy::GetMap(string r_weightFile){
	m_weight_map_vec.resize(m_thread_num);
	vector<pthread_t> pid_list(m_thread_num);
	vector<WeightThreadType> paraVec(m_thread_num);

	string strline;
	int local;

	// term权重字典，m_termweight_map_vec
	ifstream fpin_tw(r_weightFile.c_str());
	if ( !fpin_tw ){
		cerr << "term权重字典读取错误!" << endl;
		fpin_tw.close();
		exit(0);
	}
	while (getline(fpin_tw, strline)) m_line_vec.push_back(strline);  // m_line_vec
	fpin_tw.close();
	//

	// WeightThreadType结构体信息
	for (local = 0; local < m_thread_num; ++local){
		paraVec[local].local = local;
		paraVec[local].point = this;
	} // end for(local)

	// 创建多线程
	for (local = 0; local < m_thread_num; ++local) pthread_create(&(pid_list[local]), NULL, Get_TermWeightMap_Thread, &paraVec[local]);
	for (local = 0; local < m_thread_num; ++local) pthread_join(pid_list[local], NULL);

	vector<string>().swap(m_line_vec);  // 清空m_line_vec内存

	return;
} // GetMap

/************************************************
 * Function    : Get_TermWeightMap_Thread
 * Description : 多线程获取term权重字典
 * Date        : 2017.8.15
 ***********************************************/
void * CalculateRelevancy::Get_TermWeightMap_Thread(void * dat){
	WeightThreadType * para = (WeightThreadType *)dat;
	vector<string> word_vec;
	vector<string> wordweight_vec;
	string line, id, word;
	double weight;
	int local = para->local;
	int i, j;

	for (i = local; i < (int)para->point->m_line_vec.size(); i+=para->point->m_thread_num){
		line = para->point->m_line_vec[i];
		transform(line.begin(), line.end(), line.begin(), ::tolower);  // 转小写
		word_vec.clear();
		
		splitString(line, "\t", word_vec);  // 按\t切分
		if (word_vec.size() <= 1) continue;
		id = word_vec[0];

		for (j = 1; j < (int)word_vec.size(); ++j){
			wordweight_vec.clear();
			splitString(word_vec[j], " ", wordweight_vec);  // 按\t切分
			if (wordweight_vec.size() != 2) continue;
			word   = wordweight_vec[0];
			weight = atof(wordweight_vec[1].c_str());
			para->point->m_weight_map_vec[local][id][word] = weight;
		} // end for(j)
	} // end for(i)
	return (void *)0;
} // Get_TermWeightMap_Thread

/************************************************
 * Function    : Analysis_Line
 * Description : 对term串进行解析
 * Date        : 2017.8.15
 ***********************************************/
void CalculateRelevancy::Analysis_Line(string line, vector< vector<string> >& r_termVec){
	vector<string> andVec;
	vector<string> orVec;
	vector<string> termVec;
	string andline, orline;

	splitString(line, " AND ", andVec);
	for (int i = 0; i < (int)andVec.size(); ++i){
		termVec.clear();
		if (andVec[i].length() < 2) continue;
		andline = andVec[i].substr(1, andVec[i].size()-2);
		orVec.clear();
		splitString(andline, " OR ", orVec);
		for (int j = 0; j < (int)orVec.size(); ++j){
			if (orVec[j].length() < 2) continue;
			orline = orVec[j].substr(1, orVec[j].size()-2);
			termVec.push_back(orline);
		} // end for(j)
		r_termVec.push_back(termVec);
	} // end for(i)

	return;
} // Analysis_Line
