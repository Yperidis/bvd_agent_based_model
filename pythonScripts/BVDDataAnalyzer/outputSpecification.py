#!/usr/bin/env python
printTillAge = 7300
plotMeanCalvingTimeTillAge = 1000
plotFirstCalvingTimeStartingAtAge = 400
plotFirstCalvingTimeTillAge = 1400

deadCowTableName = "BVD_Dead_Cows"
deadPITableName = "BVD_Dead_PIs"
infectedCowsTableName = "BVD_Cows_Infections"
cowIDIndex = 0
lastCalvingTimeIndex = 1
cowSexIndex = 2
cowAgeIndex = 3
cowNumberOfCalvesIndex = 4
cowCauseOfDeathIndex = 5
cowFirstCalvingTimeIndex = 6
cowBirthTimeIndex = 7
knownStateIndex = 8
numberOfVaccinations = 9
firstTestTime = 10
lastTestTime = 11

cowTableDict = {}
cowTableDict["id"] = cowIDIndex
cowTableDict["lastCalvingTime"] = lastCalvingTimeIndex
cowTableDict["sex"] =cowSexIndex
cowTableDict["age"] =cowAgeIndex
cowTableDict["numberOfCalves"] =cowNumberOfCalvesIndex
cowTableDict["causeOfDeath"] =cowCauseOfDeathIndex
cowTableDict["firstCalvingTime"] =cowFirstCalvingTimeIndex
cowTableDict["birthTime"] =cowBirthTimeIndex
cowTableDict["knownState"] =knownStateIndex
cowTableDict["numberOfVaccinations"] =numberOfVaccinations
cowTableDict["firstTestTime"] =firstTestTime
cowTableDict["lastTestTime"] =lastTestTime

tradingTableName = "BVD_Trades"
tradeDateIndex = 0
tradeSRCFarmIDIndex = 1
tradeDestFarmIDIndex = 2
tradeCowIDIndex = 3
tradeCowAgeIndex = 4
tradeCowSexIndex = 5


tradeTable = {}
tradeTable["date"] = tradeDateIndex
tradeTable["srcFarm"] = tradeSRCFarmIDIndex
tradeTable["destFarm"] = tradeDestFarmIDIndex
tradeTable["cowId"] = tradeCowIDIndex
tradeTable["cowAge"] = tradeCowAgeIndex
tradeTable["cowSex"] = tradeCowSexIndex


farmTimesTableName = "BVD_Farm_Times"
tradeTimesTableName = "BVD_Trade_Times"
intermediateCalvingTimesTableName = "BVD_intermediate_calving_times"
infectionResultTableName = "BVD_INFECTION_RESULTS"
testsTableName = "BVD_Tests"



tradeTablePrefix = "Trades_t"
farmsTablePrefix = "Farms_t"


testTable = {}

testTable["testIDIndex"] = 0
testTable["testDateIndex"] = 1
testTable["testAgeIndex"] = 2
testTable["testTypeIndex"] = 3
testTable["testSexIndex"] = 4
testTable["testResultIndex"] = 5
testTable["testInfectiousStateIndex"] = 6
testTable["testKnownStateIndex"] = 7

infectiousResult = {}
infectiousResult["id"] = 0
infectiousResult["resultType"] = 1
infectiousResult["calfStatus"] = 2

intermediateCalvingTimes = {}
intermediateCalvingTimes["id"] = 0
intermediateCalvingTimes["time"] = 1
intermediateCalvingTimes["healthState"] = 2

hasBeenIllDuringPregnancy = 1.0


farmIDIndex = 0
farmSIndex = 1
farmTIIndex = 2
farmPIIndex = 3
farmRIndex = 4

farmTable = {}
farmTable["id"] = farmIDIndex
farmTable["s"] = farmSIndex
farmTable["ti"] = farmTIIndex
farmTable["pi"] = farmPIIndex
farmTable["r"] = farmRIndex

cowFemale = 1.0
cowDeathReasonNatural = 105.0
cowDeathReasonSlaughter = 201.0
cowInvalidFirstCalvingTime = -1.0
cowInfectionEvent = 107

events = {}
events["infection"] = cowInfectionEvent
events["ABORTION"] = 2
events["INSEMINATION"] = 3
events["CONCEPTION"] = 4
events["BIRTH"] = 100
events["DEATH"] = 105
events["END_OF_MA"] = 106
events["INFECTION"]           = 107
events["RECOVERY"]             = 108
events["TRADE"]             = 300
events["REMOVECOW"]				= 301
events["SLAUGHTER"]			= 200
events["CULLING"]		= 201
events["VACCINATE"]				= 202
events["END_OF_VACCINATION"] 	= 203
events["LOG_OUTPUT"]            = -1
events["WRITE_OUTPUT"]          = -2
events["STOP"]        = -3
events["MANAGE"]				= -4
events["EARTAG"]			= -5
events["TEST"]				= -100
events["ANTIBODYTEST"]			= -101
events["VIRUSTEST"]			= -102
events["VACCINATION"]			= -103
events["QUARANTINEEND"]			= -104

deathEvents = {}
deathEvents["SLAUGHTER"] = events["SLAUGHTER"]
deathEvents["CULLING"] = events["CULLING"]
deathEvents["DEATH"] = events["DEATH"]
sexes = {}
sexes["male"] = 0.0
sexes["female"] = 1.0
