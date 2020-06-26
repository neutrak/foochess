#include "Board.h"
#include "stdio.h"
#include "string.h"
#include <stdlib.h>
#include <ctype.h>

//constructor
//makes internal structures based for a starting state
Board::Board()
{
  white_check=false;
  black_check=false;
  sorting_value=0;
  
  //no parent if this is an initial board state
  p=NULL;
  
  //first, NULL out the board
  for(int file=0; file<width; file++)
  {
    for(int rank=0; rank<height; rank++)
    {
      state[(rank*width)+file]=NULL;
    }
  }
  
  //pawn placement
  for(int f=1; f<=width; f++)
  {
    //place a new white pawn at the bottom of the board
    place_piece(0,WHITE,f,2,false,'P',false,0);
    //place a new black pawn at the top of the board
    place_piece(0,BLACK,f,7,false,'P',false,0);
  }
  
  //everything else
  for(int f=1; f<=width; f++)
  {
    //if normal placement is true both black and white get the same type of piece at this file
    //if it's false they get different pieces and have to be handled specially
    bool normal_placement=true;
    int type_to_place;
    
    switch(f)
    {
      //rooks go at the ends
      case 1:
      case 8:
        type_to_place='R';
        break;
      //then knights
      case 2:
      case 7:
        type_to_place='N';
        break;
      //and finally bishops
      case 3:
      case 6:
        type_to_place='B';
        break;
      
      //queen takes the color, king gets the other value
      case 4:
        type_to_place='Q';
        break;
      case 5:
        type_to_place='K';
        break;
      default:
        break;
    }
    
    if(normal_placement)
    {
      place_piece(0,WHITE,f,1,false,type_to_place,false,0);
      place_piece(0,BLACK,f,8,false,type_to_place,false,0);
    }
  }
  
  //nothing's been moved yet
  last_moved=NULL;
  last_move_made=NULL;
  last_capture_type='\0';
  moves_since_capture=0;
  moves_since_advancement=0;
  
  //check whether or not anyone's in check
  check_in_check();
}

void Board::check_in_check()
{
  //check if we're in check (get it?)
  _SuperPiece *white_king=find_king(WHITE);
  if(white_king!=NULL)
  {
    white_check=in_check(white_king->file, white_king->rank, WHITE);
  }
  else
  {
    white_check=true;
  }
  
  _SuperPiece *black_king=find_king(BLACK);
  if(black_king!=NULL)
  {
    black_check=in_check(black_king->file, black_king->rank, BLACK);
  }
  else
  {
    black_check=true;
  }
}

//loads data from a textual description of the board in a humanly-readable format
//and sets the internal state of the board to match that file's description
//as a return/side-effect, sets the player_id to the at-play player
void Board::load_from_file(const char *fname, int *start_player_id)
{
  //NOTE: this is called after the constructor
  //and as such it does not initialize the basic state variables
  //but rather just overrides them as needed

  //clear out any existing pieces on the board to start with a blank slate
  //since normally we would start with the initial starting position board state
  for(int f=1;f<=8;f++)
  {
    for(int r=1;r<=8;r++)
    {
      if(get_element(f,r)!=NULL)
      {
        free(get_element(f,r));
        state[((r-1)*width)+(f-1)]=NULL;
      }
    }
  }
  
  FILE *fp=fopen(fname,"r");
  if(fp==NULL)
  {
    fprintf(stderr,"Warn: Could not open file %s; will have default board state instead!\n",fname);
    return;
  }
  
  //this is a buffer that's only one character long
  //because longer buffers are quicker and more efficient
  //but there's nothing wrong with reading one character at a time if you're patient
  int one_char_buf=fgetc(fp);
  
  //one line worth of input
  char linebuf[BUFFER_SIZE];
  unsigned int linebuf_idx=0;
  
  unsigned int lineno=1;

  //whether we are currently reading the board line-by-line or not
  bool in_board_lines=false;
  
  while(one_char_buf!=EOF)
  {
    linebuf[linebuf_idx]=one_char_buf;
    linebuf_idx++;
    
//    printf("%c",one_char_buf); //debug
    if(one_char_buf=='\n')
    {
      //end of line, so null-terminate the line buffer
      linebuf[linebuf_idx]='\0';
      
      //and load that line
      in_board_lines=load_one_save_line((const char *)(linebuf),in_board_lines,start_player_id);
      
      //reset the line buffer for the next line
      linebuf_idx=0;
      //and update the line number
      lineno++;
    }
    
    //read the next character
    one_char_buf=fgetc(fp);
  }

  if(ferror(fp))
  {
    fprintf(stderr,"Warn: I/O error when reading file %s; will have default board state instead!\n",fname);
    return;
  }
  else if(feof(fp))
  {
    printf("Loaded Save File %s successfully\n",fname);
  }
  
  fclose(fp);
  
}

