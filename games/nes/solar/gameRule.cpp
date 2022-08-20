#include "gameRule.hpp"

GameRule::GameRule() : Rule()
{
}

bool GameRule::parseGameAction(nlohmann::json actionJs, size_t actionId)
{
  bool recognizedActionType = false;
  std::string actionType = actionJs["Type"].get<std::string>();

  if (actionType == "Set Ship Horizontal Magnet")
   {
    if (isDefined(actionJs, "Intensity") == false) EXIT_WITH_ERROR("[ERROR] Magnet in Rule %lu Action %lu missing 'Intensity' key.\n", _label, actionId);
    if (isDefined(actionJs, "Center") == false) EXIT_WITH_ERROR("[ERROR] Magnet in Rule %lu Action %lu missing 'Center' key.\n", _label, actionId);
    if (isDefined(actionJs, "Min") == false) EXIT_WITH_ERROR("[ERROR] Magnet in Rule %lu Action %lu missing 'Min' key.\n", _label, actionId);
    if (isDefined(actionJs, "Max") == false) EXIT_WITH_ERROR("[ERROR] Magnet in Rule %lu Action %lu missing 'Max' key.\n", _label, actionId);
    _magnets.shipHorizontalMagnet = genericMagnet_t { .intensity = actionJs["Intensity"].get<float>(), .center= actionJs["Center"].get<float>(), .min = actionJs["Min"].get<float>(), .max = actionJs["Max"].get<float>() };
     recognizedActionType = true;
   }

   if (actionType == "Set Ship Vertical Magnet")
   {
    if (isDefined(actionJs, "Intensity") == false) EXIT_WITH_ERROR("[ERROR] Magnet in Rule %lu Action %lu missing 'Intensity' key.\n", _label, actionId);
    if (isDefined(actionJs, "Min") == false) EXIT_WITH_ERROR("[ERROR] Magnet in Rule %lu Action %lu missing 'Min' key.\n", _label, actionId);
    if (isDefined(actionJs, "Max") == false) EXIT_WITH_ERROR("[ERROR] Magnet in Rule %lu Action %lu missing 'Max' key.\n", _label, actionId);
    _magnets.shipVerticalMagnet = genericMagnet_t { .intensity = actionJs["Intensity"].get<float>(), .center= actionJs["Center"].get<float>(), .min = actionJs["Min"].get<float>(), .max = actionJs["Max"].get<float>() };
    recognizedActionType = true;
   }

   if (actionType == "Set Ship Vel X Magnet")
    {
     if (isDefined(actionJs, "Intensity") == false) EXIT_WITH_ERROR("[ERROR] Magnet in Rule %lu Action %lu missing 'Intensity' key.\n", _label, actionId);
     if (isDefined(actionJs, "Center") == false) EXIT_WITH_ERROR("[ERROR] Magnet in Rule %lu Action %lu missing 'Center' key.\n", _label, actionId);
     if (isDefined(actionJs, "Min") == false) EXIT_WITH_ERROR("[ERROR] Magnet in Rule %lu Action %lu missing 'Min' key.\n", _label, actionId);
     if (isDefined(actionJs, "Max") == false) EXIT_WITH_ERROR("[ERROR] Magnet in Rule %lu Action %lu missing 'Max' key.\n", _label, actionId);
     _magnets.shipVelXMagnet = genericMagnet_t { .intensity = actionJs["Intensity"].get<float>(), .center= actionJs["Center"].get<float>(), .min = actionJs["Min"].get<float>(), .max = actionJs["Max"].get<float>() };
      recognizedActionType = true;
    }

    if (actionType == "Set Ship Vel Y Magnet")
    {
     if (isDefined(actionJs, "Intensity") == false) EXIT_WITH_ERROR("[ERROR] Magnet in Rule %lu Action %lu missing 'Intensity' key.\n", _label, actionId);
     if (isDefined(actionJs, "Min") == false) EXIT_WITH_ERROR("[ERROR] Magnet in Rule %lu Action %lu missing 'Min' key.\n", _label, actionId);
     if (isDefined(actionJs, "Max") == false) EXIT_WITH_ERROR("[ERROR] Magnet in Rule %lu Action %lu missing 'Max' key.\n", _label, actionId);
     _magnets.shipVelYMagnet = genericMagnet_t { .intensity = actionJs["Intensity"].get<float>(), .center= actionJs["Center"].get<float>(), .min = actionJs["Min"].get<float>(), .max = actionJs["Max"].get<float>() };
     recognizedActionType = true;
    }

   if (actionType == "Set Ship Health Magnet")
   {
    if (isDefined(actionJs, "Intensity") == false) EXIT_WITH_ERROR("[ERROR] Ship Health Magnet in Rule %lu Action %lu missing 'Intensity' key.\n", _label, actionId);
    _magnets.shipHealthMagnet = actionJs["Intensity"].get<float>();
    recognizedActionType = true;
   }

   if (actionType == "Set Score Magnet")
   {
    if (isDefined(actionJs, "Intensity") == false) EXIT_WITH_ERROR("[ERROR] Score Magnet in Rule %lu Action %lu missing 'Intensity' key.\n", _label, actionId);
    _magnets.scoreMagnet = actionJs["Intensity"].get<float>();
    recognizedActionType = true;
   }

   if (actionType == "Set Warp Counter Magnet")
   {
    if (isDefined(actionJs, "Intensity") == false) EXIT_WITH_ERROR("[ERROR] Warp Counter Magnet in Rule %lu Action %lu missing 'Intensity' key.\n", _label, actionId);
    _magnets.warpCounterMagnet = actionJs["Intensity"].get<float>();
    recognizedActionType = true;
   }

   if (actionType == "Set Carry Magnet")
   {
    if (isDefined(actionJs, "Intensity") == false) EXIT_WITH_ERROR("[ERROR] Carry Magnet in Rule %lu Action %lu missing 'Intensity' key.\n", _label, actionId);
    _magnets.carryMagnet = actionJs["Intensity"].get<float>();
    recognizedActionType = true;
   }

   if (actionType == "Set Max Warp Magnet")
   {
    if (isDefined(actionJs, "Intensity") == false) EXIT_WITH_ERROR("[ERROR] Max Warp Magnet in Rule %lu Action %lu missing 'Intensity' key.\n", _label, actionId);
    _magnets.maxWarpMagnet = actionJs["Intensity"].get<float>();
    recognizedActionType = true;
   }

  return recognizedActionType;
}

