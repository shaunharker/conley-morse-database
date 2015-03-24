#ifndef _sql_h
#define _sql_h

#include <sqlite3.h>

// 1st string : column title
// 2nd string : data type
typedef std::pair<std::string,std::string>  SQLColumn;
// column title and its value (here int by default)
typedef std::pair<std::string,int>  SQLColumnData;

template <typename T>
std::string NumberToString ( T Number );

static int callback(void *NotUsed, int argc, char **argv, char **azColName);


void insertMorseSetRecord ( sqlite3 *db,
                            const std::string & tablename,
                            const std::vector < SQLColumnData > & data);


void createMainTableSQLDatabase ( sqlite3 *db, 
                                  const std::string & name,
                                  const std::vector <SQLColumn> & columns );

template <typename T>
std::string NumberToString ( T Number )
{
  std::stringstream ss;
  ss << Number;
  return ss.str();
}


void insertMorseSetRecord ( sqlite3 *db,
                            const std::string & tablename,
                            const std::vector < SQLColumnData > & data) { 

  std::string sqlstring, sqlstring1, sqlstring2;
  sqlstring1 = "INSERT INTO " + tablename + " ( ";
  sqlstring2 = "VALUES ( ";
  for ( unsigned int i=0; i<data.size()-1; ++i ) {
    sqlstring1 += data[i].first;
    sqlstring1 += ", ";

    sqlstring2 += NumberToString(data[i].second);
    sqlstring2 += ", ";
  }
  sqlstring1 += data[data.size()-1].first;
  sqlstring1 += " ) ";
  sqlstring2 += NumberToString(data[data.size()-1].second);
  sqlstring2 += ")";
  sqlstring = sqlstring1 + sqlstring2;
  // std::cout << sqlstring << "\n";
  /* Execute SQL statement */
  char *zErrMsg = 0;
  int rc;
  rc = sqlite3_exec(db, sqlstring.c_str(), callback, 0, &zErrMsg);
  if( rc != SQLITE_OK ){
    fprintf(stderr, "SQL error: %s\n", zErrMsg);
    sqlite3_free(zErrMsg);
    abort();
  }else{
  //        fprintf(stdout, "Records created successfully\n");
  }
}


static int callback(void *NotUsed, int argc, char **argv, char **azColName){
  int i;
  for(i=0; i<argc; i++){
    printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
  }
  printf("\n");
  return 0;
}


void createMainTableSQLDatabase ( sqlite3 *db, 
                                  const std::string & name,
                                  const std::vector <SQLColumn> & columns ) {
  char *zErrMsg = 0;
  int rc;
  std::string sqlstr;
  sqlstr = "CREATE TABLE " + name + " (";
  for ( unsigned int i=0; i<columns.size()-1; ++i ) {
    sqlstr += columns[i].first;
    sqlstr += " ";
    sqlstr += columns[i].second;
    sqlstr += ", ";
  }
  sqlstr += columns[columns.size()-1].first;
  sqlstr += " ";
  sqlstr += columns[columns.size()-1].second;
  sqlstr += ");";
  /* Execute SQL statement */
  rc = sqlite3_exec(db, sqlstr.c_str(), callback, 0, &zErrMsg);
  if( rc != SQLITE_OK ){
    fprintf(stderr, "SQL error: %s\n", zErrMsg);
    sqlite3_free(zErrMsg);
  }else{
    fprintf(stdout, "Table created successfully\n");
  }
}



#endif
