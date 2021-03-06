/*
 * LocalEnv.cpp
 *
 *  Created on: 2013-5-3
 *      Author: yangjinfeng
 */

#include "LocalEnv.h"
#include "Parameter.hpp"
#include "Logger.h"
#include "RunParameter.h"
#include "LoggerUtil.h"

LocalEnv::LocalEnv() {
	// TODO Auto-generated constructor stub

}

LocalEnv::~LocalEnv() {
	// TODO Auto-generated destructor stub
}

void LocalEnv::addAgent(WordAgent& wordAgent){
	localunit[wordAgent.toStringID()] = wordAgent;
}

bool LocalEnv::removeAgent(WordAgent& wordAgent){
	return localunit.erase(wordAgent.toStringID()) >= 1;
}

int LocalEnv::getAgentCount(){
	return localunit.size();
}

WordAgent& LocalEnv::getWordAgent(string agentID){
	return localunit[agentID];
}

bool LocalEnv::existsWordAgent(string agentID){
	return localunit.find(agentID) != localunit.end();
}

void LocalEnv::getAllAgentIDs(vector<string>& ids){
	ids.clear();
	for(map<string,WordAgent>::iterator it = localunit.begin();it != localunit.end();it ++){
		ids.push_back(it->first);
	}
}
/**
 * B细胞的交互
 * 		如果当前主体是B细胞，并且有激活值，则可以和其他B细胞交互，也可以和抗原交互
 * 		如果没有激活值，则只能与抗原交互
 */
bool LocalEnv::bCellInteraction(WordAgent& bcell){
//	Logger::logger<<"bCellInteraction :当前主体是----"<<bcell.toStringID()<<"  cat="<<bcell.getCategory()<<" addr="<<(int)(&bcell)<<"\n";
	double maxaffinity = -1;
	map<string,WordAgent>::iterator it = localunit.begin();
	WordAgent* maxAffinityAgent = NULL;
//	cout<<bcell.getCategory()<<endl;
	for(;it != localunit.end();it ++){
		WordAgent* agent = &(it->second);
		if(agent->getStatus() != ACTIVE || bcell.toStringID() == agent->toStringID()){
			continue;
		}
		if(agent->getCategory() == ANTIGEN){//是抗原
			vector<int> matchedFeature;
			bcell.matchFeatureRecptor(*agent,matchedFeature);
			if(matchedFeature.size() == agent->getIdiotopeDependentFeature().size()){//若成立，表示B细胞的paratope完全包含抗原的idiotope
				double aff = bcell.calAffinity(matchedFeature);
				if(aff > maxaffinity){
					maxaffinity = aff;
					maxAffinityAgent = agent;//此时maxAffinityAgent是抗原
				}
			}
		}else if(agent->getCategory() == BCELL){//如果是B细胞
			if(bcell.hasActivation()){//有激活值，才可以和B细胞反应
				//如果agent是wordAgent的父节点
				if(bcell.getWordInfo().hasParent(agent->getWordInfo())){
					double aff = agent->calAffinity(bcell);//bcell作为抗原，agent作为B细胞
					if(aff > maxaffinity){
						maxaffinity = aff;
						maxAffinityAgent = agent;//此时maxAffinityAgent是B细胞
					}
				}
			}
		}
	}

	if(maxAffinityAgent != NULL){
//		TIMESRC Logger::logger<<"maxaffinity :当前主体是****"<<bcell.toStringID()<<"  cat="<<bcell.getCategory()<<" addr="<<(int)(&bcell)<<"\n";
		vector<int> matchedFeature;
		if(maxAffinityAgent->getCategory() == BCELL){//wordAgent作为抗原
			TIMESRC Logger::logger<<StrHead::header+LoggerUtil::ACTIVE_B_RECOGNIZED+bcell.toStringID()+" act as antigen for active level("+bcell.getActiveLevel()+"),and being reconized by" +maxAffinityAgent->toStringID()+"\n";

			maxAffinityAgent->matchFeatureRecptor(bcell,matchedFeature);
			maxAffinityAgent->setMatchedFeatureRecptor(matchedFeature);
			maxAffinityAgent->setStatus(MATCH);
			maxAffinityAgent->setCurrentAffinity(maxaffinity);
			maxAffinityAgent->mapStatusToBehavior();
			if(bcell.getActiveLevel() - 1 > 0 && maxAffinityAgent->getActiveLevel() < bcell.getActiveLevel()){
				maxAffinityAgent->setActiveLevel(bcell.getActiveLevel() - 1);
			}

			bcell.setActiveLevel(0);
			bcell.setStatus(ACTIVATION_DIE);
			bcell.mapStatusToBehavior();
		}else{
			TIMESRC Logger::logger<<StrHead::header+LoggerUtil::AG_RECOGNIZED+maxAffinityAgent->toStringID()+" is antigen,and being reconized by current agent " +bcell.toStringID()+"\n";
//			cout<<bcell.getCategory()<<endl;
			bcell.matchFeatureRecptor(*maxAffinityAgent,matchedFeature);
			bcell.setMatchedFeatureRecptor(matchedFeature);
			bcell.setCurrentAffinity(maxaffinity);
			bcell.setStatus(MATCH);
			int initLevel = RunParameter::instance.getParameter("ACTIVATION_LEVEL").getIntValue();
			if(initLevel > 0){
				bcell.setActiveLevel(initLevel);
			}
			bcell.mapStatusToBehavior();

			maxAffinityAgent->setStatus(DIE);
			maxAffinityAgent->mapStatusToBehavior();
		}
		return true;
	}
	return false;
}


