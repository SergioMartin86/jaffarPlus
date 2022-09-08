#include "gameRule.hpp"

GameRule::GameRule() : Rule()
{
}

bool GameRule::parseGameAction(nlohmann::json actionJs, size_t actionId)
{
  bool recognizedActionType = false;
  std::string actionType = actionJs["Type"].get<std::string>();

  if (actionType == "Set Player 1 Speed Magnet")
  {
   if (isDefined(actionJs, "Intensity") == false) EXIT_WITH_ERROR("[ERROR] Magnet in Rule %lu Action %lu missing 'Intensity' key.\n", _label, actionId);
   _magnets.player1SpeedMagnet = actionJs["Intensity"].get<float>();
   recognizedActionType = true;
  }

  if (actionType == "Set Player 1 Standing Magnet")
  {
   if (isDefined(actionJs, "Intensity") == false) EXIT_WITH_ERROR("[ERROR] Magnet in Rule %lu Action %lu missing 'Intensity' key.\n", _label, actionId);
   _magnets.player1StandingMagnet = actionJs["Intensity"].get<float>();
   recognizedActionType = true;
  }

  return recognizedActionType;
}

datatype_t GameRule::getPropertyType(const nlohmann::json& condition)
{
  std::string propertyName = condition["Property"].get<std::string>();

  if (propertyName == "Player 1 Standing") return dt_uint8;
  if (propertyName == "Player 1 Speed") return dt_float;
  if (propertyName == "Player 1 Pos X") return dt_float;
  if (propertyName == "Player 1 Pos Y") return dt_float;

  EXIT_WITH_ERROR("[Error] Rule %lu, unrecognized property: %s\n", _label, propertyName.c_str());

  return dt_uint8;
}

void* GameRule::getPropertyPointer(const nlohmann::json& condition, GameInstance* gameInstance)
{
  std::string propertyName = condition["Property"].get<std::string>();

  if (propertyName == "Player 1 Standing") return gameInstance->player1Standing;
  if (propertyName == "Player 1 Speed") return &gameInstance->player1Speed;
  if (propertyName == "Player 1 Pos X") return &gameInstance->player1PosX;
  if (propertyName == "Player 1 Pos Y") return &gameInstance->player1PosY;

  EXIT_WITH_ERROR("[Error] Rule %lu, unrecognized property: %s\n", _label, propertyName.c_str());

  return NULL;
}
