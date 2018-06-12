#include "ai.h"
#include "bt.h"
#include "debug.h"
#include "matrix_map.h"

using namespace std;

AI::AI(void):isContinue("is all path decisioned?"),calcBullet("calc bullets info"),
             excludeFoolSteps("exclude fool steps"),riskAssessment("assess risk"),rescuePartner("rescue friend"),
             digDiamondInfo("dig diamonds"),escapeDeadEnd("escape dead end"),siege("siege"),finalDecision("final decision")
{
    bt.setRootChild(&strategys);
    strategys.addChild(&calcBullet);
    strategys.addChild(&excludeFoolSteps);
    strategys.addChild(&isContinue);
    strategys.addChild(&filterFire);
    strategys.addChild(&isContinue);
    strategys.addChild(&riskAssessment);
    strategys.addChild(&isContinue);
    strategys.addChild(&avoidDeath);
    strategys.addChild(&isContinue);
    strategys.addChild(&rescuePartner);
    strategys.addChild(&isContinue);
    strategys.addChild(&digDiamondInfo);
    strategys.addChild(&isContinue);
    strategys.addChild(&escapeDeadEnd);
    strategys.addChild(&isContinue);
    strategys.addChild(&siege);
    strategys.addChild(&isContinue);
    strategys.addChild(&finalDecision);
}

AI::~AI(void)
{
}

void AI::update(Leg &leg)
{
    log("AI::Update Begin\n");
    bt.run(leg);
    log("AI::Update End\n");
}
