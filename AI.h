#ifndef AI_H
#define AI_H

#include <iostream>
#include <cstdlib>
#include <string.h>
#include <unistd.h>
#include <poll.h>
#include "Board.h"

#define BUFFER_SIZE 1024

using namespace std;

///The class implementing gameplay logic.
class AI
{
private:
  //a master copy of the board that persists between moves
  Board *master;
  //the algorithm to use
  algorithm algo;
  
  //a move history
  vector <_Move*> moves;
  
  //treesearch settings
  int max_depth;
  int qs_depth;
  bool ab_prune;
  
  //the history table this AI is using (NULL for none)
  HistTable *hist;
  //number of moves before history gets cleared out (0 for never clear)
  int history_reset;
  
  //whether or not to use the entropy (branching-factor) heuristic
  bool entropy_heuristic;
  //whether or not to use the alternate entropy heuristic (sum of manhatten distances of all moves)
  //NOTE: THIS ONLY APPLIES WHEN ENTROPY_HEURISTIC IS TRUE
  bool distance_sum;
  
  //heurstic stuff
  bool heur_pawn_additions;
  bool heur_position_additions;
  
  //weight for opponent pieces and our pieces, respecitvely
  double enemy_weight;
  double owned_weight;
  
  //whether or not to time-limit
  bool time_limit;
  //how long the time should be if it is limited (in seconds)
  double timeout;
  
  //(history table is NULL for no history table, so doesn't need to be a seperate setting)
  
  //how many nodes to keep after forward-pruning (0 for no forward pruning)
  unsigned int beam_width;
  
public:
  AI();
  ~AI();
  
  void init();
  
  //set the algorithm from outside the class
  void set_algo(algorithm a){ algo=a; }
  
  //display current values of all treesearch settings
  void output_ts_settings();
  
  //set an option based on user input
  void set_ts_option(char *variable, char *value, int buffer_size);
  
  //convert a string to a boolean
  bool string_to_bool(char *string, int buffer_size);
  
  //allow the user to change all the settings of this AI object
  void configure(int player_id);
  
  //time-limited input (timeout is in seconds)
  //returns true when input is recieved, false otherwise
  bool tl_input(char *buffer, int buffer_size, int timeout);
  
  //store this move in the movement history
  void remember_move(_Move *m);
  //access remembered moves
  vector <_Move*> get_moves() { return moves; }
  
  bool handle_load_save(const char *input_buffer, Board *board);
  void user_input(char *input_buffer);
  _Move *user_move(Board *board, int player_id);
  
  //make a move depending on the algorithm in use and the time left
  _Move *ai_move(Board *board, int player_id, double time_remaining, double enemy_time_remaining);
  
  bool run(Board *board, int player_id);
  
  void end();
};

#endif