//load a single line from a save file
bool Board::load_one_save_line(const char *line, bool in_board_lines, int *start_player_id)
{
  //if line starts with "//"
  if((strlen(line)>0) && (line[0]=='/') && (line[1]=='/'))
  {
    //those are comments; ignore them
    return in_board_lines;
  }
  //if line is exactly equal to "board_start"
  else if(strncmp(line,"board_start\n",BUFFER_SIZE)==0)
  {
    //start reading the board state, noting what the starting line was
    return true;
  }
  //if the line is exactly equal to "board_end"
  else if(strncmp(line,"board_end\n",BUFFER_SIZE)==0)
  {
    //then stop reading the board state; what we read now is var=value pairs
    return false;
  }
  //if we're currently reading in board lines
  else if(in_board_lines)
  {
    load_board_save_line(line);
  }
  else
  {
    const char *eq_sign=strstr(line,"=");
    if(eq_sign!=NULL)
    {
      int eq_idx=(eq_sign-line);
      
      //parse var from everything before eq_idx
      char var[BUFFER_SIZE];
      strncpy(var,line,eq_idx);
      var[eq_idx]='\0';
      
      //parse value from everything after eq_idx
      int val_chr_cnt=strlen(line)-eq_idx;
      char val[BUFFER_SIZE];
      strncpy(val,line+eq_idx+1,val_chr_cnt);
      val[val_chr_cnt]='\0';
      
      //if either var or value is null, skip this
      if((strlen(var)==0) || (strlen(val)==0))
      {
        fprintf(stderr,"Warn: Skipping line \"%s\" because it has an empty variable and/or value\n",line);
        return in_board_lines;
      }
      
      //and set var=value in board state
      
      //if this is setting the player id
      if(!strcmp(var,"start_player_id"))
      {
        if(tolower(val[0])=='w')
        {
          *start_player_id=WHITE;
        }
        else if(tolower(val[0])=='b')
        {
          *start_player_id=BLACK;
        }
        return in_board_lines;
      }
      
      load_rank_file_setting_line(var,val);
      return in_board_lines;
    }
  }
  return in_board_lines;
}

//load a single line of board state, now that we know we're in that section
//a board line may be one of the following or something else in that format:
/*
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
*/
void Board::load_board_save_line(const char *line)
{
  unsigned int line_idx=0;
  
  char entry_buf[BUFFER_SIZE];
  unsigned int entry_buf_idx=0;
  
  //which |-delimited entry in the line we're currently at
  int entry_idx=0;
  int chess_rank=0;
  
  //read a character at a time
  while(line_idx<strlen(line))
  {
    //if this character is | then that indicates the current entry should end
    //and a new entry should start
    if((line[line_idx]=='|') || (line[line_idx]=='\n'))
    {
      entry_buf[entry_buf_idx]='\0';
      
      unsigned int entry_buf_len=entry_buf_idx;
      
      //remove any whitespace in this entry (including but not only at the ends)
      unsigned int trim_buf_idx=0;
      char trim_buf[BUFFER_SIZE];
      
      for(entry_buf_idx=0;entry_buf_idx<entry_buf_len;entry_buf_idx++)
      {
        switch(entry_buf[entry_buf_idx])
        {
          case ' ':
          case '\t':
          case '\r':
          case '\n':
            break;
          default:
            trim_buf[trim_buf_idx]=entry_buf[entry_buf_idx];
            trim_buf_idx++;
            break;
        }
      }
      //always null terminate strings
      trim_buf[trim_buf_idx]='\0';
      strcpy(entry_buf,trim_buf);
      
      //ignore blank entries
      if(strlen(entry_buf)>0)
      {
        
//        printf("chess_rank %i\n",chess_rank); //debug
//        printf("entry_idx %i: \"%s\"\n",entry_idx,entry_buf); //debug
        
        //if this is the first entry in the line
        if(entry_idx==0)
        {
          //if it's an integer then that's the rank of this line
          chess_rank=atoi(entry_buf);
          
        }
        //otherwise
        else
        {
          //get the chess file based on the column we're currently in on the line
          char chess_file='a';
          if(entry_idx>0)
          {
            chess_file=((char)entry_idx-1)+'a';
          }
          
          //if chess_file and/or chess_rank are out of bounds
          //chess_rank (aka row) in [1,8]
          //chess_file (aka column) [a,h]
          //then return without placing a piece
          if(chess_file<'a' || chess_file>'h' || chess_rank<1 || chess_rank>8)
          {
            return;
          }
          
          printf("%s at %c%i\n",entry_buf,chess_file,chess_rank);
          
          unsigned int piece_color=WHITE;
          char piece_type=entry_buf[0];
          
          //if it starts with "*" then it's a black-controlled piece
          if((strlen(entry_buf)>1) && piece_type=='*')
          {
            piece_type=entry_buf[1];
            piece_color=BLACK;
          }
          
          //set up a piece at the given rank and file
          //NOTE: movements will be set separately with location_movements= directives in the config file
          //so for now they are initialized to 0
          place_piece(0,piece_color,((chess_file-'a')+1),chess_rank,false,piece_type,false,0);
        }
      }
      
      //reset the buffer for next time
      entry_buf_idx=0;
      
      //update the index of the entry itself within the line
      entry_idx++;
    }
    else
    {
      //extend the current entry with the newly-read character until we hit the delimeter
      entry_buf[entry_buf_idx]=line[line_idx];
      entry_buf_idx++;
    }
    
    line_idx++;
  }
}

//load settings which apply to a piece at a certain
void Board::load_rank_file_setting_line(const char *var, const char *val)
{
  for(int f=1; f<=8; f++)
  {
    for(int r=1; r<=8; r++)
    {
      char var_buf[BUFFER_SIZE];
      sprintf(var_buf,"%c%i_movements",((f-1)+'a'),r);
      if(!strcmp(var,var_buf))
      {
        if(get_element(f,r)!=NULL){
          get_element(f,r)->movements=atoi(val);
          printf("Dbg: %c%i_movements=%i\n",((f-1)+'a'),r,get_element(f,r)->movements); //debug
        }
      }
    }
  }
}

//TODO: write Board::save_to_file(char *fname)
//which saves the current board state into a save file with the given name

