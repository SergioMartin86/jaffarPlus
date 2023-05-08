#include "gameRule.hpp"

GameRule::GameRule() : Rule()
{
}

bool GameRule::parseGameAction(nlohmann::json actionJs, size_t actionId)
{
  bool recognizedActionType = false;
  std::string actionType = actionJs["Type"].get<std::string>();

  if (actionType == "Set Alien Horizontal Magnet")
  {
   if (isDefined(actionJs, "Intensity") == false) EXIT_WITH_ERROR("[ERROR] Magnet in Rule %lu Action %lu missing 'Intensity' key.\n", _label, actionId);
   if (isDefined(actionJs, "Room") == false) EXIT_WITH_ERROR("[ERROR] Magnet in Rule %lu Action %lu missing 'Room' key.\n", _label, actionId);
   if (isDefined(actionJs, "Center") == false) EXIT_WITH_ERROR("[ERROR] Magnet in Rule %lu Action %lu missing 'Center' key.\n", _label, actionId);
   uint8_t room = actionJs["Room"].get<uint8_t>();
   _magnets[room].alienHorizontalMagnet = genericMagnet_t { .intensity = actionJs["Intensity"].get<float>(), .center= actionJs["Center"].get<float>(), .active = true };
    recognizedActionType = true;
  }

  if (actionType == "Set Lester Horizontal Magnet")
   {
    if (isDefined(actionJs, "Intensity") == false) EXIT_WITH_ERROR("[ERROR] Magnet in Rule %lu Action %lu missing 'Intensity' key.\n", _label, actionId);
    if (isDefined(actionJs, "Room") == false) EXIT_WITH_ERROR("[ERROR] Magnet in Rule %lu Action %lu missing 'Room' key.\n", _label, actionId);
    if (isDefined(actionJs, "Center") == false) EXIT_WITH_ERROR("[ERROR] Magnet in Rule %lu Action %lu missing 'Center' key.\n", _label, actionId);
    uint8_t room = actionJs["Room"].get<uint8_t>();
    _magnets[room].lesterHorizontalMagnet = genericMagnet_t { .intensity = actionJs["Intensity"].get<float>(), .center= actionJs["Center"].get<float>(), .active = true };
     recognizedActionType = true;
   }

   if (actionType == "Set Lester Vertical Magnet")
   {
    if (isDefined(actionJs, "Intensity") == false) EXIT_WITH_ERROR("[ERROR] Magnet in Rule %lu Action %lu missing 'Intensity' key.\n", _label, actionId);
    if (isDefined(actionJs, "Room") == false) EXIT_WITH_ERROR("[ERROR] Magnet in Rule %lu Action %lu missing 'Room' key.\n", _label, actionId);
    if (isDefined(actionJs, "Center") == false) EXIT_WITH_ERROR("[ERROR] Magnet in Rule %lu Action %lu missing 'Center' key.\n", _label, actionId);
    uint8_t room = actionJs["Room"].get<uint8_t>();
    _magnets[room].lesterVerticalMagnet = genericMagnet_t { .intensity = actionJs["Intensity"].get<float>(), .center= actionJs["Center"].get<float>(), .active = true };
    recognizedActionType = true;
   }

   if (actionType == "Set Elevator Vertical Magnet")
   {
    if (isDefined(actionJs, "Intensity") == false) EXIT_WITH_ERROR("[ERROR] Magnet in Rule %lu Action %lu missing 'Intensity' key.\n", _label, actionId);
    if (isDefined(actionJs, "Room") == false) EXIT_WITH_ERROR("[ERROR] Magnet in Rule %lu Action %lu missing 'Room' key.\n", _label, actionId);
    if (isDefined(actionJs, "Center") == false) EXIT_WITH_ERROR("[ERROR] Magnet in Rule %lu Action %lu missing 'Center' key.\n", _label, actionId);
    uint8_t room = actionJs["Room"].get<uint8_t>();
    _magnets[room].elevatorVerticalMagnet = genericMagnet_t { .intensity = actionJs["Intensity"].get<float>(), .center= actionJs["Center"].get<float>(), .active = true };
    recognizedActionType = true;
   }

   if (actionType == "Set Gun Charge Magnet")
   {
    if (isDefined(actionJs, "Intensity") == false) EXIT_WITH_ERROR("[ERROR] Magnet in Rule %lu Action %lu missing 'Intensity' key.\n", _label, actionId);
    if (isDefined(actionJs, "Room") == false) EXIT_WITH_ERROR("[ERROR] Magnet in Rule %lu Action %lu missing 'Room' key.\n", _label, actionId);
    uint8_t room = actionJs["Room"].get<uint8_t>();
    _magnets[room].gunChargeMagnet = actionJs["Intensity"].get<float>();
    recognizedActionType = true;
   }


   if (actionType == "Set Lester Gun Load Magnet")
   {
    if (isDefined(actionJs, "Intensity") == false) EXIT_WITH_ERROR("[ERROR] Magnet in Rule %lu Action %lu missing 'Intensity' key.\n", _label, actionId);
    if (isDefined(actionJs, "Room") == false) EXIT_WITH_ERROR("[ERROR] Magnet in Rule %lu Action %lu missing 'Room' key.\n", _label, actionId);
    uint8_t room = actionJs["Room"].get<uint8_t>();
    _magnets[room].lesterGunLoadMagnet = actionJs["Intensity"].get<float>();
    recognizedActionType = true;
   }

   if (actionType == "Set Shield 1 Horizontal Magnet")
   {
    if (isDefined(actionJs, "Intensity") == false) EXIT_WITH_ERROR("[ERROR] Magnet in Rule %lu Action %lu missing 'Intensity' key.\n", _label, actionId);
    if (isDefined(actionJs, "Room") == false) EXIT_WITH_ERROR("[ERROR] Magnet in Rule %lu Action %lu missing 'Room' key.\n", _label, actionId);
    uint8_t room = actionJs["Room"].get<uint8_t>();
    _magnets[room].shield1HorizontalMagnet = actionJs["Intensity"].get<float>();
    recognizedActionType = true;
   }

   if (actionType == "Set Lester Angular Momentum Magnet")
   {
    if (isDefined(actionJs, "Intensity") == false) EXIT_WITH_ERROR("[ERROR] Magnet in Rule %lu Action %lu missing 'Intensity' key.\n", _label, actionId);
    if (isDefined(actionJs, "Room") == false) EXIT_WITH_ERROR("[ERROR] Magnet in Rule %lu Action %lu missing 'Room' key.\n", _label, actionId);
    uint8_t room = actionJs["Room"].get<uint8_t>();
    _magnets[room].lesterAngularMomentumMagnet = actionJs["Intensity"].get<float>();
    recognizedActionType = true;
   }

   if (actionType == "Set Lester Direction Magnet")
   {
    if (isDefined(actionJs, "Intensity") == false) EXIT_WITH_ERROR("[ERROR] Magnet in Rule %lu Action %lu missing 'Intensity' key.\n", _label, actionId);
    if (isDefined(actionJs, "Room") == false) EXIT_WITH_ERROR("[ERROR] Magnet in Rule %lu Action %lu missing 'Room' key.\n", _label, actionId);
    uint8_t room = actionJs["Room"].get<uint8_t>();
    _magnets[room].lesterDirectionMagnet = actionJs["Intensity"].get<float>();
    recognizedActionType = true;
   }

  return recognizedActionType;
}

