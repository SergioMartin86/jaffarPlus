#include "gameRule.hpp"

GameRule::GameRule() : Rule()
{
}

bool GameRule::parseGameAction(nlohmann::json actionJs, size_t actionId)
{
  bool recognizedActionType = false;
  std::string actionType = actionJs["Type"].get<std::string>();

  if (actionType == "Set Player Current Lap Magnet")
  {
   if (isDefined(actionJs, "Intensity") == false) EXIT_WITH_ERROR("[ERROR] Magnet in Rule %lu Action %lu missing 'Intensity' key.\n", _label, actionId);
   _magnets.playerCurrentLapMagnet = actionJs["Intensity"].get<float>();
   recognizedActionType = true;
  }

  if (actionType == "Set Player Lap Progress Magnet")
  {
   if (isDefined(actionJs, "Intensity") == false) EXIT_WITH_ERROR("[ERROR] Magnet in Rule %lu Action %lu missing 'Intensity' key.\n", _label, actionId);
   _magnets.playerLapProgressMagnet = actionJs["Intensity"].get<float>();
   recognizedActionType = true;
  }

  if (actionType == "Set Player Turbo Counter Magnet")
  {
   if (isDefined(actionJs, "Intensity") == false) EXIT_WITH_ERROR("[ERROR] Magnet in Rule %lu Action %lu missing 'Intensity' key.\n", _label, actionId);
   _magnets.playerTurboCounterMagnet = actionJs["Intensity"].get<float>();
   recognizedActionType = true;
  }

  if (actionType == "Set Player Accel Magnet")
  {
   if (isDefined(actionJs, "Intensity") == false) EXIT_WITH_ERROR("[ERROR] Magnet in Rule %lu Action %lu missing 'Intensity' key.\n", _label, actionId);
   _magnets.playerAccelMagnet = actionJs["Intensity"].get<float>();
   recognizedActionType = true;
  }

  if (actionType == "Set Player Cash Magnet")
  {
   if (isDefined(actionJs, "Intensity") == false) EXIT_WITH_ERROR("[ERROR] Magnet in Rule %lu Action %lu missing 'Intensity' key.\n", _label, actionId);
   _magnets.playerCashMagnet = actionJs["Intensity"].get<float>();
   recognizedActionType = true;
  }


  return recognizedActionType;
}

datatype_t GameRule::getPropertyType(const nlohmann::json& condition)
{
  std::string propertyName = condition["Property"].get<std::string>();

  if (propertyName == "Pre Race Timer") return dt_uint8;
  if (propertyName == "Game Timer") return dt_uint8;
  if (propertyName == "Menu State 1") return dt_uint8;
  if (propertyName == "Menu State 2") return dt_uint8;
  if (propertyName == "Menu State 3") return dt_uint8;

  if (propertyName == "Player 1 Lap Counter") return dt_uint8;
  if (propertyName == "Track Type") return dt_uint8;
  if (propertyName == "Player 1 Lap Progress") return dt_uint8;
  if (propertyName == "Player 1 Pos X") return dt_uint8;
  if (propertyName == "Player 1 Pos Y") return dt_uint8;
  if (propertyName == "Player 1 Turbo Counter") return dt_uint8;
  if (propertyName == "Player 1 Cash 0") return dt_uint8;
  if (propertyName == "Player 1 Cash 1") return dt_uint8;
  if (propertyName == "Player 1 Cash 2") return dt_uint8;
  if (propertyName == "Player 1 Cash 3") return dt_uint8;
  if (propertyName == "Menu Race Start Timer") return dt_uint8;
  if (propertyName == "Menu Race State") return dt_uint8;

  EXIT_WITH_ERROR("[Error] Rule %lu, unrecognized property: %s\n", _label, propertyName.c_str());

  return dt_uint8;
}

void* GameRule::getPropertyPointer(const nlohmann::json& condition, GameInstance* gameInstance)
{
  std::string propertyName = condition["Property"].get<std::string>();

  if (propertyName == "Pre Race Timer") return gameInstance->preRaceTimer;
  if (propertyName == "Game Timer") return gameInstance->gameTimer;
  if (propertyName == "Menu State 1") return gameInstance->menuState1;
  if (propertyName == "Menu State 2") return gameInstance->menuState2;
  if (propertyName == "Menu State 3") return gameInstance->menuState3;
  if (propertyName == "Player 1 Turbo Counter") return gameInstance->p1TurboCounter;

  if (propertyName == "Player 1 Cash 0") return gameInstance->p1Cash0;
  if (propertyName == "Player 1 Cash 1") return gameInstance->p1Cash1;
  if (propertyName == "Player 1 Cash 2") return gameInstance->p1Cash2;
  if (propertyName == "Player 1 Cash 3") return gameInstance->p1Cash3;

  if (propertyName == "Player 1 Lap Counter") return gameInstance->p1CurrentLap;
  if (propertyName == "Track Type") return gameInstance->trackType;
  if (propertyName == "Player 1 Lap Progress") return gameInstance->p1LapProgress;
  if (propertyName == "Player 1 Pos X") return gameInstance->p1PosX;
  if (propertyName == "Player 1 Pos Y") return gameInstance->p1PosY;
  if (propertyName == "Menu Race Start Timer") return gameInstance->menuRaceStartTimer;
  if (propertyName == "Menu Race State") return gameInstance->menuRaceState;

  EXIT_WITH_ERROR("[Error] Rule %lu, unrecognized property: %s\n", _label, propertyName.c_str());

  return NULL;
}