//place a piece on the board given some information about the piece
void Board::place_piece(int id, int owner, int file, int rank, int hasMoved, int type, bool haveChecked, int movements)
{
  _SuperPiece *new_piece=(_SuperPiece*)(malloc(sizeof(_SuperPiece)));
  if(new_piece==NULL)
  {
    fprintf(stderr,"Err: Out of RAM!? (malloc failed)\n");
    exit(1);
  }
  
  new_piece->id=id;
  new_piece->file=file;
  new_piece->rank=rank;
  new_piece->hasMoved=hasMoved;
  new_piece->type=type;
  new_piece->owner=owner;
  
  //and of course nothing has yet been checked
  new_piece->haveChecked=haveChecked;
  new_piece->movements=movements;
  
  //the -1 is to switch 1-indexing to 0-indexing
  state[((rank-1)*width)+(file-1)]=new_piece;
}

//copy constructor
Board::Board(Board *board)
{
  //get the check information from the old board
  white_check=board->white_check;
  black_check=board->black_check;
  sorting_value=0;
  
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
  last_capture_type='\0';
  
  //the move history is carried though
  moves_since_capture=board->moves_since_capture;
  moves_since_advancement=board->moves_since_advancement;
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
  
  if(last_move_made!=NULL)
  {
    free(last_move_made);
  }
  
  //and recurse to get all the children
  for(size_t i=0; i<children.size(); i++)
  {
    if(children[i]!=NULL)
    {
      delete children[i];
    }
  }
}

//deal with other nodes in the structure
void Board::add_child(Board *board)
{
  children.push_back(board);
}

void Board::remove_child(size_t index)
{
  if(index<children.size() && (children[index]!=NULL))
  {
    delete children[index];
    children[index]=NULL;
  }
}

void Board::clear_children()
{
  //free memory for all the children
  for(size_t i=0; i<children.size(); i++)
  {
    if(children[i]!=NULL)
    {
      delete children[i];
    }
  }
  
  //remove the references
  children.clear();
}

void Board::swap_children(size_t a, size_t b)
{
  if((a<children.size()) && (b<children.size()))
  {
    //(this could be done without tmp with an xor operation, but memory isn't what we care about here)
    Board *tmp=children[a];
    children[a]=children[b];
    children[b]=tmp;
  }
  else
  {
    fprintf(stderr,"Err: Board::swap_children index out of bounds!\n");
  }
}

//a helper function to randomize children (and by extension move choices)
void Board::shuffle_children()
{
  //you can't shuffle a non-existant vector, give up
  if(children.empty())
  {
    return;
  }
  
  for(size_t swaps=0; swaps<children.size(); swaps++)
  {
    //pick the smallest thing we haven't already swapped
    size_t first=swaps;
    //pick a second random index to swap with the first one, after the first one
    size_t second=(rand()%(children.size()-swaps))+swaps;
    
    //do the swap
    swap_children(first,second);
  }
}

//order children by history table values, given a history table to use
void Board::history_order_children(HistTable *hist)
{
  //DEFENSIVE, we should never be passed null into here
  if(hist!=NULL)
  {
    //set sorting values based on history table, then do a quicksort
    for(size_t i=0; i<children.size(); i++)
    {
      children[i]->set_sorting_value(hist->get_value(children[i]));
    }
    
    quicksort_children(0,children.size()-1);
  }
}

//order children by heursitic values
void Board::heuristic_order_children(int player_id, bool max, bool entropy_heuristic, bool distance_sum, bool heur_pawn_additions, bool heur_position_additions, double enemy_weight, double owned_weight)
{
  //set sorting values based on heuristic, then do a quicksort
  for(size_t i=0; i<children.size(); i++)
  {
    double child_value;
    if(!entropy_heuristic)
    {
      child_value=children[i]->heuristic_value(player_id,max,heur_pawn_additions,heur_position_additions,enemy_weight,owned_weight);
    }
    else
    {
      child_value=children[i]->entropy_heuristic_value(player_id,max,distance_sum);
    }
    
    //if we're not sorting with respect to the max player, flip the order (by flipping the values to sort by)
    if(!max)
    {
      child_value=(-child_value);
    }
    
    children[i]->set_sorting_value(child_value);
  }
  
  quicksort_children(0,children.size()-1);
}

//an in-place quicksort implementation, sorting by history table values
void Board::quicksort_children(int lower_bound, int upper_bound)
{
  if(lower_bound<upper_bound)
  {
    int pivot_index=(upper_bound+lower_bound)/2;
//    int pivot_index=lower_bound;
    
    pivot_index=quicksort_partition_children(lower_bound,upper_bound,pivot_index);
    
    quicksort_children(lower_bound, pivot_index-1);
    quicksort_children(pivot_index+1, upper_bound);
  }
}

//quicksort helper
int Board::quicksort_partition_children(int lower_bound, int upper_bound, int pivot_index)
{
  double pivot_value=children[pivot_index]->get_sorting_value();
  
  //a swap operation, swapping pivot index and upper bound elements
  swap_children(upper_bound,pivot_index);
  
  int store_index=lower_bound;
  
  //NOTE: the upper_bound we were passed in is inclusive; we just swapped pivot_index with that element
  for(int i=lower_bound; i<upper_bound; i++)
  {
    double child_value=children[i]->get_sorting_value();
    
    //the >= is to max sort here
    if(child_value>=pivot_value)
    {
      swap_children(i,store_index);
      store_index++;
    }
  }
  swap_children(store_index,upper_bound);
  return store_index;
}

