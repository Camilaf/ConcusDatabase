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

string getCommand(int cmd) {
  string command = "";
  if (cmd == GET_ALL)
    command = "SELECTALL";
  else if (cmd == GET_WHERE)
    command = "SELECTWHERE";
  else if (cmd == ADD_RECORD)
    command = "INSERT";
  return command;
}

/* Devuelve un string con información sobre la solicitud
 * recibida.
 */
string DbManager :: getRequest() {
  int pid = this->request.pid;
  int cmd = this->request.command;
  string request = "";
  string command = getCommand(cmd);

  request += "Cliente PID " + to_string(pid) + ", comando " + command;
  if (cmd != GET_ALL) {
    record_t record = this->request.dbRecords[0];
    string name = record.nombre;
    string address = record.direccion;
    string telephone = record.telefono;

    if (!name.empty())
      request += ", nombre=" + name;
    if (!address.empty())
      request += ", direccion=" + address;
    if (!telephone.empty())
      request += ", telefono=" + telephone;

  }
  request += '\n';
  return request;
}

/* Método encargado de leer en la cola de mensajes la solicitud
 * de algún cliente, utilizando como tipo el ID del gestor definido,
 * sobre el cual los clientes envían las peticiones.
 */
bool DbManager :: receiveRequest() {
  int res = this->queue->leer(MANAGER_ID, &(this->request));
  return (res >= 0);
}

/* Método encargado de definir el mensaje de respuesta al cliente,
 * de acuerdo con los parámetros definidos.
 */
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

/* Recibe un vector con los registros a enviar. Se encarga de
 * definir un mensaje de respuesta al cliente de acuerdo a si
 * hay o no registros a enviar.
 */
void DbManager :: sendRegisters(vector<record_t> results) {
  if (results.size() == 0) {
    message_t response = createResponse(this->request.pid, 0, NULL, false);
    this->response.push_back(response);
  }
  else {
    vector<record_t> chunk;
    for (size_t i = 0; i < results.size(); i++) {
      chunk.push_back(results[i]);
      int recordNumber = i + 1;
      bool lastItem = lastRecord(recordNumber, results.size());
      if (limitReached(recordNumber) || lastItem) {
        message_t response = createResponse(this->request.pid, 0, &chunk, !lastItem);
        this->response.push_back(response);
        chunk.clear();
      }
    }
  }
}

/* Método encargado de consultar todos los registros de la base de
 * datos, y de definir el mensaje de respuesta correspondiente al cliente.
 */
void DbManager :: consultDatabase() {
   vector<record_t> database = this->db->selectAll();
   Logger :: getInstance()->registrar("Se obtienen todos los registros de la base de datos");
   sendRegisters(database);
}

/* Método encargado de filtrar los registros de la base de datos,
 * y de definir el mensaje de respuesta correspondiente al cliente.
 */
void DbManager :: consultRecords() {
  record_t filters = this->request.dbRecords[0];
  Logger :: getInstance()->registrar("Se utilizan los filtros { nombre: " + string(filters.nombre) + ", direccion: " + string(filters.direccion) + ", telefono: " + string(filters.telefono) + " }");
  vector<record_t> results = this->db->selectWhere(filters);
  Logger :: getInstance()->registrar("Se obtienen los registros filtrados de la base de datos");
  sendRegisters(results);
}

/* Método encargado de agregar un registro a la base de datos, y
 * de definir el mensaje de respuesta correspondiente al cliente.
 */
void DbManager :: addRecord() {
  record_t record = this->request.dbRecords[0];
  int result = this->db->addRecord(record);

  message_t response = createResponse(this->request.pid, result, NULL, false);
  this->response.push_back(response);
}

/* Se encarga de definir el mensaje de respuesta y prepararlo para su envío
 * para aquellos casos en donde haya algún error.
 */
void DbManager :: manageInvalidRequest() {
  message_t message;
  message.mtype = this->request.pid;
  message.command = INVALID_CMD;
  this->response.push_back(message);
}

/* Se encarga de procesar la solitud del cliente. De acuerdo con el comando
 * o acción a realizar, ejecuta distintos métodos que se encargan de realizar
 * esa acción, comunicándose con la base de datos y definiendo el mensaje de
 * respuesta.
 */
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
      consultRecords();
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

/* Se ocupa de responder la solicitud del cliente, enviando a la
 * cola de mensajes la respuesta definida.
 */
bool DbManager :: respondRequest() {
  for (size_t i = 0; i < this->response.size(); i++) {
    int res = this->queue->escribir(this->response[i]);
    if (res < 0)
      return false;
  }
  this->response.clear();
  return true;
}

/* Se encarga de la persistencia de la base de datos.
 */
void DbManager :: persistDatabase() {
  this->db->persist();
}
