#include "gameRule.hpp"

GameRule::GameRule() : Rule()
{
}

bool GameRule::parseGameAction(nlohmann::json actionJs, size_t actionId)
{
  bool recognizedActionType = false;
  std::string actionType = actionJs["Type"].get<std::string>();

  if (actionType == "Set Samus Horizontal Magnet")
  {
   if (isDefined(actionJs, "Intensity") == false) EXIT_WITH_ERROR("[ERROR] Magnet in Rule %lu Action %lu missing 'Intensity' key.\n", _label, actionId);
   if (isDefined(actionJs, "Center") == false) EXIT_WITH_ERROR("[ERROR] Magnet in Rule %lu Action %lu missing 'Center' key.\n", _label, actionId);
   _magnets.samusHorizontalMagnet = genericMagnet_t { .intensity = actionJs["Intensity"].get<float>(), .center= actionJs["Center"].get<float>() };
    recognizedActionType = true;
  }

  if (actionType == "Set Samus Vertical Magnet")
  {
   if (isDefined(actionJs, "Intensity") == false) EXIT_WITH_ERROR("[ERROR] Magnet in Rule %lu Action %lu missing 'Intensity' key.\n", _label, actionId);
   if (isDefined(actionJs, "Center") == false) EXIT_WITH_ERROR("[ERROR] Magnet in Rule %lu Action %lu missing 'Center' key.\n", _label, actionId);
   _magnets.samusVerticalMagnet = genericMagnet_t { .intensity = actionJs["Intensity"].get<float>(), .center= actionJs["Center"].get<float>() };
    recognizedActionType = true;
  }

  if (actionType == "Set Bullet 1 Horizontal Magnet")
  {
   if (isDefined(actionJs, "Intensity") == false) EXIT_WITH_ERROR("[ERROR] Magnet in Rule %lu Action %lu missing 'Intensity' key.\n", _label, actionId);
   if (isDefined(actionJs, "Center") == false) EXIT_WITH_ERROR("[ERROR] Magnet in Rule %lu Action %lu missing 'Center' key.\n", _label, actionId);
   _magnets.bullet1HorizontalMagnet = genericMagnet_t { .intensity = actionJs["Intensity"].get<float>(), .center= actionJs["Center"].get<float>() };
    recognizedActionType = true;
  }

  if (actionType == "Set Bullet 1 Vertical Magnet")
  {
   if (isDefined(actionJs, "Intensity") == false) EXIT_WITH_ERROR("[ERROR] Magnet in Rule %lu Action %lu missing 'Intensity' key.\n", _label, actionId);
   if (isDefined(actionJs, "Center") == false) EXIT_WITH_ERROR("[ERROR] Magnet in Rule %lu Action %lu missing 'Center' key.\n", _label, actionId);
   _magnets.bullet1VerticalMagnet = genericMagnet_t { .intensity = actionJs["Intensity"].get<float>(), .center= actionJs["Center"].get<float>() };
    recognizedActionType = true;
  }

  if (actionType == "Set Missile Count Magnet")
  {
   if (isDefined(actionJs, "Intensity") == false) EXIT_WITH_ERROR("[ERROR] Magnet in Rule %lu Action %lu missing 'Intensity' key.\n", _label, actionId);
   _magnets.missileCountMagnet = actionJs["Intensity"].get<float>();
    recognizedActionType = true;
  }

  if (actionType == "Set Custom Value")
  {
   if (isDefined(actionJs, "Value") == false) EXIT_WITH_ERROR("[ERROR] Magnet in Rule %lu Action %lu missing 'Value' key.\n", _label, actionId);
   _customValue = actionJs["Value"].get<uint8_t>();
   _customValueActive = true;
   recognizedActionType = true;
  }

  if (actionType == "Set Disable B")
  {
   if (isDefined(actionJs, "Value") == false) EXIT_WITH_ERROR("[ERROR] Magnet in Rule %lu Action %lu missing 'Value' key.\n", _label, actionId);
   _disableBValue = actionJs["Value"].get<bool>();
   _disableBActive = true;
   recognizedActionType = true;
  }

  if (actionType == "Set Lag Frame Counter Magnet")
  {
   if (isDefined(actionJs, "Intensity") == false) EXIT_WITH_ERROR("[ERROR] Magnet in Rule %lu Action %lu missing 'Value' key.\n", _label, actionId);
   _magnets.lagFrameCounterMagnet = actionJs["Intensity"].get<float>();
   recognizedActionType = true;
  }

  return recognizedActionType;
}

