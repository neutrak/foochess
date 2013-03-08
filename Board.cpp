#include "Board.h"
#include "stdio.h"
#include <stdlib.h>

//constructor
//makes internal structures based off of the information we're provided with
Board::Board(vector<Piece> pieces, Board *parent)
{
  white_check=false;
  black_check=false;
  
  //set the parent we were given (for root node this should be NULL)
  p=parent;
  
  //first, NULL out the board
  for(int file=0; file<width; file++)
  {
    for(int rank=0; rank<height; rank++)
    {
      state[(rank*width)+file]=NULL;
    }
  }
  
  for(size_t i=0; i<pieces.size(); i++)
  {
    int file=(pieces[i]).file();
    int rank=(pieces[i]).rank();
    
    //defensive: bounds checking in case we're passed bad data
    if(file>0 && file<=width && rank>0 && rank<=height)
    {
      _SuperPiece *new_piece=(_SuperPiece*)(malloc(sizeof(_SuperPiece)));
      if(new_piece==NULL)
      {
        fprintf(stderr,"Err: Out of RAM!? (malloc failed)\n");
        exit(1);
      }
      
      new_piece->_c=NULL;
      new_piece->id=pieces[i].id();
      new_piece->file=pieces[i].file();
      new_piece->rank=pieces[i].rank();
      new_piece->hasMoved=pieces[i].hasMoved();
      new_piece->type=pieces[i].type();
      new_piece->owner=pieces[i].owner();
      
      //and of course nothing has yet been checked
      new_piece->haveChecked=false;
//      new_piece->movements=old_piece->movements;
      new_piece->movements=0;
      
      //the -1 is to switch 1-indexing to 0-indexing
      state[((rank-1)*width)+(file-1)]=new_piece;
    }
  }
  
  //nothing's been moved yet
  last_moved=NULL;
  last_move_made=NULL;
  
  //check if we're in check (get it?)
  white_check=in_check(0);
  black_check=in_check(1);
}

//copy constructor
Board::Board(Board *board)
{
  white_check=false;
  black_check=false;
  
  //this is the child of the board it was copied from
  p=board;
  
  for(int file=0; file<width; file++)
  {
    for(int rank=0; rank<height; rank++)
    {
      //the +1 is to convert to 1-indexing
      if(board->get_element(file+1, rank+1)!=NULL)
      {
        _SuperPiece *new_piece=(_SuperPiece*)(malloc(sizeof(_SuperPiece)));
        if(new_piece==NULL)
        {
          fprintf(stderr,"Err: Out of RAM!? (malloc failed)\n");
          exit(1);
        }
        _SuperPiece *old_piece=board->get_element(file+1, rank+1);
        
        //make an exact copy of the relevant data
        new_piece->_c=old_piece->_c;
        new_piece->id=old_piece->id;
        new_piece->file=old_piece->file;
        new_piece->rank=old_piece->rank;
        new_piece->hasMoved=old_piece->hasMoved;
        new_piece->type=old_piece->type;
        new_piece->owner=old_piece->owner;
        
        //and of course nothing has yet been checked
        new_piece->haveChecked=false;
        new_piece->movements=old_piece->movements;
        
        state[(rank*width)+(file)]=new_piece;
      }
      //there was no piece here, so NULL it out
      else
      {
        state[(rank*width)+(file)]=NULL;
      }
    }
  }
  
  //nothing's been moved yet
  last_moved=NULL;
  last_move_made=NULL;
  
  //check if we're in check (get it?)
  white_check=in_check(0);
  black_check=in_check(1);
}