//output the board
//(no arguments since all relevant data is already stored in the Board class)
void Board::output_board()
{
  //file labels
  printf("   |");
  for(size_t file=1; file<=8; file++)
  {
    printf(" %c |",(char)(file-1+'a'));
  }
  printf("\n");
  
  // Print out the current board state
  printf("   +---+---+---+---+---+---+---+---+   \n");
  for(size_t rank=8; rank>0; rank--)
  {
    //rank labels
    printf(" %zu |",rank);
    for(size_t file=1; file<=8; file++)
    {
      _SuperPiece *p=get_element(file,rank);
      //if there is something on the board at this position
      if(p!=NULL)
      {
          // Checks if the piece is black
          if(p->owner == 1)
          {
            printf("*");
          }
          else
          {
            printf(" ");
          }
          // prints the piece's type
          printf("%c ",(char)(p->type));
      }
      else
      {
        printf("   ");
      }
      printf("|");
    }
    //rank labels
    printf(" %zu ",rank);
    printf("\n   +---+---+---+---+---+---+---+---+   \n");
  }
  
  //file labels
  printf("   |");
  for(size_t file=1; file<=8; file++)
  {
    printf(" %c |",(char)(file-1+'a'));
  }
  printf("\n");
}

//output the board in reverse so that a player playing as black can see it easily
void Board::output_reverse_board()
{
  //file labels
  printf("   |");
  for(size_t file=1; file<=8; file++)
  {
    printf(" %c |",(char)(file-1+'a'));
  }
  printf("\n");
  
  // Print out the current board state
  printf("   +---+---+---+---+---+---+---+---+\n");
  for(size_t rank=1; rank<=8; rank++)
  {
    //rank labels
    printf(" %zu |",rank);
    for(size_t file=1; file<=8; file++)
    {
      _SuperPiece *p=get_element(file,rank);
      //if there is something on the board at this position
      if(p!=NULL)
      {
          // Checks if the piece is black
          if(p->owner == 1)
          {
            printf("*");
          }
          else
          {
            printf(" ");
          }
          // prints the piece's type
          printf("%c ",(char)(p->type));
      }
      else
      {
        printf("   ");
      }
      printf("|");
    }
    //rank labels
    printf(" %zu ",rank);
    printf("\n   +---+---+---+---+---+---+---+---+\n");
  }
  
  //file labels
  printf("   |");
  for(size_t file=1; file<=8; file++)
  {
    printf(" %c |",(char)(file-1+'a'));
  }
  printf("\n");
}

//finds the king
//returns NULL if the king cannot be found
_SuperPiece *Board::find_king(int player_id)
{
  _SuperPiece *king=NULL;
  
  //file
  for(int f=1; f<=width; f++)
  {
    //rank
    for(int r=1; r<=height; r++)
    {
      //if this is the given player's king
      if(get_element(f,r)!=NULL && get_element(f,r)->owner==player_id && get_element(f,r)->type=='K')
      {
        king=get_element(f,r);
      }
    }
  }
  return king;
}

//checks whether a given player is in check at a given position in the current board
bool Board::in_check(int file, int rank, int player_id)
{
  
  //check for pawn one square away in attack position
  int direction_coefficient=0;
  if(player_id==WHITE)
  {
    direction_coefficient=1;
  }
  else
  {
    direction_coefficient=-1;
  }
  
  _SuperPiece *test_piece=get_element(file+1,rank+direction_coefficient);
  if(test_piece!=NULL && test_piece->type=='P' && test_piece->owner!=player_id)
  {
    return true;
  }
  test_piece=get_element(file-1,rank+direction_coefficient);
  if(test_piece!=NULL && test_piece->type=='P' && test_piece->owner!=player_id)
  {
    return true;
  }
  
  //if it's in check from a bishop or queen in the diagonal directions
  //(or king one space away)
  if(in_check_diagonal(file,rank,player_id)) return true;
  
  //if it's in check from a rook or queen in the cardinal directions
  //(or king one space away)
  if(in_check_cardinal(file,rank,player_id)) return true;
  
  //if it's in check from a knight
  if(in_check_fromknight(file,rank,player_id)) return true;
  
  //if we got through everything above and didn't end up in check from that, we're not in check!
  return false;
}

//go through diagonals, if there's an enemy bishop or queen, we're in check
//(or a king one space away)
bool Board::in_check_diagonal(int file, int rank, int player_id)
{
  for(int x_direction=-1; x_direction!=0; x_direction=next_direction(x_direction))
  {
    for(int y_direction=-1; y_direction!=0; y_direction=next_direction(y_direction))
    {
      //file
      int f=file+x_direction;
      //rank
      int r=rank+y_direction;
      while((f>0 && f<=width) && (r>0 && r<=height))
      {
        //we hit something
        if(get_element(f,r)!=NULL)
        {
          if(get_element(f,r)->owner!=(player_id))
          {
            //it's an enemy that's either a bishop or queen
            if(get_element(f,r)->type=='B' || get_element(f,r)->type=='Q')
            {
              //we're in check from it
              return true;
            }
            //it's an enemy king one space away
            else if(get_element(f,r)->type=='K' && (f==(file+x_direction)) && (r==(rank+y_direction)))
            {
              return true;
            }
          }
          
          //break the relevant loops, we're done with this direction
          f=-1;
          r=-1;
        }
        
        //try the next diagonal coordinate
        f+=x_direction;
        r+=y_direction;
      }
    }
  }
  
  return false;
}

