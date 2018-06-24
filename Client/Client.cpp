#include "Client.h"

Client :: Client(const string& file, const char letter) {
  this->queue = new Cola<message_t> (file, letter);
  memset(&(this->request), 0, sizeof(message_t));
}

Client :: ~Client () {
  //this->queue->destruir();
  delete this->queue;
}

void Client :: printRecords(record_t *records) {
  size_t pos = 0;
  bool next = true;
  while (next && (pos < RECORD_LIMIT)) {
    record_t record = records[pos];
    if (string(record.nombre).size() > 0) {
      cout << "Nombre: " << record.nombre << " - Direccion: " << record.direccion << " - Telefono: " << record.telefono << endl;
      pos++;
    }
    else
      next = false;
  }

  if (pos == 0)
    cout << "No hay registros en la base de datos" << endl;
}

void Client :: consultDatabase() {
  this->request.mtype = MANAGER_ID;
  this->request.pid = getpid();
  this->request.command = GET_ALL;
  int result = this->queue->escribir(this->request);
  if (result < 0) {
    if (errno == EINTR)
      return;
    perror("No se pudo escribir el mensaje al gestor");
  }

  message_t response;
  result = this->queue->leer(getpid(), &response);
  if (result < 0) {
    if (errno == EINTR)
      return;
    perror("No se pudo leer el mensaje del gestor");
  }

  cout << "Base de datos consultada. Registros:" << endl;
  printRecords(response.dbRecords);

  // Si la cantidad de registros supera RECORD_LIMIT los recibo tambien
  while (response.next) {
    result = this->queue->leer(getpid(), &response);
    if (result < 0) {
      if (errno == EINTR)
        return;
      perror("No se pudo leer el mensaje del gestor");
    }
    printRecords(response.dbRecords);
  }
}

void Client :: consultDatabaseRecord(map<string, string> filters) {
}

void Client :: addDatabaseRecord(map<string, string> fields) {
}