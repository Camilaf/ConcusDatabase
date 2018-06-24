#ifndef DATABASE_H_
#define DATABASE_H_

#include <vector>
#include <fstream>
#include <sstream>

#include "../DbRecord.h"
#include "../Logger.h"
using namespace std;

class Database {
private:
  vector<record_t> records;
  string fileName;
  void recoverRecords();
  bool checkRepeatedRecord(record_t record);
  bool filteredElement(record_t element, record_t filters);

public:
  Database(const string& file);
  ~Database();

  vector<record_t> selectAll();
  vector<record_t> selectWhere(record_t filters);
  int addRecord(record_t record);
  void persist();
};

#endif /* DATABASE_H_ */
