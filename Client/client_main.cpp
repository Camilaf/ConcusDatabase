#include "Client.h"
#include "../Logger.h"
#include "Parser.h"

/* Función que imprime el menú del programa Cliente.
 */
void showMenu() {
  cout << endl;
  cout << "************************** Menú Principal **************************" << endl<< endl;
  cout << "1. Ingresar 'SELECTALL' para consultar todos los registros de la base de datos." << endl << endl;
  cout << "2. Utilizar 'SELECTWHERE' para consultar con filtros." << endl;
  cout << "     Ejemplo: SELECTWHERE nombre=\"Maria\" direccion=\"Av. Paseo Colon 850\" telefono=\"1156745323\"" << endl << endl;
  cout << "3. Utilizar 'INSERT' para agregar un registro." << endl;
  cout << "     Ejemplo: INSERT nombre=\"Maria\" direccion=\"Av. Paseo Colon 850\" telefono=\"1156745323\"" << endl << endl;
  cout << "4. Ingresar 'EXIT' para salir" << endl << endl;
  cout << "********************************************************************" << endl<< endl;
}

/* Función que se encarga de verificar que la sentencia SQL
 * ingresada cumpla con el formato pedido por el menú. Solicita
 * una sentencia válida hasta recibirla y luego de eso la devuelve.
 */
string getValidStatement() {
  string statement;
  cout << ">>> ";
  getline(cin, statement);

  while (Parser :: getInstance()->invalidSyntax(statement)) {
    Logger :: getInstance()->registrar("Cliente: Petición con comando inválido '" + statement + "'");
    cout << "Opción inválida, intente nuevamente." << endl;
    cout << ">>> ";
    getline(cin, statement);
  }
  return statement;
}

/* Se encarga de determinar si el programa se inició en modo
 * debug.
 */
bool inDebugMode(int argc, char *argv[]) {
  return ((argc == 2) && (strcmp("-d", argv[1]) == 0));
}

/* Función principal del programa Cliente */
int main(int argc, char *argv[]) {
  bool debug = inDebugMode(argc, argv);
  Logger :: setDebug(debug);
  Logger :: getInstance()->registrar("------ Nuevo cliente ------");

  cout << endl;
  cout << "Iniciando cliente de la base de datos..." << endl << endl;

  Logger :: getInstance()->registrar("Creando cliente con PID: " + to_string(getpid()));
  Client client(QUEUE_FILE, QUEUE_LETTER);
  cout << "BIENVENIDO AL CLIENTE DE LA BASE DE DATOS!" << endl << endl;

  string command = "";
  while (command != EXIT_CMD) {
    showMenu();
    string statement = getValidStatement();
    cout << endl << endl;
    command = Parser :: getInstance()->toLower(statement.substr(0, statement.find(" ")));

    if (command == GET_ALL_CMD) {
      Logger :: getInstance()->registrar("Cliente " + to_string(getpid()) + " quiere consultar todos los registros de la base de datos");
      if (!client.consultDatabase())
        break;
    }

    if (command == GET_WHERE_CMD) {
      Logger :: getInstance()->registrar("Cliente " + to_string(getpid()) + " quiere consultar la base de datos utilizando filtros");
      map<string, string> filters = Parser :: getInstance()->parseData(statement);
      if (!client.consultDatabaseRecord(filters))
        break;
    }

    if (command == ADD_CMD) {
      Logger :: getInstance()->registrar("Cliente " + to_string(getpid()) + " quiere agregar un registro a la base de datos");
      map<string, string> fields = Parser :: getInstance()->parseData(statement);
      if(!client.addDatabaseRecord(fields))
        break;
    }
  }

  Logger :: getInstance()->registrar("Cliente " + to_string(getpid()) + " ingreso el comando de salida");
  cout << "Finalizando cliente..." << endl << endl;
  Logger :: destruir();
  Parser :: destruir();

  return 0;
}