//equality check (just checks type, owner, position of pieces, not history or anything)
bool Board::equals(Board *board)
{
  for(int f=1; f<=8; f++)
  {
    for(int r=1; r<=8; r++)
    {
      //if one board has a null here but the other doesn't
      if(((board->get_element(f,r)!=NULL) && (get_element(f,r)==NULL)) || ((board->get_element(f,r)==NULL) && (get_element(f,r)!=NULL)))
      {
        //then the boards are not equal
        return false;
      }
      else if(get_element(f,r)!=NULL)
      {
        _SuperPiece *internal=get_element(f,r);
        _SuperPiece *external=board->get_element(f,r);
        //if the owner or type don't match the boards aren't equal
        //(position is implicitly assured because of the loops)
        if(internal->owner!=external->owner || internal->type!=external->type)
        {
          return false;
        }
      }
    }
  }
  
  //if we got through everything above and didn't return then the boards are equal
  return true;
}

//destructor
Board::~Board()
{
  for(size_t rank=1; rank<=8; rank++)
  {
    for(size_t file=1; file<=8; file++)
    {
      if(get_element(file,rank)!=NULL)
      {
        free(get_element(file,rank));
      }
    }
  }
  
/*
  if(last_move_made!=NULL)
  {
    free(last_move_made);
  }
*/
  
  //and recurse to get all the children
  for(size_t i=0; i<children.size(); i++)
  {
    delete children[i];
  }
}

//deal with other nodes in the structure
void Board::add_child(Board *board)
{
  children.push_back(board);
}

void Board::remove_child(Board *board)
{
  for(vector<Board*>::iterator i=children.begin(); i!=children.end(); i++)
  {
    if((*i)==board)
    {
      i=children.erase(i);
      
      //the -- is so the ++ in the for loop kicks us back to the proper next element
      i--;
    }
  }
}

void Board::clear_children()
{
  //free memory for all the children
  for(size_t i=0; i<children.size(); i++)
  {
    delete children[i];
  }
  
  //remove the references
  children.clear();
}


//Output the board
//(no arguments since all relevant data is already stored in the Board class)
void Board::output_board()
{
  // Print out the current board state
  cout<<"   +---+---+---+---+---+---+---+---+"<<endl;
  for(size_t rank=8; rank>0; rank--)
  {
    cout<<"R"<<rank<<" |";
    for(size_t file=1; file<=8; file++)
    {
      _SuperPiece *p=get_element(file,rank);
      //if there is something on the board at this position
      if(p!=NULL)
      {
          // Checks if the piece is black
          if(p->owner == 1)
          {
            cout<<"*";
          }
          else
          {
            cout<<" ";
          }
          // prints the piece's type
          cout<<(char)(p->type)<<" ";
      }
      else
      {
        cout<<"   ";
      }
      cout<<"|";
    }
    cout<<endl<<"   +---+---+---+---+---+---+---+---+"<<endl;
  }
  
  cout<<"   |";
  for(size_t file=1; file<=8; file++)
  {
    cout<<"F"<<file<<" |";
  }
  cout<<endl;
}

//checks whether a given player is in check in the current board
bool Board::in_check(int player_id)
{
  //first, find the king belonging to this player
  _SuperPiece *king=NULL;
  
  //file
  for(size_t f=1; f<=width; f++)
  {
    //rank
    for(size_t r=1; r<=height; r++)
    {
      //if this is the given player's king
      if(get_element(f,r)!=NULL && get_element(f,r)->owner==player_id && get_element(f,r)->type=='K')
      {
        king=get_element(f,r);
      }
    }
  }
  
  //if we couldn't find the king
  if(king==NULL)
  {
    //I don't know if it's in check but there are certainly some problems here
    return true;
  }
  
  bool king_in_check=false;
  
  //now that we know where the king is, see if any enemy can attack it
  //file
  for(size_t f=1; f<=width; f++)
  {
    //rank
    for(size_t r=1; r<=height; r++)
    {
      //if this is an enemy of any kind
      if(get_element(f,r)!=NULL && get_element(f,r)->owner!=player_id)
      {
        //generate its moves
        _SuperPiece *p=get_element(f,r);
        vector<_Move*> enemy_moves=legal_moves(p);
        
        for(size_t m=0; m<enemy_moves.size(); m++)
        {
          //if it can attack us, then we are indeed in check
          if((enemy_moves[m]->toFile)==(king->file) && (enemy_moves[m]->toRank)==(king->rank))
          {
            king_in_check=true;
          }
          
          //free the RAM
          free(enemy_moves[m]);
        }
        
        //save some clock cycles
        //break the loop after we find an attacking piece
        if(king_in_check)
        {
          f=width+1;
          r=height+1;
        }
      }
    }
  }
  
  return king_in_check;
}