datatype_t GameRule::getPropertyType(const nlohmann::json& condition)
{
  std::string propertyName = condition["Property"].get<std::string>();

  if (propertyName == "Equipment Flags") return dt_uint8;
  if (propertyName == "Samus Pos Y Raw") return dt_uint8;
  if (propertyName == "Screen Pos Y2") return dt_uint8;
  if (propertyName == "Screen Pos Y1") return dt_uint8;
  if (propertyName == "Samus Pos X Raw") return dt_uint8;
  if (propertyName == "Screen Pos X2") return dt_uint8;
  if (propertyName == "Screen Pos X1") return dt_uint8;
  if (propertyName == "Samus Pos X") return dt_uint16;
  if (propertyName == "Samus Pos Y") return dt_uint16;
  if (propertyName == "Samus Door State") return dt_uint8;
  if (propertyName == "Samus Jump State") return dt_uint8;
  if (propertyName == "Door 1 State") return dt_uint8;
  if (propertyName == "Door 2 State") return dt_uint8;
  if (propertyName == "Door 3 State") return dt_uint8;
  if (propertyName == "Door 4 State") return dt_uint8;
  if (propertyName == "Bullet Count") return dt_uint8;
  if (propertyName == "Custom Value") return dt_uint8;
  if (propertyName == "Samus HP 1") return dt_uint8;
  if (propertyName == "Samus HP 2") return dt_uint8;

  EXIT_WITH_ERROR("[Error] Rule %lu, unrecognized property: %s\n", _label, propertyName.c_str());

  return dt_uint8;
}

void* GameRule::getPropertyPointer(const nlohmann::json& condition, GameInstance* gameInstance)
{
  std::string propertyName = condition["Property"].get<std::string>();

  if (propertyName == "Equipment Flags") return gameInstance->equipmentFlags;
  if (propertyName == "Samus Pos Y Raw") return gameInstance->samusPosYRaw;
  if (propertyName == "Screen Pos Y2") return gameInstance->screenPosY2;
  if (propertyName == "Screen Pos Y1") return gameInstance->screenPosY1;
  if (propertyName == "Samus Pos X Raw") return gameInstance->samusPosXRaw;
  if (propertyName == "Screen Pos X2") return gameInstance->screenPosX2;
  if (propertyName == "Screen Pos X1") return gameInstance->screenPosX1;
  if (propertyName == "Samus Pos X") return &gameInstance->samusPosX;
  if (propertyName == "Samus Pos Y") return &gameInstance->samusPosY;
  if (propertyName == "Samus Door State") return gameInstance->samusDoorState;
  if (propertyName == "Samus Jump State") return gameInstance->samusJumpState;
  if (propertyName == "Door 1 State") return gameInstance->door1State;
  if (propertyName == "Door 2 State") return gameInstance->door2State;
  if (propertyName == "Door 3 State") return gameInstance->door3State;
  if (propertyName == "Door 4 State") return gameInstance->door4State;
  if (propertyName == "Bullet Count") return &gameInstance->bulletCount;
  if (propertyName == "Custom Value") return &gameInstance->customValue;
  if (propertyName == "Samus HP 1") return gameInstance->samusHP1;
  if (propertyName == "Samus HP 2") return gameInstance->samusHP2;

  EXIT_WITH_ERROR("[Error] Rule %lu, unrecognized property: %s\n", _label, propertyName.c_str());

  return NULL;
}
