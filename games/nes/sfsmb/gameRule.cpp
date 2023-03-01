#include "gameRule.hpp"

GameRule::GameRule() : Rule()
{
}

bool GameRule::parseGameAction(nlohmann::json actionJs, size_t actionId)
{
  bool recognizedActionType = false;
  std::string actionType = actionJs["Type"].get<std::string>();

  if (actionType == "Set Screen Horizontal Magnet")
  {
    if (isDefined(actionJs, "Intensity") == false) EXIT_WITH_ERROR("[ERROR] Magnet in Rule %lu Action %lu missing 'Intensity' key.\n", _label, actionId);
    if (isDefined(actionJs, "Center") == false) EXIT_WITH_ERROR("[ERROR] Magnet in Rule %lu Action %lu missing 'Center' key.\n", _label, actionId);
    if (isDefined(actionJs, "Min") == false) EXIT_WITH_ERROR("[ERROR] Magnet in Rule %lu Action %lu missing 'Min' key.\n", _label, actionId);
    if (isDefined(actionJs, "Max") == false) EXIT_WITH_ERROR("[ERROR] Magnet in Rule %lu Action %lu missing 'Max' key.\n", _label, actionId);
    _magnets.screenHorizontalMagnet = genericMagnet_t { .intensity = actionJs["Intensity"].get<float>(), .center= actionJs["Center"].get<float>(), .min = actionJs["Min"].get<float>(), .max = actionJs["Max"].get<float>() };
    recognizedActionType = true;
  }

  if (actionType == "Set Mario Screen Offset Magnet")
  {
    if (isDefined(actionJs, "Intensity") == false) EXIT_WITH_ERROR("[ERROR] Magnet in Rule %lu Action %lu missing 'Intensity' key.\n", _label, actionId);
    if (isDefined(actionJs, "Center") == false) EXIT_WITH_ERROR("[ERROR] Magnet in Rule %lu Action %lu missing 'Center' key.\n", _label, actionId);
    if (isDefined(actionJs, "Min") == false) EXIT_WITH_ERROR("[ERROR] Magnet in Rule %lu Action %lu missing 'Min' key.\n", _label, actionId);
    if (isDefined(actionJs, "Max") == false) EXIT_WITH_ERROR("[ERROR] Magnet in Rule %lu Action %lu missing 'Max' key.\n", _label, actionId);
    _magnets.marioScreenOffsetMagnet = genericMagnet_t { .intensity = actionJs["Intensity"].get<float>(), .center= actionJs["Center"].get<float>(), .min = actionJs["Min"].get<float>(), .max = actionJs["Max"].get<float>() };
    recognizedActionType = true;
  }

  if (actionType == "Set Mario Horizontal Magnet")
   {
    if (isDefined(actionJs, "Intensity") == false) EXIT_WITH_ERROR("[ERROR] Magnet in Rule %lu Action %lu missing 'Intensity' key.\n", _label, actionId);
    if (isDefined(actionJs, "Center") == false) EXIT_WITH_ERROR("[ERROR] Magnet in Rule %lu Action %lu missing 'Center' key.\n", _label, actionId);
    if (isDefined(actionJs, "Min") == false) EXIT_WITH_ERROR("[ERROR] Magnet in Rule %lu Action %lu missing 'Min' key.\n", _label, actionId);
    if (isDefined(actionJs, "Max") == false) EXIT_WITH_ERROR("[ERROR] Magnet in Rule %lu Action %lu missing 'Max' key.\n", _label, actionId);
    _magnets.marioHorizontalMagnet = genericMagnet_t { .intensity = actionJs["Intensity"].get<float>(), .center= actionJs["Center"].get<float>(), .min = actionJs["Min"].get<float>(), .max = actionJs["Max"].get<float>() };
     recognizedActionType = true;
   }

   if (actionType == "Set Mario Vertical Magnet")
   {
    if (isDefined(actionJs, "Intensity") == false) EXIT_WITH_ERROR("[ERROR] Magnet in Rule %lu Action %lu missing 'Intensity' key.\n", _label, actionId);
    if (isDefined(actionJs, "Min") == false) EXIT_WITH_ERROR("[ERROR] Magnet in Rule %lu Action %lu missing 'Min' key.\n", _label, actionId);
    if (isDefined(actionJs, "Max") == false) EXIT_WITH_ERROR("[ERROR] Magnet in Rule %lu Action %lu missing 'Max' key.\n", _label, actionId);
    _magnets.marioVerticalMagnet = genericMagnet_t { .intensity = actionJs["Intensity"].get<float>(), .center= actionJs["Center"].get<float>(), .min = actionJs["Min"].get<float>(), .max = actionJs["Max"].get<float>() };
    recognizedActionType = true;
   }

  return recognizedActionType;
}

