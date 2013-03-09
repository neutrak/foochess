#include "getters.h"

DLLEXPORT int moveGetId(_Move* ptr)
{
  return ptr->id;
}
DLLEXPORT int moveGetFromFile(_Move* ptr)
{
  return ptr->fromFile;
}
DLLEXPORT int moveGetFromRank(_Move* ptr)
{
  return ptr->fromRank;
}
DLLEXPORT int moveGetToFile(_Move* ptr)
{
  return ptr->toFile;
}
DLLEXPORT int moveGetToRank(_Move* ptr)
{
  return ptr->toRank;
}
DLLEXPORT int moveGetPromoteType(_Move* ptr)
{
  return ptr->promoteType;
}
DLLEXPORT int pieceGetId(_Piece* ptr)
{
  return ptr->id;
}
DLLEXPORT int pieceGetOwner(_Piece* ptr)
{
  return ptr->owner;
}
DLLEXPORT int pieceGetFile(_Piece* ptr)
{
  return ptr->file;
}
DLLEXPORT int pieceGetRank(_Piece* ptr)
{
  return ptr->rank;
}
DLLEXPORT int pieceGetHasMoved(_Piece* ptr)
{
  return ptr->hasMoved;
}
DLLEXPORT int pieceGetType(_Piece* ptr)
{
  return ptr->type;
}
DLLEXPORT int playerGetId(_Player* ptr)
{
  return ptr->id;
}
DLLEXPORT char* playerGetPlayerName(_Player* ptr)
{
  return ptr->playerName;
}
DLLEXPORT float playerGetTime(_Player* ptr)
{
  return ptr->time;
}

