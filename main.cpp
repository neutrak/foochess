//foobar-chess, a chess program based around an AI I made for class

#include <ctype.h>
#include "Board.h"
#include "AI.h"
#include "TreeSearch.h"

//TODO: make this able to play a very configured AI against another very configured AI, either black or white for both user and AI
int main(int argc, char *argv[])
{
  //make a board to play on
  Board *board=new Board();
  
  //create some AI player objects
  AI *white_player=new AI();
  white_player->init();
  
  AI *black_player=new AI();
  black_player->init();
  
  //by default, white player is a human
//  white_player->set_algo(USER);
  
  //TODO: display options, allow user to set them
  //TODO: WRITE AI::configure()!!!
//  white_player->configure();
//  black_player->configure();
  
  bool checkmate=false;
  int winner=WHITE;
  while(!checkmate)
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
    
    if(black_legal_moves==0)
    {
      //checkmate is when someone is in check and has no legal moves
      if(board->get_check(BLACK))
      {
        printf("CHECKMATE\n");
        checkmate=true;
        winner=WHITE;
      }
      else
      {
        printf("STALEMATE\n");
        checkmate=true;
        winner=-1;
      }
    }
    
    if(!checkmate)
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
      
      if(white_legal_moves==0)
      {
        //checkmate is when someone is in check and has no legal moves
        if(board->get_check(WHITE))
        {
          printf("CHECKMATE\n");
          checkmate=true;
          winner=BLACK;
        }
        else
        {
          printf("STALEMATE\n");
          checkmate=true;
          winner=-1;
        }
      }
    }
  }
  printf("Game over, winner was %s\n",(winner==WHITE)? "White" : (winner==BLACK)? "Black" : "<stalemate>");
  
  delete board;
  
  white_player->end();
  delete white_player;
  
  black_player->end();
  delete black_player;
  
  return 0;
}