datatype_t GameRule::getPropertyType(const nlohmann::json& condition)
{
  std::string propertyName = condition["Property"].get<std::string>();

  if (propertyName == "Mario State") return dt_uint8;
  if (propertyName == "Mario Animation") return dt_uint8;
  if (propertyName == "Mario Walking Frame") return dt_uint8;
  if (propertyName == "Mario Walking Mode") return dt_uint8;
  if (propertyName == "Mario Floating Mode") return dt_uint8;

  if (propertyName == "Screen Position X") return dt_uint16;

  if (propertyName == "Mario Position Y") return dt_uint8;
  if (propertyName == "Mario Velocity X") return dt_int8;

  if (propertyName == "Mario Base Position X") return dt_uint8;
  if (propertyName == "Mario Relative Position X") return dt_uint8;
  if (propertyName == "Mario Position X") return dt_uint16;

  if (propertyName == "Current World") return dt_uint8;
  if (propertyName == "Current Stage") return dt_uint8;
  if (propertyName == "Current Screen") return dt_uint8;
  if (propertyName == "Level Entry Flag") return dt_uint8;
  if (propertyName == "Game Mode") return dt_uint8;
  if (propertyName == "Warp Area Offset") return dt_uint16;
  if (propertyName == "Enemy 1 Type") return dt_uint8;
  if (propertyName == "Enemy 2 Type") return dt_uint8;
  if (propertyName == "Enemy 3 Type") return dt_uint8;
  if (propertyName == "Enemy 4 Type") return dt_uint8;
  if (propertyName == "Enemy 5 Type") return dt_uint8;
  if (propertyName == "Mario Screen Offset") return dt_int16;
  if (propertyName == "Warp Selector") return dt_uint8;

  EXIT_WITH_ERROR("[Error] Rule %lu, unrecognized property: %s\n", _label, propertyName.c_str());

  return dt_uint8;
}

void* GameRule::getPropertyPointer(const nlohmann::json& condition, GameInstance* gameInstance)
{
  std::string propertyName = condition["Property"].get<std::string>();

  if (propertyName == "Mario State") return gameInstance->marioState;
  if (propertyName == "Mario Animation") return gameInstance->marioAnimation;
  if (propertyName == "Mario Walking Frame") return gameInstance->marioWalkingFrame;
  if (propertyName == "Mario Walking Mode") return gameInstance->marioWalkingMode;
  if (propertyName == "Mario Floating Mode") return gameInstance->marioFloatingMode;

  if (propertyName == "Screen Position X") return &gameInstance->screenPosX; // Derivative value

  if (propertyName == "Mario Base Position X") return gameInstance->marioBasePosX;
  if (propertyName == "Mario Relative Position X") return gameInstance->marioRelPosX;
  if (propertyName == "Mario Position X") return &gameInstance->marioPosX; // Derivative value

  if (propertyName == "Mario Position Y") return &gameInstance->marioPosY;
  if (propertyName == "Mario Velocity X") return gameInstance->marioVelX;
  if (propertyName == "Current World") return &gameInstance->currentWorld; // Derivative value
  if (propertyName == "Current Stage") return &gameInstance->currentStage; // Derivative value
  if (propertyName == "Current Screen") return gameInstance->screenBasePosX;
  if (propertyName == "Level Entry Flag") return gameInstance->levelEntryFlag;
  if (propertyName == "Game Mode") return gameInstance->gameMode;
  if (propertyName == "Warp Area Offset") return gameInstance->warpAreaOffset;
  if (propertyName == "Enemy 1 Type") return gameInstance->enemy1Type;
  if (propertyName == "Enemy 2 Type") return gameInstance->enemy2Type;
  if (propertyName == "Enemy 3 Type") return gameInstance->enemy3Type;
  if (propertyName == "Enemy 4 Type") return gameInstance->enemy4Type;
  if (propertyName == "Enemy 5 Type") return gameInstance->enemy5Type;
  if (propertyName == "Mario Screen Offset") return &gameInstance->marioScreenOffset; // Derivative value
  if (propertyName == "Warp Selector") return gameInstance->warpSelector;

  EXIT_WITH_ERROR("[Error] Rule %lu, unrecognized property: %s\n", _label, propertyName.c_str());

  return NULL;
}
