#include "TableBasedOutput.h"
#include "Cow.h"
#include "Herd.h"
#include "Farm.h"
#include "Events.h"

const std::string TableBasedOutput::intermediateCalvingTimeTableName = "BVD_intermediate_calving_times";
const std::string TableBasedOutput::infectionResultTabelName = "BVD_INFECTION_RESULTS";
const std::string TableBasedOutput::testsTableName = "BVD_Tests";
const std::string TableBasedOutput::vaccinationsTableName = "BVD_Vaccinations";

// const std::map<const std::string, std::vector<Data>*> TableBasedOutput::saveMap = {
// 	{TableBasedOutput::intermediateCalvingTimeTableName,this->intermediateCalvingTimes},
// 	{TableBasedOutput::infectionResultTabelName , this->infectionResultSave}
// };

TableBasedOutput::TableBasedOutput(){
	this->mode = BVDSettings::sharedInstance()->outputSettings.mode;
	this->trades = new TradeDataSave();
	this->farmData = new FarmData();
	this->farmDataTimes = new std::vector<double>();
	this->intermediateCalvingTimes = new intermediateCalvingTimeSave();
	this->CowData = new CowDataSave();
	this->InfectionData = new CowDataSave();
	this->PIDeathSave = new CowDataSave();
	this->piStorage = new CowDataSaveStorage();
	this->vaccDataSave = new VaccinationDataSave();
	this->farmNum = 0;

	this->setPath();
	this->testStorage = new TestDataSave();
	this->infectionResultSave = new infectionResultDataSave();

	MapOfSaves MOS;
	// MOS[TableBasedOutput::intermediateCalvingTimeTableName] = this->intermediateCalvingTimes;
	// this->saveMap = {
	// 	{TableBasedOutput::intermediateCalvingTimeTableName,this->intermediateCalvingTimes},
	// 	{TableBasedOutput::infectionResultTabelName , this->infectionResultSave}
	// };

}
TableBasedOutput::~TableBasedOutput(){
	this->flushStorages();
	delete this->farmData;
	delete this->intermediateCalvingTimes;
	delete this->trades;
	delete this->farmDataTimes;
	delete this->CowData;
	delete this->InfectionData;
	delete this->PIDeathSave;
	delete this->piStorage;
	delete this->testStorage;
	delete this->infectionResultSave;
	delete this->vaccDataSave;
}
#pragma mark -
#pragma mark Logging functions
void TableBasedOutput::logFarms(const double time,const std::vector< Farm* >*farms){

	this->farmDataTimes->push_back(time);
	FarmDataSave * data = new FarmDataSave();
	CowDataSave *piData = new CowDataSave();
	std::vector<Cow*>* pisOfFarm;
	for(auto farm : *farms){
		if(farm->getType() != SLAUGHTERHOUSE && farm->getType() != WELL){
			pisOfFarm = farm->getPIs();

			for(auto pi: *pisOfFarm){
				if(pi != NULL && pi->id() > 0 ){

					piData->push_back(createCowDataPointForCow(NULL,pi));
				}
			}

			delete pisOfFarm;
			int numS = farm->number_of_S();
			int numPI = farm->number_of_PI();
			int numTI = farm->number_of_TI();
			int numR = farm->number_of_R();

			FarmDataPoint p{};
			p.id = farm->id;
			p.numberS = numS;
			p.numberTI = numTI;
			p.numberPI = numPI;
			p.numberR = numR;
			// {farm->id, numS, numTI, numPI, numR};
			data->push_back(p);

		}
	}

	if(piData->size() > 0){
		this->piStorage->push_back(piData);
	}else{
		delete piData;
	}
	this->farmNum = this->farmNum > farms->size() ? farmNum : farms->size();
	this->farmData->push_back(data);

}

void TableBasedOutput::logEvent(const Event *e){
	Cow* c = Cow::get_address( e->id );

		if(c != NULL){
			switch(e->type){
				case Event_Type::TRADE:
					this->logTrade((Trade_Event*) e);
				break;
				case Event_Type::DEATH:
				case Event_Type::SLAUGHTER:
				case Event_Type::CULLING:

					this->logDyingCow(e, c);
				break;
				case Event_Type::JUNGTIER_SMALL_GROUP:
				case Event_Type::ANTIBODYTEST:
				case Event_Type::VIRUSTEST:
				case Event_Type::TEST:
					this->logTest(e, c);
				break;
				case Event_Type::ABORTION:

				break;
				case Event_Type::INFECTION:
					this->logInfection(e, c);
						break;
				case Event_Type::VACCINATE:
					this->logVaccination(e,c);
				break;
				default: break;

			}
		}
}

void TableBasedOutput::logBirth(const Cow *c){}

