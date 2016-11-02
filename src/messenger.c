#include <pebble.h>
#include <stdlib.h>
#include "messenger.h"

static void inbox_received_handler(DictionaryIterator * iter, void *context) {
  Messenger * messenger = (Messenger *) context;
  Tuple * tuple = dict_read_first(iter);
  while (tuple) {
    for(int i = 0; i < messenger->size; i++){
      Message m = messenger->messages[i];
      if(m.key == tuple->key && m.callback != NULL){
        m.callback(iter, tuple);
      }
    }
    tuple = dict_read_next(iter);
  }
  messenger->callback(iter);
}

Messenger * messenger_create(const int32_t size, MessengerCallback callback, Message * messages){
  Messenger * messenger = (Messenger *) malloc(sizeof(Messenger));
  int32_t array_size = size * sizeof(Message);
  messenger->messages = (Message *) malloc(array_size);
  memcpy(messenger->messages, messages, array_size);
  messenger->callback = callback;
  messenger->size = size;
  app_message_set_context(messenger);
  app_message_register_inbox_received(inbox_received_handler);
  app_message_open(2048, 2048);
  return messenger;
}

Messenger * messenger_destroy(Messenger * messenger){
  free(messenger->messages);
  free(messenger);
  return NULL;
}
