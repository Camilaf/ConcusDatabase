#include "Client.h"

Client :: Client(const string& file, const char letter) {
  this->queue = new Cola<message_t> (file, letter);
  memset(&(this->request), 0, sizeof(message_t));
}

Client :: ~Client () {
  delete this->queue;
}

/* Se encarga de definir la solicitud a efectuar al gestor
 * con los parámetros que recibe y de enviar esa solicitud.
 */
int Client :: sendRequest(int command, map<string, string> *fields) {
  this->request.mtype = MANAGER_ID;
  this->request.pid = getpid();
  this->request.command = command;
  if (fields) {
    record_t record;
    strcpy(record.nombre, (*fields)["nombre"].c_str());
    strcpy(record.direccion, (*fields)["direccion"].c_str());
    strcpy(record.telefono, (*fields)["telefono"].c_str());
    this->request.dbRecords[0] = record;
  }
  return this->queue->escribir(this->request);
}

/* Método que recibe un array de struct record e imprime
 * en consola los elementos que tienen información.
 */
void Client :: printRecords(record_t *records, string msg) {
  size_t pos = 0;
  bool next = true;
  while (next && (pos < RECORD_LIMIT)) {
    record_t record = records[pos];
    if (string(record.nombre).size() > 0) {
      cout << "Nombre: " << record.nombre << " - Dirección: " << record.direccion << " - Teléfono: " << record.telefono << endl;
      pos++;
    }
    else
      next = false;
  }

  if (pos == 0)
    cout << msg << endl;
}

/* Método encargado de recibir la respuesta del gestor de la base de datos
 * a la consulta realizada, y de imprimir en la consola los registros.
 */
bool Client :: receiveRegisters(string msg) {
  message_t response;
  int result = this->queue->leer(getpid(), &response);
  if (result < 0) {
    perror("No se pudo leer el mensaje del gestor");
    return false;
  }

  cout << "Base de datos consultada. Registros:" << endl << endl;
  printRecords(response.dbRecords, msg);

  // Si la cantidad de registros supera RECORD_LIMIT los recibo también
  while (response.next) {
    result = this->queue->leer(getpid(), &response);
    if (result < 0) {
      perror("No se pudo leer el mensaje del gestor");
      return false;
    }
    printRecords(response.dbRecords, msg);
  }
  return true;
}

/* Se ocupa de definir y enviar el struct message que recibirá
 * el gestor en la cola de mensajes, indicando una consulta sobre
 * todos los registros de la base de datos. Recibe e imprime el
 * resultado.
 */
bool Client :: consultDatabase() {
  Logger :: getInstance()->registrar("Cliente " + to_string(getpid()) + " envía petición para consultar todos los registros de la base de datos");
  int result = sendRequest(GET_ALL, NULL);
  if (result < 0) {
    perror("No se pudo leer el mensaje del gestor");
    return false;
  }
  string noResults = "No hay registros en la base de datos";
  if (!receiveRegisters(noResults))
    return false;
  return true;
}

/* Se ocupa de definir y enviar el struct message que recibirá
 * el gestor en la cola de mensajes, indicando una consulta sobre
 * los registros de la base de datos que cumplan con los filtros
 * indicados por parámetro. Recibe e imprime el resultado.
 */
bool Client :: consultDatabaseRecord(map<string, string> filters) {
  Logger :: getInstance()->registrar("Cliente " + to_string(getpid()) + " envía petición para filtrar la base de datos");
  int result = sendRequest(GET_WHERE, &filters);
  if (result < 0) {
    perror("No se pudo escribir el mensaje al gestor");
    return false;
  }
  string noResults = "No se encontraron registros con esos valores en la base de datos";
  if (!receiveRegisters(noResults))
    return false;
  return true;
}

/* Se ocupa de definir y enviar el struct message que recibirá
 * el gestor en la cola de mensajes, indicando la inserción de
 * un registro en la base de datos. Recibe la respuesta a la
 * operación y la imprime por consola.
 */
bool Client :: addDatabaseRecord(map<string, string> fields) {
  Logger :: getInstance()->registrar("Cliente " + to_string(getpid()) + " envía petición para agregar un registro a la base de datos");
  int result = sendRequest(ADD_RECORD, &fields);
  if (result < 0) {
    perror("No se pudo escribir el mensaje al gestor");
    return false;
  }

  message_t response;
  result = this->queue->leer(getpid(), &response);
  if (result < 0) {
    perror("No se pudo leer el mensaje del gestor");
    return false;
  }

  if (response.command == INSERT_OK)
    cout << "Registro agregado con éxito a la Base de Datos" << endl;

  else if (response.command == REPEATED_RECORD)
    cout << "No se pudo insertar el registro: ya existe en la Base de Datos" << endl;

  else if (response.command == INSERT_ERROR)
    cout << "No se pudo insertar el registro en la Base de Datos ya que tiene datos vacíos" << endl;

  return true;
}