void TableBasedOutput::logResultingEventOfInfection(const Event* e){
	Cow *c = Cow::get_address(e->id);
	InfectionResultDataPoint p{};// = {c->id(), (int) e->type , (int) c->calf_status};
	p.id = c->id();
	p.resultType = (int)e->type;
	p.calfStatus = (int)c->calf_status;
	this->infectionResultSave->push_back(p);

}

void TableBasedOutput::logVaccination(const Event* e, const Cow* c){
	VaccinationDataPoint p{};// = {c->id(), (int) e->type , (int) c->calf_status};
	p.id = c->id();
	p.age = (int)c->age();
	p.sex = (int)c->female;
	p.time = (int)e->execution_time;
	p.infection_status = (int)c->infection_status;
	p.known_status = (int)c->knownStatus;
	p.number_of_vaccinations = c->numberOfVaccinations;
	this->vaccDataSave->push_back(p);

}


inline void TableBasedOutput::logTest(const Event *e, const Cow* c){
	TestDataPoint p = this->testEventToDataPoint(e, c);
	this->testStorage->push_back(p);
}



void TableBasedOutput::logTrade(const Trade_Event* event){
//	std::cout << " amoll" << std::endl;
	Cow* c = Cow::get_address( event->id );
//	std::cout << " amoll" << std::endl;
	double destFarmID = (double) event->farm->id;
	double srcFarmID;
//	std::cout << " g" << std::endl;
	if(c->herd != NULL && c->herd->farm != NULL)
		srcFarmID = (double) c->herd->farm->id;
	else
		srcFarmID = -1.0;
//	std::cout << " g" << std::endl;
	double cowID =  (double) c->id();
	double cowStatus = (double) c->infection_status;
	double CowSex = (double) c->female;
//	std::cout << " f" << std::endl;
	TradeDataPoint point{};// = {event->execution_time, srcFarmID, destFarmID, cowID, event->execution_time - c->birth_time, CowSex};
//	std::cout << " f" << std::endl;
	point.date = event->execution_time;
	point.srcFarmID = srcFarmID;
	point.destFarmID = destFarmID;
	point.cowID = cowID;
	point.cowAge = event->execution_time - c->birth_time;
	point.cowSex = CowSex;
	point.cowStatus = cowStatus;
	this->trades->push_back(point);
//	std::cout << " amoll amoll c c g" << std::endl;
}

void TableBasedOutput::logDyingCow(const Event* event,const Cow* cow){
	this->writeCowToSave(event,cow, this->CowData);
	if(cow->infection_status == Infection_Status::PERSISTENTLY_INFECTED)
		this->writeCowToSave(event,cow, this->PIDeathSave);
}

void TableBasedOutput::logInfection(const Event* event,const Cow* cow, const bool didLooseCalf){
	this->writeCowToSave(event,cow, this->InfectionData);
}
void TableBasedOutput::logInfectionRateChangeEvent(const Event* event){}
#pragma mark -
#pragma mark Creating Data Points


TestDataPoint TableBasedOutput::testEventToDataPoint(const Event*e, const Cow* c){
	TestDataPoint point{};// = {(double) c->id(),e->execution_time,c->age(),(double) e->type,(double) c->female,(double) c->knownStatus, (double) c->infection_status, (double) c->knownStatus};
	point.id = (double) c->id();
	point.date = e->execution_time;
	point.age = c->age();
	point.testType = (double) e->type;
	point.sex = c->female;
	point.result = (double) c->knownStatus;
	point.infectiousState = (double) c->infection_status;
	point.knownState = (double) c->knownStatus;
	return point;
}
CowDataPoint TableBasedOutput::createCowDataPointForCow(const Event* event,const Cow* cow){
	CowDataPoint point{};

	double age ;
	if(event == NULL){

		age = cow->age();//System::getInstance(NULL)->getCurrentTime() - cow->birth_time;
		}
	else{
		age = event->execution_time - cow->birth_time;

	}
	int female = (int) cow->female;
	double lastCalvingTime = 0.0;


		int j = 0;
		double state;
		while(cow->birthTimesOfCalves != NULL && cow->birthTimesOfCalves[j] != -1.0){
			if(event != NULL && (event->type == Event_Type::CULLING || event->type == Event_Type::SLAUGHTER || event->type == Event_Type::DEATH )){

				state = 0.0;
				if(lastCalvingTime > 0.0 ){
					double intermediateCalvingTime = cow->birthTimesOfCalves[j] - lastCalvingTime;
					if(std::abs(intermediateCalvingTime) > 1e-1){
						if(cow->timeOfInfection > lastCalvingTime && cow->timeOfInfection < cow->birthTimesOfCalves[j])
							state = 1.0;
						else if(cow->infection_status == Infection_Status::PERSISTENTLY_INFECTED)
							state = 2.0;
						intermediateCalvingTimePoint p{};// = {(double) cow->id(),intermediateCalvingTime,state};
						p.id = (double) cow->id();
						p.intermediateCalvingTime = intermediateCalvingTime;
						p.healthState = state;
						this->intermediateCalvingTimes->push_back(	p );
					}else{
		//				std::cout << lastCalvingTime << " " << cow->birthTimesOfCalves[j] << std::endl;
		//				std::cout << intermediateCalvingTime << std::endl;
					}
				}

				lastCalvingTime = cow->birthTimesOfCalves[j];
			}
			j++;

		}

	double firstCalvingTime = -1.0;
	double eventType = -1.0;
	if(event != NULL)
		eventType = (double) event->type;

	if(cow->birthTimesOfCalves != NULL && cow->birthTimesOfCalves[0] != -1.0)
		firstCalvingTime = cow->birthTimesOfCalves[0] - cow->birth_time;

	//point = {(double) (cow->id()),cow->last_conception_time,(double) female,age, (double) (j-1),  eventType, firstCalvingTime, cow->birth_time, (double) cow->knownStatus, cow->firstTestTime, cow->lastTestTime};
	point.id = (double) (cow->id());
	point.lastConceptionTime = cow->last_conception_time;
	point.female = (double) female;
	point.age = age;
	point.numberOfCalves = (double) (j-1);
	point.causeOfDeath = eventType;
	point.firstCalvingTime = firstCalvingTime;
	point.birthTime = cow->birth_time;
	point.knownState = (double) cow->knownStatus;
	point.numberOfVaccinations = (double) cow->numberOfVaccinations;
	point.firstTestTime = cow->firstTestTime;
	point.lastTestTime = cow->lastTestTime;
	return point;

}

