#include "gameRule.hpp"

GameRule::GameRule() : Rule()
{
 // Rule initialization
}

bool GameRule::parseGameAction(nlohmann::json actionJs, size_t actionId)
{
  // Getting action type
   if (isDefined(actionJs, "Type") == false) EXIT_WITH_ERROR("[ERROR] Rule %lu Action %lu missing 'Type' key.\n", _label, actionId);
   std::string actionType = actionJs["Type"].get<std::string>();

  // Running the action, depending on the type
  bool recognizedActionType = false;

  if (actionType == "Add Reward")
  {
    if (isDefined(actionJs, "Value") == false) EXIT_WITH_ERROR("[ERROR] Rule %lu Action %lu missing 'Value' key.\n", _label, actionId);
    _reward = actionJs["Value"].get<float>();
    recognizedActionType = true;
  }

  // Storing fail state
  if (actionType == "Trigger Fail")
  {
    _isFailRule = true;
    recognizedActionType = true;
  }

  // Storing win state
  if (actionType == "Trigger Win")
  {
    _isWinRule = true;
    recognizedActionType = true;
  }

  if (actionType == "Set Kid Horizontal Magnet Intensity")
  {
    magnet_t newMagnet;
    if (isDefined(actionJs, "Room") == false) EXIT_WITH_ERROR("[ERROR] Rule %lu Action %lu missing 'Room' key.\n", _label, actionId);
    if (isDefined(actionJs, "Value") == false) EXIT_WITH_ERROR("[ERROR] Rule %lu Action %lu missing 'Value' key.\n", _label, actionId);
    newMagnet.value = actionJs["Value"].get<float>();
    newMagnet.room = actionJs["Room"].get<byte>();
    _kidMagnetIntensityX.push_back(newMagnet);
    recognizedActionType = true;
  }

  if (actionType == "Set Kid Horizontal Magnet Position")
  {
    magnet_t newMagnet;
    if (isDefined(actionJs, "Room") == false) EXIT_WITH_ERROR("[ERROR] Rule %lu Action %lu missing 'Room' key.\n", _label, actionId);
    if (isDefined(actionJs, "Value") == false) EXIT_WITH_ERROR("[ERROR] Rule %lu Action %lu missing 'Value' key.\n", _label, actionId);
    newMagnet.value = actionJs["Value"].get<float>();
    newMagnet.room = actionJs["Room"].get<byte>();
    _kidMagnetPositionX.push_back(newMagnet);
    recognizedActionType = true;
  }

  if (actionType == "Set Kid Vertical Magnet Intensity")
  {
    magnet_t newMagnet;
    if (isDefined(actionJs, "Room") == false) EXIT_WITH_ERROR("[ERROR] Rule %lu Action %lu missing 'Room' key.\n", _label, actionId);
    if (isDefined(actionJs, "Value") == false) EXIT_WITH_ERROR("[ERROR] Rule %lu Action %lu missing 'Value' key.\n", _label, actionId);
    newMagnet.value = actionJs["Value"].get<float>();
    newMagnet.room = actionJs["Room"].get<byte>();
    _kidMagnetIntensityY.push_back(newMagnet);
    recognizedActionType = true;
  }

  if (actionType == "Set Guard Horizontal Magnet Intensity")
  {
    magnet_t newMagnet;
    if (isDefined(actionJs, "Room") == false) EXIT_WITH_ERROR("[ERROR] Rule %lu Action %lu missing 'Room' key.\n", _label, actionId);
    if (isDefined(actionJs, "Value") == false) EXIT_WITH_ERROR("[ERROR] Rule %lu Action %lu missing 'Value' key.\n", _label, actionId);
    newMagnet.value = actionJs["Value"].get<float>();
    newMagnet.room = actionJs["Room"].get<byte>();
    _guardMagnetIntensityX.push_back(newMagnet);
    recognizedActionType = true;
  }

  if (actionType == "Set Guard Horizontal Magnet Position")
  {
    magnet_t newMagnet;
    if (isDefined(actionJs, "Room") == false) EXIT_WITH_ERROR("[ERROR] Rule %lu Action %lu missing 'Room' key.\n", _label, actionId);
    if (isDefined(actionJs, "Value") == false) EXIT_WITH_ERROR("[ERROR] Rule %lu Action %lu missing 'Value' key.\n", _label, actionId);
    newMagnet.value = actionJs["Value"].get<float>();
    newMagnet.room = actionJs["Room"].get<byte>();
    _guardMagnetPositionX.push_back(newMagnet);
    recognizedActionType = true;
  }

  if (actionType == "Set Guard Vertical Magnet Intensity")
  {
    magnet_t newMagnet;
    if (isDefined(actionJs, "Room") == false) EXIT_WITH_ERROR("[ERROR] Rule %lu Action %lu missing 'Room' key.\n", _label, actionId);
    if (isDefined(actionJs, "Value") == false) EXIT_WITH_ERROR("[ERROR] Rule %lu Action %lu missing 'Value' key.\n", _label, actionId);
    newMagnet.value = actionJs["Value"].get<float>();
    newMagnet.room = actionJs["Room"].get<byte>();
    _guardMagnetIntensityY.push_back(newMagnet);
    recognizedActionType = true;
  }

  return recognizedActionType;
}

