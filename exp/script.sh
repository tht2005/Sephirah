# chess engines installed into $PATH

elo="$1"
cutechess-cli \
  -engine name=Sephirah cmd=sephirah proto=uci \
    option.Hash=128 \
    option.Threads=1 \
  -engine name=SF$elo cmd=stockfish proto=uci \
    option.UCI_LimitStrength=true \
    option.UCI_Elo=$elo \
    option.Hash=128 \
    option.Threads=1 \
  -each tc=40/60 \
  -games 10 \
  -repeat \
  -concurrency 3 \
  -pgnout "sephirah_vs_sf$elo.pgn"

