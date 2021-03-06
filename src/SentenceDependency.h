/*
 * SentenceDependency.h
 *
 *  Created on: 2013-5-6
 *      Author: hp
 */

#ifndef SENTENCEDEPENDENCY_H_
#define SENTENCEDEPENDENCY_H_
#include "Sentence.hpp"
#include <vector>
#include "PredictedResult.h"
using namespace std;

class SentenceDependency {
public:
	SentenceDependency();
	virtual ~SentenceDependency();
	void setSentenceAndDependency(const Sentence& sen,const vector<int>& parent);
	void setCurrentPredictedParent(vector<int>& parent);
	double getCurrentSentencePrecision(){
		return currerntPrecision;
	}
	Sentence& getCurrentSentence(){
		return currenSentence;
	}

	vector<int>& getRealParent(){
		return realParent;
	}

	vector<PredictedResult>& getPredictedResults(){
		return predictedResults;
	}

	void addPredictedResult(const vector<int>& predict,int deltaIndex);
	/**
	 * 选择最好的预测，也就是精度最高的预测
	 */
	void selectBestPredicts(vector<int> & indexs);

	int selectMinScoreDifference();


    void setPredictedScore(int index,double score);

    void setRealScore(int index,double score);

    vector<int>& getPredictedParent(int index);

    double getMaxPredictedPrecision() const
    {
        return maxPredictedPrecision;
    }

    void setMaxPredictedPrecision(double maxPredictedPrecision)
    {
        this->maxPredictedPrecision = maxPredictedPrecision;
    }

    int getPredictedResultDeltaIndex(int index){
    	return predictedResults[index].getDeltaIndex();
    }

    void reset();

private:
	Sentence currenSentence;
	vector<int> realParent;
	//突变前的预测结果
	vector<int> currentPredictedParent;
	//突变前预测结果的精确度
	double currerntPrecision;
	//突变后最大的预测精度
	double maxPredictedPrecision;

	//多个突变的结果
	vector<PredictedResult> predictedResults;


	double calPrecision(const vector<int>& predict);
	void resetForNextMutate();

};

#endif /* SENTENCEDEPENDENCY_H_ */