//go through cardinal directions, if there's an enemy rook or queen, we're in check
//(or a king one space away)
bool Board::in_check_cardinal(int file, int rank, int player_id)
{
  for(int direction=-1; direction!=0; direction=next_direction(direction))
  {
    //check a rank
    int r;
    //the +direction in initialization is because no-op is not a valid move
    for(r=rank+direction; (r>0 && r<=height); r+=direction)
    {
      //if there's a piece there
      if(get_element(file,r)!=NULL)
      {
        //if it's an enemy
        if(get_element(file,r)->owner!=player_id)
        {
          //if it's a rook or queen
          if(get_element(file,r)->type=='R' || get_element(file,r)->type=='Q')
          {
            return true;
          }
          //it's a king one space away
          else if(get_element(file,r)->type=='K' && (r==(rank+direction)))
          {
            return true;
          }
        }
        //stop looking after we hit someone regardless of its owner
        r+=(height*direction);
      }
    }
  }
  for(int direction=-1; direction!=0; direction=next_direction(direction))
  {
    //check a file
    int f;
    //the +direction in initialization is because no-op is not a valid move
    for(f=file+direction; (f>0 && f<=width); f+=direction)
    {
      //if there's a piece there
      if(get_element(f,rank)!=NULL)
      {
        //if it's an enemy
        if(get_element(f,rank)->owner!=player_id)
        {
          //if it's a rook or queen
          if(get_element(f,rank)->type=='R' || get_element(f,rank)->type=='Q')
          {
            return true;
          }
          //it's a king one space away
          else if(get_element(f,rank)->type=='K' && (f==(file+direction)))
          {
            return true;
          }
        }
        //stop looking after we hit someone regardless of its owner
        f+=(width*direction);
      }

    }
  }
  
  return false;
}

//go through the knight move locations, if there's an enemy knight there we're in check
bool Board::in_check_fromknight(int file, int rank, int player_id)
{
  //x and y are offsets from the file and rank, respectively
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
    for(int x_direction=-1; x_direction!=0; x_direction=next_direction(x_direction))
    {
      x*=-1;
      for(int y_direction=-1; y_direction!=0; y_direction=next_direction(y_direction))
      {
        y*=-1;
        
        //NOTE: bounds checking is done in get_element, which will return NULL if we're not in the bounds
        _SuperPiece *enemy=get_element((file+x), (rank)+y);
        
        //if there's an enemy knight there
        if(enemy!=NULL && enemy->owner!=player_id && enemy->type=='N')
        {
          //then we're in check
          return true;
        }
      }
    }
  }
  
  return false;
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
_Move *Board::make_move(_SuperPiece *p, int to_file, int to_rank, int promote_type)
{
  _Move *new_move=(_Move*)(malloc(sizeof(_Move)));
  if(new_move==NULL)
  {
    fprintf(stderr,"Err: Out of RAM!? (malloc failed)");
    exit(1);
  }
  
  //I'm not sure what the id is for in a move, so ignore it for now
  new_move->id=0;
  new_move->fromFile=p->file;
  new_move->fromRank=p->rank;
  new_move->toFile=to_file;
  new_move->toRank=to_rank;
  new_move->promoteType=promote_type;
  
  return new_move;
}

//copies a move
//(remember to free this later)
_Move *Board::copy_move(_Move *move)
{
  _Move *new_move=(_Move*)(malloc(sizeof(_Move)));
  if(new_move==NULL)
  {
    fprintf(stderr,"Err: Out of RAM!? (malloc failed)");
    exit(1);
  }
  
  //connection is not something we're dealing with here
  new_move->id=move->id;
  new_move->fromFile=move->fromFile;
  new_move->fromRank=move->fromRank;
  new_move->toFile=move->toFile;
  new_move->toRank=move->toRank;
  new_move->promoteType=move->promoteType;
  
  return new_move;
}

