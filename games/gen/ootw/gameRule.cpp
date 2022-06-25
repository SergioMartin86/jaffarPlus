#include "gameRule.hpp"

GameRule::GameRule() : Rule()
{
}

bool GameRule::parseGameAction(nlohmann::json actionJs, size_t actionId)
{
  bool recognizedActionType = false;
  std::string actionType = actionJs["Type"].get<std::string>();

  if (actionType == "Set Lester Horizontal Magnet")
  {
   if (isDefined(actionJs, "Intensity") == false) EXIT_WITH_ERROR("[ERROR] Magnet in Rule %lu Action %lu missing 'Intensity' key.\n", _label, actionId);
   if (isDefined(actionJs, "Center") == false) EXIT_WITH_ERROR("[ERROR] Magnet in Rule %lu Action %lu missing 'Center' key.\n", _label, actionId);
   if (isDefined(actionJs, "Room") == false) EXIT_WITH_ERROR("[ERROR] Magnet in Rule %lu Action %lu missing 'Room' key.\n", _label, actionId);
   if (isDefined(actionJs, "Min") == false) EXIT_WITH_ERROR("[ERROR] Magnet in Rule %lu Action %lu missing 'Min' key.\n", _label, actionId);
   if (isDefined(actionJs, "Max") == false) EXIT_WITH_ERROR("[ERROR] Magnet in Rule %lu Action %lu missing 'Max' key.\n", _label, actionId);
   _magnets.lesterHorizontalMagnet[actionJs["Room"].get<uint8_t>()] = genericMagnet_t { .intensity = actionJs["Intensity"].get<float>(), .center= actionJs["Center"].get<float>(), .min = actionJs["Min"].get<float>(), .max = actionJs["Max"].get<float>() };
    recognizedActionType = true;
  }

  if (actionType == "Set Lester Vertical Magnet")
  {
   if (isDefined(actionJs, "Intensity") == false) EXIT_WITH_ERROR("[ERROR] Magnet in Rule %lu Action %lu missing 'Intensity' key.\n", _label, actionId);
   if (isDefined(actionJs, "Room") == false) EXIT_WITH_ERROR("[ERROR] Magnet in Rule %lu Action %lu missing 'Room' key.\n", _label, actionId);
   if (isDefined(actionJs, "Min") == false) EXIT_WITH_ERROR("[ERROR] Magnet in Rule %lu Action %lu missing 'Min' key.\n", _label, actionId);
   if (isDefined(actionJs, "Max") == false) EXIT_WITH_ERROR("[ERROR] Magnet in Rule %lu Action %lu missing 'Max' key.\n", _label, actionId);
   _magnets.lesterVerticalMagnet[actionJs["Room"].get<uint8_t>()] = genericMagnet_t { .intensity = actionJs["Intensity"].get<float>(), .center= actionJs["Center"].get<float>(), .min = actionJs["Min"].get<float>(), .max = actionJs["Max"].get<float>() };
   recognizedActionType = true;
  }

  if (actionType == "Set Stage 01 Vine State Magnet")
  {
   if (isDefined(actionJs, "Intensity") == false) EXIT_WITH_ERROR("[ERROR] Magnet in Rule %lu Action %lu missing 'Intensity' key.\n", _label, actionId);
   _magnets.stage01VineStateMagnet = actionJs["Intensity"].get<float>();
   recognizedActionType = true;
  }


  if (actionType == "Set Lester Angular Momentum Magnet")
  {
   if (isDefined(actionJs, "Intensity") == false) EXIT_WITH_ERROR("[ERROR] Magnet in Rule %lu Action %lu missing 'Intensity' key.\n", _label, actionId);
   _magnets.lesterAngularMomentumMagnet = actionJs["Intensity"].get<float>();
   recognizedActionType = true;
  }

  return recognizedActionType;
}

datatype_t GameRule::getPropertyType(const nlohmann::json& condition)
{
  std::string propertyName = condition["Property"].get<std::string>();

  if (propertyName == "Game Timer") return dt_uint16;
  if (propertyName == "Animation Frame") return dt_uint8;
  if (propertyName == "Game Mode") return dt_uint8;
  if (propertyName == "Lester Position X") return dt_int16;
  if (propertyName == "Lester Absolute Position X") return dt_uint16;
  if (propertyName == "Lester Position Y") return dt_int16;
  if (propertyName == "Lester Room") return dt_uint8;
  if (propertyName == "Lester Frame") return dt_uint16;

  if (propertyName == "Stage 01 Appear Time") return dt_uint8;
  if (propertyName == "Stage 01 Vine State") return dt_int16;
  if (propertyName == "Stage 01 Skip Monster Flag") return dt_int8;
  if (propertyName == "Stage 01 Finish") return dt_uint8;

  EXIT_WITH_ERROR("[Error] Rule %lu, unrecognized property: %s\n", _label, propertyName.c_str());

  return dt_uint8;
}

void* GameRule::getPropertyPointer(const nlohmann::json& condition, GameInstance* gameInstance)
{
  std::string propertyName = condition["Property"].get<std::string>();

  if (propertyName == "Game Timer") return gameInstance->gameTimer;
  if (propertyName == "Animation Frame") return gameInstance->animationFrame;
  if (propertyName == "Game Mode") return gameInstance->gameMode;
  if (propertyName == "Lester Position X") return gameInstance->lesterPosX;
  if (propertyName == "Lester Absolute Position X") return &gameInstance->lesterAbsolutePosX;
  if (propertyName == "Lester Position Y") return gameInstance->lesterPosY;
  if (propertyName == "Lester Room") return gameInstance->lesterRoom;
  if (propertyName == "Lester Frame") return gameInstance->lesterFrame;

  if (propertyName == "Stage 01 Appear Time") return gameInstance->stage01AppearTimer;
  if (propertyName == "Stage 01 Vine State") return gameInstance->stage01VineState;
  if (propertyName == "Stage 01 Skip Monster Flag") return gameInstance->stage01SkipMonsterFlag;
  if (propertyName == "Stage 01 Finish") return gameInstance->stage01Finish;

  EXIT_WITH_ERROR("[Error] Rule %lu, unrecognized property: %s\n", _label, propertyName.c_str());

  return NULL;
}
