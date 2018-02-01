#include "BVDSettings.h"
#include "BVDContainmentStrategy.h"
#include "projectImports/inih/cpp/INIReader.h"
BVDSettings* BVDSettings::_instance = 0;
BVDSettings* BVDSettings::sharedInstance(INIReader* reader){
   static CGuard g;   // Speicherbereinigung
   if (!BVDSettings::_instance)
      BVDSettings::_instance = new BVDSettings (reader);
   return _instance;
}

BVDSettings::BVDSettings(INIReader* reader){
    this->reader = reader;
    this->initializeStrategies();
    this->initializeOutputSettings();
}
BVDSettings::BVDSettings(const BVDSettings&){


}

void BVDSettings::initializeStrategies(){
    std::string strategyNamesString = this->reader->Get("containment", "strategies", "");
    std::cout << strategyNamesString << std::endl;
    this->strategies = StrategyQueue();
    if(strategyNamesString.compare("") != 0){//there are strings
        //split up the string on every ,
        std::vector<std::string> stratNames;
        std::stringstream ss;

        ss.str(strategyNamesString);
        std::string item;
        while (std::getline(ss, item, ',')) {
            *(std::back_inserter(stratNames)++) = item;
        }

        //iterate over all strategy names
        for(auto name: stratNames){
            std::cout << " strategy named: " << name << std::endl;
            this->initializeStrategy(name);

        }
    }else{


        this->strategies.push(new BVDContainmentStrategy( BVDContainmentStrategyFactory::defaultStrategy ) );
    }
}


void BVDSettings::initializeStrategy(const std::string& name){
    BVDContainmentStrategy *newStrat = new BVDContainmentStrategy();
    newStrat->usesEartag=this->reader->GetBoolean(name, "usesEartag", BVDContainmentStrategyFactory::defaultStrategy.usesEartag);
    newStrat->usesVaccination=this->reader->GetBoolean(name, "usesVaccination", BVDContainmentStrategyFactory::defaultStrategy.usesVaccination);
    newStrat->usesJungtierFenster=this->reader->GetBoolean(name, "usesJungtierFenster", BVDContainmentStrategyFactory::defaultStrategy.usesJungtierFenster);
    newStrat->startTime=this->reader->GetReal(name, "startTime", BVDContainmentStrategyFactory::defaultStrategy.startTime);
    newStrat->bloodTestAccuracy=this->reader->GetReal(name, "bloodTestAccuracy", BVDContainmentStrategyFactory::defaultStrategy.bloodTestAccuracy);
    newStrat->retestingTimeBlood=this->reader->GetReal(name, "retestingTimeBlood", BVDContainmentStrategyFactory::defaultStrategy.retestingTimeBlood);
    newStrat->vaccinationWorkingProbability=this->reader->GetReal(name, "vaccinationWorkingProbability", BVDContainmentStrategyFactory::defaultStrategy.vaccinationWorkingProbability);
    newStrat->vaccinationTimeBeforeInsemination=this->reader->GetReal(name, "vaccinationTimeBeforeInsemination", BVDContainmentStrategyFactory::defaultStrategy.vaccinationTimeBeforeInsemination);
    newStrat->quarantineTime=this->reader->GetReal(name, "quarantineTime", BVDContainmentStrategyFactory::defaultStrategy.quarantineTime);
    newStrat->applyQuarantineOnBuying=this->reader->GetBoolean(name, "applyQuarantineOnBuying", BVDContainmentStrategyFactory::defaultStrategy.applyQuarantineOnBuying);
    newStrat->respectQuarantine=this->reader->GetBoolean(name, "respectQuarantine", BVDContainmentStrategyFactory::defaultStrategy.respectQuarantine);
    newStrat->quarantineAfterPositiveTest=this->reader->GetBoolean(name, "quarantineAfterPositiveTest", BVDContainmentStrategyFactory::defaultStrategy.quarantineAfterPositiveTest);
    newStrat->jungtierzeit=this->reader->GetReal(name, "jungtierzeit", BVDContainmentStrategyFactory::defaultStrategy.jungtierzeit);
    newStrat->vaccinationTimeOfDefense=this->reader->GetReal(name, "vaccinationTimeOfDefense", BVDContainmentStrategyFactory::defaultStrategy.vaccinationTimeOfDefense);
    std::cout << "starting strategy at : " << newStrat->startTime << std::endl;
    this->strategies.push(newStrat);
}
void BVDSettings::initializeOutputSettings(){
    this->outputSettings = {};
    this->outputSettings.overwrite = reader->GetBoolean("output", "overwrite", false);
    this->outputSettings.fileprefix = reader->Get("output", "fileprefix", "output_");
    this->outputSettings.path = reader->Get("output", "path", "./");
    std::string fileMode = reader->Get("output", "mode", "single_file");
	this->outputSettings.mode = BVDSettings::iniInputToFileMode(fileMode);
    this->outputSettings.fullFilePath = this->outputSettings.path + this->outputSettings.fileprefix + this->outputSettings.fileExtension;
    this->outputSettings.writeAllDeadCows = reader->GetBoolean("output", "writeAllDeadCows", true);
    this->outputSettings.writeDeadPIS = reader->GetBoolean("output", "writeDeadPIS", true);
    this->outputSettings.writeTradeAdjacencyMatrix = reader->GetBoolean("output", "writeTradeAdjacencyMatrix", true);
    this->outputSettings.postFileWriteCall = reader->Get("output", "postFileWriteCall", "");
}

BVDSettings::~BVDSettings(){
    while(!this->strategies.empty()){
        delete this->strategies.top();
        this->strategies.pop();
    }
}

BVDSettings::CGuard::~CGuard(){
	if(BVDSettings::_instance != nullptr){
		delete BVDSettings::_instance;
		BVDSettings::_instance = nullptr;
	}
}
fileMode BVDSettings::iniInputToFileMode(std::string& fileMode,bool shallExit){
	if(fileMode.compare("single_file") == 0){
		return single_file;
	}else if(fileMode.compare("multi_file") == 0 || fileMode.compare("split_file") == 0){
		return multi_file;
	}else{
		std::cout << "The specified output mode \"" << fileMode <<"\" does not exist" <<std::endl;
		if(shallExit)
			exit(4);
	}
}