//returns the piece at a given location
_SuperPiece *Board::get_element(int file, int rank)
{
  //the people who wrote the API 1-index for some crazy reason
  //so this maps to proper 0-indexing
  file-=1;
  rank-=1;
  
  //if the given location is within the board's bounds
  if(file>=0 && file<width && rank>=0 && rank<height)
  {
    //return the piece!
    return state[(rank*width)+(file)];
  }
  
  //if the location given was out of bounds, there can't be a piece there
  return NULL;
}

//returns memory for a move structure for a piece
//(remember to free this later)
_Move *Board::make_move(_SuperPiece *p, int to_file, int to_rank)
{
  _Move *new_move=(_Move*)(malloc(sizeof(_Move)));
  if(new_move==NULL)
  {
    fprintf(stderr,"Err: Out of RAM!? (malloc failed)");
    exit(1);
  }
  
  //connection is not something we're dealing with here
  new_move->_c=NULL;
  //I'm not sure what the id is for in a move, so ignore it for now
  new_move->id=0;
  new_move->fromFile=p->file;
  new_move->fromRank=p->rank;
  new_move->toFile=to_file;
  new_move->toRank=to_rank;
  //for the moment anything we want to promote would be to a queen
  new_move->promoteType='Q';
  
  return new_move;
}

//transforms the internal board to be
//what it should be after a given move is applied
void Board::apply_move(_Move *move)
{
  //if it's a king and they're moving 2 spaces, do a castle
  if(get_element(move->fromFile, move->fromRank)->type=='K' && (abs((move->fromFile)-(move->toFile))==2))
  {
    _SuperPiece *king=get_element(move->fromFile, move->fromRank);
    
    //move the rook and apply that
    int direction;
    _SuperPiece *rook;
    
    if(move->toFile > move->fromFile)
    {
      direction=-1;
      rook=get_element(8,king->rank);
    }
    else
    {
      direction=1;
      rook=get_element(1,king->rank);
    }
    apply_move(make_move(rook, (move->toFile)+direction, move->toRank));
    
    //then move the king by continuing after this if
  }
  
  //free any piece that would be "captured"
  if(get_element(move->toFile, move->toRank)!=NULL)
  {
    _SuperPiece *victim=get_element(move->toFile, move->toRank);
    free(victim);
  }
  
  //NOTE: the -1 everywhere is because the API I was given 1-indexes and I try to be consistent with it where it's not impossible to do so
  
  //move the relevant piece
  state[((move->toRank-1)*width)+(move->toFile-1)]=state[((move->fromRank-1)*width)+(move->fromFile-1)];
  
  _SuperPiece *moved_piece=get_element(move->toFile, move->toRank);

  
  //update that piece's file and rank information so it knows where it now is
  moved_piece->file=(move->toFile);
  moved_piece->rank=(move->toRank);
  
  //and update its move count
  moved_piece->movements++;
  
  //there is now nothing where the piece previously was
  state[((move->fromRank-1)*width)+(move->fromFile-1)]=NULL;
  
  //if it was a pawn and it got to the end, promote it! (using move->promotionType)
  if(moved_piece->type=='P' && (move->toRank==1 || move->toRank==8))
  {
    moved_piece->type=move->promoteType;
  }
  
  //update the internal board structure to know what was the last thing moved
  last_moved=moved_piece;
  last_move_made=move;
  
  //update the check data for move generation
  white_check=in_check(0);
  black_check=in_check(1);
}