void TableBasedOutput::createIntermediateCalvingTimePoints(Cow*c){}

#pragma mark -
#pragma mark Storage Management
inline void TableBasedOutput::writeCowToSave(const Event* event,const Cow* cow,CowDataSave* save){

	save->push_back(createCowDataPointForCow(event,cow));
}

void TableBasedOutput::flushStorages(){
	this->clearMyStorage(this->farmData);
	//this->farmData->clear();
	this->farmDataTimes->clear();
	this->intermediateCalvingTimes->clear();
	this->clearMySave(this->trades);
	this->clearMySave(this->CowData);
	this->clearMySave(this->InfectionData);
	this->clearMySave(this->PIDeathSave);
	this->clearMySave(this->testStorage);
	this->clearMySave(this->infectionResultSave);
	this->clearMyStorage(this->piStorage);
	this->clearMySave(this->vaccDataSave);
}

template<typename T>
void TableBasedOutput::clearMySave(std::vector<T>* save, bool clearThis){
	if(clearThis)
		save->clear();
}

template<typename T>
void TableBasedOutput::clearMyStorage(std::vector<T>* storage){
	for (auto save: *storage){
		if(save->size() > 0){
			this->clearMySave(save,false);
			}
		delete save;
	}
	storage->clear();
}



#pragma mark -
#pragma mark Definitions

double CowDataPoint::operator[] (int i){
	int retVal = -1;
	switch(i){
		case 0:
			retVal = this->id;
			break;
		case 1:
			retVal = this->lastConceptionTime;
			break;
		case 2:
			retVal = this->female;
			break;
		case 3:
			retVal = this->age;
			break;
		case 4:
			retVal = this->numberOfCalves;
			break;

		case 5:
			retVal = this->causeOfDeath;
			break;
		case 6:
			retVal = this->firstCalvingTime;
			break;
		case 7:
			retVal = this->birthTime;
			break;
		case 8:
			retVal = this->knownState;
			break;
		case 9:
			retVal = this->numberOfVaccinations;
			break;
		case 10:
			retVal = this->firstTestTime;
			break;
		case 11:
			retVal = this->lastTestTime;
			break;
		default:
			std::cerr << "Error: Asked CowDataPoint for index " << i << std::endl;
			break;

	}
	return retVal;
}
const int CowDataPoint::size = 12;
CowDataPoint::operator double*(){

	double *dat = new double[CowDataPoint::size];
	for(int i=0; i< CowDataPoint::size; i++){
		dat[i] = (*this)[i];

	}
	return dat;
}
int FarmDataPoint::operator[] (int i){
	int retVal = -1;
	switch(i){
		case 0:
			retVal = this->id;
			break;
		case 1:
			retVal = this->numberS;
			break;
		case 2:
			retVal = this->numberTI;
			break;
		case 3:
			retVal = this->numberPI;
			break;
		case 4:
			retVal = this->numberR;
			break;
		default:
			std::cerr << "Error: Asked FarmDataPoint for index " << i << std::endl;
			break;

	}
	return retVal;
}
const int FarmDataPoint::size = 5;
FarmDataPoint::operator int*(){

	int *dat = new int[FarmDataPoint::size];
	for(int i=0; i< FarmDataPoint::size; i++){
		dat[i] = (*this)[i];

	}

	return dat;
}

