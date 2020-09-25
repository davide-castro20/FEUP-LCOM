#ifndef _LCOM_I8042_H_
#define _LCOM_I8042_H_

#include <lcom/lcf.h>

/*Keyboard Macros*/

#define KBC_IRQ 1

#define OUT_BUF 0x60
#define STAT_REG 0x64
#define KBC_ARG_REG 0x60 //KBC commands arguments
#define KBC_CMD_REG 0x64 //KBC issue command register
#define PAR_ERR BIT(7)   //Parity error
#define TO_ERR BIT(6)    //Timeout error
#define AUX BIT(5)       //Mouse data
#define IBF BIT(1)       //Input buffer full
#define OBF BIT(0)       //Output buffer full
#define READ_CMD 0x20    //Command to read command byte
#define WRITE_CMD 0x60   //Command to write command byte

#define MAX_TRIES 4 //Number of tries of the loops

#define ESC_BREAKCODE 0x81

/*Mouse Macros */

#define MOUSE_IRQ 12
#define MOUSE_LB_PRESSED BIT(0)
#define MOUSE_RB_PRESSED BIT(1)
#define MOUSE_MB_PRESSED BIT(2)
#define MOUSE_X_DELTA_SIGN BIT(4)
#define MOUSE_Y_DELTA_SIGN BIT(5)
#define MOUSE_X_OVF BIT(6)
#define MOUSE_Y_OVF BIT(7)
#define MOUSE_CMD 0xD4
#define MOUSE_ENABLE_CMD 0xF4
#define MOUSE_DISABLE_CMD 0xF5
#define MOUSE_ACK_ERROR 0xFC
#define MOUSE_NACK 0xFE
#define MOUSE_STREAM 0xEA
#define MOUSE_READ 0xEB

#endif /* _LCOM_I8042_H */
