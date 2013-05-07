#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>


#include "DependencyPaser.hpp"
#include "Parameter.hpp"
#include "Logger.h"
#include "StrHead.h"
#include "RunParameter.h"

using namespace std;

DependencyPaser::DependencyPaser()
{
	pModel = new Model();
	pPredictor = new Predictor((Model *)pModel);
	pEvaluation = new Evaluation((Predictor *) pPredictor,(Model *) pModel);
	pTrainer = new Trainer((Model *) pModel, (Predictor *) pPredictor);
}

DependencyPaser::~DependencyPaser()
{
	delete pModel;
	delete pTrainer;
	delete pPredictor;
}

/**
 * add by yangjinfeng
 */
bool DependencyPaser::train(const char * file){
	Logger::logger<<StrHead::header + "Initilizing B cell Network..." +"\n";
	initBCell(file);
	Logger::logger<<StrHead::header + "Initilizing finished!" +"\n";


	Logger::logger<<StrHead::header + "Online learning..." +"\n";
	trainFromFile(file);//读取依存树库，逐句训练
	Logger::logger<<StrHead::header + "Online learning finished!" +"\n";

	Logger::logger<<StrHead::header + "saving b cells and feature weight" +"\n";
	pTrainer->saveBCells();
	pModel->saveWeight();
	return true;

}

/**
 * 从训练树库里读取依存树，构建B细胞词
 * add by yangjinfeng
 */
bool DependencyPaser::initBCell(const char * file)
{
	ifstream fin(file);
	string line;
	vector<vector<string> > senes;
	while(getline(fin, line)){
		if(line == ""){
			vector<int> father;
			Sentence sen;
			sen.push_back(make_pair("ROOT", "ORG"));//第0个是root
			father.push_back(-1);
			for(size_t i = 0; i < senes.size(); i++){
				sen.push_back(make_pair(senes[i][1], senes[i][3]));//第一个是词，第三个是词性
				father.push_back(atoi(senes[i][6].c_str()));//取得父节点的序号
			}

			pTrainer->initBCells(sen, father);//词、词性、父节点，作为参数
			senes.clear();
		}
		else{
			vector<string> item;
			string tmp;
			istringstream sin(line);
			while(sin >> tmp){
				item.push_back(tmp);
			}

			senes.push_back(item);
		}

	}

	pTrainer->reduceWordFreq();//词频缩减
	return true;
}

/**
 *file是用作训练语料的依存树库文件
 *add by yangjinfeng
 */
bool DependencyPaser::trainFromFile(const char * file)
{
	pTrainer->distributeBCells();//构造B细胞网络
	string line;
	vector<vector<string> > senes;
	pModel->initFeatureWeight();//初始化特征权重
	int learnTimes = RunParameter::instance.getParameter("LEARNTIMES").getIntValue();
	for(int i = 0; i < learnTimes; i++)//迭代次数
	{
		Logger::logger<<StrHead::header +"Learning "+(i+1)+ " times" +"\n";
		ifstream fin(file);
		while(getline(fin, line)){
			if(line == ""){
				vector<int> father;
				Sentence sen;
				sen.push_back(make_pair("ROOT", "ORG"));
				father.push_back(-1);
				for(size_t i = 0; i < senes.size(); i++){
					sen.push_back(make_pair(senes[i][1], senes[i][3]));
					father.push_back(atoi(senes[i][6].c_str()));
				}

				//逐句训练
				pTrainer->trainBySentence(sen,  father);

				senes.clear();
			}
			else{
				vector<string> item;
				string tmp;
				istringstream sin(line);
				while(sin >> tmp){
					item.push_back(tmp);
				}
				senes.push_back(item);
			}
		}
		//cout<<"number of sentences is "<<num<<endl;
		fin.close();
//		pTrainer->initSentenceID();

	}
	return true;
}


bool DependencyPaser::predict(const char * testFile, const char * outFile)
{
	Logger::logger<<StrHead::header + "Predicting..." +"\n";

	ifstream fin(testFile);
	ofstream fout(outFile, ios::out|ios::app);
	string line;
	vector<vector<string> > senes;

	int senNum = 0;
	double sum  = 0.0;
	while(getline(fin, line)){
		if(line == ""){
			vector<int> predictedFather;
			Sentence sen;
			sen.push_back(make_pair("ROOT", "ORG"));
			for(size_t i = 0; i < senes.size(); i++){
				sen.push_back(make_pair(senes[i][1], senes[i][3]));
			}
			predictedFather.resize(sen.size());
			predict(sen,predictedFather);
			int rightFather = 0;
			for(size_t i = 0; i < senes.size(); i++){
				if(predictedFather[i+1] == atoi(senes[i][6].c_str())){
					rightFather++;
				}
			}

			double acc = (double)rightFather/(double)senes.size();
			sum += acc;
			senNum++;

			fout<<senNum<<","<<senes.size()<<","<<rightFather<<","<<acc<<endl;
			senes.clear();

		}
		else{
			vector<string> item;
			string tmp;
			istringstream sin(line);
			while(sin >> tmp){
				item.push_back(tmp);
			}
			senes.push_back(item);
		}
	}
	double average = sum /(double)senNum;
	Logger::logger<<StrHead::header + "Accuracy is " + average +"\n";
	fout<<"Accuracy:"<<average<<endl;
	fout.close();
	Logger::logger<<StrHead::header + "Predicting finished!" +"\n";
	return true;
}


/////////////////////////////////////////////////////////////////////////////////////////////////



double DependencyPaser::predict(const Sentence & sen,std::vector<int> & fa)
{
        return pPredictor->predict(sen, fa);
}



