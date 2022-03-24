#include "gameRule.hpp"

GameRule::GameRule() : Rule()
{
 for(size_t i = 0; i < ROOM_COUNT; i++)
 {
  _magnets[i].kidHorizontalMagnet = genericMagnet_t { .intensity = 0.0f, .center = 0.0f, .min = 0.0f, .max = 0.0f, .active = false };
  _magnets[i].kidVerticalMagnet = genericMagnet_t { .intensity = 0.0f, .center = 0.0f, .min = 0.0f, .max = 0.0f, .active = false };
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

  return recognizedActionType;
}

datatype_t GameRule::getPropertyType(const nlohmann::json& condition)
{
  std::string propertyName = condition["Property"].get<std::string>();

  if (propertyName == "Game State") return dt_uint8;
  if (propertyName == "Is Correct Render") return dt_uint8;
  if (propertyName == "Is Paused") return dt_uint8;
  if (propertyName == "Frame Phase") return dt_uint8;
  if (propertyName == "Current Level") return dt_uint8;
  if (propertyName == "Kid Position X") return dt_int16;
  if (propertyName == "Kid Position Y") return dt_uint8;
  if (propertyName == "Kid Frame") return dt_uint8;
  if (propertyName == "Kid Movement") return dt_uint8;
  if (propertyName == "Kid HP") return dt_uint8;
  if (propertyName == "Kid Room") return dt_uint8;
  if (propertyName == "Kid Fight Mode") return dt_uint8;
  if (propertyName == "Drawn Room") return dt_uint8;
  if (propertyName == "Guard Pos X") return dt_uint8;
  if (propertyName == "Guard HP") return dt_uint8;
  if (propertyName == "Exit Door State") return dt_uint8;
  if (propertyName == "Level 2 Exit Door State") return dt_uint8;
  if (propertyName == "Level 3 Checkpoint Gate Timer") return dt_uint8;
  if (propertyName == "Level 3 Exit Door State") return dt_uint8;
  if (propertyName == "Level 4 Exit Door State") return dt_uint8;
  if (propertyName == "Door Opening Timer") return dt_uint8;
  if (propertyName == "Level 5 Gate Timer") return dt_uint8;

  EXIT_WITH_ERROR("[Error] Rule %lu, unrecognized property: %s\n", _label, propertyName.c_str());

  return dt_uint8;
}

void* GameRule::getPropertyPointer(const nlohmann::json& condition, GameInstance* gameInstance)
{
  std::string propertyName = condition["Property"].get<std::string>();

  if (propertyName == "Game State") return gameInstance->gameState;
  if (propertyName == "Is Correct Render") return &gameInstance->isCorrectRender;
  if (propertyName == "Is Paused") return gameInstance->isPaused;
  if (propertyName == "Frame Phase") return gameInstance->framePhase;
  if (propertyName == "Current Level") return gameInstance->currentLevel;
  if (propertyName == "Kid Position X") return gameInstance->kidPosX;
  if (propertyName == "Kid Position Y") return gameInstance->kidPosY;
  if (propertyName == "Kid Frame") return gameInstance->kidFrame;
  if (propertyName == "Kid Movement") return gameInstance->kidMovement;
  if (propertyName == "Kid HP") return gameInstance->kidHP;
  if (propertyName == "Kid Room") return gameInstance->kidRoom;
  if (propertyName == "Kid Fight Mode") return gameInstance->kidFightMode;
  if (propertyName == "Drawn Room") return gameInstance->drawnRoom;
  if (propertyName == "Guard Pos X") return gameInstance->guardPosX;
  if (propertyName == "Guard HP") return gameInstance->guardHP;
  if (propertyName == "Exit Door State") return gameInstance->exitDoorState;
  if (propertyName == "Level 2 Exit Door State") return gameInstance->lvl2ExitDoorState;
  if (propertyName == "Level 3 Checkpoint Gate Timer") return gameInstance->lvl3PreCheckpointGateTimer;
  if (propertyName == "Level 3 Exit Door State") return gameInstance->lvl3ExitDoorState;
  if (propertyName == "Level 4 Exit Door State") return gameInstance->lvl4ExitDoorState;
  if (propertyName == "Door Opening Timer") return gameInstance->doorOpeningTimer;
  if (propertyName == "Level 5 Gate Timer") return gameInstance->lvl5GateTimer;


  EXIT_WITH_ERROR("[Error] Rule %lu, unrecognized property: %s\n", _label, propertyName.c_str());

  return NULL;
}
