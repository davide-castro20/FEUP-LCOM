#include <lcom/lcf.h>

#include <lcom/lab3.h>


#include <stdbool.h>
#include <stdint.h>

#include "kbc.h"
#include "i8042.h"
#include "timer.h"

uint32_t sys_inb_count = 0;

extern int kbc_hook_id;
extern uint8_t scancode;
extern int ih_error;

extern int timer0_hook_id;
extern int timer_int_count;

int main(int argc, char *argv[]) {
	// sets the language of LCF messages (can be either EN-US or PT-PT)
	lcf_set_language("EN-US");

	// enables to log function invocations that are being "wrapped" by LCF
	// [comment this out if you don't want/need it]
	lcf_trace_calls("/home/lcom/labs/lab3/trace.txt");

	// enables to save the output of printf function calls on a file
	// [comment this out if you don't want/need it]
	lcf_log_output("/home/lcom/labs/lab3/output.txt");

	// handles control over to LCF
	// [LCF handles command line arguments and invokes the right function]
	if (lcf_start(argc, argv))
		return 1;

	// LCF clean up tasks
	// [must be the last statement before return]
	lcf_cleanup();

	return 0;
}

int(kbd_test_scan)() {

	int ipc_status;
	message msg;
	int r;

	//Set to true when a scancode has two bytes
	bool two_bytes = false;

	//Used for the kbd_print_scancode function
	bool make;
	uint8_t size;
	uint8_t bytes[2];
    
	uint8_t irq_set = BIT(kbc_hook_id);

	//Subscribing the interruptions
	if (kbc_subscribe_int(&kbc_hook_id))
		return 1;

	//Ends when the breakcode of the ESC key(0x81)
	while (scancode != ESC_BREAKCODE) {
		if ((r = driver_receive(ANY, &msg, &ipc_status)) != 0) {
			printf("driver_receive failed with: %d", r);
			continue;
		}
		if (is_ipc_notify(ipc_status)) { /* Received notication */
			switch (_ENDPOINT_P(msg.m_source)) {
			case HARDWARE:  /* Hardware interrupt notification */
				if (msg.m_notify.interrupts & irq_set) {

					//Code has 2 bytes
					if (two_bytes) {
                        
                        
						kbc_ih();

                        //Interrupt handler error handling
						if (ih_error) {
							ih_error = 0;
							continue;
						}

						//Check if its makecode or breakcode(Breakcode has MSB as 1)
						if (scancode & BIT(7))
							make = false;
						else
							make = true;
							
						size = 2;
						bytes[0] = 0xE0;
						bytes[1] = scancode;

                        //Resetting the flag for the next while loop
						two_bytes = false;

					} else {

						kbc_ih();

                        //Reading of the status of the keyboard
						if (ih_error) {
							ih_error = 0;
							continue;
						}

                        //If the the msb is 0xE0 we set the flag of two_bytes to true and we skip to the next iteration
						if (scancode == 0xE0) {
							two_bytes = true;
							continue;
						}
						else
							two_bytes = false;

						if (scancode & BIT(7))
							make = false;
						else
							make = true;
						size = 1;
						bytes[0] = scancode;
					}

					//Print the scancode
					if (kbd_print_scancode(make, size, bytes))
						return 1;

				}
				break;

			default:
				break; /*No other interruptions expected*/
			}
		}
		else {}

	}

	//Unsubcribing the interruptions
	if (kbc_unsubscribe_int())
		return 1;

	//Print the number of calls to sys_inb
	if (kbd_print_no_sysinb(sys_inb_count))
		return 1;

	return 0;
}


