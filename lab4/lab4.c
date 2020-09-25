// IMPORTANT: you must include the following line in all your C files
#include <lcom/lcf.h>

#include <stdint.h>
#include <stdio.h>

#include "i8042.h"
#include "kbc.h"
#include "mouse.h"
#include "utils.h"

// Any header files included below this line should have been created by you

extern int mouse_hook_id;
extern uint8_t scancode;
extern int ih_error;

extern int timer0_hook_id;
extern int timer_int_count;

bool eve = false;

extern enum States state;
extern struct packet prevP;

int main(int argc, char *argv[]) {
  // sets the language of LCF messages (can be either EN-US or PT-PT)
  lcf_set_language("EN-US");

  // enables to log function invocations that are being "wrapped" by LCF
  // [comment this out if you don't want/need/ it]
  lcf_trace_calls("/home/lcom/labs/lab4/trace.txt");

  // enables to save the output of printf function calls on a file
  // [comment this out if you don't want/need it]
  lcf_log_output("/home/lcom/labs/lab4/output.txt");

  // handles control over to LCF
  // [LCF handles command line arguments and invokes the right function]
  if (lcf_start(argc, argv))
	return 1;

  // LCF clean up tasks
  // [must be the last statement before return]
  lcf_cleanup();

  return 0;
}

int(mouse_test_packet)(uint32_t cnt) {
  /* To be completed */
  //printf("%s(%u): under construction\n", __func__, cnt);

  int ipc_status;
  message msg;
  int r;

  int counter = 1;

  bool sign_y;
  bool sign_x;

  struct packet pp;

  uint32_t mouse_irq_set = BIT(mouse_hook_id);

  //mouse_enable_data_reporting();
  if (mouse_send_cmd(MOUSE_ENABLE_CMD))
	return 1;

	if (mouse_subscribe_int(&mouse_hook_id))
		return 1;

  //Provided by lcf

  while (cnt > 0) {

	if ((r = driver_receive(ANY, &msg, &ipc_status)) != 0) {
	  printf("driver_receive failed with: %d", r);
	  continue;
	}

	if (is_ipc_notify(ipc_status)) { /* Received notication */
	  switch (_ENDPOINT_P(msg.m_source)) {
		case HARDWARE: /* Hardware interrupt notification */
		  if (msg.m_notify.interrupts & mouse_irq_set) {

			mouse_ih();
			if (ih_error)
			  continue;


			if (counter == 1) {
			  if ((scancode & BIT(3)) == 0)
				continue;
			  pp.bytes[0] = (uint16_t) scancode;
			  counter++;
			  if (scancode & MOUSE_X_DELTA_SIGN)
				sign_x = true;
			  else
				sign_x = false;

			  if (scancode & MOUSE_Y_DELTA_SIGN)
				sign_y = true;
			  else
				sign_y = false;
			}
			else if (counter == 2) {

			  pp.bytes[1] = scancode;
			  counter++;
			}
		  
			else if (counter == 3) {

				pp.bytes[2] = scancode;

				pp.delta_x = sign_x;
				pp.delta_y = sign_y;
				pp.lb = pp.bytes[0] & MOUSE_LB_PRESSED;
				pp.rb = pp.bytes[0] & MOUSE_RB_PRESSED;
				pp.mb = pp.bytes[0] & MOUSE_MB_PRESSED;
				pp.delta_x = ninebit16bit(sign_x, pp.bytes[1]);
				pp.delta_y = ninebit16bit(sign_y, pp.bytes[2]);
				pp.x_ov = pp.bytes[0] & MOUSE_X_OVF;
				pp.y_ov = pp.bytes[0] & MOUSE_Y_OVF;

				counter = 1;
				mouse_print_packet(&pp);
				cnt--;
		  	}
		  }

		  break;

		default:
			break; /*No other interruptions expected*/
	  }
	}
	else {
	}
  }

  if (mouse_unsubscribe_int()) {
	return 1;
  }

  if (mouse_send_cmd(MOUSE_DISABLE_CMD))
	return 1;

  return 0;
}



