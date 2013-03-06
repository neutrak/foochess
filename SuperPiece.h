#ifndef SUPERPIECE_H
#define SUPERPIECE_H

struct _SuperPiece;

//a superset of the _Piece the api defined
//hence, a _SuperPiece
struct _SuperPiece
{
  //from _Piece
  Connection* _c;
  int id;
  int owner;
  int file;
  int rank;
  int hasMoved;
  int type;
  
  //whether this has been checked yet for valid moves
  bool haveChecked;
  
  //how many movements this piece has taken so far
  int movements;
};


#endif

