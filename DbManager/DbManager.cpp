#include "DbManager.h"

DbManager :: DbManager(const string& file, const char letter) {
  this->queue = new Cola<message_t> (file, letter);
  this->db = new Database(DB_FILE);
  memset(&(this->request), 0, sizeof(message_t));
}

DbManager :: ~DbManager () {
  this->queue->destruir();
  delete this->db;
  delete this->queue;
}

bool DbManager :: receiveRequest() {
  int res = this->queue->leer(MANAGER_ID, &(this->request));
  return (res >= 0);
}

message_t DbManager :: createResponse(long mtype, int command, vector<record_t> *records, bool next) {
  message_t message;
  message.mtype = mtype;
  message.command = command;
  memset(message.dbRecords, 0, sizeof(message.dbRecords));
  if (records)
    copy((*records).begin(), (*records).end(), message.dbRecords);

  message.next = next;
  return message;
}

bool limitReached(int recordNumber) {
  return (recordNumber % RECORD_LIMIT == 0);
}

bool lastRecord(int recordNumber, int databaseItems) {
  return (recordNumber == databaseItems);
}

void DbManager :: consultDatabase() {
   vector<record_t> database = this->db->selectAll();

   if (database.size() == 0) {
     Logger :: getInstance()->registrar("La base de datos consultada no tiene registros");
     message_t response = createResponse(this->request.pid, 0, NULL, false);
     this->response.push_back(response);
   }
   else {
     Logger :: getInstance()->registrar("La base de datos consultada tiene registros");
     vector<record_t> chunk;
     for (size_t i = 0; i < database.size(); i++) {
       chunk.push_back(database[i]);
       int recordNumber = i + 1;
       bool lastItem = lastRecord(recordNumber, database.size());
       if (limitReached(recordNumber) || lastItem) {
         message_t response = createResponse(this->request.pid, 0, &chunk, !lastItem);
         this->response.push_back(response);
         chunk.clear();
       }
     }
   }
}

void DbManager :: consultRecord() {
  record_t filters = this->request.dbRecords[0];
  vector<record_t> results = this->db->selectWhere(filters);
  if (results.size() == 0) {
    message_t response = createResponse(this->request.pid, 0, NULL, false);
    this->response.push_back(response);
  }
  else {
    // REFACTOR CON LO DE ARRIBA
    vector<record_t> chunk;
    for (size_t i = 0; i < results.size(); i++) {
      chunk.push_back(results[i]);
      int recordNumber = i + 1;
      bool lastItem = lastRecord(recordNumber, results.size());
      if (limitReached(recordNumber) || lastItem) {
        message_t response = createResponse(this->request.mtype, 0, &chunk, !lastItem);
        this->response.push_back(response);
        chunk.clear();
      }
    }
  }
}

void DbManager :: addRecord() {
  record_t record = this->request.dbRecords[0];
  int result = this->db->addRecord(record);

  message_t response = createResponse(this->request.pid, result, NULL, false);
  this->response.push_back(response);
}

void DbManager :: manageInvalidRequest() {
  message_t message;
  message.mtype = this->request.pid;
  message.command = INVALID_CMD;
  this->response.push_back(message);
}

bool DbManager :: processRequest() {
  Logger :: getInstance()->registrar("Recibe la peticion con PID: " + to_string(this->request.pid) + " command: " + to_string(this->request.command));
  int command = this->request.command;
  switch(command) {
    case GET_ALL:
      Logger :: getInstance()->registrar("Se pide consultar todos los registros de la base de datos");
      consultDatabase();
      break;
    case GET_WHERE:
      Logger :: getInstance()->registrar("Se pide consultar la base de datos mediante filtros");
      consultRecord();
      break;
    case ADD_RECORD:
      Logger :: getInstance()->registrar("Se pide agregar un registro en la base de datos");
      addRecord();
      break;
    default:
      Logger :: getInstance()->registrar("Peticion con comando invalido");
      manageInvalidRequest();
      return false;
  }
  return true;
}

bool DbManager :: respondRequest() {
  for (size_t i = 0; i < this->response.size(); i++) {
    int res = this->queue->escribir(this->response[i]);
    if ((res < 0) && (errno == EINTR))
      return false;
  }
  this->response.clear();
  return true;
}

void DbManager :: persistDatabase() {
  this->db->persist();
}
