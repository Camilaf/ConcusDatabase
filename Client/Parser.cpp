#include "Parser.h"

Parser* Parser :: instance = NULL;

Parser :: Parser() {
}

Parser* Parser :: getInstance() {
  if (instance == NULL) {
    instance = new Parser();
  }
  return instance;
}

Parser :: ~Parser () {
}

void Parser :: destruir() {
  if (instance != NULL) {
    delete(instance);
    instance = NULL;
  }
}

/* Recibe un string separado por espacios y devuelve un vector
 * con cada una de las palabras del string. Respeta los espacios
 * dentro de una misma "palabra" cuando no le precede una comilla,
 * excepto que se trate del primer espacio encontrado en el string.
 */
vector<string> Parser :: tokenizeString(string str) {
  vector<string> tokens;
  string token = "";
  bool firstSpace = true;

  for (size_t i = 0; i < str.size(); i++) {
    if (str[i] == ' ') {
      if (firstSpace || (str[i - 1] == '"')) {
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

/* Recibe un string y devuelve el mismo string pero con
 * todos sus caracteres en minúscula.
 */
string Parser :: toLower(string input) {
  transform(input.begin(), input.end(), input.begin(), ::tolower);
  return input;
}

/* Recibe un string con el atributo y su valor, la posición de
 * inicio y la de fin del valor (correspondientes a las comillas
 * de la sintaxis) y determina si dentro de las comillas se ingresó
 * información y si el cliente consultó utilizando las comillas
 * correctamente.
 */
bool Parser :: hasValidData(string element, size_t start, size_t end, string field) {
  // Verificamos que exista algún dato dentro de las comillas
  size_t dataLength = end - start;
  size_t maxLength;
  if (field == "nombre")
    maxLength = NAME_SIZE;
  else if (field == "direccion")
    maxLength = ADDRESS_SIZE;
  else if (field == "telefono")
    maxLength = TELEPHONE_SIZE;
  else
    return false;


  if ((dataLength <= 1) || (dataLength > maxLength)) {
    return false;
  }

  char firstElement = element[start];
  char lastElement = element[end];
  return ((firstElement == '"') && (lastElement  == '"'));
}

/* Recibe un vector con cada campo y su valor asignado, junto con un
 * booleano que indica si el comando requiere que el cliente ingrese
 * obligatoriamente o no los tres campos: nombre, dirección y teléfono.
 * Determina si ingresó campos inválidos, o en distinta cantidad de la
 * debida.
 * Formato de token correcto dentro del vector: nombre="Juan"
 */
bool Parser :: invalidFields(vector<string> tokens, bool allFields) {
  size_t fieldsNumber = tokens.size() - 1;
  size_t counter = 0;
  bool hasName = false;
  bool hasAddress = false;
  bool hasTelephone = false;

  // El INSERT debe tener todos los campos
  if (allFields && (fieldsNumber != 3))
    return true;

  // El SELECTWHERE debe tener al menos un campo
  else if (!allFields) {
    if (fieldsNumber < 1)
      return true;
  }

  for (size_t i = 1; i < tokens.size(); i++) {
    size_t endOfField =  tokens[i].find("=");

    // Verificamos que tenga el '=' y que luego de eso contenga algún dato
    if ((endOfField == string::npos) || (endOfField == tokens[i].size() - 1))
      return true;

    string field = tokens[i].substr(0, endOfField);
    if (!hasValidData(tokens[i], endOfField + 1, tokens[i].size() - 1, field))
      return true;

    if (!hasName && (field == "nombre")) {
      counter++;
      hasName = true;
    }

    else if (!hasAddress && (field == "direccion")) {
      counter++;
      hasAddress = true;
    }

    else if (!hasTelephone && (field == "telefono")) {
      counter++;
      hasTelephone = true;
    }
  }

  // Verificamos que tenga los campos correctos
  return (counter != fieldsNumber);
}

/* Recibe un string con el comando, atributos y valores;
 * y devuelve un diccionario mapeando los campos con sus
 * valores correspondientes.
 */
map<string, string> Parser :: parseData(string statement) {
  vector<string> tokens = tokenizeString(statement);
  map<string, string> data;
  for (size_t i = 1; i < tokens.size(); i++) {
    size_t endOfCommand =  tokens[i].find("=");
    string field = tokens[i].substr(0, endOfCommand);

    // Elimino las comillas para almacenar el dato
    size_t endOfValue = tokens[i].find('"', endOfCommand + 2) - (endOfCommand + 2);
    string value = tokens[i].substr(endOfCommand + 2, endOfValue);
    data.insert(pair<string, string> (field, value));
  }
  return data;
}

/* Recibe una sentencia SQL y determina si es una sintaxis
 * inválida o no, de acuerdo a lo definido por el programa
 * Cliente.
 * Ejemplo sentencia válida:
 * insert nombre="Juana" direccion="Arenales 234" telefono="45678990"
 */
bool Parser :: invalidSyntax(string statement) {
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