//transforms the internal board to be
//what it should be after a given move is applied
void Board::apply_move(_Move *move, bool update_check)
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
    
    //NOTE: since this cannot be a capture or pawn advancement, those variables can't get changed here
    _Move* rook_move=make_move(rook, (move->toFile)+direction, move->toRank, move->promoteType);
    apply_move(rook_move, true);
    
    //then move the king by continuing after this if
  }
  
  //en passant handling
  //if it's a pawn and they're moving diagonally but there's no piece at the destination, it must be an en passant
  if((get_element(move->fromFile, move->fromRank)->type=='P') && (get_element(move->toFile, move->toRank)==NULL) && (move->toFile!=move->fromFile))
  {
    int file_to_capture=move->toFile;
    int rank_to_capture=move->fromRank;
    
    //DEFENSIVE: this should never be null
    if(get_element(file_to_capture,rank_to_capture)!=NULL)
    {
      last_capture_type=get_element(file_to_capture,rank_to_capture)->type;
      
      free(get_element(file_to_capture,rank_to_capture));
      state[((rank_to_capture-1)*width)+(file_to_capture-1)]=NULL;
      moves_since_capture=0;
      //moves_since_advancement is updated later (on any pawn movement), and so doesn't need to be here
    }
  }
  
  //free any piece that would be "captured"
  if(get_element(move->toFile, move->toRank)!=NULL)
  {
    _SuperPiece *victim=get_element(move->toFile, move->toRank);
    
    last_capture_type=victim->type;
    
    free(victim);
    moves_since_capture=0;
  }
  else
  {
    moves_since_capture++;
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
  
  //if it was a pawn
  if(moved_piece->type=='P')
  {
    //if it got to the end, promote it! (using move->promotionType)
    if(move->toRank==1 || move->toRank==8)
    {
      moved_piece->type=move->promoteType;
    }
    //if it was any kind of pawn movement reset pawn advancement counter
    moves_since_advancement=0;
  }
  else
  {
    moves_since_advancement++;
  }
  
  
  //if there was a previous move there's no longer a reference here so free it
  if(last_move_made!=NULL)
  {
    free(last_move_made);
  }
  
  //update the internal board structure to know what was the last thing moved
  last_moved=moved_piece;
  last_move_made=move;
  
  //update the check data for move generation, if desired
  if(update_check)
  {
    _SuperPiece *white_king=find_king(WHITE);
    if(white_king!=NULL)
    {
      white_check=in_check(white_king->file, white_king->rank, WHITE);
    }
    else
    {
      white_check=true;
    }
    
    _SuperPiece *black_king=find_king(BLACK);
    if(black_king!=NULL)
    {
      black_check=in_check(black_king->file, black_king->rank, BLACK);
    }
    else
    {
      black_check=true;
    }
  }
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
    //if this piece may be promoted
    if((direction_coefficient==-1 && piece->rank==2) || (direction_coefficient==1 && piece->rank==7))
    {
      //count every possible promotion type as a possible move (null-terminated)
      const char *promotion_types="RNBQ";
      
      int i=0;
      while(promotion_types[i]!='\0')
      {
        valid_moves.push_back(make_move(piece, piece->file, piece->rank+direction_coefficient, promotion_types[i]));
        i++;
      }
    }
    else
    {
      //in general we're not being promoted so just carry a queen
      valid_moves.push_back(make_move(piece, piece->file, piece->rank+direction_coefficient,'Q'));
    }
  }
  
  //if there is someone to attack on either or both diagonals, add that to the legal moves
  _SuperPiece *to_attack=get_element((piece->file)+1, piece->rank+direction_coefficient);
  if(to_attack!=NULL && (to_attack->owner!=piece->owner))
  {
    valid_moves.push_back(make_move(piece, to_attack->file, to_attack->rank, 'Q'));
  }
  to_attack=get_element((piece->file)-1, piece->rank+direction_coefficient);
  if(to_attack!=NULL && (to_attack->owner!=piece->owner))
  {
    valid_moves.push_back(make_move(piece, to_attack->file, to_attack->rank, 'Q'));
  }
  
  //also account for en passant captures
  for(int x_direction=-1; x_direction!=0; x_direction=next_direction(x_direction))
  {
    _SuperPiece *adjacent=get_element(piece->file+x_direction, piece->rank);
    //if we can en passant in this direction
    if((adjacent!=NULL) && (last_moved==adjacent) && (adjacent->type=='P') && (adjacent->owner!=piece->owner) && (adjacent->movements==1) && (adjacent->rank>3 && adjacent->rank<6))
    {
      //if no one's there
      if(get_element(adjacent->file, adjacent->rank+direction_coefficient)==NULL)
      {
        valid_moves.push_back(make_move(piece, adjacent->file, adjacent->rank+direction_coefficient, 'Q'));
//        printf("Board::pawn_moves() debug 0, made an En Passant with file offset as %i; move is (%i,%i) to (%i,%i)\n", x_direction, piece->file, piece->rank, adjacent->file, adjacent->rank+direction_coefficient);
      }
    }
  }
  
  
  //if we're still on the starting line and can move two ahead, add that to the legal moves
  if(piece->movements==0)
  {
    if(get_element(piece->file, piece->rank+direction_coefficient)==NULL && get_element(piece->file, piece->rank+(2*direction_coefficient))==NULL)
    {
      valid_moves.push_back(make_move(piece, piece->file, piece->rank+(2*direction_coefficient), 'Q'));
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
  for(int direction=-1; direction!=0; direction=next_direction(direction))
  {
    //check a rank
    int rank;
    //the +direction in initialization is because no-op is not a valid move
    for(rank=piece->rank+direction; ((rank>0 && rank<=height) && (get_element(piece->file, rank)==NULL)); rank+=direction)
    {
      //any move until an obstacle or board end is valid
      valid_moves.push_back(make_move(piece, piece->file, rank, 'Q'));
    }
    
    //if there was an enemy piece in the way
    if((rank>0 && rank<=height) && (get_element(piece->file, rank)->owner!=(piece->owner)))
    {
      //attacking the enemy is a valid move
      valid_moves.push_back(make_move(piece, piece->file, rank, 'Q'));
    }
  }
  
  //same thing we did for rank, just do it for file
  //TODO: try to compress this and the rank code into one loop, may need to use a bitmask for that...
  for(int direction=-1; direction!=0; direction=next_direction(direction))
  {
    //check a file
    int file;
    //the +direction in initialization is because no-op is not a valid move
    for(file=piece->file+direction; ((file>0 && file<=width) && (get_element(file, piece->rank)==NULL)); file+=direction)
    {
      //any move until an obstacle or board end is valid
      valid_moves.push_back(make_move(piece, file, piece->rank, 'Q'));
    }
    
    //if there was an enemy piece in the way
    if((file>0 && file<=width) && (get_element(file, piece->rank)->owner!=(piece->owner)))
    {
      //attacking the enemy is a valid move
      valid_moves.push_back(make_move(piece, file, piece->rank, 'Q'));
    }
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
    for(int x_direction=-1; x_direction!=0; x_direction=next_direction(x_direction))
    {
      x*=-1;
      for(int y_direction=-1; y_direction!=0; y_direction=next_direction(y_direction))
      {
        y*=-1;
        
        //if this destination is in the bounds
        if(((piece->file)+x>0) && ((piece->file)+x<=width) && ((piece->rank+y)>0) && ((piece->rank)+y<=height))
        {
          //if there's no one there or it's an enemy
          if(get_element((piece->file+x), (piece->rank)+y)==NULL || get_element((piece->file)+x, (piece->rank)+y)->owner!=piece->owner)
          {
            //then it's a valid move
            valid_moves.push_back(make_move(piece, (piece->file)+x, (piece->rank)+y, 'Q'));
          }
        }
      }
    }
  }
  
  return valid_moves;
}

vector<_Move*> Board::bishop_moves(_SuperPiece *piece)
{
  vector<_Move *> valid_moves;
  
  //diagonals, account for pieces in the way the same way rook does
  
  for(int x_direction=-1; x_direction!=0; x_direction=next_direction(x_direction))
  {
    for(int y_direction=-1; y_direction!=0; y_direction=next_direction(y_direction))
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
          valid_moves.push_back(make_move(piece, f, r, 'Q'));
        }
        //we hit something
        else
        {
          //if we don't own it, attacking it is a valid move
          if(get_element(f,r)->owner!=(piece->owner))
          {
            valid_moves.push_back(make_move(piece, f, r, 'Q'));
          }
          
          //break the relevant loops, we're done with this direction
          f=-1;
          r=-1;
        }
        
        //try the next diagonal coordinate
        f+=x_direction;
        r+=y_direction;
      }
    }
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
          valid_moves.push_back(make_move(piece, f, r, 'Q'));
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
        //if the empty location in between start and destination isn't in check
        if(!in_check(piece->file+1, piece->rank, piece->owner))
        {
          valid_moves.push_back(make_move(piece, (piece->file)+2, piece->rank, 'Q'));
        }
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
        //if the empty location in between start and destination isn't in check
        if(!in_check(piece->file-1, piece->rank, piece->owner))
        {
          valid_moves.push_back(make_move(piece, (piece->file)-2, piece->rank, 'Q'));
        }
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

