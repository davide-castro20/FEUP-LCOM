#include <lcom/lcf.h>

#include <stdbool.h>
#include <stdint.h>

int (kbc_subscribe_int)(int *bit_no);
int (kbc_unsubscribe_int)();
int (kbc_poll)();
int (kbc_cmd)(uint8_t cmd);
int (kbc_write)(uint8_t data);
int (kbc_read)(uint8_t *data);
int (kbc_reenable_int)(uint8_t *command);