int(kbd_test_poll)() {
	
	bool make;
	uint8_t size;
	uint8_t bytes[2];
	uint8_t command = 0;

	while (scancode != ESC_BREAKCODE) {

		if (kbc_poll() == 0)
		{
			//If scancode has two bytes
			if (scancode == 0xE0) {

				int count = 0;
				//Loop to make sure the second byte is read
				while (count < MAX_TRIES)
				{
					if (kbc_poll() == 0)
						break;
					count++;
					tickdelay(micros_to_ticks(20000));
				}

				//Check if its makecode or breakcode
				if (scancode & BIT(7))
					make = false;
				else
					make = true;
				size = 2;
				bytes[0] = 0xE0;
				bytes[1] = scancode;
			}
			else {
				if (scancode & BIT(7))
					make = false;
				else
					make = true;
				size = 1;
				bytes[0] = scancode;
			}

			//Print the scancode
			if (kbd_print_scancode(make, size, bytes))
			{
				if (kbc_reenable_int(&command))
					return 1;

				return 1;

			}
		}
		tickdelay(micros_to_ticks(20000));
    }

	//Reenable the keyboard interruptions
	if (kbd_print_no_sysinb(sys_inb_count))
	{
        if(kbc_reenable_int(&command))
            return 1;

		return 1;
	}
    
	//Print the number of calls to sys_inb
    if(kbc_reenable_int(&command))
        return 1;

	return 0;

}


int(kbd_test_timed_scan)(uint8_t n) {

	int ipc_status;
	message msg;
	int r;

	//Here we select the bit in the hook_id needed to check if we got the right interruption
	

	//Set to true when a scancode has two bytes
	bool two_bytes = false;

	//Used for the kbd_print_scancode function
	bool make;
	uint8_t size;
	uint8_t bytes[2];

	uint32_t irq_timer0 = BIT(timer0_hook_id);
	uint32_t irq_kbd = BIT(kbc_hook_id);

	uint8_t aux_timer0 = (uint8_t)timer0_hook_id;

	//Subscription of the interruption
	
	if (kbc_subscribe_int(&kbc_hook_id))
		return 1;
    if (timer_subscribe_int(&aux_timer0))
		return 1;

	timer0_hook_id = (int)aux_timer0;
	
	//If the counter reaches the time n*60 (because the timer interrupts 60 times every second) the loop must end 
	while ((timer_int_count < n * 60) && (scancode != ESC_BREAKCODE)) {
		/* Get a request message */
		if ((r = driver_receive(ANY, &msg, &ipc_status)) != 0) {
			printf("driver_receive failed with: %d", r);
			continue;
		}
		if (is_ipc_notify(ipc_status)) { /* Received notication */
			switch (_ENDPOINT_P(msg.m_source)) {
			case HARDWARE:  /* Hardware interrupt notification */
				if (msg.m_notify.interrupts & irq_timer0) {

					//Each interruption we increment the counter
					timer_int_handler();
					
				}
				if (msg.m_notify.interrupts & irq_kbd) {

					//Code has 2 bytes
					if (two_bytes) {

						kbc_ih();

						//Interrupt handler error handling
						if (ih_error) {
							ih_error = 0;
							continue;
						}

						//Check if its makecode or breakcode(Breakcode has MSB as 1)
						if (scancode & BIT(7))
							make = false;
						else
							make = true;
						size = 2;
						bytes[0] = 0xE0;
						bytes[1] = scancode;

						//Resetting the flag for the next while loop
						two_bytes = false;

					}
					else {

						kbc_ih();

						//Reading of the status of the keyboard
						if (ih_error) {
							ih_error = 0;
							continue;
						}

						//If the the msb is 0xE0 we set the flag of two_bytes to true and we skip to the next iteration
						if (scancode == 0xE0) {
							two_bytes = true;
							continue;
						}
						else
							two_bytes = false;

						if (scancode & BIT(7))
							make = false;
						else
							make = true;
						size = 1;
						bytes[0] = scancode;
					}

					//Print the scancode
					if (kbd_print_scancode(make, size, bytes))
						return 1;
					timer_int_count = 0;
				}
				break;

			default:
				break; /*No other interruptions expected*/
			}
		}
		else {}
	}

	//Unsubcribing the interruptions
	//KBC:
	if (kbc_unsubscribe_int())
		return 1;
	//TIMER0:
	if (timer_unsubscribe_int())
		return 1;

	//Print the number of calls to sys_inb
	if (kbd_print_no_sysinb(sys_inb_count))
		return 1;

	return 0;
	
}
