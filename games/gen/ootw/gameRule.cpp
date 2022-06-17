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
   if (isDefined(actionJs, "Min") == false) EXIT_WITH_ERROR("[ERROR] Magnet in Rule %lu Action %lu missing 'Min' key.\n", _label, actionId);
   if (isDefined(actionJs, "Max") == false) EXIT_WITH_ERROR("[ERROR] Magnet in Rule %lu Action %lu missing 'Max' key.\n", _label, actionId);
   _magnets.lesterHorizontalMagnet = genericMagnet_t { .intensity = actionJs["Intensity"].get<float>(), .center= actionJs["Center"].get<float>(), .min = actionJs["Min"].get<float>(), .max = actionJs["Max"].get<float>() };
    recognizedActionType = true;
  }

  if (actionType == "Set Lester Vertical Magnet")
  {
   if (isDefined(actionJs, "Intensity") == false) EXIT_WITH_ERROR("[ERROR] Magnet in Rule %lu Action %lu missing 'Intensity' key.\n", _label, actionId);
   if (isDefined(actionJs, "Min") == false) EXIT_WITH_ERROR("[ERROR] Magnet in Rule %lu Action %lu missing 'Min' key.\n", _label, actionId);
   if (isDefined(actionJs, "Max") == false) EXIT_WITH_ERROR("[ERROR] Magnet in Rule %lu Action %lu missing 'Max' key.\n", _label, actionId);
   _magnets.lesterVerticalMagnet = genericMagnet_t { .intensity = actionJs["Intensity"].get<float>(), .center= actionJs["Center"].get<float>(), .min = actionJs["Min"].get<float>(), .max = actionJs["Max"].get<float>() };
   recognizedActionType = true;
  }

  return recognizedActionType;
}

datatype_t GameRule::getPropertyType(const nlohmann::json& condition)
{
  std::string propertyName = condition["Property"].get<std::string>();

  if (propertyName == "Game Timer") return dt_uint16;

  if (propertyName == "Lester Position X") return dt_uint16;
  if (propertyName == "Lester Position Y") return dt_uint16;
  if (propertyName == "Lester Frame 1") return dt_uint16;
  if (propertyName == "Lester Frame 2") return dt_uint16;
  if (propertyName == "Lester Frame 3") return dt_uint16;
  if (propertyName == "Lester Room") return dt_uint8;

  if (propertyName == "Stage 01 Appear Time") return dt_uint8;
  if (propertyName == "Stage 01 Monster Animation") return dt_uint8;
  if (propertyName == "Stage 01 Met Monster") return dt_uint8;
  if (propertyName == "Stage 01 Vine State") return dt_int8;
  if (propertyName == "Stage 01 Finish") return dt_uint8;

  EXIT_WITH_ERROR("[Error] Rule %lu, unrecognized property: %s\n", _label, propertyName.c_str());

  return dt_uint8;
}

void* GameRule::getPropertyPointer(const nlohmann::json& condition, GameInstance* gameInstance)
{
  std::string propertyName = condition["Property"].get<std::string>();

  if (propertyName == "Game Timer") return gameInstance->gameTimer;

  if (propertyName == "Lester Position X") return gameInstance->lesterPosX;
  if (propertyName == "Lester Position Y") return gameInstance->lesterPosY;
  if (propertyName == "Lester Frame 1") return gameInstance->lesterFrame1;
  if (propertyName == "Lester Frame 2") return gameInstance->lesterFrame2;
  if (propertyName == "Lester Frame 3") return gameInstance->lesterFrame3;
  if (propertyName == "Lester Room") return gameInstance->lesterRoom;

  if (propertyName == "Stage 01 Appear Time") return gameInstance->stage01AppearTimer;
  if (propertyName == "Stage 01 Monster Animation") return gameInstance->stage01MonsterAnimation;
  if (propertyName == "Stage 01 Met Monster") return gameInstance->stage01MetMonster;
  if (propertyName == "Stage 01 Vine State") return gameInstance->stage01VineState;
  if (propertyName == "Stage 01 Finish") return gameInstance->stage01Finish;

  EXIT_WITH_ERROR("[Error] Rule %lu, unrecognized property: %s\n", _label, propertyName.c_str());

  return NULL;
}
