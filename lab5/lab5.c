// IMPORTANT: you must include the following line in all your C files
#include <lcom/lcf.h>

#include <lcom/lab5.h>

#include "i8042.h"
#include "i8254.h"
#include "kbc.h"
#include "vbe.h"
#include "vbe_macros.h"

#include <stdint.h>
#include <stdio.h>

extern void *video_mem;

extern int kbc_hook_id;
extern uint8_t scancode;
extern int ih_error;

extern int timer0_hook_id;
extern int timer_int_count;

// Any header files included below this line should have been created by you

int main(int argc, char *argv[]) {
  // sets the language of LCF messages (can be either EN-US or PT-PT)
  lcf_set_language("EN-US");

  // enables to log function invocations that are being "wrapped" by LCF
  // [comment this out if you don't want/need it]
  lcf_trace_calls("/home/lcom/labs/lab5/trace.txt");

  // enables to save the output of printf function calls on a file
  // [comment this out if you don't want/need it]
  lcf_log_output("/home/lcom/labs/lab5/output.txt");

  // handles control over to LCF
  // [LCF handles command line arguments and invokes the right function]
  if (lcf_start(argc, argv))
    return 1;

  // LCF clean up tasks
  // [must be the last statement before return]
  lcf_cleanup();

  return 0;
}

int(video_test_init)(uint16_t mode, uint8_t delay) {

  /*Map memory*/
  if (vg_init(mode) == NULL)
    return 1;

  tickdelay(micros_to_ticks(delay * 1000000));

  if (vg_exit())
    return 1;

  return 0;
}

int(video_test_rectangle)(uint16_t mode, uint16_t x, uint16_t y, uint16_t width,
                          uint16_t height, uint32_t color) {

  int ipc_status;
  message msg;
  int r;
  // Set to true when a scancode has two bytes
  bool two_bytes = false;
  // Used for the kbd_print_scancode function
  bool make;
  uint8_t size;
  uint8_t bytes[2];
  uint8_t irq_set = BIT(kbc_hook_id);

  if (vg_init(mode) == NULL)
    return 1;

  vg_make_rectangle(x, y, width, height, color);

  // Subscribing the interruptions
  if (kbc_subscribe_int(&kbc_hook_id))
    return 1;

  while (scancode != ESC_BREAKCODE) {
    if ((r = driver_receive(ANY, &msg, &ipc_status)) != 0) {
      printf("driver_receive failed with: %d", r);
      continue;
    }
    if (is_ipc_notify(ipc_status)) { /* Received notication */
      switch (_ENDPOINT_P(msg.m_source)) {
      case HARDWARE: /* Hardware interrupt notification */
        if (msg.m_notify.interrupts & irq_set) {

          // Code has 2 bytes
          if (two_bytes) {

            kbc_ih();

            // Interrupt handler error handling
            if (ih_error) {
              ih_error = 0;
              continue;
            }

            // Check if its makecode or breakcode(Breakcode has MSB as 1)
            if (scancode & BIT(7))
              make = false;
            else
              make = true;

            size = 2;
            bytes[0] = 0xE0;
            bytes[1] = scancode;

            // Resetting the flag for the next while loop
            two_bytes = false;
          } else {

            kbc_ih();

            // Reading of the status of the keyboard
            if (ih_error) {
              ih_error = 0;
              continue;
            }

            // If the the msb is 0xE0 we set the flag of two_bytes to true and
            // we skip to the next iteration
            if (scancode == 0xE0) {
              two_bytes = true;
              continue;
            } else
              two_bytes = false;

            if (scancode & BIT(7))
              make = false;
            else
              make = true;
            size = 1;
            bytes[0] = scancode;
          }

          // Print the scancode
          if (kbd_print_scancode(make, size, bytes))
            return 1;
        }
        break;

      default:
        break; /*No other interruptions expected*/
      }
    } else {
    }
  }

  // Unsubcribing the interruptions
  if (kbc_unsubscribe_int())
    return 1;

  if (vg_exit())
    return 1;

  return 0;
}

