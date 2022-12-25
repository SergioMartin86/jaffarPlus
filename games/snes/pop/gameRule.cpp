#include "gameRule.hpp"

GameRule::GameRule() : Rule()
{
 for(size_t i = 0; i < ROOM_COUNT; i++)
 {
  _magnets[i].kidHorizontalMagnet = genericMagnet_t { .intensity = 0.0f, .center = 0.0f, .min = 0.0f, .max = 0.0f, .active = false };
  _magnets[i].kidVerticalMagnet = genericMagnet_t { .intensity = 0.0f, .center = 0.0f, .min = 0.0f, .max = 0.0f, .active = false };
  _magnets[i].guardHorizontalMagnet = genericMagnet_t { .intensity = 0.0f, .center = 0.0f, .min = 0.0f, .max = 0.0f, .active = false };
  _magnets[i].guardVerticalMagnet = genericMagnet_t { .intensity = 0.0f, .center = 0.0f, .min = 0.0f, .max = 0.0f, .active = false };
 }
}

bool GameRule::parseGameAction(nlohmann::json actionJs, size_t actionId)
{
  bool recognizedActionType = false;
  std::string actionType = actionJs["Type"].get<std::string>();

  if (actionType == "Set Kid Horizontal Magnet")
  {
   if (isDefined(actionJs, "Intensity") == false) EXIT_WITH_ERROR("[ERROR] Magnet in Rule %lu Action %lu missing 'Intensity' key.\n", _label, actionId);
   if (isDefined(actionJs, "Room") == false) EXIT_WITH_ERROR("[ERROR] Magnet in Rule %lu Action %lu missing 'Room' key.\n", _label, actionId);
   if (isDefined(actionJs, "Center") == false) EXIT_WITH_ERROR("[ERROR] Magnet in Rule %lu Action %lu missing 'Center' key.\n", _label, actionId);
   if (isDefined(actionJs, "Min") == false) EXIT_WITH_ERROR("[ERROR] Magnet in Rule %lu Action %lu missing 'Min' key.\n", _label, actionId);
   if (isDefined(actionJs, "Max") == false) EXIT_WITH_ERROR("[ERROR] Magnet in Rule %lu Action %lu missing 'Max' key.\n", _label, actionId);
   uint8_t room = actionJs["Room"].get<uint8_t>();
   _magnets[room].kidHorizontalMagnet = genericMagnet_t { .intensity = actionJs["Intensity"].get<float>(), .center= actionJs["Center"].get<float>(), .min = actionJs["Min"].get<float>(), .max = actionJs["Max"].get<float>(), .active = true };
    recognizedActionType = true;
  }

  if (actionType == "Set Kid Vertical Magnet")
  {
   if (isDefined(actionJs, "Intensity") == false) EXIT_WITH_ERROR("[ERROR] Magnet in Rule %lu Action %lu missing 'Intensity' key.\n", _label, actionId);
   if (isDefined(actionJs, "Room") == false) EXIT_WITH_ERROR("[ERROR] Magnet in Rule %lu Action %lu missing 'Room' key.\n", _label, actionId);
   if (isDefined(actionJs, "Center") == false) EXIT_WITH_ERROR("[ERROR] Magnet in Rule %lu Action %lu missing 'Center' key.\n", _label, actionId);
   if (isDefined(actionJs, "Min") == false) EXIT_WITH_ERROR("[ERROR] Magnet in Rule %lu Action %lu missing 'Min' key.\n", _label, actionId);
   if (isDefined(actionJs, "Max") == false) EXIT_WITH_ERROR("[ERROR] Magnet in Rule %lu Action %lu missing 'Max' key.\n", _label, actionId);
   uint8_t room = actionJs["Room"].get<uint8_t>();
   _magnets[room].kidVerticalMagnet = genericMagnet_t { .intensity = actionJs["Intensity"].get<float>(), .center= actionJs["Center"].get<float>(), .min = actionJs["Min"].get<float>(), .max = actionJs["Max"].get<float>(), .active = true };
   recognizedActionType = true;
  }

  if (actionType == "Set Guard Horizontal Magnet")
  {
   if (isDefined(actionJs, "Intensity") == false) EXIT_WITH_ERROR("[ERROR] Magnet in Rule %lu Action %lu missing 'Intensity' key.\n", _label, actionId);
   if (isDefined(actionJs, "Room") == false) EXIT_WITH_ERROR("[ERROR] Magnet in Rule %lu Action %lu missing 'Room' key.\n", _label, actionId);
   if (isDefined(actionJs, "Center") == false) EXIT_WITH_ERROR("[ERROR] Magnet in Rule %lu Action %lu missing 'Center' key.\n", _label, actionId);
   if (isDefined(actionJs, "Min") == false) EXIT_WITH_ERROR("[ERROR] Magnet in Rule %lu Action %lu missing 'Min' key.\n", _label, actionId);
   if (isDefined(actionJs, "Max") == false) EXIT_WITH_ERROR("[ERROR] Magnet in Rule %lu Action %lu missing 'Max' key.\n", _label, actionId);
   uint8_t room = actionJs["Room"].get<uint8_t>();
   _magnets[room].guardHorizontalMagnet = genericMagnet_t { .intensity = actionJs["Intensity"].get<float>(), .center= actionJs["Center"].get<float>(), .min = actionJs["Min"].get<float>(), .max = actionJs["Max"].get<float>(), .active = true };
    recognizedActionType = true;
  }

  if (actionType == "Set Guard Vertical Magnet")
  {
   if (isDefined(actionJs, "Intensity") == false) EXIT_WITH_ERROR("[ERROR] Magnet in Rule %lu Action %lu missing 'Intensity' key.\n", _label, actionId);
   if (isDefined(actionJs, "Room") == false) EXIT_WITH_ERROR("[ERROR] Magnet in Rule %lu Action %lu missing 'Room' key.\n", _label, actionId);
   if (isDefined(actionJs, "Center") == false) EXIT_WITH_ERROR("[ERROR] Magnet in Rule %lu Action %lu missing 'Center' key.\n", _label, actionId);
   if (isDefined(actionJs, "Min") == false) EXIT_WITH_ERROR("[ERROR] Magnet in Rule %lu Action %lu missing 'Min' key.\n", _label, actionId);
   if (isDefined(actionJs, "Max") == false) EXIT_WITH_ERROR("[ERROR] Magnet in Rule %lu Action %lu missing 'Max' key.\n", _label, actionId);
   uint8_t room = actionJs["Room"].get<uint8_t>();
   _magnets[room].guardVerticalMagnet = genericMagnet_t { .intensity = actionJs["Intensity"].get<float>(), .center= actionJs["Center"].get<float>(), .min = actionJs["Min"].get<float>(), .max = actionJs["Max"].get<float>(), .active = true };
   recognizedActionType = true;
  }

  if (actionType == "Set Kid Direction Magnet")
  {
   if (isDefined(actionJs, "Intensity") == false) EXIT_WITH_ERROR("[ERROR] Magnet in Rule %lu Action %lu missing 'Intensity' key.\n", _label, actionId);
   if (isDefined(actionJs, "Room") == false) EXIT_WITH_ERROR("[ERROR] Magnet in Rule %lu Action %lu missing 'Room' key.\n", _label, actionId);
   uint8_t room = actionJs["Room"].get<uint8_t>();
   _magnets[room].kidDirectionMagnet = actionJs["Intensity"].get<float>();
   recognizedActionType = true;
  }

  if (actionType == "Set Kid HP Magnet")
  {
   if (isDefined(actionJs, "Intensity") == false) EXIT_WITH_ERROR("[ERROR] Magnet in Rule %lu Action %lu missing 'Intensity' key.\n", _label, actionId);
   if (isDefined(actionJs, "Room") == false) EXIT_WITH_ERROR("[ERROR] Magnet in Rule %lu Action %lu missing 'Room' key.\n", _label, actionId);
   uint8_t room = actionJs["Room"].get<uint8_t>();
   _magnets[room].kidHPMagnet = actionJs["Intensity"].get<float>();
   recognizedActionType = true;
  }

  if (actionType == "Set Guard HP Magnet")
  {
   if (isDefined(actionJs, "Intensity") == false) EXIT_WITH_ERROR("[ERROR] Magnet in Rule %lu Action %lu missing 'Intensity' key.\n", _label, actionId);
   if (isDefined(actionJs, "Room") == false) EXIT_WITH_ERROR("[ERROR] Magnet in Rule %lu Action %lu missing 'Room' key.\n", _label, actionId);
   uint8_t room = actionJs["Room"].get<uint8_t>();
   _magnets[room].guardHPMagnet = actionJs["Intensity"].get<float>();
   recognizedActionType = true;
  }

  if (actionType == "Set Kid Guard Distance Magnet")
  {
   if (isDefined(actionJs, "Intensity") == false) EXIT_WITH_ERROR("[ERROR] Magnet in Rule %lu Action %lu missing 'Intensity' key.\n", _label, actionId);
   if (isDefined(actionJs, "Room") == false) EXIT_WITH_ERROR("[ERROR] Magnet in Rule %lu Action %lu missing 'Room' key.\n", _label, actionId);
   uint8_t room = actionJs["Room"].get<uint8_t>();
   _magnets[room].kidGuardDistanceMagnet = actionJs["Intensity"].get<float>();
   recognizedActionType = true;
  }

  if (actionType == "Set Custom Value")
  {
   if (isDefined(actionJs, "Value") == false) EXIT_WITH_ERROR("[ERROR] Magnet in Rule %lu Action %lu missing 'Value' key.\n", _label, actionId);
   _customValue = actionJs["Value"].get<uint8_t>();
   _customValueActive = true;
   recognizedActionType = true;
  }

  return recognizedActionType;
}