datatype_t GameRule::getPropertyType(const nlohmann::json& condition)
{
  std::string propertyName = condition["Property"].get<std::string>();

  if (propertyName == "Ship Pos X") return dt_float;
  if (propertyName == "Ship Pos Y") return dt_float;
  if (propertyName == "Ship Health") return dt_float;
  if (propertyName == "Warp Counter") return dt_float;
  if (propertyName == "Ship Shields") return dt_uint8;
  if (propertyName == "Current Stage") return dt_uint8;
  if (propertyName == "Max Warp") return dt_uint8;
  if (propertyName == "Fuel Delivered") return dt_uint8;

  EXIT_WITH_ERROR("[Error] Rule %lu, unrecognized property: %s\n", _label, propertyName.c_str());

  return dt_uint8;
}

void* GameRule::getPropertyPointer(const nlohmann::json& condition, GameInstance* gameInstance)
{
  std::string propertyName = condition["Property"].get<std::string>();

  if (propertyName == "Ship Pos X") return &gameInstance->shipPosX;
  if (propertyName == "Ship Pos Y") return &gameInstance->shipPosY;
  if (propertyName == "Ship Health") return &gameInstance->shipHealth;
  if (propertyName == "Ship Shields") return gameInstance->shipShields;
  if (propertyName == "Warp Counter") return &gameInstance->warpCounter;
  if (propertyName == "Current Stage") return gameInstance->currentStage;
  if (propertyName == "Max Warp") return &gameInstance->maxWarp;
  if (propertyName == "Fuel Delivered") return gameInstance->fuelDelivered;

  EXIT_WITH_ERROR("[Error] Rule %lu, unrecognized property: %s\n", _label, propertyName.c_str());

  return NULL;
}