int(video_test_pattern)(uint16_t mode, uint8_t no_rectangles, uint32_t first,
                        uint8_t step) {

  int ipc_status;
  message msg;
  int r;
  // Set to true when a scancode has two bytes
  bool two_bytes = false;
  // Used for the kbd_print_scancode function
  bool make;
  uint8_t size;
  uint8_t bytes[2];
  uint8_t irq_set = BIT(kbc_hook_id);

  if (vg_init(mode) == NULL)
    return 1;

  if(vg_draw_pattern(no_rectangles, first, step))
        return 1;

  // Subscribing the interruptions
  if (kbc_subscribe_int(&kbc_hook_id))
    return 1;

  while (scancode != ESC_BREAKCODE) {
    if ((r = driver_receive(ANY, &msg, &ipc_status)) != 0) {
      printf("driver_receive failed with: %d", r);
      continue;
    }
    if (is_ipc_notify(ipc_status)) { /* Received notication */
      switch (_ENDPOINT_P(msg.m_source)) {
      case HARDWARE: /* Hardware interrupt notification */
        if (msg.m_notify.interrupts & irq_set) {

          // Code has 2 bytes
          if (two_bytes) {

            kbc_ih();

            // Interrupt handler error handling
            if (ih_error) {
              ih_error = 0;
              continue;
            }

            // Check if its makecode or breakcode(Breakcode has MSB as 1)
            if (scancode & BIT(7))
              make = false;
            else
              make = true;

            size = 2;
            bytes[0] = 0xE0;
            bytes[1] = scancode;

            // Resetting the flag for the next while loop
            two_bytes = false;
          } else {

            kbc_ih();

            // Reading of the status of the keyboard
            if (ih_error) {
              ih_error = 0;
              continue;
            }

            // If the the msb is 0xE0 we set the flag of two_bytes to true and
            // we skip to the next iteration
            if (scancode == 0xE0) {
              two_bytes = true;
              continue;
            } else
                 two_bytes = false;

            if (scancode & BIT(7))
              make = false;
            else
              make = true;
            size = 1;
            bytes[0] = scancode;
          }

          // Print the scancode
          if (kbd_print_scancode(make, size, bytes))
            return 1;
        }
        break;

      default:
        break; /*No other interruptions expected*/
      }
    } else {
    }
  }

  // Unsubcribing the interruptions
  if (kbc_unsubscribe_int())
    return 1;

  if (vg_exit())
    return 1;

  return 0;
}

int(video_test_xpm)(xpm_map_t xpm, uint16_t x, uint16_t y) {
  /* To be completed */
  // printf("%s(%8p, %u, %u): under construction\n", __func__, xpm, x, y);
  int ipc_status;
  message msg;
  int r;
  // Set to true when a scancode has two bytes
  bool two_bytes = false;
  // Used for the kbd_print_scancode function
  bool make;
  uint8_t size;
  uint8_t bytes[2];
  uint8_t irq_set = BIT(kbc_hook_id);

  if (vg_init(0x105) == NULL) // MODO INDEXADO
    return 1;

  vg_draw_pixmap(xpm, x, y);

  // Subscribing the interruptions
  if (kbc_subscribe_int(&kbc_hook_id))
    return 1;

  while (scancode != ESC_BREAKCODE) {
    if ((r = driver_receive(ANY, &msg, &ipc_status)) != 0) {
      printf("driver_receive failed with: %d", r);
      continue;
    }
    if (is_ipc_notify(ipc_status)) { /* Received notication */
      switch (_ENDPOINT_P(msg.m_source)) {
      case HARDWARE: /* Hardware interrupt notification */
        if (msg.m_notify.interrupts & irq_set) {

          // Code has 2 bytes
          if (two_bytes) {

            kbc_ih();

            // Interrupt handler error handling
            if (ih_error) {
              ih_error = 0;
              continue;
            }

            // Check if its makecode or breakcode(Breakcode has MSB as 1)
            if (scancode & BIT(7))
              make = false;
            else
              make = true;

            size = 2;
            bytes[0] = 0xE0;
            bytes[1] = scancode;

            // Resetting the flag for the next while loop
            two_bytes = false;
          } else {

            kbc_ih();

            // Reading of the status of the keyboard
            if (ih_error) {
              ih_error = 0;
              continue;
            }

            // If the the msb is 0xE0 we set the flag of two_bytes to true and
            // we skip to the next iteration
            if (scancode == 0xE0) {
              two_bytes = true;
              continue;
            } else
              two_bytes = false;

            if (scancode & BIT(7))
              make = false;
            else
              make = true;
            size = 1;
            bytes[0] = scancode;
          }
        }
        break;

      default:
        break; /*No other interruptions expected*/
      }
    } else {
    }
  }

  // Unsubcribing the interruptions
  if (kbc_unsubscribe_int())
    return 1;

  if (vg_exit())
    return 1;

  return 0;
}

