#ifndef CLIENT_H_
#define CLIENT_H_

#include <string>
#include <vector>
#include <map>

#include "../Cola.h"
#include "../QueueMessage.h"
#include "../Logger.h"
using namespace std;

class Client {
private:
  Cola<message_t>* queue;
  message_t request;
  void printRecords(record_t *records);

public:
  Client(const string& file, const char letter);
  ~Client();

  void consultDatabase();
  void consultDatabaseRecord(map<string, string> filters);
  void addDatabaseRecord(map<string, string> fields);

};

#endif /* CLIENT_H_ */
