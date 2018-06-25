#ifndef PARSER_H_
#define PARSER_H_

#include <string>
#include <vector>
#include <map>
#include <algorithm>

#include "../Logger.h"

#define GET_ALL_CMD "selectall"
#define GET_WHERE_CMD "selectwhere"
#define ADD_CMD "insert"
#define EXIT_CMD "exit"

using namespace std;

class Parser {
private:
  static Parser* instance;
  vector<string> tokenizeString(string str);
  bool hasData(string element, size_t start, size_t end);
  bool invalidFields(vector<string> tokens, bool allFields);

public:
  Parser();
  ~Parser();

  static Parser* getInstance();
  static void destruir();
  string toLower(string input);
  map<string, string> parseData(string statement);
  bool invalidSyntax(string statement);
};

#endif /* PARSER_H_ */
