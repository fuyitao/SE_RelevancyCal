#include "../queryAnalyze/QueryPreprocess.h"
#include "../queryAnalyze/Synonym_Termweight.h"
#include "Calculate_Relevancy.h"
#include "ic_url.h"
#include "cJSON.h"
#include "url_code.h"
#include <time.h>

void Output(PreproType& preStruct, 
			TermweightType& termStruct,
			RelevancyType& relStruct);
string path = "../dict/";

int main(){
	// 字典----------------------------
	// 分词工具初始化
	string word_dictFile = path + "queryAnalyze_dict/seg_dict/";          // queryanalyze
	word_segment_init(word_dictFile.c_str());                               // queryanalyze
	string synNtoNFile    = path + "queryAnalyze_dict/Synonym_NtoNResult";  // queryanalyze
	string syn1toNFile    = path + "queryAnalyze_dict/Synonym_1toNResult";  // queryanalyze
	string stopwordFile   = path + "queryAnalyze_dict/stopwords.txt";       // queryanalyze
	string termweightFile = path + "queryAnalyze_dict/xihuan_termweight";   // queryanalyze
	string titleweightFile   = path + "relevancyCalculate_dict/xihuan_bm25score_title";    // relevancy
	string contentweightFile = path + "relevancyCalculate_dict/xihuan_bm25f_new";  // relevancy
	string id_sourcenameFile = path + "relevancyCalculate_dict/xihuan_id_sourcename";      // relevancy
	string timestampFile     = path + "relevancyCalculate_dict/xihuan_timestamp";          // relevancy
	//---------------------------------
	
	// 结构体--------------------------
	PreproType preStruct;       // queryanalyze
	TermweightType termStruct;  // queryanalyze
	RelevancyType relStruct;  // relevancy
	//---------------------------------

	// 类对象
	QueryPreprocess queryPre(id_sourcenameFile);  // queryanalyze
	SynonymTermweight queryTerm(synNtoNFile, syn1toNFile, stopwordFile, termweightFile);  // queryanalyze
	CalculateRelevancy calRelevancy(titleweightFile, contentweightFile, id_sourcenameFile, timestampFile);  // relevancy
	
	string query, s_topN;
	vector<string> id_vec;
	vector<string> word_vec;
	int topN;
	vector<string> idVec;
	string sourcename;
	while (1){
		cerr << "query:";
		getline(cin, query);
		if (query == "-1") break;
		cerr << "topN:";
		getline(cin, s_topN);
		topN = atoi(s_topN.c_str());


		cerr << "id:";
		string id;
		idVec.clear();
		while (1){
			getline(cin, id);
			if (id == "-1") break;
			idVec.push_back(id);
			cerr << "id:";
		}

		queryPre.Preprocess(query, "basic", preStruct);
		queryTerm.GetTermList(preStruct, termStruct);
		sourcename = preStruct.sourcename;
		calRelevancy.GetScoreList(termStruct.termStr, idVec, relStruct, topN, sourcename);
		// output
		Output(preStruct, termStruct, relStruct);
	}
	exit(0);
	return 0;
}

void Output(PreproType& preStruct, 
			TermweightType& termStruct, 
			RelevancyType& relStruct){
	// preStruct
	string segline = "";
	for (int i = 0; i < (int)preStruct.pre_segVec.size(); ++i){
		segline += preStruct.pre_segVec[i] + " ";
	}
	// termStruct
	string weightStr;
	string term;
	stringstream css;
	for (int i = 0; i < (int)termStruct.termweightVec.size(); ++i){
		css.str("");
		term = termStruct.termweightVec[i].first;
		css << termStruct.termweightVec[i].second;
		if (i == 0) weightStr = term + ":" + css.str();
		else weightStr += "\n   " + term + ":" + css.str();
	}
	// relStruct
	stringstream relStr;
	string id;
	string scoreStr;
	for (int i = 0; i < (int)relStruct.id_scoreStrVec.size(); ++i){
		id = relStruct.id_scoreStrVec[i].first;
		scoreStr = relStruct.id_scoreStrVec[i].second;
		if (i != 0) relStr << "\n	    ";
		relStr << "id:" << id << " | scoreStr:" << scoreStr;
	}

	// output
	cerr << "------------- output ---------------" << endl;
	cerr << "segline    :" << segline << endl;
	cerr << "termline   :" << termStruct.termStr << endl;
	cerr << "sourcename :" << preStruct.sourcename << endl;
	cerr << "weightline :" << weightStr << endl;
	cerr << "relline.size:" << relStruct.id_scoreStrVec.size() << endl;
	cout << "relline    :" << relStr.str() << endl;
	cerr << "-----------------------------------" << endl;
}