datatype_t GameRule::getPropertyType(const nlohmann::json& condition)
{
 std::string propertyName = condition["Property"].get<std::string>();

 if (propertyName == "Kid Frame") return dt_uint8;
 if (propertyName == "Kid Current HP") return dt_int16;
 if (propertyName == "Kid Max HP") return dt_int16;
 if (propertyName == "Kid Position X") return dt_uint8;
 if (propertyName == "Kid Position Y") return dt_uint8;
 if (propertyName == "Kid Direction") return dt_int8;
 if (propertyName == "Kid Current Column") return dt_int8;
 if (propertyName == "Kid Current Row") return dt_int8;
 if (propertyName == "Kid Action") return dt_uint8;
 if (propertyName == "Kid Fall Velocity X") return dt_int8;
 if (propertyName == "Kid Fall Velocity Y") return dt_int8;
 if (propertyName == "Kid Room") return dt_uint8;
 if (propertyName == "Kid Repeat") return dt_uint8;
 if (propertyName == "Kid Character Id") return dt_uint8;
 if (propertyName == "Kid Hold Sword") return dt_uint8;
 if (propertyName == "Kid Has Sword") return dt_int16;
 if (propertyName == "Kid Is Alive") return dt_int8;
 if (propertyName == "Kid Current Sequence") return dt_int16;

 if (propertyName == "Guard Frame") return dt_uint8;
 if (propertyName == "Guard Current HP") return dt_int16;
 if (propertyName == "Guard Max HP") return dt_int16;
 if (propertyName == "Guard Position X") return dt_uint8;
 if (propertyName == "Guard Position Y") return dt_uint8;
 if (propertyName == "Guard Direction") return dt_int8;
 if (propertyName == "Guard Current Column") return dt_int8;
 if (propertyName == "Guard Current Row") return dt_int8;
 if (propertyName == "Guard Action") return dt_uint8;
 if (propertyName == "Guard Fall Velocity X") return dt_int8;
 if (propertyName == "Guard Fall Velocity Y") return dt_int8;
 if (propertyName == "Guard Room") return dt_uint8;
 if (propertyName == "Guard Repeat") return dt_uint8;
 if (propertyName == "Guard Character Id") return dt_uint8;
 if (propertyName == "Guard Has Sword") return dt_uint8;
 if (propertyName == "Guard Is Alive") return dt_int8;
 if (propertyName == "Guard Current Sequence") return dt_int16;

 if (propertyName == "Current Level") return dt_int16;
 if (propertyName == "Next Level") return dt_int16;
 if (propertyName == "Drawn Room") return dt_int16;
 if (propertyName == "Checkpoint Reached") return dt_int16;
 if (propertyName == "Needs Level 1 Music") return dt_int16;
 if (propertyName == "United With Shadow") return dt_int8;
 if (propertyName == "Exit Door Timer") return dt_int16;
 if (propertyName == "Is Feather Fall") return dt_int16;

 // Tile Configuration
 if (propertyName == "Tile FG State") return dt_uint8;
 if (propertyName == "Tile BG State") return dt_uint8;

 EXIT_WITH_ERROR("[Error] Rule %lu, unrecognized property: %s\n", _label, propertyName.c_str());

 return dt_uint8;
}

