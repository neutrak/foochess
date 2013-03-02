#include "Board.h"
#include "stdio.h"
#include <stdlib.h>

//constructor
//makes internal structures based off of the information we're provided with
Board::Board(vector<Piece> pieces, Board *parent)
{
  //set the parent we were given (for root node this should be NULL)
  p=parent;
  
  //first, NULL out the board to start with
  int file;
  for(file=0; file<width; file++)
  {
    int rank;
    for(rank=0; rank<height; rank++)
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
      _Piece *new_piece=(_Piece*)(malloc(sizeof(_Piece)));
      new_piece->_c=NULL;
      new_piece->id=pieces[i].id();
      new_piece->file=pieces[i].file();
      new_piece->rank=pieces[i].rank();
      new_piece->hasMoved=pieces[i].hasMoved();
      new_piece->type=pieces[i].type();
      new_piece->owner=pieces[i].owner();
      
      //the -1 is to switch 1-indexing to 0-indexing
      state[((rank-1)*width)+(file-1)]=new_piece;
    }
  }
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
}

//Output the board
//(no arguments since all relevant data is already stored in the BaseAI class)
void Board::output_board()
{
  // Print out the current board state
  cout<<"   +---+---+---+---+---+---+---+---+"<<endl;
  for(size_t rank=8; rank>0; rank--)
  {
    cout<<"R"<<rank<<" |";
    for(size_t file=1; file<=8; file++)
    {
      _Piece *p=get_element(file,rank);
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
}


//returns the piece at a given location
_Piece *Board::get_element(int file, int rank)
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
_Move *Board::make_move(_Piece *p, int to_file, int to_rank)
{
  _Move *new_move=(_Move*)(malloc(sizeof(_Move)));
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

//a vector of random moves that can be done the piece in question
vector<_Move*> Board::legal_moves(Piece p)
{
  //first, map onto the internal data structure
  _Piece *piece=get_element(p.file(),p.rank());
  
  //allocate the data structure we'll be returning
  vector<_Move*> valid_moves;
  
  printf("legal_moves debug 0, got a %c at file=%i rank=%i\n", piece->type, piece->file, piece->rank);
  
  switch(piece->type)
  {
    //pawn
    case 'P':
      //make a new scope, because I want some scope-specific variables
      //(the jump table shouldn't care)
      {
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
        _Piece *to_attack=get_element((piece->file)+1, piece->rank+direction_coefficient);
        if(to_attack!=NULL && (to_attack->owner!=piece->owner))
        {
          valid_moves.push_back(make_move(piece, to_attack->file, to_attack->rank));
        }
        to_attack=get_element((piece->file)-1, piece->rank+direction_coefficient);
        if(to_attack!=NULL && (to_attack->owner!=piece->owner))
        {
          valid_moves.push_back(make_move(piece, to_attack->file, to_attack->rank));
        }
        
        //if we're still on the starting line and can move two ahead, add that to the legal moves
        if((piece->owner==1 && piece->rank==7) || (piece->owner==0 && piece->rank==2))
        {
          if(get_element(piece->file, piece->rank+direction_coefficient)==NULL && get_element(piece->file, piece->rank+(2*direction_coefficient))==NULL)
          {
            valid_moves.push_back(make_move(piece, piece->file, piece->rank+(2*direction_coefficient)));
          }
        }
      }
      break;
    //rook
    case 'R':
      {
        //add any movement left, right, up, or down
        //from 1 to the number of tiles away the nearest other piece is in that direction
        //for an enemy piece, include that tile, for an owned piece, don't
        
        //save some code, re-use the rank checking for up and down
        int direction=-1;
        while(direction!=0)
        {
          //check a rank
          size_t rank;
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
          
          switch(direction)
          {
            case -1:
              direction=1;
              break;
            case 1:
              //falls through; end condition
            default:
              direction=0;
              break;
          }
        }
        
        //same thing we did for rank, just do it for file
        //TODO: try to compress this and the rank code into one loop, may need to use a bitmask for that...
        direction=-1;
        while(direction!=0)
        {
          //check a file
          size_t file;
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
          
          switch(direction)
          {
            case -1:
              direction=1;
              break;
            case 1:
              //falls through; end condition
            default:
              direction=0;
              break;
          }
        }
      }
      break;
    //knight
    case 'N':
      //add any of the 4 possible points, so long as none of our own pieces are already there
      break;
    //bishop
    case 'B':
      //diagonals, account for pieces in the way the same way rook does
      break;
    //queen
    case 'Q':
      //diagonals and cardinal directions
      //(just total of legal moves for rook and for bishop)
      break;
    //king
    case 'K':
      //one space away in any direction, providing we're not putting ourselves in check, etc.
      //special case of castling?
      break;
  }
  
  return valid_moves;
}


