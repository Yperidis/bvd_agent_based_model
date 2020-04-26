#include "BVDContainmentStrategy.h"
BVDContainmentStrategy BVDContainmentStrategyFactory::defaultStrategy = {
    false, //usesEartag
    false, //usesVaccination
    false, //usesJungtierFenster
    0.0, //startTime
    0.99, //bloodTestAccuracy
    60.0, //retestingTimeBlood;
    0.985, //vaccinationWorkingProbability;
    42., //vaccinationTimeBeforeInsemination;
    365., //vaccinationTimeOfDefense;
    40., //quarantineTime;
    false, //applyQuarantineOnBuying;
    true, //respectQuarantine;
    true, //quarantineAfterPositiveTest;
    186.//jungtierzeit;
};

bool BVDContainmentStrategySortCriterion::operator() (BVDContainmentStrategy const * const  s1 , BVDContainmentStrategy const * const s2){
    return s1->startTime > s2->startTime;
}
