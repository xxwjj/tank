#pragma once

#include "define.h"
#include "bt.h"
#include "game.h"
#include <string>

using namespace std;
//策略节点
class ContiDecision : public BehaviourTree::Node
{
 public :
 	 string nodename;
	 ContiDecision(string name = "")
	 	{
	 	nodename = name;
	 	}
 private :
 	 virtual bool run(Leg &leg) override;
	 void CheckConflictActions(Leg &leg);
	 void GivePosTo(Leg &leg,Vector2D pos,PlayerVecIt player);
};

//更新地图信息，刷新敌方的刷新范围描述信息
class CalcBullet : public BehaviourTree::Node
{
public :
	string nodename;
	CalcBullet(string name="")
		{
		nodename = name;
		}
private:
	 virtual bool run(Leg &leg) override;
	 void calcPlayerDangerousInfo(Leg &leg);
	 bool bulletDestroy(Bullet &foeBullet,Bullet &ownBullet);
};


class CalcSituation : public BehaviourTree::Node
{
public :
	string nodename;
	CalcSituation(string name = "")
		{
		nodename = name;
		}
private :
	 virtual bool run(Leg &l) override;
};

//遍历可能的下一步路径信息
class ExcludeFoolSteps : public BehaviourTree::Node
{
public:
	string nodename;
	ExcludeFoolSteps(string name = "")
		{
		nodename = name;
		}
private:
	 virtual bool run(Leg &l) override;
	 void calcPlayerNumInVision(Leg &l);
};

//避免死亡局面
class AvoidDeath : public BehaviourTree::Node{
	public:
		string nodename;
		AvoidDeath(string name = "")
		{
		nodename = name;
		}
	private:
	 virtual bool run(Leg &l);
};

//安全度评估
class RiskAssess : public BehaviourTree::Node{
public :
	string nodename;
	RiskAssess(string name = "")
		{
		nodename = name;
		}
	private:
	 virtual bool run(Leg &l) override;
	 void procFoeOwnDiff(Leg &l);
	 void procFoeBullet(Leg &l);
};

//救援
class Rescue: public BehaviourTree::Node{
	public :
	string nodename;
	Rescue(string name = "")
		{
		nodename = name;
		}
	private:
	 virtual bool run(Leg &l) override
	 	{
	 	return true;
	 	}
};

//过滤子弹方向
class FilterFire : public BehaviourTree::Node{
	public:
		string nodename;
	FilterFire(string name = "")
		{
		nodename = name;
		}
	private:
	 virtual bool run(Leg &l);
};

//挖宝
class DigDiamond : public BehaviourTree::Node{
	public:
		string nodename;
	DigDiamond(string name = "")
		{
		nodename = name;
		}
	private:
	 virtual bool run(Leg &l);
};

//避免陷入死胡同
class EsacpeDeadEnd : public BehaviourTree::Node{
	public:
		string nodename;
	EsacpeDeadEnd(string name = "")
		{
		nodename = name;
		}
	private:
	 virtual bool run(Leg &l) override;
};

class Siege: public BehaviourTree::Node{
	public:
		string nodename;
	Siege(string name = "")
		{
		nodename = name;
		}
	private:
	 virtual bool run(Leg &l) override;
	 void SearchLines(Leg &l);
};

class FinalDecision: public BehaviourTree::Node{
	public:
		string nodename;
	FinalDecision(string name = "")
	{
		nodename = name;
	}
	private:
	 virtual bool run(Leg &l) ;
};



///
class AI
{
public:
	AI(void);
	~AI(void);
	void stop(){bt.stop();}
	void update(Leg &leg/*, std::vector<Action> &actions*/);
private:
	BehaviourTree bt;
	BehaviourTree::Sequence strategys;
	//策略点
	ContiDecision isContinue;
	CalcBullet   calcBullet;
	ExcludeFoolSteps excludeFoolSteps;
	AvoidDeath avoidDeath;
	RiskAssess   riskAssessment;
	Rescue  rescuePartner;
	FilterFire filterFire;
	DigDiamond  digDiamondInfo;
	EsacpeDeadEnd escapeDeadEnd;
	Siege  siege;
	FinalDecision  finalDecision;
};