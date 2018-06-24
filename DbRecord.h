#ifndef DBRECORD_H_
#define DBRECORD_H_

#define DB_FILE "database.txt"

#define INSERT_OK 0
#define INSERT_ERROR -1
#define REPEATED_RECORD -2

#define NAME_SIZE 61
#define ADDRESS_SIZE 120
#define TELEPHONE_SIZE 13

typedef struct record {
  char nombre[NAME_SIZE];
  char direccion[ADDRESS_SIZE];
  char telefono[TELEPHONE_SIZE];
} record_t;

#endif /* DBRECORD_H_ */
