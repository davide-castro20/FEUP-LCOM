#include <lcom/lcf.h>

#include <stdbool.h>
#include <stdint.h>
#include "i8042.h"

int(mouse_subscribe_int)(int *bit_no);
void(mouse_ih)();
int (mouse_unsubscribe_int)();
int(mouse_send_cmd)(uint8_t cmd);
int(mouse_remote)();

void(handle_mouse)(struct mouse_ev *ev, int16_t *x_len, int16_t *tolerance);
struct mouse_ev(mouse_event)(struct packet *pp);

enum States {
  INITIAL,
  D_FIRSTLINE,
  ENDFIRSTLINE,
  D_SECONDLINE,
  COMPLETE
};

enum mouse_ev_t transitions;
