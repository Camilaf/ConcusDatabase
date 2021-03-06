#include "Database.h"

Database :: Database(const string& file) {
  this->fileName = file;
  recoverRecords();
}


/* Método que se ocupa de recuperar los registros en la base de datos almacenada
 * para la nueva sesión del gestor de la base de datos. Si no existe la base de
 * datos, no toma ningún dato y comienza con la base de datos vacía.
 */
void Database :: recoverRecords() {
  ifstream database(this->fileName.c_str());
  Logger :: getInstance()->registrar("Se abre el archivo que contiene a la base de datos en caso de existir");

  if (database.fail()) {
    Logger :: getInstance()->registrar("No hay una base de datos existente: comienza vacía");
  }
  else {
    string record;
    string name;
    string address;
    string telephone;

    // Obtengo los registros de la base de datos
    while (getline(database, record)) {
      stringstream linestream(record);
      getline(linestream, name, ',');
      getline(linestream, address, ',');
      getline(linestream, telephone, ',');

      record_t dbRecord;
      strcpy(dbRecord.nombre, name.c_str());
      strcpy(dbRecord.direccion, address.c_str());
      strcpy(dbRecord.telefono, telephone.c_str());
      Logger :: getInstance()->registrar("Se recuperó el registro: {nombre: " + string(dbRecord.nombre) + ", direccion: " + string(dbRecord.direccion) + ", telefono: " + string(dbRecord.telefono) + "}");
      this->records.push_back(dbRecord);
    }
    database.close();
  }
}

Database :: ~Database () {
}

bool isEmpty(char* field) {
  return (strlen(field) == 0);
}

bool emptyFields(record_t record) {
  return (isEmpty(record.nombre) || isEmpty(record.direccion) || isEmpty(record.telefono));
}

/* Devuelve todos los registros de la base de datos
 */
vector<record_t> Database :: selectAll() {
  return this->records;
}


/* Dado un registro y filtros sobre los campos del mismo, determina
 * si ese registro cumple con todas las condiciones impuestas por los
 * filtros, en caso de existir.
 */
bool Database :: filteredElement(record_t element, record_t filters) {
  if (!isEmpty(filters.nombre) && (strcmp(filters.nombre, element.nombre) != 0))
    return false;

  if (!isEmpty(filters.direccion) && (strcmp(filters.direccion, element.direccion) != 0))
    return false;

  if (!isEmpty(filters.telefono) && (strcmp(filters.telefono, element.telefono) != 0))
    return false;

  return true;
}


/* Dado un struct con filtros sobre los campos de la base de datos,
 * selecciona y devuelve un vector con aquellos registros que cumplen
 * con todos los valores indicados en el struct de filtros.
 */
vector<record_t> Database :: selectWhere(record_t filters) {
  vector<record_t> result;
  for (size_t i = 0; i < this->records.size(); i++) {
    if (filteredElement(this->records[i], filters)) {
      Logger :: getInstance()->registrar("Se filtró el registro { " + string(this->records[i].nombre) + ", " + string(this->records[i].direccion) + ", " + string(this->records[i].telefono) + " }");
      result.push_back(this->records[i]);
    }
  }
  return result;
}

bool equalRecords(record_t record, record_t otherRecord) {
  string name = record.nombre;
  string otherName = otherRecord.nombre;

  string address = record.direccion;
  string otherAddress = otherRecord.direccion;

  string telephone = record.telefono;
  string otherTelephone = otherRecord.telefono;
  return ((name == otherName) && (address == otherAddress) && (telephone == otherTelephone));
}

/* Dado un registro, determina si el mismo ya existe en la base
 * de datos.
 */
bool Database :: checkRepeatedRecord(record_t record) {
  for (size_t i = 0; i < this->records.size(); i++) {
    if (equalRecords(this->records[i], record)) {
      Logger :: getInstance()->registrar("Ya existe un registro exactamente igual al que se desea agregar");
      return true;
    }
  }
  Logger :: getInstance()->registrar("El registro a insertar no existe en la base de datos: se puede agregar");
  return false;
}


/* Se ocupa de insertar un registro en la base de datos, siempre
 * que el mismo no se encuentre en la base de datos y tenga todos
 * sus campos con información.
 */
int Database :: addRecord(record_t record) {
  if (emptyFields(record))
    return INSERT_ERROR;

  if (checkRepeatedRecord(record))
    return REPEATED_RECORD;

  this->records.push_back(record);
  return INSERT_OK;
}


/* Método encargado de persistir la base de datos en un archivo de texto.
 * Escribe los campos de cada registro separaos por coma.
 */
void Database :: persist() {
  ofstream database(this->fileName.c_str(), ios :: out | ios :: trunc);
  if (database.fail()) {
    perror("Error al abrir el archivo para persistir la base de datos");
  }
  else {
    Logger :: getInstance()->registrar("Almacenando la base de datos en forma permanente");
    for (size_t i = 0; i < this->records.size(); i++) {
      string name = this->records[i].nombre;
      string address = this->records[i].direccion;
      string telephone = this->records[i].telefono;
      database << name << ',' << address << ',' << telephone << endl;
    }
    Logger :: getInstance()->registrar("Base de datos almacenada en " + this->fileName);
    database.close();
  }
}
