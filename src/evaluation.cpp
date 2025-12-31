// temporary eval function, the complete function follows
// https://hxim.github.io/Stockfish-Evaluation-Guide/

#include "evaluation.h"
#include "bitboard.h"
#include "position.h"
#include "types.h"

int count(const Position &pos, Color c, PieceType pt) {
  Bitboard b = pos.pieces(c, pt);
  return __builtin_popcountll(b); 
}

Score piece_value_bonus(const Position &pos, Color col) {
  Score res = SCORE_ZERO;
  Bitboard sqset = pos.pieces(col);
  while (sqset > 0) {
    Square sq = pop_lsb(sqset);
    Piece pc = pos.piece_on(sq);
    PieceType pt = get_piece_type(pc);
    res += PSQT::PieceValue[pt];
  }
  return res;
}

Score psqt_bonus(const Position &pos, Color col) {
  Score res = SCORE_ZERO;
  Bitboard sqset = pos.pieces(col);
  while (sqset > 0) {
    Square sq = pop_lsb(sqset);
    Piece pc = pos.piece_on(sq);
    res += PSQT::psq[pc][sq];
  }
  return res;
}

Score non_pawn_material(const Position &pos, Color col) {
  Score res = SCORE_ZERO;
  Bitboard sqset = pos.pieces(col) & ~(pos.pieces(PAWN));
  while (sqset > 0) {
    Square sq = pop_lsb(sqset);
    Piece pc = pos.piece_on(sq);
    res += PSQT::psq[pc][sq];
  }
  return res;
}

Value phase(const Position &pos, Color c) {
  Value npm = mg_value(non_pawn_material(pos, c) + non_pawn_material(pos, ~c));
  npm = std::max(EndgameLimit, std::min(npm, MidgameLimit));
  return (((npm - EndgameLimit) * 128) / int(MidgameLimit - EndgameLimit)) * 2;
}