//update an internal variable based on a board position
void Board::set_last_moved(int file, int rank)
{
  if(get_element(file,rank)!=NULL)
  {
    last_moved=get_element(file,rank);
  }
}

//a transformation to get to the next direction
//returns 0 when there are none left
int Board::next_direction(int direction)
{
    switch(direction)
    {
      case -1:
        return 1;
        break;
      case 1:
        //falls through; end condition
      default:
        return 0;
        break;
    }
}

vector<_Move*> Board::pawn_moves(_SuperPiece *piece)
{
  vector<_Move *> valid_moves;
  
  int direction_coefficient=0;
  //the legal moves for a pawn depend on what color it is, so check that
  if(piece->owner==1)
  {
    direction_coefficient=-1;
  }
  else
  {
    direction_coefficient=1;
  }
  
  //if we can move forward one, add that to the legal moves
  if(get_element(piece->file, piece->rank+direction_coefficient)==NULL)
  {
    valid_moves.push_back(make_move(piece, piece->file, piece->rank+direction_coefficient));
  }
  
  //if there is someone to attack on either or both diagonals, add that to the legal moves
  _SuperPiece *to_attack=get_element((piece->file)+1, piece->rank+direction_coefficient);
  if(to_attack!=NULL && (to_attack->owner!=piece->owner))
  {
    valid_moves.push_back(make_move(piece, to_attack->file, to_attack->rank));
  }
  to_attack=get_element((piece->file)-1, piece->rank+direction_coefficient);
  if(to_attack!=NULL && (to_attack->owner!=piece->owner))
  {
    valid_moves.push_back(make_move(piece, to_attack->file, to_attack->rank));
  }
  
  //also account for en passant captures
  int x_direction=-1;
  while(x_direction!=0)
  {
    _SuperPiece *adjacent=get_element(piece->file+x_direction, piece->rank);
    //if we can en passant in this direction
    if((adjacent!=NULL) && (last_moved==adjacent) && (adjacent->type=='P') && (adjacent->owner!=piece->owner) && (adjacent->movements==1) && (adjacent->rank>3 && adjacent->rank<6))
    {
      //if no one's there
      if(get_element(adjacent->file, adjacent->rank+direction_coefficient)==NULL)
      {
        valid_moves.push_back(make_move(piece, adjacent->file, adjacent->rank+direction_coefficient));
        printf("Board::pawn_moves() debug 0, made an En Passant with file offset as %i; move is (%i,%i) to (%i,%i)\n", x_direction, piece->file, piece->rank, adjacent->file, adjacent->rank+direction_coefficient);
      }
    }
    
    x_direction=next_direction(x_direction);
  }
  
  
  //if we're still on the starting line and can move two ahead, add that to the legal moves
  if(piece->movements==0)
  {
    if(get_element(piece->file, piece->rank+direction_coefficient)==NULL && get_element(piece->file, piece->rank+(2*direction_coefficient))==NULL)
    {
      valid_moves.push_back(make_move(piece, piece->file, piece->rank+(2*direction_coefficient)));
    }
  }
  
  return valid_moves;
}

