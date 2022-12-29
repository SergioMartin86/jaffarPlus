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

  return recognizedActionType;
}

datatype_t GameRule::getPropertyType(const nlohmann::json& condition)
{
  std::string propertyName = condition["Property"].get<std::string>();

  if (propertyName == "Equipment Flags") return dt_uint8;
  if (propertyName == "Samus Pos X") return dt_float;
  if (propertyName == "Samus Pos Y") return dt_float;
  if (propertyName == "Samus Door State") return dt_uint8;
  if (propertyName == "Door 1 State") return dt_uint8;
  if (propertyName == "Door 2 State") return dt_uint8;
  if (propertyName == "Door 3 State") return dt_uint8;
  if (propertyName == "Door 4 State") return dt_uint8;

  EXIT_WITH_ERROR("[Error] Rule %lu, unrecognized property: %s\n", _label, propertyName.c_str());

  return dt_uint8;
}

void* GameRule::getPropertyPointer(const nlohmann::json& condition, GameInstance* gameInstance)
{
  std::string propertyName = condition["Property"].get<std::string>();

  if (propertyName == "Equipment Flags") return gameInstance->equipmentFlags;
  if (propertyName == "Samus Pos X") return &gameInstance->samusPosX;
  if (propertyName == "Samus Pos Y") return &gameInstance->samusPosY;
  if (propertyName == "Samus Door State") return gameInstance->samusDoorState;
  if (propertyName == "Door 1 State") return gameInstance->door1State;
  if (propertyName == "Door 2 State") return gameInstance->door2State;
  if (propertyName == "Door 3 State") return gameInstance->door3State;
  if (propertyName == "Door 4 State") return gameInstance->door4State;

  EXIT_WITH_ERROR("[Error] Rule %lu, unrecognized property: %s\n", _label, propertyName.c_str());

  return NULL;
}
