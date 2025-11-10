#include "option.h"
#include "transposition.h"
#include "uci.h"

int main(int argc, char **argv)
{
	Option::init();
	
	ttable.init();
	return UCI::main(argc, argv);
}