Value eval(const Position &pos) {
  constexpr Score BishopPairBonus =
      make_score(64, 64); // Bonus for having the bishop pair
  constexpr Score PawnMajorityBonus =
      make_score(16, 16); // Small bonus per pawn majority on a wing
  constexpr Score RookOpenFileBonus =
      make_score(32, 16); // Rook on fully open file (no pawns)
  constexpr Score RookHalfOpenBonus =
      make_score(16, 8); // Rook on half-open file (no friendly pawn)
  constexpr Score DoubledPawnPenalty = make_score(
      16, 16); // Penalty per extra pawn on same file (doubled/tripled)
  constexpr Score IsolatedPawnPenalty = make_score(
      32, 20); // Penalty for isolated pawn (no friendly pawn on adjacent files)
  constexpr int PassedPawnMgFactor =
      20; // Scoring factor per rank for passed pawn (MG)
  constexpr int PassedPawnEgFactor =
      30; // Scoring factor per rank for passed pawn (EG)
  // **Mobility**: bonus per legal move for each piece type
  constexpr Score KnightMobilityBonus = make_score(4, 2);
  constexpr Score BishopMobilityBonus = make_score(4, 4);
  constexpr Score RookMobilityBonus = make_score(2, 4);
  constexpr Score QueenMobilityBonus = make_score(1, 2);
  constexpr Score KingMobilityBonus =
      make_score(0, 2); // King mobility (only endgame relevant)
  // **Threats**: bonus for attacking an enemy piece that is not protected
  constexpr Score ThreatByPawn = make_score(32, 16);
  constexpr Score ThreatByMinor =
      make_score(64, 32); // Knight or Bishop threatened
  constexpr Score ThreatByRook = make_score(96, 48);
  constexpr Score ThreatByQueen = make_score(128, 64);
  // **Space**: bonus per controlled square in opponent's half (non-pawn pieces)
  constexpr Score SpaceBonus = make_score(2, 0);

  // Precomputed direction offsets for piece movement
  static const Bitboard FileMask[8] = {
      0x0101010101010101ULL, 0x0202020202020202ULL, 0x0404040404040404ULL,
      0x0808080808080808ULL, 0x1010101010101010ULL, 0x2020202020202020ULL,
      0x4040404040404040ULL, 0x8080808080808080ULL};
  static const int KnightDirs[8][2] = {{1, 2},   {2, 1},   {2, -1}, {1, -2},
                                       {-1, -2}, {-2, -1}, {-2, 1}, {-1, 2}};
  static const int KingDirs[8][2] = {{0, 1}, {0, -1}, {1, 0},  {-1, 0},
                                     {1, 1}, {-1, 1}, {1, -1}, {-1, -1}};
  static const int BishopDirs[4][2] = {{1, 1}, {-1, 1}, {1, -1}, {-1, -1}};
  static const int RookDirs[4][2] = {{0, 1}, {0, -1}, {1, 0}, {-1, 0}};
  static const int QueenDirs[8][2] = {{0, 1}, {0, -1}, {1, 0},  {-1, 0},
                                      {1, 1}, {-1, 1}, {1, -1}, {-1, -1}};

  // Piece counts for material
  int whitePawnCount = count(pos, WHITE, PAWN);
  int whiteKnightCount = count(pos, WHITE, KNIGHT);
  int whiteBishopCount = count(pos, WHITE, BISHOP);
  int whiteRookCount = count(pos, WHITE, ROOK);
  int whiteQueenCount = count(pos, WHITE, QUEEN);
  int blackPawnCount = count(pos, BLACK, PAWN);
  int blackKnightCount = count(pos, BLACK, KNIGHT);
  int blackBishopCount = count(pos, BLACK, BISHOP);
  int blackRookCount = count(pos, BLACK, ROOK);
  int blackQueenCount = count(pos, BLACK, QUEEN);

  // Material score (base piece values times counts)
  int mgMaterialWhite =
      whitePawnCount * PawnValueMg + whiteKnightCount * KnightValueMg +
      whiteBishopCount * BishopValueMg + whiteRookCount * RookValueMg +
      whiteQueenCount * QueenValueMg;
  int egMaterialWhite =
      whitePawnCount * PawnValueEg + whiteKnightCount * KnightValueEg +
      whiteBishopCount * BishopValueEg + whiteRookCount * RookValueEg +
      whiteQueenCount * QueenValueEg;
  int mgMaterialBlack =
      blackPawnCount * PawnValueMg + blackKnightCount * KnightValueMg +
      blackBishopCount * BishopValueMg + blackRookCount * RookValueMg +
      blackQueenCount * QueenValueMg;
  int egMaterialBlack =
      blackPawnCount * PawnValueEg + blackKnightCount * KnightValueEg +
      blackBishopCount * BishopValueEg + blackRookCount * RookValueEg +
      blackQueenCount * QueenValueEg;
  Score materialScoreWhite = make_score(mgMaterialWhite, egMaterialWhite);
  Score materialScoreBlack = make_score(mgMaterialBlack, egMaterialBlack);

  // Piece-Square Table (PSQT) positional score (use precomputed tables)
  Score psqtScoreWhite = psqt_bonus(pos, WHITE) - piece_value_bonus(pos, WHITE);
  Score psqtScoreBlack = -(psqt_bonus(pos, BLACK) + piece_value_bonus(pos, BLACK));

  // Initialize aggregate scores for each side
  Score scoreWhite = materialScoreWhite + psqtScoreWhite;
  Score scoreBlack = materialScoreBlack + psqtScoreBlack;

  // **Imbalance: Bishop pair**
  if (whiteBishopCount >= 2)
    scoreWhite += BishopPairBonus;
  if (blackBishopCount >= 2)
    scoreBlack += BishopPairBonus;

  // **Imbalance: Pawn majorities** (pawn count advantage on queen-side or
  // king-side) Count pawns on Queen side (files A-D) and King side (files E-H)
  int whiteQueenSidePawns = 0, whiteKingSidePawns = 0;
  int blackQueenSidePawns = 0, blackKingSidePawns = 0;
  Bitboard whitePawnBB = pos.pieces(WHITE, PAWN);
  Bitboard blackPawnBB = pos.pieces(BLACK, PAWN);
  for (int f = 0; f < 4; ++f) {
    whiteQueenSidePawns += __builtin_popcountll(whitePawnBB & FileMask[f]);
    blackQueenSidePawns += __builtin_popcountll(blackPawnBB & FileMask[f]);
  }
  for (int f = 4; f < 8; ++f) {
    whiteKingSidePawns += __builtin_popcountll(whitePawnBB & FileMask[f]);
    blackKingSidePawns += __builtin_popcountll(blackPawnBB & FileMask[f]);
  }
  if (whiteQueenSidePawns > blackQueenSidePawns) {
    int diff = whiteQueenSidePawns - blackQueenSidePawns;
    scoreWhite += diff * PawnMajorityBonus;
  } else if (blackQueenSidePawns > whiteQueenSidePawns) {
    int diff = blackQueenSidePawns - whiteQueenSidePawns;
    scoreBlack += diff * PawnMajorityBonus;
  }
  if (whiteKingSidePawns > blackKingSidePawns) {
    int diff = whiteKingSidePawns - blackKingSidePawns;
    scoreWhite += diff * PawnMajorityBonus;
  } else if (blackKingSidePawns > whiteKingSidePawns) {
    int diff = blackKingSidePawns - whiteKingSidePawns;
    scoreBlack += diff * PawnMajorityBonus;
  }

  // **Rook activity: open and half-open files**
  Bitboard whiteRookBB = pos.pieces(WHITE, ROOK);
  Bitboard blackRookBB = pos.pieces(BLACK, ROOK);
  while (whiteRookBB) {
    Square sq = pop_lsb(whiteRookBB);
    int file = get_file(sq);
    if (!(whitePawnBB & FileMask[file])) { // no white pawn on this file
      if (!(blackPawnBB & FileMask[file]))
        scoreWhite += RookOpenFileBonus; // fully open file
      else
        scoreWhite += RookHalfOpenBonus; // half-open file
    }
  }
  while (blackRookBB) {
    Square sq = pop_lsb(blackRookBB);
    int file = get_file(sq);
    if (!(blackPawnBB & FileMask[file])) {
      if (!(whitePawnBB & FileMask[file]))
        scoreBlack += RookOpenFileBonus;
      else
        scoreBlack += RookHalfOpenBonus;
    }
  }

  // **Pawn structure: doubled, isolated, passed pawns**
  // Doubled pawns: for each file, penalize each extra pawn beyond one
  for (int f = 0; f < 8; ++f) {
    int wCount = __builtin_popcountll(whitePawnBB & FileMask[f]);
    int bCount = __builtin_popcountll(blackPawnBB & FileMask[f]);
    if (wCount > 1)
      scoreWhite -= (wCount - 1) * DoubledPawnPenalty;
    if (bCount > 1)
      scoreBlack -= (bCount - 1) * DoubledPawnPenalty;
  }
  // Isolated pawns: no friendly pawn on adjacent files
  Bitboard tempPawns = whitePawnBB;
  while (tempPawns) {
    Square s = pop_lsb(tempPawns);
    int f = get_file(s);
    bool neighborPawn = false;
    if (f > FILE_A && (whitePawnBB & FileMask[f - 1]))
      neighborPawn = true;
    if (f < FILE_H && (whitePawnBB & FileMask[f + 1]))
      neighborPawn = true;
    if (!neighborPawn)
      scoreWhite -= IsolatedPawnPenalty;
  }
  tempPawns = blackPawnBB;
  while (tempPawns) {
    Square s = pop_lsb(tempPawns);
    int f = get_file(s);
    bool neighborPawn = false;
    if (f > FILE_A && (blackPawnBB & FileMask[f - 1]))
      neighborPawn = true;
    if (f < FILE_H && (blackPawnBB & FileMask[f + 1]))
      neighborPawn = true;
    if (!neighborPawn)
      scoreBlack -= IsolatedPawnPenalty;
  }
  // Passed pawns: no enemy pawn blocking on same or adjacent files ahead
  tempPawns = whitePawnBB;
  while (tempPawns) {
    Square s = pop_lsb(tempPawns);
    int file = get_file(s);
    int rank = get_rank(s);
    // Determine if no black pawns are on file `file` or adjacent files in front
    // of this pawn
    Bitboard blockers = 0ULL;
    for (int df = -1; df <= 1; ++df) {
      if (file + df < 0 || file + df > 7)
        continue;
      // mask of all squares in (file+df) with rank greater than `rank`
      if (rank < 7) {
        Bitboard aheadMask = ~((1ULL << ((rank + 1) * 8)) -
                               1); // all squares with index > rank*8+7
        blockers |= blackPawnBB & (FileMask[file + df] & aheadMask);
      }
    }
    if (!blockers) {
      // Pawn is passed – award bonus increasing with its rank (more advanced =
      // larger bonus)
      int advance = rank; // rank index (0=rank1 ... 7=rank8)
      scoreWhite += make_score(PassedPawnMgFactor * advance,
                               PassedPawnEgFactor * advance);
    }
  }
  tempPawns = blackPawnBB;
  while (tempPawns) {
    Square s = pop_lsb(tempPawns);
    int file = get_file(s);
    int rank = get_rank(s);
    Bitboard blockers = 0ULL;
    for (int df = -1; df <= 1; ++df) {
      if (file + df < 0 || file + df > 7)
        continue;
      if (rank > 0) {
        Bitboard aheadMask =
            ((1ULL << (rank * 8)) - 1); // squares with index < rank*8 (i.e.,
                                        // ahead from black's perspective)
        blockers |= whitePawnBB & (FileMask[file + df] & aheadMask);
      }
    }
    if (!blockers) {
      int advance = (7 - rank); // distance from promotion for black pawn
      scoreBlack += make_score(PassedPawnMgFactor * advance,
                               PassedPawnEgFactor * advance);
    }
  }

  // **Mobility and piece activity**: count legal moves for pieces to reward
  // activity
  Bitboard attacksWhiteNonPawns = 0ULL;
  Bitboard attacksBlackNonPawns = 0ULL;
  // Knights
  Bitboard whiteKnightBB = pos.pieces(WHITE, KNIGHT);
  Bitboard blackKnightBB = pos.pieces(BLACK, KNIGHT);
  while (whiteKnightBB) {
    Square sq = pop_lsb(whiteKnightBB);
    for (auto dir : KnightDirs) {
      int tf = get_file(sq) + dir[0];
      int tr = get_rank(sq) + dir[1];
      if (tf < 0 || tf > 7 || tr < 0 || tr > 7)
        continue;
      Square toSq = make_square(File(tf), Rank(tr));
      Piece p = pos.piece_on(toSq);
      if (p != NO_PIECE && get_color(p) == WHITE)
        continue; // friendly piece blocks
      // legal knight move (empty or captures enemy)
      scoreWhite += KnightMobilityBonus;
      if (p == NO_PIECE || get_color(p) == BLACK) {
        // add attacked square (for space calculation)
        attacksWhiteNonPawns |= (1ULL << toSq);
      }
    }
  }
  while (blackKnightBB) {
    Square sq = pop_lsb(blackKnightBB);
    for (auto dir : KnightDirs) {
      int tf = get_file(sq) + dir[0];
      int tr = get_rank(sq) + dir[1];
      if (tf < 0 || tf > 7 || tr < 0 || tr > 7)
        continue;
      Square toSq = make_square(File(tf), Rank(tr));
      Piece p = pos.piece_on(toSq);
      if (p != NO_PIECE && get_color(p) == BLACK)
        continue;
      scoreBlack += KnightMobilityBonus;
      if (p == NO_PIECE || get_color(p) == WHITE) {
        attacksBlackNonPawns |= (1ULL << toSq);
      }
    }
  }
  // Bishops
  Bitboard whiteBishopBB = pos.pieces(WHITE, BISHOP);
  Bitboard blackBishopBB = pos.pieces(BLACK, BISHOP);
  while (whiteBishopBB) {
    Square sq = pop_lsb(whiteBishopBB);
    for (auto dir : BishopDirs) {
      int tf = get_file(sq) + dir[0];
      int tr = get_rank(sq) + dir[1];
      // traverse along the ray in this direction
      while (tf >= 0 && tf < 8 && tr >= 0 && tr < 8) {
        Square toSq = make_square(File(tf), Rank(tr));
        Piece p = pos.piece_on(toSq);
        if (p != NO_PIECE && get_color(p) == WHITE)
          break; // blocked by own piece
        // empty or enemy piece square is reachable
        scoreWhite += BishopMobilityBonus;
        attacksWhiteNonPawns |= (1ULL << toSq);
        if (p != NO_PIECE &&
            get_color(p) == BLACK) { // can capture enemy, then stop
          break;
        }
        // continue ray if empty
        tf += dir[0];
        tr += dir[1];
      }
    }
  }
  while (blackBishopBB) {
    Square sq = pop_lsb(blackBishopBB);
    for (auto dir : BishopDirs) {
      int tf = get_file(sq) + dir[0];
      int tr = get_rank(sq) + dir[1];
      while (tf >= 0 && tf < 8 && tr >= 0 && tr < 8) {
        Square toSq = make_square(File(tf), Rank(tr));
        Piece p = pos.piece_on(toSq);
        if (p != NO_PIECE && get_color(p) == BLACK)
          break;
        scoreBlack += BishopMobilityBonus;
        attacksBlackNonPawns |= (1ULL << toSq);
        if (p != NO_PIECE && get_color(p) == WHITE) {
          break;
        }
        tf += dir[0];
        tr += dir[1];
      }
    }
  }
  // Rooks
  // (we already have whiteRookBB and blackRookBB from above)
  whiteRookBB = pos.pieces(WHITE, ROOK);
  blackRookBB = pos.pieces(BLACK, ROOK);
  while (whiteRookBB) {
    Square sq = pop_lsb(whiteRookBB);
    for (auto dir : RookDirs) {
      int tf = get_file(sq) + dir[0];
      int tr = get_rank(sq) + dir[1];
      while (tf >= 0 && tf < 8 && tr >= 0 && tr < 8) {
        Square toSq = make_square(File(tf), Rank(tr));
        Piece p = pos.piece_on(toSq);
        if (p != NO_PIECE && get_color(p) == WHITE)
          break;
        scoreWhite += RookMobilityBonus;
        attacksWhiteNonPawns |= (1ULL << toSq);
        if (p != NO_PIECE && get_color(p) == BLACK) {
          break;
        }
        tf += dir[0];
        tr += dir[1];
      }
    }
  }
  while (blackRookBB) {
    Square sq = pop_lsb(blackRookBB);
    for (auto dir : RookDirs) {
      int tf = get_file(sq) + dir[0];
      int tr = get_rank(sq) + dir[1];
      while (tf >= 0 && tf < 8 && tr >= 0 && tr < 8) {
        Square toSq = make_square(File(tf), Rank(tr));
        Piece p = pos.piece_on(toSq);
        if (p != NO_PIECE && get_color(p) == BLACK)
          break;
        scoreBlack += RookMobilityBonus;
        attacksBlackNonPawns |= (1ULL << toSq);
        if (p != NO_PIECE && get_color(p) == WHITE) {
          break;
        }
        tf += dir[0];
        tr += dir[1];
      }
    }
  }
  // Queens
  Bitboard whiteQueenBB = pos.pieces(WHITE, QUEEN);
  Bitboard blackQueenBB = pos.pieces(BLACK, QUEEN);
  while (whiteQueenBB) {
    Square sq = pop_lsb(whiteQueenBB);
    for (auto dir : QueenDirs) {
      int tf = get_file(sq) + dir[0];
      int tr = get_rank(sq) + dir[1];
      while (tf >= 0 && tf < 8 && tr >= 0 && tr < 8) {
        Square toSq = make_square(File(tf), Rank(tr));
        Piece p = pos.piece_on(toSq);
        if (p != NO_PIECE && get_color(p) == WHITE)
          break;
        scoreWhite += QueenMobilityBonus;
        attacksWhiteNonPawns |= (1ULL << toSq);
        if (p != NO_PIECE && get_color(p) == BLACK) {
          break;
        }
        tf += dir[0];
        tr += dir[1];
      }
    }
  }
  while (blackQueenBB) {
    Square sq = pop_lsb(blackQueenBB);
    for (auto dir : QueenDirs) {
      int tf = get_file(sq) + dir[0];
      int tr = get_rank(sq) + dir[1];
      while (tf >= 0 && tf < 8 && tr >= 0 && tr < 8) {
        Square toSq = make_square(File(tf), Rank(tr));
        Piece p = pos.piece_on(toSq);
        if (p != NO_PIECE && get_color(p) == BLACK)
          break;
        scoreBlack += QueenMobilityBonus;
        attacksBlackNonPawns |= (1ULL << toSq);
        if (p != NO_PIECE && get_color(p) == WHITE) {
          break;
        }
        tf += dir[0];
        tr += dir[1];
      }
    }
  }
  // Kings (for mobility and king safety measures)
  Square whiteKingSq = lsb(pos.pieces(WHITE, KING));
  Square blackKingSq = lsb(pos.pieces(BLACK, KING));
  // King mobility (only count in endgame phase)
  for (auto dir : KingDirs) {
    int wf = get_file(whiteKingSq) + dir[0];
    int wr = get_rank(whiteKingSq) + dir[1];
    if (wf >= 0 && wf < 8 && wr >= 0 && wr < 8) {
      Square toSq = make_square(File(wf), Rank(wr));
      Piece p = pos.piece_on(toSq);
      if (p == NO_PIECE || get_color(p) == BLACK) {
        // legal king move (ignore moves into own pieces or off-board)
        scoreWhite += KingMobilityBonus;
        // (We do not add king moves to attacksWhiteNonPawns for space count)
      }
    }
  }
  for (auto dir : KingDirs) {
    int bf = get_file(blackKingSq) + dir[0];
    int br = get_rank(blackKingSq) + dir[1];
    if (bf >= 0 && bf < 8 && br >= 0 && br < 8) {
      Square toSq = make_square(File(bf), Rank(br));
      Piece p = pos.piece_on(toSq);
      if (p == NO_PIECE || get_color(p) == WHITE) {
        scoreBlack += KingMobilityBonus;
      }
    }
  }

  // **King safety**: King position safety factors
  // 1. Distance from center (encourage corner safety in middlegame, center
  // activity in endgame)
  int whiteKingFile = get_file(whiteKingSq),
      whiteKingRank = get_rank(whiteKingSq);
  int blackKingFile = get_file(blackKingSq),
      blackKingRank = get_rank(blackKingSq);
  // Manhattan distance from center (d4/e4/d5/e5 center area roughly at
  // (3.5,3.5))
  int whiteKingDistCenter = abs(whiteKingFile - 3) + abs(whiteKingRank - 3);
  int blackKingDistCenter = abs(blackKingFile - 3) + abs(blackKingRank - 3);
  // Apply as positive in MG (farther is better) and negative in EG (farther is
  // worse)
  Score kingCenterWeight = make_score(5, -5);
  scoreWhite += kingCenterWeight * whiteKingDistCenter;
  scoreBlack += kingCenterWeight * blackKingDistCenter;
  // 2. Pawn shield: missing pawns in front of the king
  if (whiteKingRank <= 2) {
    // check two ranks ahead of white king for pawn shield
    for (int r = whiteKingRank + 1; r <= std::min(whiteKingRank + 2, 7); ++r) {
      bool pawnFound = false;
      for (int df = -1; df <= 1; ++df) {
        int f = whiteKingFile + df;
        if (f < 0 || f > 7)
          continue;
        Square sq = make_square(File(f), Rank(r));
        if (pos.piece_on(sq) == make_piece(WHITE, PAWN)) {
          pawnFound = true;
          break;
        }
      }
      if (!pawnFound) {
        // missing pawn on this rank in front of king: penalize in MG
        scoreWhite -= make_score((r == whiteKingRank + 1 ? 30 : 15), 0);
      }
    }
  }
  if (blackKingRank >= 5) {
    for (int r = blackKingRank - 1; r >= std::max(blackKingRank - 2, 0); --r) {
      bool pawnFound = false;
      for (int df = -1; df <= 1; ++df) {
        int f = blackKingFile + df;
        if (f < 0 || f > 7)
          continue;
        Square sq = make_square(File(f), Rank(r));
        if (pos.piece_on(sq) == make_piece(BLACK, PAWN)) {
          pawnFound = true;
          break;
        }
      }
      if (!pawnFound) {
        scoreBlack -= make_score((r == blackKingRank - 1 ? 30 : 15), 0);
      }
    }
  }
  // 3. Exposed king (open/half-open files near king)
  for (int df = -1; df <= 1; ++df) {
    int wf = whiteKingFile + df;
    if (wf < 0 || wf > 7)
      continue;
    // if file has no white pawn, king is exposed along that file
    if (!(whitePawnBB & FileMask[wf])) {
      // open file (no black pawn either) is worse than half-open
      if (!(blackPawnBB & FileMask[wf]))
        scoreWhite -= make_score(20, 0);
      else
        scoreWhite -= make_score(10, 0);
    }
  }
  for (int df = -1; df <= 1; ++df) {
    int bf = blackKingFile + df;
    if (bf < 0 || bf > 7)
      continue;
    if (!(blackPawnBB & FileMask[bf])) {
      if (!(whitePawnBB & FileMask[bf]))
        scoreBlack -= make_score(20, 0);
      else
        scoreBlack -= make_score(10, 0);
    }
  }

  // **Threats**: attacks on opponent pieces that are not defended
  Bitboard allWhitePieces = pos.pieces(WHITE);
  Bitboard allBlackPieces = pos.pieces(BLACK);
  // Check each white piece to see if it is attacked by a black piece and not
  // protected by white
  Bitboard tempPieces = allWhitePieces;
  while (tempPieces) {
    Square sq = pop_lsb(tempPieces);
    Piece pc = pos.piece_on(sq);
    PieceType pt = get_piece_type(pc);
    if (pt == KING)
      continue; // skip king threats in static eval
    if (pos.square_is_attacked(BLACK, sq) &&
        !pos.square_is_attacked(WHITE, sq)) {
      // White piece is hanging (attacked by black, no defender) -> good for
      // Black
      switch (pt) {
      case PAWN:
        scoreBlack += ThreatByPawn;
        break;
      case KNIGHT:
      case BISHOP:
        scoreBlack += ThreatByMinor;
        break;
      case ROOK:
        scoreBlack += ThreatByRook;
        break;
      case QUEEN:
        scoreBlack += ThreatByQueen;
        break;
      default:
        break;
      }
    }
  }
  // Symmetrically, check each black piece for white threats
  tempPieces = allBlackPieces;
  while (tempPieces) {
    Square sq = pop_lsb(tempPieces);
    Piece pc = pos.piece_on(sq);
    PieceType pt = get_piece_type(pc);
    if (pt == KING)
      continue;
    if (pos.square_is_attacked(WHITE, sq) &&
        !pos.square_is_attacked(BLACK, sq)) {
      switch (pt) {
      case PAWN:
        scoreWhite += ThreatByPawn;
        break;
      case KNIGHT:
      case BISHOP:
        scoreWhite += ThreatByMinor;
        break;
      case ROOK:
        scoreWhite += ThreatByRook;
        break;
      case QUEEN:
        scoreWhite += ThreatByQueen;
        break;
      default:
        break;
      }
    }
  }

  // **Space**: count controlled squares (by pieces other than pawns) in
  // opponent's half
  Bitboard occupied = pos.pieces(); // all occupied squares
  // Only count empty squares attacked
  Bitboard whiteSpaceSquares = attacksWhiteNonPawns & ~occupied;
  Bitboard blackSpaceSquares = attacksBlackNonPawns & ~occupied;
  // Mask to opponent half: for White, squares with rank >= 4; for Black, rank
  // <= 3
  Bitboard whiteSpaceMask = ~((1ULL << (4 * 8)) - 1); // bits 32-63 (ranks 5-8)
  Bitboard blackSpaceMask = ((1ULL << (4 * 8)) - 1);  // bits 0-31 (ranks 1-4)
  int spaceCountWhite =
      __builtin_popcountll(whiteSpaceSquares & whiteSpaceMask);
  int spaceCountBlack =
      __builtin_popcountll(blackSpaceSquares & blackSpaceMask);
  scoreWhite += spaceCountWhite * SpaceBonus;
  scoreBlack += spaceCountBlack * SpaceBonus;

  // Combine total scores and apply game phase interpolation
  Score totalScore = scoreWhite - scoreBlack;
  // Game phase (0 = endgame, 128 = midgame) based on remaining material
  int npmWhite = whiteKnightCount * KnightValueMg +
                 whiteBishopCount * BishopValueMg +
                 whiteRookCount * RookValueMg + whiteQueenCount * QueenValueMg;
  int npmBlack = blackKnightCount * KnightValueMg +
                 blackBishopCount * BishopValueMg +
                 blackRookCount * RookValueMg + blackQueenCount * QueenValueMg;
  int npmTotal = npmWhite + npmBlack;
  // Clamp phase between EndgameLimit and MidgameLimit
  int phase;
  npmTotal = std::min(std::max(npmTotal, int(EndgameLimit)), int(MidgameLimit));
  phase = ((npmTotal - EndgameLimit) * 128) / (MidgameLimit - EndgameLimit);
  // Interpolate between MG and EG scores
  Value mgScore = mg_value(totalScore);
  Value egScore = eg_value(totalScore);
  Value blended =
      Value((int(mgScore) * phase + int(egScore) * (128 - phase)) / 128);
  // Apply side to move: return score from the perspective of the side to move
  return (pos.side_to_move() == WHITE ? blended : -blended);
}
