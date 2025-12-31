#ifndef EVALUATION_H_INCLUDED
#define EVALUATION_H_INCLUDED

#include "position.h"
#include "types.h"

namespace PSQT {
	extern Score PieceValue[PIECE_NB];
	extern Score psq[PIECE_NB][SQ_NB];
	void init();
}

Value eval (const Position& pos);

#endif
