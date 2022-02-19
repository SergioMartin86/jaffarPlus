#include "gameRule.hpp"

GameRule::GameRule() : Rule()
{
 _magnets.simonHorizontalMagnet = magnet_t { .intensity = 0.0f, .min = 0.0f, .max = 0.0f };
 _magnets.simonVerticalMagnet = magnet_t { .intensity = 0.0f, .min = 0.0f, .max = 0.0f };
}

bool GameRule::parseGameAction(nlohmann::json actionJs, size_t actionId)
{
  bool recognizedActionType = false;
  std::string actionType = actionJs["Type"].get<std::string>();

  if (actionType == "Set Simon Horizontal Magnet")
  {
   if (isDefined(actionJs, "Intensity") == false) EXIT_WITH_ERROR("[ERROR] Magnet in Rule %lu Action %lu missing 'Intensity' key.\n", _label, actionId);
   if (isDefined(actionJs, "Min") == false) EXIT_WITH_ERROR("[ERROR] Magnet in Rule %lu Action %lu missing 'Min' key.\n", _label, actionId);
   if (isDefined(actionJs, "Max") == false) EXIT_WITH_ERROR("[ERROR] Magnet in Rule %lu Action %lu missing 'Max' key.\n", _label, actionId);
   _magnets.simonHorizontalMagnet = magnet_t { .intensity = actionJs["Intensity"].get<float>(), .min = actionJs["Min"].get<float>(), .max = actionJs["Max"].get<float>() };
    recognizedActionType = true;
  }

  if (actionType == "Set Simon Vertical Magnet")
  {
   if (isDefined(actionJs, "Intensity") == false) EXIT_WITH_ERROR("[ERROR] Magnet in Rule %lu Action %lu missing 'Intensity' key.\n", _label, actionId);
   if (isDefined(actionJs, "Min") == false) EXIT_WITH_ERROR("[ERROR] Magnet in Rule %lu Action %lu missing 'Min' key.\n", _label, actionId);
   if (isDefined(actionJs, "Max") == false) EXIT_WITH_ERROR("[ERROR] Magnet in Rule %lu Action %lu missing 'Max' key.\n", _label, actionId);
   _magnets.simonVerticalMagnet = magnet_t { .intensity = actionJs["Intensity"].get<float>(), .min = actionJs["Min"].get<float>(), .max = actionJs["Max"].get<float>() };
    recognizedActionType = true;
  }

  return recognizedActionType;
}

datatype_t GameRule::getPropertyType(const nlohmann::json& condition)
{
  std::string propertyName = condition["Property"].get<std::string>();

  if (propertyName == "Game Mode") return dt_uint8;
  if (propertyName == "Game SubMode") return dt_uint8;
  if (propertyName == "Current Stage") return dt_uint8;
  if (propertyName == "Current SubStage") return dt_uint8;
  if (propertyName == "Simon Lives") return dt_uint8;
  if (propertyName == "Screen Offset") return dt_uint16;
  if (propertyName == "Simon Stair Mode") return dt_uint8;
  if (propertyName == "Simon Position Y") return dt_uint8;
  if (propertyName == "Simon Position X") return dt_uint16;
  if (propertyName == "Simon Life Meter") return dt_uint8;
  if (propertyName == "Simon Invulnerability") return dt_uint8;
  if (propertyName == "Simon Kneeling Mode") return dt_uint8;
  if (propertyName == "Subweapon Shot Count") return dt_uint8;
  if (propertyName == "Whip Length") return dt_uint8;
  if (propertyName == "Simon Heart Count") return dt_uint8;
  if (propertyName == "Simon Image") return dt_uint8;
  if (propertyName == "Subweapon Number") return dt_uint8;
  if (propertyName == "Simon Facing Direction") return dt_uint8;
  if (propertyName == "Simon State") return dt_uint8;
  if (propertyName == "Simon SubState") return dt_uint8;
  if (propertyName == "Simon Vertical Speed") return dt_uint8;
  if (propertyName == "Simon Vertical Direction") return dt_uint8;

  EXIT_WITH_ERROR("[Error] Rule %lu, unrecognized property: %s\n", _label, propertyName.c_str());

  return dt_uint8;
}

void* GameRule::getPropertyPointer(const nlohmann::json& condition, GameInstance* gameInstance)
{
  std::string propertyName = condition["Property"].get<std::string>();

  if (propertyName == "Game Mode") return gameInstance->gameMode;
  if (propertyName == "Game SubMode") return gameInstance->gameSubMode;
  if (propertyName == "Current Stage") return gameInstance->currentStage;
  if (propertyName == "Current SubStage") return gameInstance->currentSubStage;
  if (propertyName == "Simon Lives") return gameInstance->simonLives;
  if (propertyName == "Screen Offset") return gameInstance->screenOffset;
  if (propertyName == "Simon Stair Mode") return gameInstance->simonStairMode;
  if (propertyName == "Simon Position Y") return gameInstance->simonPosY;
  if (propertyName == "Simon Position X") return gameInstance->simonPosX;
  if (propertyName == "Simon Life Meter") return gameInstance->simonLifeMeter;
  if (propertyName == "Simon Invulnerability") return gameInstance->simonInvulnerability;
  if (propertyName == "Simon Kneeling Mode") return gameInstance->simonKneelingMode;
  if (propertyName == "Subweapon Shot Count") return gameInstance->subweaponShotCount;
  if (propertyName == "Whip Length") return gameInstance->whipLength;
  if (propertyName == "Simon Heart Count") return gameInstance->simonHeartCount;
  if (propertyName == "Simon Image") return gameInstance->simonImage;
  if (propertyName == "Subweapon Number") return gameInstance->subweaponNumber;
  if (propertyName == "Simon Facing Direction") return gameInstance->simonFacingDirection;
  if (propertyName == "Simon State") return gameInstance->simonState;
  if (propertyName == "Simon SubState") return gameInstance->simonSubState;
  if (propertyName == "Simon Vertical Speed") return gameInstance->simonVerticalSpeed;
  if (propertyName == "Simon Vertical Direction") return gameInstance->simonVerticalDirection;

  EXIT_WITH_ERROR("[Error] Rule %lu, unrecognized property: %s\n", _label, propertyName.c_str());

  return NULL;
}
