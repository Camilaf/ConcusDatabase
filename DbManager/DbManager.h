#ifndef DBMANAGER_H_
#define DBMANAGER_H_

#include <string>

#include "../Cola.h"
#include "../QueueMessage.h"
#include "../Logger.h"
#include "Database.h"
using namespace std;

class DbManager {
private:
  Cola<message_t>* queue;
  message_t request;
  vector<message_t> response;
  Database* db;

  void consultDatabase();
  void consultRecord();
  void addRecord();
  void manageInvalidRequest();
  void sendRegisters(vector<record_t> results);
  message_t createResponse(long mtype, int command, vector<record_t> *records, bool next);

public:
  DbManager(const string& file, const char letter);
  ~DbManager();

  string getRequest();
  bool receiveRequest();
  bool processRequest();
  bool respondRequest();
  void persistDatabase();
};

#endif /* DBMANAGER_H_ */
