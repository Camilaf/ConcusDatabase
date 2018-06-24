#ifndef QUEUEMESSAGE_H_
#define QUEUEMESSAGE_H_

#include "DbRecord.h"

#define QUEUE_FILE "../DbRecord.h"
#define QUEUE_LETTER 'A'

#define MANAGER_ID 10
#define GET_ALL 1
#define GET_WHERE 2
#define ADD_RECORD 3
#define INVALID_CMD -1

#define RECORD_LIMIT 10

typedef struct message {
  long mtype;
  int pid;
  int command;
  record_t dbRecords[RECORD_LIMIT];
  bool next;
} message_t;

#endif /* QUEUEMESSAGE_H_ */