vector<_Move*> Board::rook_moves(_SuperPiece *piece)
{
  vector<_Move *> valid_moves;
  
  //add any movement left, right, up, or down
  //from 1 to the number of tiles away the nearest other piece is in that direction
  //for an enemy piece, include that tile, for an owned piece, don't
  
  //save some code, re-use the rank checking for up and down
  int direction=-1;
  while(direction!=0)
  {
    //check a rank
    int rank;
    //the +direction in initialization is because no-op is not a valid move
    for(rank=piece->rank+direction; ((rank>0 && rank<=height) && (get_element(piece->file, rank)==NULL)); rank+=direction)
    {
      //any move until an obstacle or board end is valid
      valid_moves.push_back(make_move(piece, piece->file, rank));
    }
    
    //if there was an enemy piece in the way
    if((rank>0 && rank<=height) && (get_element(piece->file, rank)->owner!=(piece->owner)))
    {
      //attacking the enemy is a valid move
      valid_moves.push_back(make_move(piece, piece->file, rank));
    }
    
    direction=next_direction(direction); 
  }
  
  //same thing we did for rank, just do it for file
  //TODO: try to compress this and the rank code into one loop, may need to use a bitmask for that...
  direction=-1;
  while(direction!=0)
  {
    //check a file
    int file;
    //the +direction in initialization is because no-op is not a valid move
    for(file=piece->file+direction; ((file>0 && file<=width) && (get_element(file, piece->rank)==NULL)); file+=direction)
    {
      //any move until an obstacle or board end is valid
      valid_moves.push_back(make_move(piece, file, piece->rank));
    }
    
    //if there was an enemy piece in the way
    if((file>0 && file<=width) && (get_element(file, piece->rank)->owner!=(piece->owner)))
    {
      //attacking the enemy is a valid move
      valid_moves.push_back(make_move(piece, file, piece->rank));
    }
    
    direction=next_direction(direction); 
  }
  
  return valid_moves;
}

vector<_Move*> Board::knight_moves(_SuperPiece *piece)
{
  vector<_Move *> valid_moves;
  
  //add any of the 8 possible points, so long as none of our own pieces are already there
  //(and the point isn't off the edge of the board)
  
  //x and y are offsets from the piece's file and rank, respectively
  for(int x=1; x<=2; x++)
  {
    //y and x must never be equal in magnitude
    //the below if-else ensures that
    int y;
    
    if(x==1)
    {
      y=2;
    }
    else
    {
      y=1;
    }
    
    //the direction in which we're checking
    int x_direction=-1;
    while(x_direction!=0)
    {
      x*=-1;
      int y_direction=-1;
      while(y_direction!=0)
      {
        y*=-1;
        
        //if this destination is in the bounds
        if(((piece->file)+x>0) && ((piece->file)+x<=width) && ((piece->rank+y)>0) && ((piece->rank)+y<=height))
        {
          //if there's no one there or it's an enemy
          if(get_element((piece->file+x), (piece->rank)+y)==NULL || get_element((piece->file)+x, (piece->rank)+y)->owner!=piece->owner)
          {
            //then it's a valid move
            valid_moves.push_back(make_move(piece, (piece->file)+x, (piece->rank)+y));
          }
        }
        
        y_direction=next_direction(y_direction);
      }
      
      x_direction=next_direction(x_direction);
    }
  }
  
  return valid_moves;
}

vector<_Move*> Board::bishop_moves(_SuperPiece *piece)
{
  vector<_Move *> valid_moves;
  
  //diagonals, account for pieces in the way the same way rook does
  
  int x_direction=-1;
  while(x_direction!=0)
  {
    int y_direction=-1;
    while(y_direction!=0)
    {
      //file
      int f=(piece->file)+x_direction;
      //rank
      int r=(piece->rank)+y_direction;
      while((f>0 && f<=width) && (r>0 && r<=height))
      {
        //there's nothing in the way in a given direction
        if(get_element(f,r)==NULL)
        {
          valid_moves.push_back(make_move(piece, f, r));
        }
        //we hit something
        else
        {
          //if we don't own it, attacking it is a valid move
          if(get_element(f,r)->owner!=(piece->owner))
          {
            valid_moves.push_back(make_move(piece, f, r));
          }
          
          //break the relevant loops, we're done with this direction
          f=-1;
          r=-1;
        }
        
        //try the next diagonal coordinate
        f+=x_direction;
        r+=y_direction;
      }
      
      y_direction=next_direction(y_direction);
    }
    
    x_direction=next_direction(x_direction);
  }
  
  return valid_moves;
}

