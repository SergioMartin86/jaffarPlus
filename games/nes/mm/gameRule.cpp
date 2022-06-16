#include "gameRule.hpp"

GameRule::GameRule() : Rule()
{
 _magnets.marbleXMagnet = genericMagnet_t { .intensity = 0.0f, .center = 0.0f, .min = 0.0f, .max = 0.0f };
 _magnets.marbleYMagnet = genericMagnet_t { .intensity = 0.0f, .center = 0.0f, .min = 0.0f, .max = 0.0f };
 _magnets.marbleZMagnet = genericMagnet_t { .intensity = 0.0f, .center = 0.0f, .min = 0.0f, .max = 0.0f };
}

bool GameRule::parseGameAction(nlohmann::json actionJs, size_t actionId)
{
  bool recognizedActionType = false;
  std::string actionType = actionJs["Type"].get<std::string>();

  if (actionType == "Set Marble X Magnet")
  {
   if (isDefined(actionJs, "Intensity") == false) EXIT_WITH_ERROR("[ERROR] Magnet in Rule %lu Action %lu missing 'Intensity' key.\n", _label, actionId);
   if (isDefined(actionJs, "Center") == false) EXIT_WITH_ERROR("[ERROR] Magnet in Rule %lu Action %lu missing 'Center' key.\n", _label, actionId);
   if (isDefined(actionJs, "Min") == false) EXIT_WITH_ERROR("[ERROR] Magnet in Rule %lu Action %lu missing 'Min' key.\n", _label, actionId);
   if (isDefined(actionJs, "Max") == false) EXIT_WITH_ERROR("[ERROR] Magnet in Rule %lu Action %lu missing 'Max' key.\n", _label, actionId);
   _magnets.marbleXMagnet = genericMagnet_t { .intensity = actionJs["Intensity"].get<float>(), .center= actionJs["Center"].get<float>(), .min = actionJs["Min"].get<float>(), .max = actionJs["Max"].get<float>() };
    recognizedActionType = true;
  }

  if (actionType == "Set Marble Y Magnet")
  {
   if (isDefined(actionJs, "Intensity") == false) EXIT_WITH_ERROR("[ERROR] Magnet in Rule %lu Action %lu missing 'Intensity' key.\n", _label, actionId);
   if (isDefined(actionJs, "Center") == false) EXIT_WITH_ERROR("[ERROR] Magnet in Rule %lu Action %lu missing 'Center' key.\n", _label, actionId);
   if (isDefined(actionJs, "Min") == false) EXIT_WITH_ERROR("[ERROR] Magnet in Rule %lu Action %lu missing 'Min' key.\n", _label, actionId);
   if (isDefined(actionJs, "Max") == false) EXIT_WITH_ERROR("[ERROR] Magnet in Rule %lu Action %lu missing 'Max' key.\n", _label, actionId);
   _magnets.marbleYMagnet = genericMagnet_t { .intensity = actionJs["Intensity"].get<float>(), .center= actionJs["Center"].get<float>(), .min = actionJs["Min"].get<float>(), .max = actionJs["Max"].get<float>() };
    recognizedActionType = true;
  }

  if (actionType == "Set Marble Z Magnet")
  {
   if (isDefined(actionJs, "Intensity") == false) EXIT_WITH_ERROR("[ERROR] Magnet in Rule %lu Action %lu missing 'Intensity' key.\n", _label, actionId);
   if (isDefined(actionJs, "Center") == false) EXIT_WITH_ERROR("[ERROR] Magnet in Rule %lu Action %lu missing 'Center' key.\n", _label, actionId);
   if (isDefined(actionJs, "Min") == false) EXIT_WITH_ERROR("[ERROR] Magnet in Rule %lu Action %lu missing 'Min' key.\n", _label, actionId);
   if (isDefined(actionJs, "Max") == false) EXIT_WITH_ERROR("[ERROR] Magnet in Rule %lu Action %lu missing 'Max' key.\n", _label, actionId);
   _magnets.marbleZMagnet = genericMagnet_t { .intensity = actionJs["Intensity"].get<float>(), .center= actionJs["Center"].get<float>(), .min = actionJs["Min"].get<float>(), .max = actionJs["Max"].get<float>() };
    recognizedActionType = true;
  }

  return recognizedActionType;
}

datatype_t GameRule::getPropertyType(const nlohmann::json& condition)
{
  std::string propertyName = condition["Property"].get<std::string>();

  if (propertyName == "Win Flag") return dt_uint8;
  if (propertyName == "Marble Position X") return dt_uint16;
  if (propertyName == "Marble Position Y") return dt_uint16;
  if (propertyName == "Marble Position Z") return dt_uint8;
  if (propertyName == "Marble Velocity X") return dt_int8;
  if (propertyName == "Marble Velocity Y") return dt_int8;
  if (propertyName == "Marble Dead Flag") return dt_uint8;
  if (propertyName == "Marble Flags") return dt_uint8;

  EXIT_WITH_ERROR("[Error] Rule %lu, unrecognized property: %s\n", _label, propertyName.c_str());

  return dt_uint8;
}

void* GameRule::getPropertyPointer(const nlohmann::json& condition, GameInstance* gameInstance)
{
  std::string propertyName = condition["Property"].get<std::string>();

  if (propertyName == "Win Flag") return gameInstance->winFlag;
  if (propertyName == "Marble Position X") return &gameInstance->marblePosX;
  if (propertyName == "Marble Position Y") return &gameInstance->marblePosY;
  if (propertyName == "Marble Position Z") return gameInstance->marblePosZ;
  if (propertyName == "Marble Velocity X") return gameInstance->marbleVelX;
  if (propertyName == "Marble Velocity Y") return gameInstance->marbleVelY;
  if (propertyName == "Marble Dead Flag") return gameInstance->marbleDeadFlag;
  if (propertyName == "Marble Flags") return gameInstance->marbleFlags;

  EXIT_WITH_ERROR("[Error] Rule %lu, unrecognized property: %s\n", _label, propertyName.c_str());

  return NULL;
}
