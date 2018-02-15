#include "SQLiteHandler.h"
#include "Cow.h"
#include "Farm.h"
#include "Events.h"
#include "Herd.h"
const std::string SQLiteHandler::eventTableName = "Event";
const std::string SQLiteHandler::cowTableName = "Cow";
const std::string SQLiteHandler::templateDBPath = "templates/databaseTemplate.sqlite";
SQLiteHandler::SQLiteHandler():FileHandler(){
	this->fileExtension = ".sqlite";
	this->setPath();
	struct stat buffer;
	bool exists = (stat (fullFilePath.c_str(), &buffer) == 0);
	if(!exists || this->overwrite){
		std::ifstream  src(templateDBPath.c_str(), std::ios::binary);
		std::ofstream  dst(fullFilePath.c_str(),   std::ios::binary);

		dst << src.rdbuf();
	}
	int rc = sqlite3_open(":memory:", &this->pMemory);
	if( rc ){
		fprintf(stderr, "Can't open in memory database: %s\n", sqlite3_errmsg(this->pMemory));
		exit(-5000);
	}
	this->loadOrSaveDb(this->pMemory, templateDBPath.c_str(), 0);

}

SQLiteHandler::~SQLiteHandler(){
	sqlite3_close(this->pMemory);

}

void SQLiteHandler::logFarms(const double time,const std::vector< Farm* >*farms){}
void SQLiteHandler::write_to_file(const double time){

	//int rc = this->loadOrSaveDb(this->pMemory, this->fullFilePath.c_str(), 1);

}

inline void SQLiteHandler::runSQLQuery(const std::ostringstream& sqlstream){


   	char *zErrMsg = 0;
   	int rc;
   	const std::string sqlStr = sqlstream.str();
	char* sql =(char*) sqlStr.c_str();


	rc = sqlite3_exec(this->pMemory, sql, NULL/*callback*/, 0, &zErrMsg);
   	if( rc != SQLITE_OK ){
    	fprintf(stderr, "SQL error: %s\n", zErrMsg);
    	sqlite3_free(zErrMsg);
   	}

}

void SQLiteHandler::logEvent(const Event* e){
	std::ostringstream sql;

   /* Create SQL statement */

	   sql << "INSERT INTO '";
	   sql << eventTableName;
	   sql << "' (type_id,exec_time,farm_id) ";
	   sql << "VALUES (";
	   sql << (int) e->type;
	   sql << ",";
	   sql << e->execution_time;
	   sql << " , ";
	   sql << (e->farm != NULL ? e->farm->id : -1);
	   sql << ");";

		this->runSQLQuery(sql);
}
void SQLiteHandler::logResultingEventOfInfection(const Event* e){}
void SQLiteHandler::logBirth(const Cow *c){
	if(c == NULL) return;

		std::ostringstream sql;

   /* Create SQL statement */

	   sql << "INSERT INTO '";
	   sql << cowTableName;
	   sql << "' (id,time_of_birth,mother_id,birth_farm_id, birth_health_status_id, sex) ";
	   sql << "VALUES (";
	   sql << (int) c->id() << ",";
	   sql << c->birth_time << ",";
	   sql << (c->mother != NULL ? c->mother->id() : -1) << ",";
	  	sql << ((c->herd != NULL && c->herd->farm != NULL) ? c->herd->farm->id : -1) << ",";
	   sql << (int) c->infection_status;
	   sql << " , ";
	   sql << (int) c->female ;
	   sql << ");";

		this->runSQLQuery(sql);


}
int SQLiteHandler::loadOrSaveDb(sqlite3 *pInMemory, const char *zFilename, int isSave){ //copied from http://www.sqlite.org/backup.html
  int rc;                   /* Function return code */
  sqlite3 *pFile;           /* Database connection opened on zFilename */
  sqlite3_backup *pBackup;  /* Backup object used to copy data */
  sqlite3 *pTo;             /* Database to copy to (pFile or pInMemory) */
  sqlite3 *pFrom;           /* Database to copy from (pFile or pInMemory) */

  /* Open the database file identified by zFilename. Exit early if this fails
  ** for any reason. */
  rc = sqlite3_open(zFilename, &pFile);
  if( rc==SQLITE_OK ){

    /* If this is a 'load' operation (isSave==0), then data is copied
    ** from the database file just opened to database pInMemory.
    ** Otherwise, if this is a 'save' operation (isSave==1), then data
    ** is copied from pInMemory to pFile.  Set the variables pFrom and
    ** pTo accordingly. */
    pFrom = (isSave ? pInMemory : pFile);
    pTo   = (isSave ? pFile     : pInMemory);

    /* Set up the backup procedure to copy from the "main" database of
    ** connection pFile to the main database of connection pInMemory.
    ** If something goes wrong, pBackup will be set to NULL and an error
    ** code and message left in connection pTo.
    **
    ** If the backup object is successfully created, call backup_step()
    ** to copy data from pFile to pInMemory. Then call backup_finish()
    ** to release resources associated with the pBackup object.  If an
    ** error occurred, then an error code and message will be left in
    ** connection pTo. If no error occurred, then the error code belonging
    ** to pTo is set to SQLITE_OK.
    */
    pBackup = sqlite3_backup_init(pTo, "main", pFrom, "main");
    if( pBackup ){
      (void)sqlite3_backup_step(pBackup, -1);
      (void)sqlite3_backup_finish(pBackup);
    }
    rc = sqlite3_errcode(pTo);

  }
if(rc != SQLITE_OK){
	    std::cout << "dieeeeee" << std::endl;
	    exit(-5001);

    }
  /* Close the database connection opened on database file zFilename
  ** and return the result of this function. */
  (void)sqlite3_close(pFile);
  return rc;
}
