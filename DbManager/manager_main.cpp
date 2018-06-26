#include "DbManager.h"
#include "../Logger.h"
#include "../Signals/SIGINT_Handler.h"
#include "../Signals/SignalHandler.h"

/* Se encarga de determinar si el programa se inició en modo
 * debug.
 */
bool inDebugMode(int argc, char *argv[]) {
  return ((argc == 2) && (strcmp("-d", argv[1]) == 0));
}

/* Función principal del programa Gestor de la Base de Datos */
int main(int argc, char *argv[]) {
  bool debug = inDebugMode(argc, argv);
  Logger :: setDebug(debug);
  Logger :: getInstance()->registrar("---- Gestor de base de datos iniciado ----");

  // Signal handler
  SIGINT_Handler sigint_handler;
  SignalHandler :: getInstance()->registrarHandler(SIGINT, &sigint_handler);

  cout << endl;
  cout << "Iniciando gestor de la base de datos..." << endl << endl;

  Logger :: getInstance()->registrar("Creando server con PID: " + to_string(getpid()));
  DbManager manager(QUEUE_FILE, QUEUE_LETTER);

  while (sigint_handler.getGracefulQuit() == 0) {
    cout << "------------------------------------" << endl;
    cout << "Esperando peticiones..." << endl;
    if (!manager.receiveRequest()) {
      if (errno == EINTR)
        break;
      perror("Error al recibir la petición");
    }

    cout << "> Petición recibida: " << manager.getRequest() << endl;
    if (!manager.processRequest()) {
      if (errno == EINTR)
        break;
      perror("Error al procesar la petición");
    }
    cout << "> Petición procesada" << endl;
    if (!manager.respondRequest()) {
      if (errno == EINTR)
        break;
      perror("Error al responder la petición");
    }
    cout << "> Petición enviada" << endl;

  }

  cout << endl;
  cout << "Finalizando gestor..." << endl << endl;
  manager.persistDatabase();
  SignalHandler :: destruir();
  Logger :: destruir();

  return 0;
}
