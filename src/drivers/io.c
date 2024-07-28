#include "drivers/io.h"
#include "common/defines.h"


void io_set_select(io_e io,io_select_e select)
{
	UNUSED(io);
	UNUSED(select);

	//TODO: Implement
	
}

void io_set_direection(io_e io,io_dir_e direction)
{
	UNUSED(io);
	UNUSED(direction);

	//TODO: Implement
	
}

void io_set_pupd(io_e io, io_pupd_e pupd_resistor)
{
	UNUSED(io);
	UNUSED(pupd_resistor);

	//TODO: Implement

}

void io_set_out(io_e io,io_out_e out)
{
	UNUSED(io);
	UNUSED(out);

	//TODO: Implement
	
}

io_in_e io_get_input(io_e io)
{
	UNUSED(io);

	//TODO: Implement
	
	return 0;
}