datatype_t GameRule::getPropertyType(const nlohmann::json& condition)
{
  std::string propertyName = condition["Property"].get<std::string>();

  if (propertyName == "Game Frame") return dt_uint8;
  if (propertyName == "Kid Room") return dt_uint8;
  if (propertyName == "Kid Pos X") return dt_uint8;
  if (propertyName == "Kid Pos Y") return dt_uint8;
  if (propertyName == "Kid Fight Mode") return dt_uint8;
  if (propertyName == "Kid Direction") return dt_uint8;
  if (propertyName == "Kid HP") return dt_uint8;
  if (propertyName == "Kid Frame") return dt_uint8;
  if (propertyName == "Jingle State") return dt_uint8;
  if (propertyName == "Tile State") return dt_uint8;
  if (propertyName == "Tile Type") return dt_uint8;
  if (propertyName == "Exit Door State") return dt_uint8;
  if (propertyName == "Kid Teleporting") return dt_uint8;
  if (propertyName == "Kid Col") return dt_uint8;
  if (propertyName == "Boss Sequence") return dt_uint8;

  if (propertyName == "Guard Room") return dt_uint8;
  if (propertyName == "Guard Pos X") return dt_uint8;
  if (propertyName == "Guard Pos Y") return dt_uint8;
  if (propertyName == "Guard HP") return dt_uint8;
  if (propertyName == "Custom Value") return dt_uint8;
  if (propertyName == "Lag Counter") return dt_uint16;

  if (propertyName == "Exit Level Mode") return dt_uint8;

  EXIT_WITH_ERROR("[Error] Rule %lu, unrecognized property: %s\n", _label, propertyName.c_str());

  return dt_uint8;
}

