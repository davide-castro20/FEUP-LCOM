#include <lcom/lcf.h>

#include "kbc.h"
#include "mouse.h"

extern uint8_t scancode;

//Status code to get from the status register
extern uint8_t stat; //Not sure if needed

//Hook id to be used to set the interrupt policy
int mouse_hook_id = 12;

//Used to check if the interrupt handler fails or not
extern int ih_error;

//eve is true if there is an event
extern bool eve; 

enum States state;

struct packet prevP;


void(mouse_ih)() {

  //Reads the status from the keyboard
  util_sys_inb(STAT_REG, &stat);

  //Checks if there is a parity or timeout error and if the output buffer ins't empty
  if (((stat & (PAR_ERR | TO_ERR)) == 0) && (stat & OBF)) {
    util_sys_inb(OUT_BUF, &scancode);
    ih_error = 0;
  }
  else
    ih_error = 1;
}

int(mouse_subscribe_int)(int *bit_no) {
  if (bit_no == NULL)
    return 1;

  /*Subscribe the interruption using also the IRQ_EXCLUSIVE policy to prevent
	Minix's IH from the reading the output buffer*/
  if (sys_irqsetpolicy(MOUSE_IRQ, (IRQ_REENABLE | IRQ_EXCLUSIVE), &mouse_hook_id))
    return 1;

  return 0;
}

int(mouse_unsubscribe_int)() {

  //Unsubscribing the interruptions
  if (sys_irqrmpolicy(&mouse_hook_id))
    return 1;

  return 0;
}

int(mouse_send_cmd)(uint8_t cmd) {

  uint8_t ack;

  do{

  if (kbc_cmd(MOUSE_CMD))
    return 1;

  if (kbc_write(cmd))
    return 1;

  if (kbc_read(&ack))
    return 1;

  if (ack == MOUSE_ACK_ERROR)
    return 1;

  } while(ack == MOUSE_NACK);

  return 0;
}

int(mouse_remote)() {

  //Reading of the status of the keyboard
  util_sys_inb(STAT_REG, &stat);

  //Checks if there is a parity or timeout error and if AUX is cleared
  if ((stat & (PAR_ERR | TO_ERR)) == 0) {
    //Check if the output buffer isn't empty
    if (stat & OBF) {
      //If it isn't empty read the scancode
      util_sys_inb(OUT_BUF, &scancode);
      return 0;
    }
  }
  return 1;
}

struct mouse_ev(mouse_event)(struct packet *pp) {
  
  struct mouse_ev newEv;

  //LB is pressed when no other button is pressed
  if ((prevP.lb == 0) && (pp->lb)) {
    if ((pp->rb) || (pp->mb))
      newEv.type = BUTTON_EV; //event BUTTON_EV for when there are more than 1 buttons pressed (used for failure on drawing the lines)
    else
      newEv.type = LB_PRESSED;
    eve = true;
  }

  //RB is pressed when no other button is pressed
  else if ((prevP.rb == 0) && (pp->rb)) {
    if ((pp->lb) || (pp->mb))
      newEv.type = BUTTON_EV; 
    else
      newEv.type = RB_PRESSED;
    eve = true;
  }
  else if ((prevP.lb) && (pp->lb == 0)) {
    newEv.type = LB_RELEASED;
    eve = true;
  }
  else if ((prevP.rb) && (pp->rb == 0)) {
    newEv.type = RB_RELEASED;
    eve = true;
  }
  else if ((pp->delta_x != 0) || (pp->delta_y != 0)) {
    newEv.type = MOUSE_MOV;
    newEv.delta_x = pp->delta_x;
    newEv.delta_y = pp->delta_y;
    eve = true;
  }
  else
    eve = false;

  prevP = *pp;
  return newEv;
}

void(handle_mouse)(struct mouse_ev *ev, int16_t *x_len, int16_t *tolerance) {

  static int16_t movX = 0; //total movement on x and y
  static int16_t movY = 0;

  //change the state according to the event and current state
  switch (state) {

    case INITIAL:
      if (ev->type == LB_PRESSED) {
        state = D_FIRSTLINE;
      }
      break;

    case D_FIRSTLINE:

      switch (ev->type) {

        case MOUSE_MOV:
          movX += ev->delta_x;
          movY += ev->delta_y;
          if (ev->delta_x < 0)
            if (-ev->delta_x > *tolerance) {
              movX = 0;
              movY = 0;
            }
          if (ev->delta_y < 0)
            if (-ev->delta_y > *tolerance) {
              movX = 0;
              movY = 0;
            }
          break;

        case LB_RELEASED:
          if ((movX >= *x_len) && (movY > movX)) {
            movX = 0;
            movY = 0;
            state = ENDFIRSTLINE;
          }
          else {
            movX = 0;
            movY = 0;
            state = INITIAL;
          }
          break;

        case BUTTON_EV:
          movX = 0;
          movY = 0;
          state = INITIAL;
          break;

        default:
          break;
      }
      break;

    case ENDFIRSTLINE:

      switch (ev->type) {

        case RB_PRESSED:
          state = D_SECONDLINE;
          break;

        case LB_PRESSED:
          state = D_FIRSTLINE;
          break;

		//Accepting movement tolerance between the end of the first line and beginning of the second line (vertex)
        case MOUSE_MOV:
          if ((abs(ev->delta_x) > *tolerance) || (abs(ev->delta_y) > *tolerance))
              state = INITIAL;
          break;

        case BUTTON_EV:
          state = INITIAL;
          break;

        default:
          break;
      }
      break;

    case D_SECONDLINE:

      switch (ev->type) {

        case MOUSE_MOV:
          movX += ev->delta_x;
          movY += ev->delta_y;
          if (ev->delta_x < 0)
            if (-ev->delta_x > *tolerance) {
              movX = 0;
              movY = 0;
            }
          if (ev->delta_y > 0)
            if (ev->delta_y > *tolerance) {
              movX = 0;
              movY = 0;
            }
          break;

        case RB_RELEASED:
          if ((movX >= *x_len) && (abs(movY) > movX)) {
            movX = 0;
            movY = 0;
            state = COMPLETE;
          }
          else {
            movX = 0;
            movY = 0;
            state = INITIAL;
          }
          break;

        case BUTTON_EV:
          movX = 0;
          movY = 0;
          state = INITIAL;
          break;

        default:
          break;
      }
      break;

    default:
      break;
  }

  eve = false;
}