vector<_Move*> Board::queen_moves(_SuperPiece *piece)
{
  vector<_Move *> valid_moves;
  
  //diagonals and cardinal directions
  //(just total of legal moves for rook and for bishop)
  vector<_Move *> cardinal_moves=rook_moves(piece);
  for(size_t i=0; i<cardinal_moves.size(); i++)
  {
    valid_moves.push_back(cardinal_moves[i]);
  }
  
  vector<_Move *> diagonal_moves=bishop_moves(piece);
  for(size_t i=0; i<diagonal_moves.size(); i++)
  {
    valid_moves.push_back(diagonal_moves[i]);
  }
  
  return valid_moves;
}

vector<_Move*> Board::king_moves(_SuperPiece *piece)
{
  vector<_Move *> valid_moves;
  
  //one space away in any direction, providing we're not putting ourselves in check, etc.
  
  //f for file
  for(int f=(piece->file)-1; f<=(piece->file+1); f++)
  {
    //r for rank
    for(int r=(piece->rank)-1; r<=(piece->rank)+1; r++)
    {
      //if this spot is on the board and
      //this isn't the position the king is already in
      if(((f>0 && f<=width) && (r>0 && r<=height)) && !(f==piece->file && r==piece->rank))
      {
        //if the space is empty or it's an enemy piece
        if(get_element(f,r)==NULL || get_element(f,r)->owner!=piece->owner)
        {
          //it's a valid move to go to f,r
          valid_moves.push_back(make_move(piece, f, r));
        }
      }
    }
  }
  
  //verify the king isn't in check before this, and check the number of movements already done
  int r=(piece->rank);
  if(((piece->owner==0 && !white_check) || (piece->owner==1 && !black_check)) && piece->movements==0)
  {
    //short castle check
    for(int f=(piece->file+1); f<=8; f++)
    {
      //if there's a piece and it's the one allowed rook
      if((get_element(f,r)!=NULL) && (f==8 && get_element(f,r)->type=='R' && get_element(f,r)->owner==piece->owner && get_element(f,r)->movements==0))
      {
        valid_moves.push_back(make_move(piece, (piece->file)+2, piece->rank));
        printf("Board::king_moves() debug 0, made a Castle; move is (%i,%i) to (%i,%i)\n", piece->file, piece->rank, piece->file+2, piece->rank);
      }
      //someone was there that shouldn't have been; break prematurely
      else if(get_element(f,r)!=NULL)
      {
        break;
      }
    }
    //long castle check
    for(int f=(piece->file-1); f>0; f--)
    {
      //if there's a piece and it's the one allowed rook
      if((get_element(f,r)!=NULL) && (f==1 && get_element(f,r)->type=='R' && get_element(f,r)->owner==piece->owner && get_element(f,r)->movements==0))
      {
        valid_moves.push_back(make_move(piece, (piece->file)-2, piece->rank));
        printf("Board::king_moves() debug 1, made a Castle; move is (%i,%i) to (%i,%i)\n", piece->file, piece->rank, piece->file-2, piece->rank);
      }
      //someone was there that shouldn't have been; break prematurely
      else if(get_element(f,r)!=NULL)
      {
        break;
      }
    }
  }
  
  return valid_moves;
}


//a vector of random moves that can be done the piece in question
vector<_Move*> Board::legal_moves(_SuperPiece *piece)
{
  //allocate the data structure we'll be returning
  vector<_Move*> valid_moves;
  
//  printf("legal_moves debug 0, got a %c at file=%i rank=%i\n", piece->type, piece->file, piece->rank);
  
  switch(piece->type)
  {
    //pawn
    case 'P':
      valid_moves=pawn_moves(piece);
      break;
    //rook
    case 'R':
      valid_moves=rook_moves(piece);
      break;
    //knight
    case 'N':
      valid_moves=knight_moves(piece);
      break;
    //bishop
    case 'B':
      valid_moves=bishop_moves(piece);
      break;
    //queen
    case 'Q':
      valid_moves=queen_moves(piece);
      break;
    //king
    case 'K':
      valid_moves=king_moves(piece);
      break;
  }
  
  return valid_moves;
}


