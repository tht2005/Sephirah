#ifndef UCI_H_INCLUDED
#define UCI_H_INCLUDED

namespace UCI {

int main(int argc, char **argv);

}

#include "types.h"
#include "position.h"

Move find_best_move(Position& pos, StateListPtr& dq, int depth);

#endif