void* GameRule::getPropertyPointer(const nlohmann::json& condition, GameInstance* gameInstance)
{
  std::string propertyName = condition["Property"].get<std::string>();

  if (propertyName == "Game Frame") return gameInstance->gameFrame;
  if (propertyName == "Kid Room") return gameInstance->kidRoom;
  if (propertyName == "Kid Fight Mode") return gameInstance->kidFightMode;
  if (propertyName == "Kid Pos X") return gameInstance->kidPosX;
  if (propertyName == "Kid Pos Y") return gameInstance->kidPosY;
  if (propertyName == "Kid Direction") return gameInstance->kidDirection;
  if (propertyName == "Kid HP") return gameInstance->kidHP;
  if (propertyName == "Kid Frame") return gameInstance->kidFrame;
  if (propertyName == "Jingle State") return gameInstance->jingleState;
  if (propertyName == "Kid Teleporting") return gameInstance->kidTeleporting;
  if (propertyName == "Kid Col") return gameInstance->kidCol;
  if (propertyName == "Boss Sequence") return gameInstance->bossSequence;

  if (propertyName == "Guard Room") return gameInstance->guardRoom;
  if (propertyName == "Guard Pos X") return gameInstance->guardPosX;
  if (propertyName == "Guard Pos Y") return gameInstance->guardPosY;
  if (propertyName == "Guard HP") return gameInstance->guardHP;

  if (propertyName == "Custom Value") return gameInstance->customValue;
  if (propertyName == "Lag Counter") return gameInstance->lagFrameCounter;

  if (propertyName == "Exit Door State") return gameInstance->exitDoorState;
  if (propertyName == "Exit Level Mode") return gameInstance->exitLevelMode;


  int room = -1;
  if (isDefined(condition, "Room") == true)
  {
   if (condition["Room"].is_number() == false) EXIT_WITH_ERROR("[ERROR] Rule %lu tile room must be an integer.\n", _label);
   room = condition["Room"].get<int>();
  }

  int row = -1;
  if (isDefined(condition, "Row") == true)
  {
   if (condition["Row"].is_number() == false) EXIT_WITH_ERROR("[ERROR] Rule %lu row must be an integer.\n", _label);
   row = condition["Row"].get<int>();
  }

  int col = -1;
  if (isDefined(condition, "Col") == true)
  {
   if (condition["Col"].is_number() == false) EXIT_WITH_ERROR("[ERROR] Rule %lu col must be an integer.\n", _label);
   col = condition["Col"].get<int>();
  }

  int index = room * 30 + row * 10 + col;
  gameInstance->tileWatchList[index] = tileWatch_t {.room = room, .row = row, .col = col, .index = index};

  if (propertyName == "Tile State") return &gameInstance->tileStateBase[index];
  if (propertyName == "Tile Type") return &gameInstance->tileTypeBase[index];

  EXIT_WITH_ERROR("[Error] Rule %lu, unrecognized property: %s\n", _label, propertyName.c_str());

  return NULL;
}
