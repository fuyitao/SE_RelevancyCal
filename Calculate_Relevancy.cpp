#include "Calculate_Relevancy.h"

pthread_mutex_t pre_mutex;

CalculateRelevancy::CalculateRelevancy(string r_titleweightFile, string r_contentweightFile, string r_id_sourcenameFile, string r_timestampFile){
	m_thread_num = 10;  // 线程数量
	GetMap(r_titleweightFile, r_contentweightFile, r_id_sourcenameFile, r_timestampFile);
	int title = 0, content = 0;
	for (int i = 0; i < (int)m_titleweight_map_vec.size(); ++i) title += m_titleweight_map_vec[i].size();
	for (int i = 0; i < (int)m_contentweight_map_vec.size(); ++i) content += m_contentweight_map_vec[i].size();
	cout << "title:" << title << "|content:" << content << "|source:" << m_id_sourcename_map.size() << "|time:" << m_timestamp_map.size() << endl;
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
									int r_topN,
									string r_sourcename){
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
		paraVec[local].local      = local;         // 第几个线程
		paraVec[local].idVec      = &r_idVec;      // id vector
		paraVec[local].termVec    = &termVec;      // term vector
		paraVec[local].point      = this;          // 类指针
		paraVec[local].sourcename = r_sourcename;  // 来源词
		paraVec[local].topN       = r_topN;        // 排序topN
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
	string sourcename = para->sourcename;
	string scoreStr;
	IdScoreType id_score;
	string min_time = "0000000000";

	for (int i = para->local; i < (int)(*para->idVec).size(); i+=para->point->m_thread_num){
		id = (*para->idVec)[i];
		id_score.is_source_name = (sourcename == para->point->m_id_sourcename_map[id] && sourcename != "")? 1:0;  // 判断sourcename是否相同
		id_score.title_score    = para->point->Calculate_Score(id, *para->termVec, para->point->m_titleweight_map_vec, id_score.title_match_num);    // title相关性得分
		id_score.content_score  = para->point->Calculate_Score(id, *para->termVec, para->point->m_contentweight_map_vec, id_score.content_match_num);  // content相关性得分
		id_score.timestamp = (para->point->m_timestamp_map.find(id) != para->point->m_timestamp_map.end())? para->point->m_timestamp_map[id]:min_time;  // timestamp
		
		scoreStr = para->point->ScoreJoinStr(id_score);
		para->id_scoreStrVec.push_back(make_pair(id, scoreStr));
	} // end for(i)

	return (void *)0;
} // Get_IdScore_Thread

/************************************************
 * Function    : Calculate_Score
 * Description : 计算相关性得分
 * Date        : 2017.8.15
 ***********************************************/
