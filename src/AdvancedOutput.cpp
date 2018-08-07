#include "AdvancedOutput.h"
#include "projectImports/inih/cpp/INIReader.h"
#include "SQLiteHandler.h"
#include "HDF5Handler.h"

//AdvancedOutput::tradeDataSave::operator const double*(){
//	return new double[this->size()][tradeDataPoint::size](this);
//}


AdvancedOutput::AdvancedOutput(INIReader& reader){


//	auto hasEnding =[this] (std::string const &fullString, std::string const &ending) {
//		if (fullString.length() >= ending.length()) {
//			return (0 == fullString.compare (fullString.length() - ending.length(), ending.length(), ending));
//		} else {
//			return false;
//		}
//	};



	std::string filetype = reader.Get("output", "filetype", "hdf5");
	this->type = AdvancedOutput::iniInputToFileType(filetype);



	switch(this->type ){
		case SQLITE:
			this->handler = new SQLiteHandler();
			break;
		default:
		case HDF5:
			this->handler = new HDF5FileHandler();
			break;
	}



}
AdvancedOutput::~AdvancedOutput()
{
	delete handler;

}
void AdvancedOutput::logBirth(const Cow *c){
	if(c == nullptr) return;
	this->handler->logBirth(c);
}

void AdvancedOutput::logFarms(const double time,const std::vector< Farm* >*farms){
	this->handler->logFarms(time, farms);
}
void AdvancedOutput::logEvent(const Event *e){
	if(e == nullptr) return;
	this->handler->logEvent(e);
}


void AdvancedOutput::write_to_file(double time){
	this->handler->write_to_file(time);
}
void AdvancedOutput::logResultingEventOfInfection(const Event* e){
	this->handler->logResultingEventOfInfection( e);
}

std::string AdvancedOutput::fileTypeToExtension(fileType type){
	std::string extension = "";
	switch(type){
		case HDF5:
			extension = ".h5";
			break;
		case SQLITE:
		 	extension = ".sqlite";
			break;
	}
	return extension;
}

fileType AdvancedOutput::iniInputToFileType(std::string& fileType, bool shallExit){
	if(fileType.compare("hdf5") != 0 && fileType.compare("sqlite") != 0 ){std::cout << "The specified filetype is not supported" <<std::endl;
		if(shallExit)
			exit(3);
	}else if(fileType.compare("sqlite") == 0){
		return SQLITE;
	}else{
		return HDF5;
	}

}
