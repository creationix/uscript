#include "ae.h"

#define MAX_CLIENTS 16
aeEventLoop* loop;
int main() {
  loop = aeCreateEventLoop(MAX_CLIENTS + REDIS_EVENTLOOP_FDSET_INCR);
}
