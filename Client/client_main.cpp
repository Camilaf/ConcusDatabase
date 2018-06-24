#include <algorithm>

#include "Client.h"
#include "../Logger.h"

#define GET_ALL_CMD "selectall"
#define GET_WHERE_CMD "selectwhere"
#define ADD_CMD "insert"
#define EXIT_CMD "exit"

void showMenu() {
  cout << endl;
  cout << "#*********************** Menu Principal ***********************#" << endl<< endl;
  cout << "- Ingresar 'SELECTALL' para consultar todos los registros de la base de datos." << endl << endl;
  cout << "- Utilizar 'SELECTWHERE' para consultar con filtros." << endl;
  cout << "     Ejemplo: SELECTWHERE nombre=\"Maria\" direccion=\"Av. Paseo Colon 850\" telefono=\"1156745323\"" << endl << endl;
  cout << "- Utilizar 'INSERT' para agregar un registro." << endl;
  cout << "     Ejemplo: INSERT nombre=\"Maria\" direccion=\"Av. Paseo Colon 850\" telefono=\"1156745323\"" << endl << endl;
  cout << "- Ingresar 'exit' para salir" << endl << endl;
}

vector<string> tokenizeString(string str) {
  vector<string> tokens;
  string token = "";
  bool firstSpace = true;

  for (size_t i = 0; i < str.size(); i++) {
    if (str[i] == ' ') {
      if (firstSpace || (str[i - 1] == '"') || (str[i - 1] == '\'')) {
        if (!token.empty()) {
          tokens.push_back(token);
          token = "";
          if (firstSpace)
            firstSpace = false;
        }
      }
      else
        token += str[i];
    }
    else
      token += str[i];
  }

  if (!token.empty()) {
    tokens.push_back(token);
  }

  return tokens;
}

string toLower(string input) {
  transform(input.begin(), input.end(), input.begin(), ::tolower);
  return input;
}

bool hasData(string element, size_t start, size_t end) {
  // Consideramos las comillas que forman parte del dato
  if (end - start <= 1) {
    return false;
  }

  char firstElement = element[start];
  char lastElement = element[end];

  return (((firstElement == '"') && (lastElement  == '"')) || ((firstElement == '\'') && (lastElement  == '\'')));
}

bool invalidFields(vector<string> tokens, bool allFields) {
  size_t fieldsNumber = tokens.size() - 1;
  size_t counter = 0;

  // El INSERT debe tener todos los campos
  if (allFields && (fieldsNumber != 3))
    return true;

  // El SELECTWHERE debe tener al menos un campo
  else if (!allFields) {
    if (fieldsNumber < 1)
      return true;
  }

  for (size_t i = 1; i < tokens.size(); i++) {
    size_t endOfCommand =  tokens[i].find("=");

    // Verificamos que tenga el '=' y que luego de eso contenga algun dato
    if ((endOfCommand == string::npos) || (endOfCommand == tokens[i].size() - 1))
      return true;

    if (!hasData(tokens[i], endOfCommand + 1, tokens[i].size() - 1))
      return true;

    string field = tokens[i].substr(0, endOfCommand);
    if (field == "nombre")
      counter++;

    else if (field == "direccion")
      counter++;

    else if (field == "telefono")
      counter++;
  }
  return (counter != fieldsNumber);
}

map<string, string> parseData(string statement) {
  vector<string> tokens = tokenizeString(statement);
  map<string, string> data;
  for (size_t i = 1; i < tokens.size(); i++) {
    size_t endOfCommand =  tokens[i].find("=");
    string field = tokens[i].substr(0, endOfCommand);

    // Elimino las comillas para almacenar el dato
    string value = tokens[i].substr(endOfCommand + 2, tokens[i].find('"', endOfCommand + 2) - (endOfCommand + 2));
    data.insert(pair<string, string> (field, value));
  }
  return data;
}

bool invalidSyntax(string statement) {
  vector<string> tokens = tokenizeString(statement);
  if (tokens.size() == 0)
    return true;

  string command = toLower(tokens[0]);
  if (command == GET_ALL_CMD) {
    if (tokens.size() > 1) {
      Logger :: getInstance()->registrar("Cliente: Peticion con comando invalido: '" + statement + "'");
      return true;
    }
  }

  else if (command == GET_WHERE_CMD) {
    if (tokens.size() > 4 || invalidFields(tokens, false)) {
      Logger :: getInstance()->registrar("Cliente: Peticion con comando invalido: '" + statement + "'");
      return true;
    }
  }

  else if (command == ADD_CMD) {
    if (tokens.size() > 4 || invalidFields(tokens, true)) {
      Logger :: getInstance()->registrar("Cliente: Peticion con comando invalido: '" + statement + "'");
      return true;
    }
  }

  else if (command == EXIT_CMD) {
    if (tokens.size() > 1)
      return true;
  }

  else {
    Logger :: getInstance()->registrar("Cliente: Peticion con comando invalido: '" + statement + "'");
    return true;
  }

  Logger :: getInstance()->registrar("Cliente: Peticion con comando valido: '" + statement + "'");
  return false;
}

string getValidStatement() {
  string statement;
  getline(cin, statement);

  while (invalidSyntax(statement)) {
    Logger :: getInstance()->registrar("Cliente: Peticion con comando invalido '" + statement + "'");
    cout << "Ingrese una opcion valida: ";
    getline(cin, statement);
  }
  return statement;
}

bool inDebugMode(int argc, char *argv[]) {
  return ((argc == 2) && (strcmp("-d", argv[1]) == 0));
}

int main(int argc, char *argv[]) {
  bool debug = inDebugMode(argc, argv);
  Logger :: setDebug(debug);
  Logger :: getInstance()->registrar("------ Nuevo cliente ------");

  string statement = "";
  cout << endl;
  cout << "Iniciando cliente..." << endl << endl;

  Logger :: getInstance()->registrar("Creando cliente con PID: " + to_string(getpid()));
  Client client(QUEUE_FILE, QUEUE_LETTER);

  while (toLower(statement) != EXIT_CMD) {
    showMenu();
    statement = getValidStatement();
    cout << endl;
    cout << "Comando correcto: '" << statement <<  "'" << endl << endl;
    string command = toLower(statement.substr(0, statement.find(" ")));

    if (command == GET_ALL_CMD) {
      Logger :: getInstance()->registrar("Cliente " + to_string(getpid()) + " quiere consultar todos los registros de la base de datos");
      client.consultDatabase();
    }

    if (command == GET_WHERE_CMD) {
      Logger :: getInstance()->registrar("Cliente " + to_string(getpid()) + " quiere consultar la base de datos utilizando filtros");
      map<string, string> filters = parseData(statement);
      client.consultDatabaseRecord(filters);
    }

    if (command == ADD_CMD) {
      Logger :: getInstance()->registrar("Cliente " + to_string(getpid()) + " quiere agregar un registro a la base de datos");
      map<string, string> fields = parseData(statement);
      client.addDatabaseRecord(fields);
    }
  }

  Logger :: getInstance()->registrar("Cliente " + to_string(getpid()) + " ingreso el comando de salida");
  cout << "Finalizando cliente..." << endl << endl;
  Logger :: destruir();

  return 0;
}
