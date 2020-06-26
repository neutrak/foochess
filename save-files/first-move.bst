//bst: Board State File
//this file represents a chess board state

board_start
   | a | b | c | d | e | f | g | h |
   +---+---+---+---+---+---+---+---+   
 8 |*R |*N |*B |*Q |*K |*B |*N |*R | 8 
   +---+---+---+---+---+---+---+---+   
 7 |*P |*P |*P |*P |*P |*P |*P |*P | 7 
   +---+---+---+---+---+---+---+---+   
 6 |   |   |   |   |   |   |   |   | 6 
   +---+---+---+---+---+---+---+---+   
 5 |   |   |   |   |   |   |   |   | 5 
   +---+---+---+---+---+---+---+---+   
 4 |   |   |   |   |   |   |   |   | 4 
   +---+---+---+---+---+---+---+---+   
 3 |   |   |   |   |   |   |   |   | 3 
   +---+---+---+---+---+---+---+---+   
 2 | P | P | P | P | P | P | P | P | 2 
   +---+---+---+---+---+---+---+---+   
 1 | R | N | B | Q | K | B | N | R | 1 
   +---+---+---+---+---+---+---+---+   
   | a | b | c | d | e | f | g | h |
board_end

start_player_id=WHITE

//NOTE:these are movement counts for various pieces
//in file-rank notation for their current position on the board
//any pieces whose movement counts are not given
//will be considered to have 0 movements
//movement counts are necessary for en passant and castle calculations

a1_movements=0
e1_movements=0
h1_movements=0
a8_movements=0
e8_movements=0
h8_movements=0


