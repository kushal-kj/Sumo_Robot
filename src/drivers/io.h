#ifndef IO_H
#define IO_H

/* IO pins handling including pinmapping,initialization, and configuration.
 * This wraps the more crude register defines provided in the headers from
 * Texas instrument */

//TODO: Improve multiple HW targets handling

#define LAUNCHPAD


//TODO: enums

typedef enum
{
#if defined(LAUNCHPAD)
	
	IO_TEST_LED,
	IO_UART_RXD,
	IO_UART_TXD,
	IO_UNUSED_1,
	IO_UNUSED_2,
	IO_UNUSED_3,
	IO_UNUSED_4,
	IO_UNUSED_5,
	IO_UNUSED_6,
	IO_UNUSED_7,
	IO_UNUSED_8,
	IO_UNUSED_9,
	IO_UNUSED_10,
	IO_UNUSED_11,
	IO_UNUSED_12,
	IO_UNUSED_13,
	IO_UNUSED_14,
#endif	
}io_e;

typedef enum
{
	IO_SELECT_GPIO,
	IO_SELECT_ALT1,
	IO_SELECT_ALT_2,
	IO_SELECT_ALT_3,
}io_select_e;			//function selection registers

typedef enum
{
	IO_DIR_INPUT,
	IO_DIR_OUTPUT,
}io_dir_e;			//direction registers

typedef enum
{
	IO_PUPD_DISABLED,
	IO_PUPD_ENABLED,
}io_pupd_e;			//pull-up pull-down resistors enable registers

typedef enum
{
	IO_OUT_LOW,	//pull-down
	IO_OUT_HIGH,	//pull-up
}io_out_e;

typedef enum
{
	IO_IN_LOW,
	IO_IN_HIGH,
}io_in_e;			//input register


//TODO: structs







//TODO: functions

void io_set_select(io_e io, io_select_e select);
void io_set_direction(io_e io,io_dir_e direction);
void io_set_pupd(io_e io,io_pupd_e pupd_resistor);
void io_set_out(io_e io,io_out_e out);
io_in_e io_get_input(io_e io,);				//the input register function returns a value




#endif	//IO_H
