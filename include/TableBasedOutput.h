#ifndef __table_output_h_
#define __table_output_h_

#include "FileHandler.h"
#include "BVDSettings.h"
#include <map>
#include <cmath>
class Trade_Event;
class Event;
class Cow;
class Farm;

struct Data{};

template<typename T>
struct DataPoint:public Data{
	virtual T operator[] (int i) = 0;
	//virtual operator const T*() = 0;
	virtual operator T*() = 0;
	static const int size;
};

struct TradeDataPoint: public DataPoint<double>{
	// TradeDataPoint(double data, double)
	double date;
	double srcFarmID;
	double destFarmID;
	double cowID;
	double cowAge;
	double cowSex;
	double cowStatus;

	double operator[] (int i);
	operator const double*();
	operator double*();
	static const int size;
};

struct FarmDataPoint: public DataPoint<int>{
	int id;
	int numberS;
	int numberTI;
	int numberPI;
	int numberR;
	//int numYoung;
	int operator[] (int i);
	operator int*();
	static const int size;
};

struct CowDataPoint: public DataPoint<double>{
	double id;
	double lastConceptionTime;
	double female;
	double age;
	double numberOfCalves;
	double causeOfDeath;
	double firstCalvingTime;
	double birthTime;
	double knownState;
	double numberOfVaccinations;
	double firstTestTime;
	double lastTestTime;
	double operator[] (int i);
	operator double*();
	static const int size;
};

struct TestDataPoint: public DataPoint<double>{
	double id;
	double date;
	double age;
	double testType;
	double sex;
	double result;
	double infectiousState;
	double knownState;
	double operator[] (int i);
	operator double*();
	operator const double*();
	static const int size;
};

struct InfectionResultDataPoint: public DataPoint<int>{
	int id;
	int resultType;
	int calfStatus;
	int operator[] (int i);
	operator int*();
	static const int size;
};

struct VaccinationDataPoint: public DataPoint<int>{
	int id;
	int time;
	int age;
	int sex;
	int number_of_vaccinations;
	int infection_status;
	int known_status;
	int operator[] (int i);
	operator int*();
	operator const int*();
	static const int size;
};
struct intermediateCalvingTimePoint: public DataPoint<double>{
	double id;
	double intermediateCalvingTime;
	double healthState;
	double operator[] (int i);
	operator double*();
	operator const double*();
	static const int size;
};



class TableBasedOutput : public FileHandler{
private:

public:
    TableBasedOutput();
    ~TableBasedOutput();
    void logFarms(const double time,const std::vector< Farm* >*farms);
    void logEvent(const Event *e);
    void logBirth(const Cow *c);
    void logResultingEventOfInfection(const Event* e);
    void logTest(const Event *e, const Cow *c);



protected:
    typedef std::vector<TradeDataPoint> TradeDataSave;
    typedef std::vector<FarmDataPoint> FarmDataSave;
    typedef std::vector<FarmDataSave *> FarmData;
    typedef std::vector<CowDataPoint> CowDataSave;
    typedef std::vector<CowDataSave* > CowDataSaveStorage;
    typedef std::vector<TestDataPoint> TestDataSave;
    typedef std::vector<intermediateCalvingTimePoint> intermediateCalvingTimeSave;
    typedef std::vector<InfectionResultDataPoint> infectionResultDataSave;
		typedef std::vector<VaccinationDataPoint> VaccinationDataSave;


    void logTrade(const Trade_Event* event);
    void logDyingCow(const Event* event,const Cow* cow);
    void logInfection(const Event* event,const Cow* cow,const bool didLooseCalf=false);
    void logInfectionRateChangeEvent(const Event* event);
		void logVaccination(const Event* event,const Cow* cow);
    TestDataPoint testEventToDataPoint(const Event*, const Cow* c);
    CowDataPoint createCowDataPointForCow(const Event* event,const Cow* cow);
    void createIntermediateCalvingTimePoints(Cow*c);

    void writeCowToSave(const Event* event,const Cow* cow, CowDataSave* save);
    void flushStorages();

    template<typename T>
    void clearMySave(std::vector<T>* save, bool clearThis = true);
    template<typename T>
    void clearMyStorage(std::vector<T>* storage);

	typedef std::map<const std::string, std::vector<Data>*> MapOfSaves;

	const MapOfSaves saveMap;

	const MapOfSaves prefixedSaveMap;

	std::vector<std::string> tableNames;
	std::vector<std::string> tablePrefixes;


	std::vector<std::vector<Data>*> stores;

	static const std::string intermediateCalvingTimeTableName;
	static const std::string infectionResultTabelName;
	static const std::string testsTableName;
	static const std::string vaccinationsTableName;


    std::vector<double>* farmDataTimes;
    intermediateCalvingTimeSave* intermediateCalvingTimes;
    FarmData* farmData;
    TradeDataSave* trades;
    CowDataSave* CowData;
    CowDataSave* InfectionData;
    CowDataSave* PIDeathSave;
    CowDataSaveStorage* piStorage;
    TestDataSave* testStorage;
    infectionResultDataSave *infectionResultSave;
		VaccinationDataSave *vaccDataSave;
    int farmNum;

    fileMode mode;

};

#endif
