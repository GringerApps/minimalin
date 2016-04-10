typedef void(* MessageCallback)(DictionaryIterator * iter, Tuple * tuple);
typedef void(* MessengerCallback)(DictionaryIterator * iter);

typedef struct {
  uint32_t key;
  MessageCallback callback;
} Message;

typedef struct {
  Message * messages;
  MessengerCallback callback;
  int32_t size;
} Messenger;

Messenger * messenger_create(const int32_t size, MessengerCallback callback, Message * messages);
Messenger * messenger_destroy(Messenger * messenger);
