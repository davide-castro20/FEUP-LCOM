#ifndef _LCOM_I8042_H_
#define _LCOM_I8042_H_

#include <lcom/lcf.h>

#define KBC_IRQ 1

#define OUT_BUF 0x60
#define STAT_REG 0x64
#define KBC_ARG_REG 0x60 //KBC commands arguments
#define KBC_CMD_REG 0x64 //KBC issue command register
#define PAR_ERR BIT(7) //Parity error
#define TO_ERR BIT(6) //Timeout error
#define AUX BIT(5) //Mouse data
#define IBF BIT(1) //Input buffer full
#define OBF BIT(0) //Output buffer full
#define READ_CMD 0x20 //Command to read command byte
#define WRITE_CMD 0x60 //Command to write command byte

#define MAX_TRIES 4 //Number of tries of the loops

#define ESC_BREAKCODE 0x81


#endif /* _LCOM_I8042_H */

