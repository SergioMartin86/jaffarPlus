#include "gameRule.hpp"

GameRule::GameRule() : Rule()
{
}

bool GameRule::parseGameAction(nlohmann::json actionJs, size_t actionId)
{
  bool recognizedActionType = false;
  std::string actionType = actionJs["Type"].get<std::string>();

  if (actionType == "Set Remaining Blocks Magnet")
  {
   if (isDefined(actionJs, "Intensity") == false) EXIT_WITH_ERROR("[ERROR] Magnet in Rule %lu Action %lu missing 'Intensity' key.\n", _label, actionId);
   _magnets.remainingBlocksMagnet = actionJs["Intensity"].get<float>();
   recognizedActionType = true;
  }

  if (actionType == "Set Remaining Hits Magnet")
  {
   if (isDefined(actionJs, "Intensity") == false) EXIT_WITH_ERROR("[ERROR] Magnet in Rule %lu Action %lu missing 'Intensity' key.\n", _label, actionId);
   _magnets.remainingHitsMagnet = actionJs["Intensity"].get<float>();
   recognizedActionType = true;
  }

  if (actionType == "Set Ball 1 Pos Y Magnet")
  {
   if (isDefined(actionJs, "Intensity") == false) EXIT_WITH_ERROR("[ERROR] Magnet in Rule %lu Action %lu missing 'Intensity' key.\n", _label, actionId);
   _magnets.ball1PosYMagnet = actionJs["Intensity"].get<float>();
   recognizedActionType = true;
  }

  if (actionType == "Set Falling Power Up Pos Y Magnet")
  {
   if (isDefined(actionJs, "Intensity") == false) EXIT_WITH_ERROR("[ERROR] Magnet in Rule %lu Action %lu missing 'Intensity' key.\n", _label, actionId);
   _magnets.fallingPowerUpPosYMagnet = actionJs["Intensity"].get<float>();
   recognizedActionType = true;
  }

  if (actionType == "Set Horizontal Distance To Lowest Ball Magnet")
  {
   if (isDefined(actionJs, "Intensity") == false) EXIT_WITH_ERROR("[ERROR] Magnet in Rule %lu Action %lu missing 'Intensity' key.\n", _label, actionId);
   _magnets.horizontalDistaceToLowestBallMagnet = actionJs["Intensity"].get<float>();
   recognizedActionType = true;
  }

  if (actionType == "Set Lowest Ball Pos Y Magnet")
  {
   if (isDefined(actionJs, "Intensity") == false) EXIT_WITH_ERROR("[ERROR] Magnet in Rule %lu Action %lu missing 'Intensity' key.\n", _label, actionId);
   _magnets.lowestBallPosYMagnet = actionJs["Intensity"].get<float>();
   recognizedActionType = true;
  }

  return recognizedActionType;
}

datatype_t GameRule::getPropertyType(const nlohmann::json& condition)
{
  std::string propertyName = condition["Property"].get<std::string>();

  if (propertyName == "Game Mode") return dt_uint8;
  if (propertyName == "Remaining Blocks") return dt_uint8;
  if (propertyName == "Ball 1 Pos X") return dt_uint8;
  if (propertyName == "Ball 1 Pos Y") return dt_uint8;
  if (propertyName == "Ball 2 Pos X") return dt_uint8;
  if (propertyName == "Ball 2 Pos Y") return dt_uint8;
  if (propertyName == "Ball 3 Pos X") return dt_uint8;
  if (propertyName == "Ball 3 Pos Y") return dt_uint8;
  if (propertyName == "Paddle State") return dt_uint8;
  if (propertyName == "Paddle Pos X") return dt_uint8;
  if (propertyName == "Paddle Power Up 1") return dt_uint8;
  if (propertyName == "Paddle Power Up 2") return dt_uint8;
  if (propertyName == "Falling Power Up Type") return dt_uint8;
  if (propertyName == "Falling Power Up Pos Y") return dt_uint8;
  if (propertyName == "Warp Is Active") return dt_uint8;


  EXIT_WITH_ERROR("[Error] Rule %lu, unrecognized property: %s\n", _label, propertyName.c_str());

  return dt_uint8;
}

void* GameRule::getPropertyPointer(const nlohmann::json& condition, GameInstance* gameInstance)
{
  std::string propertyName = condition["Property"].get<std::string>();

  if (propertyName == "Game Mode") return gameInstance->gameMode;
  if (propertyName == "Remaining Blocks") return gameInstance->remainingBlocks;
  if (propertyName == "Ball 1 Pos X") return gameInstance->ball1X;
  if (propertyName == "Ball 1 Pos Y") return gameInstance->ball1Y;
  if (propertyName == "Ball 2 Pos X") return gameInstance->ball2X;
  if (propertyName == "Ball 2 Pos Y") return gameInstance->ball2Y;
  if (propertyName == "Ball 3 Pos X") return gameInstance->ball3X;
  if (propertyName == "Ball 3 Pos Y") return gameInstance->ball3Y;
  if (propertyName == "Paddle State") return gameInstance->paddleState;
  if (propertyName == "Paddle Pos X") return gameInstance->paddlePosX;
  if (propertyName == "Paddle Power Up 1") return gameInstance->paddlePowerUp1;
  if (propertyName == "Paddle Power Up 2") return gameInstance->paddlePowerUp2;
  if (propertyName == "Falling Power Up Type") return gameInstance->fallingPowerUpType;
  if (propertyName == "Falling Power Up Pos Y") return gameInstance->fallingPowerUpPosY;
  if (propertyName == "Warp Is Active") return gameInstance->warpIsActive;

  EXIT_WITH_ERROR("[Error] Rule %lu, unrecognized property: %s\n", _label, propertyName.c_str());

  return NULL;
}
