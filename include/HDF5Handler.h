#ifndef __HDF5Handler_h_
#define __HDF5Handler_h_
#include "FileHandler.h"
#include <vector>
#include "hdf5.h"
#include "hdf5_hl.h"
#include <fstream>
#include "TableBasedOutput.h"
// #include <stdlib.h>

class HDF5FileHandler: public TableBasedOutput{
	public:
		HDF5FileHandler();
		~HDF5FileHandler();




	private:
		void writeLivingPIsToFile(const hid_t& file);
		void writeFarmData(const hid_t& file);
		void writeCowData(const hid_t& file);
		void writeTestData(const hid_t& file);
		void writeInfectionResultData(const hid_t& file);
		void writeTradeData(const hid_t& file);
		void writeVaccinationData(const hid_t& file);
		template<typename T>
		void writeTemplateSaveToFile(const hid_t& file, std::vector<T>* save,const std::string tableName);


		hid_t open_file(const std::string filename ,const bool overwrite);
		void write_to_file(const double time);
		void writeSaveToFile(const hid_t& file, CowDataSave* save,const std::string tableName);
		template<typename T, typename returnType>
		void createWritableData(std::vector<T>* save, returnType** data);

		bool deleteList;
		std::string fileExtension;


};


#endif
