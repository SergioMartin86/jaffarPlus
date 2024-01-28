#include "gameRule.hpp"

GameRule::GameRule() : Rule()
{
}

bool GameRule::parseGameAction(nlohmann::json actionJs, size_t actionId)
{
  bool recognizedActionType = false;
  std::string actionType = actionJs["Type"].get<std::string>();

  if (actionType == "Set Point Magnet")
  {
   if (isDefined(actionJs, "Intensity") == false) EXIT_WITH_ERROR("[ERROR] Magnet in Rule %lu Action %lu missing 'Intensity' key.\n", _label, actionId);
   if (isDefined(actionJs, "X") == false) EXIT_WITH_ERROR("[ERROR] Magnet in Rule %lu Action %lu missing 'X' key.\n", _label, actionId);
   if (isDefined(actionJs, "Y") == false) EXIT_WITH_ERROR("[ERROR] Magnet in Rule %lu Action %lu missing 'Y' key.\n", _label, actionId);
   _magnets.pointMagnet = pointMagnet_t { .intensity = actionJs["Intensity"].get<float>(), .x = actionJs["X"].get<float>(), .y = actionJs["Y"].get<float>() };
    recognizedActionType = true;
  }

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

  if (actionType == "Set Player Accel Magnet")
  {
   if (isDefined(actionJs, "Intensity") == false) EXIT_WITH_ERROR("[ERROR] Magnet in Rule %lu Action %lu missing 'Intensity' key.\n", _label, actionId);
   _magnets.playerAccelMagnet = actionJs["Intensity"].get<float>();
   recognizedActionType = true;
  }

  if (actionType == "Set Camera Distance Magnet")
  {
   if (isDefined(actionJs, "Intensity") == false) EXIT_WITH_ERROR("[ERROR] Magnet in Rule %lu Action %lu missing 'Intensity' key.\n", _label, actionId);
   _magnets.cameraDistanceMagnet = actionJs["Intensity"].get<float>();
   recognizedActionType = true;
  }

  if (actionType == "Set Recovery Timer Magnet")
  {
   if (isDefined(actionJs, "Intensity") == false) EXIT_WITH_ERROR("[ERROR] Magnet in Rule %lu Action %lu missing 'Intensity' key.\n", _label, actionId);
   _magnets.recoveryTimerMagnet = actionJs["Intensity"].get<float>();
   recognizedActionType = true;
  }

  if (actionType == "Set Car 1 Angle Magnet")
  {
   if (isDefined(actionJs, "Intensity") == false) EXIT_WITH_ERROR("[ERROR] Magnet in Rule %lu Action %lu missing 'Intensity' key.\n", _label, actionId);
   if (isDefined(actionJs, "Angle") == false) EXIT_WITH_ERROR("[ERROR] Magnet in Rule %lu Action %lu missing 'Angle' key.\n", _label, actionId);
   _magnets.car1AngleMagnet = angleMagnet_t { .intensity = actionJs["Intensity"].get<float>(), .angle = actionJs["Angle"].get<float>() };
   recognizedActionType = true;
  }


  return recognizedActionType;
}

datatype_t GameRule::getPropertyType(const nlohmann::json& condition)
{
  std::string propertyName = condition["Property"].get<std::string>();

  if (propertyName == "Player 1 Laps Remaining") return dt_uint8;
  if (propertyName == "Player 1 Laps Remaining Prev") return dt_uint8;
  if (propertyName == "Player 1 Checkpoint") return dt_uint8;
  if (propertyName == "Player 1 Recovery Mode") return dt_uint8;
  if (propertyName == "Player 1 Pos X") return dt_uint16;
  if (propertyName == "Player 1 Pos Y") return dt_uint16;
  if (propertyName == "Global Timer") return dt_uint16;

  EXIT_WITH_ERROR("[Error] Rule %lu, unrecognized property: %s\n", _label, propertyName.c_str());

  return dt_uint8;
}

void* GameRule::getPropertyPointer(const nlohmann::json& condition, GameInstance* gameInstance)
{
  std::string propertyName = condition["Property"].get<std::string>();

  if (propertyName == "Player 1 Laps Remaining") return gameInstance->player1LapsRemaining;
  if (propertyName == "Player 1 Laps Remaining Prev") return gameInstance->player1LapsRemainingPrev;
  if (propertyName == "Player 1 Checkpoint") return gameInstance->player1Checkpoint;
  if (propertyName == "Player 1 Recovery Mode") return gameInstance->player1RecoveryMode;
  if (propertyName == "Player 1 Pos X") return &gameInstance->player1PosX;
  if (propertyName == "Player 1 Pos Y") return &gameInstance->player1PosY;
  if (propertyName == "Global Timer") return gameInstance->globalTimer;

  EXIT_WITH_ERROR("[Error] Rule %lu, unrecognized property: %s\n", _label, propertyName.c_str());

  return NULL;
}
