#include <lcom/lcf.h>

#include <stdint.h>

int(util_get_LSB)(uint16_t val, uint8_t *lsb) {

  //Here we check if the given pointer for the result is pointing to something valid
  if(lsb == NULL)
    return 1;

  //We convert the 16 bit value to 8 bit so we only keep the less significant byte
  *lsb = (uint8_t) val;	

  return 0;
}

int(util_get_MSB)(uint16_t val, uint8_t *msb) {

  //Here we check if the given pointer for the result is pointing to something valid
  if(msb == NULL)
    return 1;

  //We shift the original value 1 byte to the right so we only keep the original most significant byte
  uint16_t temp_msb = val >> 8;
  *msb = (uint8_t) temp_msb;

  return 0;
}

int (util_sys_inb)(int port, uint8_t *value) {
  
  //Here we check if the given pointer for the result is pointing to something valid
  if(value == NULL)
    return 1;

  //Temporary value for the return of the sys_inb funtion
  uint32_t temp_value = 0x0;

  //If the sys_inb function fails this function also fails
  if(sys_inb(port,&temp_value))            
    return 1;
                             
  //Cast from the return of the sys_inb funtion(32 bits) to the desired format(8 bits)
  *value = (uint8_t)temp_value;

  return 0;   //Success
}