void* GameRule::getPropertyPointer(const nlohmann::json& condition, GameInstance* gameInstance)
{
 std::string propertyName = condition["Property"].get<std::string>();

 if (propertyName == "Kid Frame") return &gameState.Kid.frame;
 if (propertyName == "Kid Current HP") return &gameState.hitp_curr;
 if (propertyName == "Kid Max HP") return &gameState.hitp_max;
 if (propertyName == "Kid Position X") return &gameState.Kid.x;
 if (propertyName == "Kid Position Y") return &gameState.Kid.y;
 if (propertyName == "Kid Direction") return &gameState.Kid.direction;
 if (propertyName == "Kid Current Column") return &gameState.Kid.curr_col;
 if (propertyName == "Kid Current Row") return &gameState.Kid.curr_row;
 if (propertyName == "Kid Action") return &gameState.Kid.action;
 if (propertyName == "Kid Fall Velocity X") return &gameState.Kid.fall_x;
 if (propertyName == "Kid Fall Velocity Y") return &gameState.Kid.fall_y;
 if (propertyName == "Kid Room") return &gameState.Kid.room;
 if (propertyName == "Kid Repeat") return &gameState.Kid.repeat;
 if (propertyName == "Kid Character Id") return &gameState.Kid.charid;
 if (propertyName == "Kid Holds Sword") return &gameState.Kid.sword;
 if (propertyName == "Kid Has Sword") return &gameState.have_sword;
 if (propertyName == "Kid Is Alive") return &gameState.Kid.alive;
 if (propertyName == "Kid Current Sequence") return &gameState.Kid.curr_seq;

 if (propertyName == "Guard Frame") return &gameState.Guard.frame;
 if (propertyName == "Guard Current HP") return &gameState.guardhp_curr;
 if (propertyName == "Guard Max HP") return &gameState.guardhp_max;
 if (propertyName == "Guard Position X") return &gameState.Guard.x;
 if (propertyName == "Guard Position Y") return &gameState.Guard.y;
 if (propertyName == "Guard Direction") return &gameState.Guard.direction;
 if (propertyName == "Guard Current Column") return &gameState.Guard.curr_col;
 if (propertyName == "Guard Current Row") return &gameState.Guard.curr_row;
 if (propertyName == "Guard Action") return &gameState.Guard.action;
 if (propertyName == "Guard Fall Velocity X") return &gameState.Guard.fall_x;
 if (propertyName == "Guard Fall Velocity Y") return &gameState.Guard.fall_y;
 if (propertyName == "Guard Room") return &gameState.Guard.room;
 if (propertyName == "Guard Repeat") return &gameState.Guard.repeat;
 if (propertyName == "Guard Character Id") return &gameState.Guard.charid;
 if (propertyName == "Guard Has Sword") return &gameState.Guard.sword;
 if (propertyName == "Guard Is Alive") return &gameState.Guard.alive;
 if (propertyName == "Guard Current Sequence") return &gameState.Guard.curr_seq;

 if (propertyName == "Current Level") return &gameState.current_level;
 if (propertyName == "Next Level") return &gameState.next_level;
 if (propertyName == "Drawn Room") return &gameState.drawn_room;
 if (propertyName == "Checkpoint Reached") return &gameState.checkpoint;
 if (propertyName == "Is Feather Fall") return &gameState.is_feather_fall;
 if (propertyName == "Needs Level 1 Music") return &gameState.need_level1_music;
 if (propertyName == "United With Shadow") return &gameState.united_with_shadow;
 if (propertyName == "Exit Door Timer") return &gameState.leveldoor_open;

 int room = -1;
 if (isDefined(condition, "Room") == true)
 {
  if (condition["Room"].is_number() == false) EXIT_WITH_ERROR("[ERROR] Rule %lu tile room must be an integer.\n", _label);
  room = condition["Room"].get<int>();
 }

 int tile = -1;
 if (isDefined(condition, "Tile") == true)
 {
  if (condition["Tile"].is_number() == false) EXIT_WITH_ERROR("[ERROR] Rule %lu tile index must be an integer.\n", _label);
  tile = condition["Tile"].get<int>();
 }

 int index = (room-1) * 30 + (tile-1);

 if (propertyName == "Tile FG State") { if (index == -1) EXIT_WITH_ERROR("[ERROR] Invalid or missing index for %s.\n", propertyName.c_str()); return &gameState.level.fg[index]; }
 if (propertyName == "Tile BG State") { if (index == -1) EXIT_WITH_ERROR("[ERROR] Invalid or missing index for %s.\n", propertyName.c_str()); return &gameState.level.bg[index]; }

 EXIT_WITH_ERROR("[Error] Rule %lu, unrecognized property: %s\n", _label, propertyName.c_str());

 return NULL;
}