/**
 * 抗原的交互，只能和B细胞交互
 */
bool LocalEnv::agInterfaction(WordAgent& ag){
	double maxaffinity = -1;
	map<string,WordAgent>::iterator it = localunit.begin();
	WordAgent* maxAffinityAgent = NULL;
	for(;it != localunit.end();it ++){
		WordAgent* agent = &(it->second);
		if(agent->getCategory() == BCELL && agent->getStatus() == ACTIVE){
			vector<int> matchedFeature;
			agent->matchFeatureRecptor(ag,matchedFeature);
			if(matchedFeature.size() == ag.getIdiotopeDependentFeature().size()){//若成立，表示B细胞的paratope完全包含抗原的idiotope
				double aff = ag.calAffinity(matchedFeature);
				if(aff > maxaffinity){
					maxaffinity = aff;
					maxAffinityAgent = agent;
				}
			}
		}
	}
	if(maxAffinityAgent != NULL){
		TIMESRC Logger::logger<<StrHead::header+LoggerUtil::AG_RECOGNIZED+ag.toStringID()+" is antigen,and being reconized by" +maxAffinityAgent->toStringID()+"\n";
		vector<int> matchedFeature;
		maxAffinityAgent->matchFeatureRecptor(ag,matchedFeature);
		maxAffinityAgent->setMatchedFeatureRecptor(matchedFeature);
		maxAffinityAgent->setCurrentAffinity(maxaffinity);
		maxAffinityAgent->setStatus(MATCH);
		int initLevel = RunParameter::instance.getParameter("ACTIVATION_LEVEL").getIntValue();
		if(initLevel > 0){
			maxAffinityAgent->setActiveLevel(initLevel);
		}
		maxAffinityAgent->mapStatusToBehavior();

		ag.setStatus(DIE);
		ag.mapStatusToBehavior();
		return true;
	}
	return false;
}


/**
 * 交互之后，突变、选择一起完成
 * 交互过程如下：
 * 如果当前主体是B细胞:B细胞可以和其他B细胞交互，也可以和抗原交互
 * 		如果当前主体是B细胞，并且有激活值，则可以和其他B细胞交互，也可以和抗原交互
 * 		如果没有激活值，则只能与抗原交互
 * 如果当前主体是抗原，则只能和B细胞交互
 */
bool LocalEnv::interact(WordAgent& wordAgent){
//	TIMESRC Logger::logger<<"词主体在局部环境交互"<<"\n";
	bool interacted = false;
	if(wordAgent.getCategory() == BCELL){
//		TIMESRC Logger::logger<<"current agent is b cell\n";
		interacted = bCellInteraction(wordAgent);

	}else if(wordAgent.getCategory() == ANTIGEN){//如果主体是抗原
//		TIMESRC Logger::logger<<"current agent is antigen\n";
		interacted = agInterfaction(wordAgent);
	}
	return interacted;
}
