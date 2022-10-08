#include "gameRule.hpp"

GameRule::GameRule() : Rule()
{
}

bool GameRule::parseGameAction(nlohmann::json actionJs, size_t actionId)
{
  bool recognizedActionType = false;
  std::string actionType = actionJs["Type"].get<std::string>();

  if (actionType == "Set Screen Scroll Magnet")
   {
    if (isDefined(actionJs, "Intensity") == false) EXIT_WITH_ERROR("[ERROR] Magnet in Rule %lu Action %lu missing 'Intensity' key.\n", _label, actionId);
    if (isDefined(actionJs, "Center") == false) EXIT_WITH_ERROR("[ERROR] Magnet in Rule %lu Action %lu missing 'Center' key.\n", _label, actionId);
    if (isDefined(actionJs, "Min") == false) EXIT_WITH_ERROR("[ERROR] Magnet in Rule %lu Action %lu missing 'Min' key.\n", _label, actionId);
    if (isDefined(actionJs, "Max") == false) EXIT_WITH_ERROR("[ERROR] Magnet in Rule %lu Action %lu missing 'Max' key.\n", _label, actionId);
    _magnets.screenScrollMagnet = genericMagnet_t { .intensity = actionJs["Intensity"].get<float>(), .center= actionJs["Center"].get<float>(), .min = actionJs["Min"].get<float>(), .max = actionJs["Max"].get<float>() };
     recognizedActionType = true;
   }


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

   if (actionType == "Set Eye 1 Health Magnet")
   {
    if (isDefined(actionJs, "Intensity") == false) EXIT_WITH_ERROR("[ERROR] Magnet in Rule %lu Action %lu missing 'Intensity' key.\n", _label, actionId);
    _magnets.eye1HealthMagnet = actionJs["Intensity"].get<float>();
    recognizedActionType = true;
   }

   if (actionType == "Set Eye 2 Health Magnet")
   {
    if (isDefined(actionJs, "Intensity") == false) EXIT_WITH_ERROR("[ERROR] Magnet in Rule %lu Action %lu missing 'Intensity' key.\n", _label, actionId);
    _magnets.eye2HealthMagnet = actionJs["Intensity"].get<float>();
    recognizedActionType = true;
   }

   if (actionType == "Set Eye 3 Health Magnet")
   {
    if (isDefined(actionJs, "Intensity") == false) EXIT_WITH_ERROR("[ERROR] Magnet in Rule %lu Action %lu missing 'Intensity' key.\n", _label, actionId);
    _magnets.eye3HealthMagnet = actionJs["Intensity"].get<float>();
    recognizedActionType = true;
   }

   if (actionType == "Set Eye 4 Health Magnet")
   {
    if (isDefined(actionJs, "Intensity") == false) EXIT_WITH_ERROR("[ERROR] Magnet in Rule %lu Action %lu missing 'Intensity' key.\n", _label, actionId);
    _magnets.eye4HealthMagnet = actionJs["Intensity"].get<float>();
    recognizedActionType = true;
   }

   if (actionType == "Set Eye 1 Readiness Magnet")
   {
    if (isDefined(actionJs, "Intensity") == false) EXIT_WITH_ERROR("[ERROR] Magnet in Rule %lu Action %lu missing 'Intensity' key.\n", _label, actionId);
    _magnets.eye1ReadinessMagnet = actionJs["Intensity"].get<float>();
    recognizedActionType = true;
   }

   if (actionType == "Set Eye 2 Readiness Magnet")
   {
    if (isDefined(actionJs, "Intensity") == false) EXIT_WITH_ERROR("[ERROR] Magnet in Rule %lu Action %lu missing 'Intensity' key.\n", _label, actionId);
    _magnets.eye2ReadinessMagnet = actionJs["Intensity"].get<float>();
    recognizedActionType = true;
   }

   if (actionType == "Set Eye 3 Readiness Magnet")
   {
    if (isDefined(actionJs, "Intensity") == false) EXIT_WITH_ERROR("[ERROR] Magnet in Rule %lu Action %lu missing 'Intensity' key.\n", _label, actionId);
    _magnets.eye3ReadinessMagnet = actionJs["Intensity"].get<float>();
    recognizedActionType = true;
   }

   if (actionType == "Set Eye 4 Readiness Magnet")
   {
    if (isDefined(actionJs, "Intensity") == false) EXIT_WITH_ERROR("[ERROR] Magnet in Rule %lu Action %lu missing 'Intensity' key.\n", _label, actionId);
    _magnets.eye4ReadinessMagnet = actionJs["Intensity"].get<float>();
    recognizedActionType = true;
   }

   if (actionType == "Set Shots Active Magnet")
   {
    if (isDefined(actionJs, "Intensity") == false) EXIT_WITH_ERROR("[ERROR] Magnet in Rule %lu Action %lu missing 'Intensity' key.\n", _label, actionId);
    _magnets.shotsActiveMagnet = actionJs["Intensity"].get<float>();
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
  if (propertyName == "Eye Count") return dt_uint8;
  if (propertyName == "Eye 0 State") return dt_uint8;
  if (propertyName == "Eye 1 State") return dt_uint8;
  if (propertyName == "Eye 2 State") return dt_uint8;
  if (propertyName == "Eye 3 State") return dt_uint8;
  if (propertyName == "Eye 4 State") return dt_uint8;
  if (propertyName == "Eye 1 Health") return dt_uint8;
  if (propertyName == "Eye 2 Health") return dt_uint8;
  if (propertyName == "Eye 3 Health") return dt_uint8;
  if (propertyName == "Eye 4 Health") return dt_uint8;
  if (propertyName == "Eye 1 Aperture") return dt_uint8;
  if (propertyName == "Eye 2 Aperture") return dt_uint8;
  if (propertyName == "Eye 3 Aperture") return dt_uint8;
  if (propertyName == "Eye 4 Aperture") return dt_uint8;
  if (propertyName == "Eye 1 Timer") return dt_uint8;
  if (propertyName == "Eye 2 Timer") return dt_uint8;
  if (propertyName == "Eye 3 Timer") return dt_uint8;
  if (propertyName == "Eye 4 Timer") return dt_uint8;

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
  if (propertyName == "Eye Count") return &gameInstance->eyeCount;

  if (propertyName == "Eye 0 State") return gameInstance->eye0State;
  if (propertyName == "Eye 1 State") return gameInstance->eye1State;
  if (propertyName == "Eye 2 State") return gameInstance->eye2State;
  if (propertyName == "Eye 3 State") return gameInstance->eye3State;
  if (propertyName == "Eye 4 State") return gameInstance->eye4State;
  if (propertyName == "Eye 1 Health") return gameInstance->eye1Health;
  if (propertyName == "Eye 2 Health") return gameInstance->eye2Health;
  if (propertyName == "Eye 3 Health") return gameInstance->eye3Health;
  if (propertyName == "Eye 4 Health") return gameInstance->eye4Health;
  if (propertyName == "Eye 1 Aperture") return gameInstance->eye1Aperture;
  if (propertyName == "Eye 2 Aperture") return gameInstance->eye2Aperture;
  if (propertyName == "Eye 3 Aperture") return gameInstance->eye3Aperture;
  if (propertyName == "Eye 4 Aperture") return gameInstance->eye4Aperture;
  if (propertyName == "Eye 1 Timer") return gameInstance->eye1Timer;
  if (propertyName == "Eye 2 Timer") return gameInstance->eye2Timer;
  if (propertyName == "Eye 3 Timer") return gameInstance->eye3Timer;
  if (propertyName == "Eye 4 Timer") return gameInstance->eye4Timer;

  EXIT_WITH_ERROR("[Error] Rule %lu, unrecognized property: %s\n", _label, propertyName.c_str());

  return NULL;
}