datatype_t GameRule::getPropertyType(const nlohmann::json& condition)
{
  std::string propertyName = condition["Property"].get<std::string>();

  if (propertyName == "Lester Room") return dt_int16;
  if (propertyName == "Lester Next Room") return dt_int16;
  if (propertyName == "Lester Pos Y") return dt_int16;
  if (propertyName == "Lester Pos X") return dt_int16;
  if (propertyName == "Lester State") return dt_int16;
  if (propertyName == "Lester Dead State") return dt_int16;
  if (propertyName == "Game Script State") return dt_int16;
  if (propertyName == "Game Anim State") return dt_int16;
  if (propertyName == "VM Value") return dt_int16;
  if (propertyName == "Lester Has Gun") return dt_int16;
  if (propertyName == "Lester Gun Ammo") return dt_int16;
  if (propertyName == "Lester Gun Load") return dt_int16;
  if (propertyName == "Alien State") return dt_int16;
  if (propertyName == "Alien Room") return dt_int16;
  if (propertyName == "Alien Pos X") return dt_int16;
  if (propertyName == "Elevator Pos Y") return dt_int16;

  EXIT_WITH_ERROR("[Error] Rule %lu, unrecognized property: %s\n", _label, propertyName.c_str());

  return dt_uint8;
}

void* GameRule::getPropertyPointer(const nlohmann::json& condition, GameInstance* gameInstance)
{
  std::string propertyName = condition["Property"].get<std::string>();

  if (propertyName == "Lester Room") return gameInstance->lesterRoom;
  if (propertyName == "Lester Next Room") return gameInstance->lesterNextRoom;
  if (propertyName == "Lester Pos Y") return gameInstance->lesterPosY;
  if (propertyName == "Lester Pos X") return gameInstance->lesterPosX;
  if (propertyName == "Lester State") return gameInstance->lesterState;
  if (propertyName == "Lester Dead State") return gameInstance->lesterDeadState;
  if (propertyName == "Game Script State") return gameInstance->gameScriptState;
  if (propertyName == "Game Anim State") return gameInstance->gameAnimState;

  if (propertyName == "Lester Has Gun") return gameInstance->lesterHasGun;
  if (propertyName == "Lester Gun Ammo") return gameInstance->lesterGunAmmo;
  if (propertyName == "Lester Gun Load") return gameInstance->lesterGunLoad;

  if (propertyName == "Alien State") return gameInstance->alienState;
  if (propertyName == "Alien Room") return gameInstance->alienRoom;
  if (propertyName == "Alien Pos X") return gameInstance->alienPosX;
  if (propertyName == "Elevator Pos Y") return gameInstance->elevatorPosY;

  int index = -1;
  if (isDefined(condition, "Index") == true)
  {
   if (condition["Index"].is_number() == false) EXIT_WITH_ERROR("[ERROR] Rule %lu Index must be an integer.\n", _label);
   index = condition["Index"].get<int>();
  }

  if (propertyName == "VM Value")
  {
   gameInstance->watchVMVariables.insert(index);
   return &gameInstance->vmVariables[index];
  }

  EXIT_WITH_ERROR("[Error] Rule %lu, unrecognized property: %s\n", _label, propertyName.c_str());

  return NULL;
}