int(mouse_test_remote)(uint16_t period, uint8_t cnt) {
  //printf("%s(%u, %u): under construction\n", __func__, period, cnt);

	int counter = 1;

	bool sign_y;
	bool sign_x;

	struct packet pp;


	while (cnt > 0) {

		counter = 1;
		if(mouse_send_cmd(MOUSE_READ))
			return 1;

		while(counter <= 3) {
			

			if (counter == 1) {
				if(kbc_read(&scancode))
					return 1;

				if ((scancode & BIT(3)) == 0)
					continue;
				pp.bytes[0] = (uint16_t) scancode;
				counter++;
				if (scancode & MOUSE_X_DELTA_SIGN)
					sign_x = true;
				else
					sign_x = false;

				if (scancode & MOUSE_Y_DELTA_SIGN)
					sign_y = true;
				else
					sign_y = false;
			}
			else if (counter == 2) {
				if(kbc_read(&scancode))
					return 1;

				pp.bytes[1] = scancode;
				counter++;
			}
			
			else if (counter == 3) {

				if(kbc_read(&scancode))
					return 1;

				pp.bytes[2] = scancode;

				pp.delta_x = sign_x;
				pp.delta_y = sign_y;
				pp.lb = pp.bytes[0] & MOUSE_LB_PRESSED;
				pp.rb = pp.bytes[0] & MOUSE_RB_PRESSED;
				pp.mb = pp.bytes[0] & MOUSE_MB_PRESSED;
				pp.delta_x = ninebit16bit(sign_x, pp.bytes[1]);
				pp.delta_y = ninebit16bit(sign_y, pp.bytes[2]);
				pp.x_ov = pp.bytes[0] & MOUSE_X_OVF;
				pp.y_ov = pp.bytes[0] & MOUSE_Y_OVF;

				counter = 4;
				mouse_print_packet(&pp);
				cnt--;
			}
			
		}
		tickdelay(micros_to_ticks(period*1000));
  }



	if (mouse_send_cmd(MOUSE_STREAM))
		return 1;

    if (mouse_send_cmd(MOUSE_DISABLE_CMD))
        return 1;

	if (kbc_cmd(WRITE_CMD))
		return 1;

	if (kbc_write(minix_get_dflt_kbc_cmd_byte()))
		return 1;
	
  return 0;
}


int (mouse_test_async)(uint8_t idle_time) {

	int ipc_status;
  message msg;
  int r;

  int counter = 1;

  bool sign_y;
  bool sign_x;

  struct packet pp;

  uint32_t mouse_irq_set = BIT(mouse_hook_id);
  uint32_t irq_timer0 = BIT(timer0_hook_id);
  uint8_t aux_timer0 = (uint8_t) timer0_hook_id;

 

  //mouse_enable_data_reporting();
  if (mouse_send_cmd(MOUSE_ENABLE_CMD))
    return 1;

  if (mouse_subscribe_int(&mouse_hook_id))
    return 1;

  if (timer_subscribe_int(&aux_timer0))
    return 1;

  timer0_hook_id = (int) aux_timer0;

  //Provided by lcf

  uint8_t freq = (uint8_t)sys_hz();

  while (timer_int_count < (idle_time * freq)) {

    if ((r = driver_receive(ANY, &msg, &ipc_status)) != 0) {
      printf("driver_receive failed with: %d", r);
      continue;
    }

    if (is_ipc_notify(ipc_status)) { /* Received notication */
      switch (_ENDPOINT_P(msg.m_source)) {
        case HARDWARE: /* Hardware interrupt notification */
          if (msg.m_notify.interrupts & irq_timer0) {

            //Each interruption we increment the counter
            timer_int_handler();
          }
          if (msg.m_notify.interrupts & mouse_irq_set) {

            mouse_ih();
            if (ih_error)
              continue;

            if (counter == 1) {
              if ((scancode & BIT(3)) == 0)
                continue;
              pp.bytes[0] = (uint16_t) scancode;
              counter++;
              if (scancode & MOUSE_X_DELTA_SIGN)
                sign_x = true;
              else
                sign_x = false;

              if (scancode & MOUSE_Y_DELTA_SIGN)
                sign_y = true;
              else
                sign_y = false;
            }
            else if (counter == 2) {

              pp.bytes[1] = scancode;
              counter++;
            }

            else if (counter == 3) {

              pp.bytes[2] = scancode;

              pp.delta_x = sign_x;
              pp.delta_y = sign_y;
              pp.lb = pp.bytes[0] & MOUSE_LB_PRESSED;
              pp.rb = pp.bytes[0] & MOUSE_RB_PRESSED;
              pp.mb = pp.bytes[0] & MOUSE_MB_PRESSED;
              pp.delta_x = ninebit16bit(sign_x, pp.bytes[1]);
              pp.delta_y = ninebit16bit(sign_y, pp.bytes[2]);
              pp.x_ov = pp.bytes[0] & MOUSE_X_OVF;
              pp.y_ov = pp.bytes[0] & MOUSE_Y_OVF;

              counter = 1;
              mouse_print_packet(&pp);
            }
            timer_int_count = 0;
          }

          break;

        default:
          break; /*No other interruptions expected*/
      }
    }
    else {
    }
  }

  if (timer_unsubscribe_int())
    return 1;

  if (mouse_unsubscribe_int()) {
    return 1;
  }

  if (mouse_send_cmd(MOUSE_DISABLE_CMD))
    return 1;

  return 0;
}


