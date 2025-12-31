#include "option.h"
#include "position.h"
#include "transposition.h"
#include "evaluation.h"
#include "uci.h"

int main(int argc, char **argv)
{
	PSQT::init();
	Option::init();
	Position::init();
	// ttable.init(); // maybe put it somewhere else

	return UCI::main(argc, argv);
}