//the value of a given type of piece
double Board::point_value(int type)
{
  switch(type)
  {
    //pawns are worth 1
    case 'P':
      return 1;
    //knights are worth 3
    case 'N':
      return 3;
    //bishops are worth 3
    case 'B':
      return 3;
    //rooks are worth 5
    case 'R':
      return 5;
    //queens are worth 9
    case 'Q':
      return 9;
    //NOTE: kings are ignored; they are always on the board
    //DEFENSIVE: if we don't know what type it is just ignore it
    default:
      return 0;
  }
}

//TODO: improve the points heuristic with additional things taken into account
//point values with position taken into account, etc.
//1 point added for pawns past center line when informed
//a value proportional to the points value of a piece found is added if we are capable of attacking it when attack_ability
//  in the case this is our own piece, we are able to capture anything that attacks it one move later
//  in the case this is an enemy piece, we are able to capture it
//9 points added for opponent in check when informed
double Board::points(int player_id, bool informed, bool attack_ability)
{
  //accumulator, 0 until we find our pieces
  register double point_accumulator=0;
  
  //the best enemy piece we are capable of attacking in this state
  int best_piece=0;
  
  //file
  for(int f=1; f<=width; f++)
  {
    //rank
    for(int r=1; r<=height; r++)
    {
      //if there is any kind of piece here, think about things
      if(get_element(f,r)!=NULL)
      {
        //uninformed (naive) counting of this piece
        //if there's a piece there and we own it, count it
        if(get_element(f,r)->owner==player_id)
        {
          point_accumulator+=point_value(get_element(f,r)->type);
        }
        
        //below this is all various types of "informed" piece counting
        if(informed)
        {
          if(get_element(f,r)->owner==player_id && get_element(f,r)->type=='P')
          {
            //if this position is past the center line ("past the center" depends on who is at play)
            //add a value proportional to how far away the pawn is from the end
            
            if(player_id==WHITE && r>=5)
            {
              point_accumulator+=(4-(8-r));
            }
            else if(player_id==BLACK && r<=4)
            {
              point_accumulator+=(5-r);
            }
            
/*
            if((player_id==WHITE && r>=5) || (player_id==BLACK && r<=4))
            {
              point_accumulator+=1;
            }
*/
          }
        }
        
        //positioning is accounted for based on attack ability
        if(attack_ability)
        {
          //if our own pieces can attack this (the enemy would be in check were their king here)
          if(in_check(f,r,!player_id))
          {
            //add a value proportional to the points value of a piece for every one of our own pieces we can attack (using in_check)
            if(get_element(f,r)->owner==player_id)
            {
              point_accumulator+=(point_value(get_element(f,r)->type)/5);
//              point_accumulator+=(point_value(get_element(f,r)->type));
            }
            //add a value proportional to the points value of a piece for every enemy piece we can capture (using in_check)
            else
            {
//              point_accumulator+=(point_value(get_element(f,r)->type)/10);
//              point_accumulator+=(point_value(get_element(f,r)->type));
              
              if(point_value(get_element(f,r)->type)>best_piece)
              {
                best_piece=point_value(get_element(f,r)->type);
              }
            }
          }
        }
      }
    }
  }
  
  //if we're accounting for who we can attack
  if(attack_ability)
  {
    //add in something for the best attack we can do next turn
    point_accumulator+=(best_piece/3);
  }
  
  if(informed)
  {
    //add in for the case the given player is checking the opponent
    //NOTE: the player_id player cannot end a move with itself in check, so we don't need to verify they aren't in check
    if(get_check(!player_id))
    {
      point_accumulator+=9;
    }
  }
  
  return point_accumulator;
}