int (mouse_test_gesture)(uint8_t x_len, uint8_t tolerance) {

  int ipc_status;
  message msg;
  int r;

  int counter = 1;

  bool sign_y;
  bool sign_x;

  int16_t xlen = x_len;
  int16_t toler = tolerance;

  struct packet pp;

  struct mouse_ev m_event;

  prevP.lb = 0;
  prevP.rb = 0;
  prevP.mb = 0;

  uint32_t mouse_irq_set = BIT(mouse_hook_id);

  if (mouse_subscribe_int(&mouse_hook_id))
    return 1;

  if(sys_irqdisable(&mouse_hook_id))
    return 1;

  if (mouse_send_cmd(MOUSE_ENABLE_CMD))
   return 1;

  if(sys_irqenable(&mouse_hook_id))
    return 1;


  while (state != COMPLETE) {
    
    if ((r = driver_receive(ANY, &msg, &ipc_status)) != 0) {
      printf("driver_receive failed with: %d", r);
      continue;
    }

    if (is_ipc_notify(ipc_status)) { /* Received notication */
      switch (_ENDPOINT_P(msg.m_source)) {
        case HARDWARE: /* Hardware interrupt notification */
          if (msg.m_notify.interrupts & mouse_irq_set) {

            mouse_ih();
            if (ih_error)
              continue;

            if (counter == 1) {
              if ((scancode & BIT(3)) == 0)
                continue;
              pp.bytes[0] = (uint16_t) scancode;
              counter++;
              if (scancode & MOUSE_X_DELTA_SIGN)
                sign_x = true;
              else
                sign_x = false;

              if (scancode & MOUSE_Y_DELTA_SIGN)
                sign_y = true;
              else
                sign_y = false;
            }
            else if (counter == 2) {

              pp.bytes[1] = scancode;
              counter++;
            }

            else if (counter == 3) {

              pp.bytes[2] = scancode;

              pp.delta_x = sign_x;
              pp.delta_y = sign_y;
              pp.lb = pp.bytes[0] & MOUSE_LB_PRESSED;
              pp.rb = pp.bytes[0] & MOUSE_RB_PRESSED;
              pp.mb = pp.bytes[0] & MOUSE_MB_PRESSED;
              pp.delta_x = ninebit16bit(sign_x, pp.bytes[1]);
              pp.delta_y = ninebit16bit(sign_y, pp.bytes[2]);
              pp.x_ov = pp.bytes[0] & MOUSE_X_OVF;
              pp.y_ov = pp.bytes[0] & MOUSE_Y_OVF;

              counter = 1;
              mouse_print_packet(&pp);
				
              m_event = mouse_event(&pp);

            }
          }

          break;

        default:
          break; /*No other interruptions expected*/
      }
    }
    else {
    }
  //executed when an event is detected in mouse_event
    if (eve)
      handle_mouse(&m_event, &xlen, &toler);
  }

  if (mouse_unsubscribe_int()) {
    return 1;
  }

  if (mouse_send_cmd(MOUSE_DISABLE_CMD))
    return 1;

  return 0;
}