int(video_test_move)(xpm_map_t xpm, uint16_t xi, uint16_t yi, uint16_t xf,
                     uint16_t yf, int16_t speed, uint8_t fr_rate) {
  /* To be completed */
  // printf("%s(%8p, %u, %u, %u, %u, %d, %u): under construction\n",
  //       __func__, xpm, xi, yi, xf, yf, speed, fr_rate);
  int ipc_status;
  message msg;
  int r;

  // Set to true when a scancode has two bytes
  bool two_bytes = false;

  // Used for the kbd_print_scancode function
  bool make;
  uint8_t size;
  uint8_t bytes[2];

  uint32_t irq_timer0 = BIT(timer0_hook_id);
  uint32_t irq_kbd = BIT(kbc_hook_id);

  uint8_t aux_timer0 = (uint8_t)timer0_hook_id;

  int toDraw = 60 / fr_rate;

  if (vg_init(0x105) == NULL) // MODO INDEXADO
    return 1;

  xpm_image_t image;

  bool endMove = false;
  int xmov = 0;
  int ymov = 0;

  if (yi == yf) // Only x movement
    xmov = 1;
  else if (xi == xf) // Only y movement
    ymov = 1;

  image = vg_draw_pixmap(xpm, xi, yi);

  // Subscription of the interruptions

  if (kbc_subscribe_int(&kbc_hook_id))
    return 1;
  if (timer_subscribe_int(&aux_timer0))
    return 1;



  timer0_hook_id = (int)aux_timer0;

  // If the counter reaches the time n*60 (because the timer interrupts 60 times
  // every second) the loop must end
  while (scancode != ESC_BREAKCODE) {
    /* Get a request message */
    if ((r = driver_receive(ANY, &msg, &ipc_status)) != 0) {
      printf("driver_receive failed with: %d", r);
      continue;
    }
    if (is_ipc_notify(ipc_status)) { /* Received notication */
      switch (_ENDPOINT_P(msg.m_source)) {
      case HARDWARE: /* Hardware interrupt notification */
        if (msg.m_notify.interrupts & irq_timer0) {

          // Each interruption we increment the counter
          timer_int_handler();
        }
        if (msg.m_notify.interrupts & irq_kbd) {

          // Code has 2 bytes
          if (two_bytes) {

            kbc_ih();

            // Interrupt handler error handling
            if (ih_error) {
              ih_error = 0;
              continue;
            }

            // Check if its makecode or breakcode(Breakcode has MSB as 1)
            if (scancode & BIT(7))
              make = false;
            else
              make = true;
            size = 2;
            bytes[0] = 0xE0;
            bytes[1] = scancode;

            // Resetting the flag for the next while loop
            two_bytes = false;
          } else {

            kbc_ih();

            // Reading of the status of the keyboard
            if (ih_error) {
              ih_error = 0;
              continue;
            }

            // If the the msb is 0xE0 we set the flag of two_bytes to true and
            // we skip to the next iteration
            if (scancode == 0xE0) {
              two_bytes = true;
              continue;
            } else
              two_bytes = false;

            if (scancode & BIT(7))
              make = false;
            else
              make = true;
            size = 1;
            bytes[0] = scancode;
          }
        }
        break;

      default:
        break; /*No other interruptions expected*/
      }
    } else {
    }

    if (!endMove) {
      if (speed > 0) {
        if (timer_int_count == toDraw) {
          timer_int_count = 0;

          //Erase the previous frame map
          vg_make_rectangle(xi, yi, image.width * ymov + speed * xmov, image.height * xmov + speed * ymov, xpm_transparency_color(XPM_INDEXED));

          xi += speed * xmov;
          yi += speed * ymov;
          if (xi > xf) {
            endMove = true;
            xi = xf;
          } else if (xi == xf && xmov == 1)
            endMove = true;

          if (yi > yf) {
            endMove = true;
            yi = yf;
          } else if (yi == yf && ymov == 1)
            endMove = true;

          image = vg_draw_pixmap(xpm, xi, yi);
        }
      } else {
        if (timer_int_count == -speed * toDraw) {

          vg_make_rectangle(xi, yi, ymov * (image.width - 1) + 1, xmov * (image.height - 1) + 1, xpm_transparency_color(XPM_INDEXED));
          xi += xmov;
          yi += ymov;

          if (xi == xf && xmov == 1)
            endMove = true;
          if (yi == yf && ymov == 1)
            endMove = true;

          image = vg_draw_pixmap(xpm, xi, yi);
          timer_int_count = 0;
        }
      }
    }
  }

  // Unsubcribing the interruptions
  // KBC:
  if (kbc_unsubscribe_int())
    return 1;
  // TIMER0:
  if (timer_unsubscribe_int())
    return 1;

  if (vg_exit())
    return 1;

  return 0;
}

int(video_test_controller)() {

        vg_vbe_contr_info_t contr_info_1;

        if(vbe_get_contr_info(&contr_info_1))
                return 1;

        if(vg_display_vbe_contr_info(&contr_info_1))
                return 1;


        return 0;
}