double CalculateRelevancy::Calculate_Score(string r_id,
								vector< vector<string> >& r_termVec,
								vector< unmap<string, unmap<string, double> > >& weight_map_vec,
								int& match_num){
	double score = 0.0;
	match_num = 0;
	// 判断权重字典中是否包含id, 并获取存放该id的map位置
	bool judge = false;
	unmap<string, double> * weight_map;

	for (int i = 0; i < (int)weight_map_vec.size(); ++i){  // 判断是否存在id
		if (weight_map_vec[i].find(r_id) != weight_map_vec[i].end()){
			judge = true;
			weight_map = &(weight_map_vec[i][r_id]);
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
				++match_num;
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
string CalculateRelevancy::ScoreJoinStr(IdScoreType& scoreType){
	stringstream score_css;
	int is_source_name;
	int title_num, content_num;
	double title_score, content_score;
	string timestamp;

	is_source_name = scoreType.is_source_name;
	title_num      = scoreType.title_match_num;
	title_score    = (int)(scoreType.title_score*1000);
	content_num    = scoreType.content_match_num;
	content_score  = (int)(scoreType.content_score*1000);
	timestamp      = scoreType.timestamp;

	score_css.str("");
//	score_css << setfill('0') << setw(2) << title_num;
//	score_css << "-" << is_source_name;
//	score_css << "-" << setfill('0') << setw(10) << timestamp;
//	score_css << "-" << setfill('0') << setw(6) << title_score;
//	score_css << "-" << setfill('0') << setw(2) << content_num;
	score_css << setfill('0') << setw(6) << content_score;

	return score_css.str();
}  // ScoreJoinStr

/************************************************
 * Function    : GetMap
 * Description : 获取所有需要的字典
 * Date        : 2017.8.14
 ***********************************************/
void CalculateRelevancy::GetMap(string r_titleweightFile, string r_contentweightFile, string r_id_sourcenameFile, string r_timestampFile){
	string strline, id, sourcename;
	vector<string> word_vec;
	// id 来源词典
	ifstream fpin_idsource(r_id_sourcenameFile.c_str());
	while (getline(fpin_idsource, strline)){
		transform(strline.begin(), strline.end(), strline.begin(), ::tolower);  // 转小写
		word_vec.clear();
		splitString(strline, "\t", word_vec);
		if (word_vec.size() != 2) continue;
		id         = word_vec[0];
		sourcename = word_vec[1];
		if (id == "" || sourcename == "") continue;
		m_id_sourcename_map[id] = sourcename;
	} // end while(getline)
	fpin_idsource.close();

	// id 时间戳字典
	string timestamp;
	ifstream fpin_timestamp(r_timestampFile.c_str());
	while (getline(fpin_timestamp, strline)){
		transform(strline.begin(), strline.end(), strline.begin(), ::tolower);  // 转小写
		word_vec.clear();
		splitString(strline, "\t", word_vec);
		if (word_vec.size() != 2) continue;
		id        = word_vec[0];
		timestamp = word_vec[1];
		if (id == "" || timestamp == "") continue;
		m_timestamp_map[id] = timestamp;
	} // end while(getline)
	fpin_timestamp.close();

	// term权重(title、content)
	m_titleweight_map_vec.resize(m_thread_num);
	m_contentweight_map_vec.resize(m_thread_num);
	// term权重字典，m_titleweight_map_vec
	GetMapThread(r_titleweightFile, "title");
	// term权重字典，m_contentweight_map_vec
	GetMapThread(r_contentweightFile, "content");

	return;
} // GetMap

/************************************************
 * Function    : GetMapThread
 * Description : 使用多线程获取字典
 * Date        : 2017.8.28
 ***********************************************/
void CalculateRelevancy::GetMapThread(string r_weightFile, string r_mode){
	vector<pthread_t> pid_list(m_thread_num);
	vector<WeightThreadType> paraVec(m_thread_num);
	string strline;
	int local;

	// term权重字典，m_termweight_map_vec
	ifstream fpin_tw(r_weightFile.c_str());
	if ( !fpin_tw ){
		cerr << r_mode << "_term权重字典读取错误!" << endl;
		fpin_tw.close();
		exit(0);
	}

	// m_line_vec
	while (getline(fpin_tw, strline)){
		m_line_vec.push_back(strline);
	} // end while

	// WeightThreadType结构体信息
	for (local = 0; local < m_thread_num; ++local){
		paraVec[local].local = local;
		paraVec[local].mode  = r_mode;
		paraVec[local].point = this;
	} // end for(local)

	// 创建多线程
	for (local = 0; local < m_thread_num; ++local) pthread_create(&(pid_list[local]), NULL, Get_TermWeightMap_Thread, &paraVec[local]);
	for (local = 0; local < m_thread_num; ++local) pthread_join(pid_list[local], NULL);

	vector<string>().swap(m_line_vec);  // 清空m_line_vec内存
	fpin_tw.close();

	return;
} // GetMapThread

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
			// 判断是哪个term权重字典
			if (para->mode == "title"){
				para->point->m_titleweight_map_vec[local][id][word] = weight;
			}else if (para->mode == "content"){
				para->point->m_contentweight_map_vec[local][id][word] = weight;
			}
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
