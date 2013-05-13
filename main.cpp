//foobar-chess, a chess program based around an AI I made for class

#include <ctype.h>
#include "Board.h"
#include "AI.h"
#include "TreeSearch.h"
#define VERSION "0.1"

//this is able to play a very configured AI against another very configured AI, either black or white for both user and AI
int main(int argc, char *argv[])
{
  //command line arguments, only for help text
  if(argc>1)
  {
    if(!strcmp(argv[1],"--help"))
    {
      printf("For help see the manual page\n");
    }
    else if(!strcmp(argv[1],"--version"))
    {
      printf("foochess version %s\n",VERSION);
    }
    //any cli arguments terminate the program before really running
    exit(0);
  }
  
  //make a board to play on
  Board *board=new Board();
  
  //create some AI player objects
  AI *white_player=new AI();
  white_player->init();
  
  AI *black_player=new AI();
  black_player->init();
  
  //by default, white player is a human
  white_player->set_algo(USER);
  
  //display options, allow user to set them
  white_player->configure(WHITE);
  black_player->configure(BLACK);
  
  bool game_over=false;
  int winner=WHITE;
  while(!game_over)
  {
    board->output_board();
    
    if(board->get_last_move_made()!=NULL)
    {
      white_player->remember_move(board->copy_move(board->get_last_move_made()));
    }
    
    printf("White player's turn...\n");
    white_player->run(board,WHITE);
    
    TreeSearch::generate_moves(board,BLACK);
    int black_legal_moves=board->get_children().size();
    board->clear_children();
    
    //checkmate is when someone is in check and has no legal moves
    if((black_legal_moves==0) && (board->get_check(BLACK)))
    {
      printf("CHECKMATE\n");
      game_over=true;
      winner=WHITE;
    }
    //stalemate is when there are no legal moves or there has been repetition, insufficient material, etc.
    else if(black_legal_moves==0 || TreeSearch::stalemate(board,white_player->get_moves()))
    {
      printf("STALEMATE\n");
      game_over=true;
      winner=-1;
    }
    
    if(!game_over)
    {
      board->output_board();
      
      if(board->get_last_move_made()!=NULL)
      {
        black_player->remember_move(board->copy_move(board->get_last_move_made()));
      }
      
      printf("Black player's turn...\n");
      black_player->run(board,BLACK);
      
      TreeSearch::generate_moves(board,WHITE);
      int white_legal_moves=board->get_children().size();
      board->clear_children();
      
      //checkmate is when someone is in check and has no legal moves
      if((white_legal_moves==0) && (board->get_check(WHITE)))
      {
        printf("CHECKMATE\n");
        game_over=true;
        winner=BLACK;
      }
      //stalemate is when there are no legal moves or there has been repetition, insufficient material, etc.
      else if(white_legal_moves==0 || TreeSearch::stalemate(board,black_player->get_moves()))
      {
        printf("STALEMATE\n");
        game_over=true;
        winner=-1;
      }
    }
  }
  board->output_board();
  printf("Game over, winner was %s\n",(winner==WHITE)? "White" : (winner==BLACK)? "Black" : "<stalemate>");
  
  delete board;
  
  white_player->end();
  delete white_player;
  
  black_player->end();
  delete black_player;
  
  return 0;
}