int InfectionResultDataPoint::operator[] (int i){
	int retVal = -1;
	switch(i){
		case 0:
			retVal = this->id;
			break;
		case 1:
			retVal = this->resultType;
			break;
		case 2:
			retVal = this->calfStatus;
			break;

		default:
			std::cerr << "Error: Asked infection result data point for index " << i << std::endl;
			break;

	}
	return retVal;
}
const int InfectionResultDataPoint::size = 3;
InfectionResultDataPoint::operator int*(){

	int *dat = new int[InfectionResultDataPoint::size];
	for(int i=0; i< InfectionResultDataPoint::size; i++){
		dat[i] = (*this)[i];

	}

	return dat;
}
int VaccinationDataPoint::operator[] (int i){
	int retVal = -1;
	switch(i){
		case 0:
			retVal = this->id;
			break;
		case 1:
			retVal = this->time;
			break;
		case 2:
			retVal = this->age;
			break;
		case 3:
			retVal = this->sex;
			break;
		case 4:
			retVal = this->number_of_vaccinations;
			break;
		case 5:
			retVal = this->infection_status;
			break;
		case 6:
			retVal = this->known_status;
			break;
		default:
			std::cerr << "Error: Asked infection result data point for index " << i << std::endl;
			break;

	}
	return retVal;
}
const int VaccinationDataPoint::size = 7;
VaccinationDataPoint::operator int*(){

	int *dat = new int[VaccinationDataPoint::size];
	for(int i=0; i< VaccinationDataPoint::size; i++){
		dat[i] = (*this)[i];

	}

	return dat;
}

double TradeDataPoint::operator[] (int i){
	double retVal = -1.0;
	switch(i){
		case 0:
			retVal = this->date;
			break;
		case 1:
			retVal = this->srcFarmID;
			break;
		case 2:
			retVal = this->destFarmID;
			break;
		case 3:
			retVal = this->cowID;
			break;
		case 4:
			retVal = this->cowAge;
			break;
		case 5:
			retVal = this->cowSex;
			break;
        case 6:
            retVal = this->cowStatus;
            break;
		default:
			std::cerr << "Error: Asked TradeDataPoint for index " << i << std::endl;
			break;
	}
	return retVal;
}

const int TradeDataPoint::size = 7; //number of data points given in a tradeDataPoint
TradeDataPoint::operator const double*(){

	double *dat = new double[TradeDataPoint::size];
	for(int i=0; i< TradeDataPoint::size; i++){
		dat[i] = (*this)[i];

	}

	return dat;
}
TradeDataPoint::operator double*(){

	double *dat = new double[TradeDataPoint::size];
	for(int i=0; i< TradeDataPoint::size; i++){
		dat[i] = (*this)[i];

	}

	return dat;
}
double TestDataPoint::operator[] (int i){
	double retVal = -1.0;
	switch(i){
		case 0:
			retVal = this->id;
			break;
		case 1:
			retVal = this->date;
			break;
		case 2:
			retVal = this->age;
			break;
		case 3:
			retVal = this->testType;
			break;
		case 4:
			retVal = this->sex;
			break;
		case 5:
			retVal = this->result;
			break;
		case 6:
			retVal = this->infectiousState;
			break;
		case 7:
			retVal = this->knownState;
			break;
		default:
			std::cerr << "Error: Asked TestDataPoint for index " << i << std::endl;
			break;
	}
	return retVal;
}

const int TestDataPoint::size = 8; //number of data points given in a tradeDataPoint
TestDataPoint::operator const double*(){
	double *dat = new double[TestDataPoint::size];
	for(int i=0; i< TestDataPoint::size; i++){
		dat[i] = (*this)[i];
	}
	return dat;
}
TestDataPoint::operator double*(){
	double *dat = new double[TestDataPoint::size];
	for(int i=0; i< TestDataPoint::size; i++){
		dat[i] = (*this)[i];
	}
	return dat;
}
double intermediateCalvingTimePoint::operator[] (int i){
	int retVal = -1;
	switch(i){
		case 0:
			retVal = this->id;
			break;
		case 1:
			retVal = this->intermediateCalvingTime;
			break;
		case 2:
			retVal = this->healthState;
			break;

		default:
			std::cerr << "Error: Asked intermediate calving time data point for index " << i << std::endl;
			break;

	}
	return retVal;
}
const int intermediateCalvingTimePoint::size = 3;
intermediateCalvingTimePoint::operator double*(){

	double *dat = new double[intermediateCalvingTimePoint::size];
	for(int i=0; i< intermediateCalvingTimePoint::size; i++){
		dat[i] = (*this)[i];

	}

	return dat;
}