//a general heuristic function to call, with parameters for heuristic options
double Board::heuristic_value(int player_id, bool max, bool heur_pawn_additions, bool heur_position_additions, double enemy_weight, double owned_weight)
{
  //a local player id
  //the heuristic is always calculated with respect to the max player
  //if the max player is not at move, calculate with respect to it anyway
  int pid=max? player_id : !player_id;
  
  return (points(pid,heur_pawn_additions,heur_position_additions)*owned_weight)-(points(!pid,heur_pawn_additions,heur_position_additions)*enemy_weight);
}

//a heuristic where the most possible states (branches) for you and the least possible states for the enemy is considered the best
//note that because we don't prune this heuristic gets REALLY SLOW, since it's seeking towards the largest branching factor possible
int Board::entropy_heuristic_value(int player_id, bool max, bool distance_sum)
{
  //if this is the alternate entropy-based heuristic; the manhatten distances of all movments
  if(distance_sum)
  {
    int dist_point_total=0;
    
    for(size_t i=0;i<children.size();i++)
    {
      if(children[i]->last_move_made!=NULL)
      {
        int delta_file=abs((children[i]->last_move_made->fromFile)-(children[i]->last_move_made->toFile));
        int delta_rank=abs((children[i]->last_move_made->fromRank)-(children[i]->last_move_made->toRank));
	
        int manhatten_distance=(delta_file+delta_rank);
        int piece_value_multiplyer=point_value(get_element(children[i]->last_move_made->fromFile,children[i]->last_move_made->fromRank)->type);
        dist_point_total+=((manhatten_distance)*piece_value_multiplyer);
      }
    }
    
    if(!max)
    {
      return (-dist_point_total);
    }
    return dist_point_total;
  }
  else
  {
    //try to minimize the enemy's moves while maximizing our moves
    int enemy_move_value=0;
    int total_move_value=0;
    
    //we've combined entropy with normal points in this heuristic
    //this is how much to multiply the point value by for scale before adding entropy for the piece
    int point_weight=5;
    
    //file
    for(int f=1; f<=width; f++)
    {
      //rank
      for(int r=1; r<=height; r++)
      {
        //if there is any kind of piece here, think about things
        if(get_element(f,r)!=NULL)
        {
          vector<_Move*> piece_moves=legal_moves(get_element(f,r));
          
          int piece_point_value=point_value(get_element(f,r)->type);
          
          //this is the pawn addition to the points heuristic
          if(get_element(f,r)->type=='P')
          {
            //if this position is past the center line ("past the center" depends on who is at play)
            //add a value proportional to how far away the pawn is from the end
            
            if(player_id==WHITE && r>=5)
            {
              piece_point_value+=(4-(8-r));
            }
            else if(player_id==BLACK && r<=4)
            {
              piece_point_value+=(5-r);
            }
          }
          
          //if there's a piece there and we own it, count it
          if(get_element(f,r)->owner==player_id)
          {
            //move value proportional to points and possible moves
            total_move_value+=((piece_point_value)*(point_weight))+(piece_moves.size());
          }
          //if there's a piece there and we DON'T, count it (as an enemy)
//          else if(get_element(f,r)->owner!=player_id)
          else
          {
            //move value proportional to points and possible moves
            enemy_move_value+=((piece_point_value)*(point_weight))+(piece_moves.size());
          }
          
          for(size_t i=0;i<piece_moves.size();i++)
          {
            free(piece_moves[i]);
          }
        }
      }
    }
    
    if(!max)
    {
      return (-total_move_value+enemy_move_value);
//      return (-total_move_value);
    }
    
    return (total_move_value-enemy_move_value);
//    return (total_move_value);
  }
}

//this is a count of how many tiles on the board are attackable by the given player
//it's something I'm playing with as part of heuristic calculation
double Board::board_ownership(int player_id)
{
  double position_accumulator=0;
  
  for(int f=1; f<=width; f++)
  {
    for(int r=1; r<=height; r++)
    {
      //if the other player would be in check were their king here
      //NOTE: an en passant cannot capture a king so that is not accounted for here
      //since an en passant can only take a pawn anyway this is deemed acceptable
      if(in_check(f,r,!player_id))
      {
        //then we can attack this square, we "own" it
        position_accumulator+=1;
      }
    }
  }
  
  return position_accumulator;
}

//returns true if this board state is "quiescent"; else false
bool Board::quiescent()
{
  //a state that includes a check is not quiescent
  if(get_check(WHITE) || get_check(BLACK))
  {
    return false;
  }
  
  //if the last move was a capture this is not a quiescent state
  if(moves_since_capture==0)
  {
    return false;
  }
  
/*
  //the max number of points being "attacked" before this stated is considered non-quiescent
  //NOTE: this is for BOTH PLAYERS IN TOTAL, not per player
  double quiescent_attack_point_threshold=14.0;
  
  //the accumulator for the above noted bound condition
  double points_attacked=0.0;
  
  for(int f=1; f<=width; f++)
  {
    for(int r=1; r<=height; r++)
    {
      if(get_element(f,r)!=NULL)
      {
        //if this piece can be attacked here (except a few weird cases like en passants)
        if(in_check(f,r,get_element(f,r)->owner))
        {
          points_attacked+=point_value(get_element(f,r)->type);
        }
      }
    }
  }
  
  //if a bunch of stuff is being attacked this is not a quiescent state
  if(points_attacked >= quiescent_attack_point_threshold)
  {
    return false;
  }
*/
  
  //if none of the conditions for non-quiescent states were met, this /is/ a quiescent state
  return true;
}



