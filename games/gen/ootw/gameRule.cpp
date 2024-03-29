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

  if (actionType == "Set Alien Horizontal Magnet")
  {
   if (isDefined(actionJs, "Intensity") == false) EXIT_WITH_ERROR("[ERROR] Magnet in Rule %lu Action %lu missing 'Intensity' key.\n", _label, actionId);
   if (isDefined(actionJs, "Center") == false) EXIT_WITH_ERROR("[ERROR] Magnet in Rule %lu Action %lu missing 'Center' key.\n", _label, actionId);
   if (isDefined(actionJs, "Room") == false) EXIT_WITH_ERROR("[ERROR] Magnet in Rule %lu Action %lu missing 'Room' key.\n", _label, actionId);
   if (isDefined(actionJs, "Min") == false) EXIT_WITH_ERROR("[ERROR] Magnet in Rule %lu Action %lu missing 'Min' key.\n", _label, actionId);
   if (isDefined(actionJs, "Max") == false) EXIT_WITH_ERROR("[ERROR] Magnet in Rule %lu Action %lu missing 'Max' key.\n", _label, actionId);
   _magnets.alienHorizontalMagnet[actionJs["Room"].get<uint8_t>()] = genericMagnet_t { .intensity = actionJs["Intensity"].get<float>(), .center= actionJs["Center"].get<float>(), .min = actionJs["Min"].get<float>(), .max = actionJs["Max"].get<float>() };
    recognizedActionType = true;
  }

  if (actionType == "Set Gun Charge Magnet")
  {
   if (isDefined(actionJs, "Intensity") == false) EXIT_WITH_ERROR("[ERROR] Magnet in Rule %lu Action %lu missing 'Intensity' key.\n", _label, actionId);
   _magnets.gunChargeMagnet = actionJs["Intensity"].get<float>();
   recognizedActionType = true;
  }


  if (actionType == "Set Lester Gun Power Load Magnet")
  {
   if (isDefined(actionJs, "Intensity") == false) EXIT_WITH_ERROR("[ERROR] Magnet in Rule %lu Action %lu missing 'Intensity' key.\n", _label, actionId);
   _magnets.gunPowerLoadMagnet = actionJs["Intensity"].get<float>();
   recognizedActionType = true;
  }

  if (actionType == "Set Shield 1 Horizontal Magnet")
  {
   if (isDefined(actionJs, "Intensity") == false) EXIT_WITH_ERROR("[ERROR] Magnet in Rule %lu Action %lu missing 'Intensity' key.\n", _label, actionId);
   _magnets.shield1HorizontalMagnet = actionJs["Intensity"].get<float>();
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
  if (propertyName == "Current Stage") return dt_uint8;
  if (propertyName == "Lester Position X") return dt_int16;
  if (propertyName == "Lester Absolute Position X") return dt_uint16;
  if (propertyName == "Lester Position Y") return dt_int16;
  if (propertyName == "Lester Room") return dt_uint8;
  if (propertyName == "Lester Next Room") return dt_uint8;
  if (propertyName == "Lester Frame") return dt_uint16;
  if (propertyName == "Lester Has Gun") return dt_int16;
  if (propertyName == "Lester Gun Charge") return dt_int16;
  if (propertyName == "Lester Dead Flag") return dt_uint8;
  if (propertyName == "Lester Gun Charge") return dt_int16;
  if (propertyName == "Lester Gun Power Load") return dt_uint8;
  if (propertyName == "Lift Status") return dt_int16;

  if (propertyName == "Shield 1 Pos X") return dt_int16;
  if (propertyName == "Shield 1 Duration") return dt_int16;
  if (propertyName == "Shield 2 Duration") return dt_int16;
  if (propertyName == "Shield 3 Duration") return dt_int16;
  if (propertyName == "Shield 4 Duration") return dt_int16;
  if (propertyName == "Shield 5 Duration") return dt_int16;

  if (propertyName == "Alien Dead Flag") return dt_uint8;
  if (propertyName == "Alien Position X") return dt_int16;
  if (propertyName == "Alien Room") return dt_uint8;

  if (propertyName == "Stage 01 Appear Time") return dt_uint8;
  if (propertyName == "Stage 01 Vine State") return dt_int16;
  if (propertyName == "Stage 01 Skip Monster Flag") return dt_int8;
  if (propertyName == "Stage 01 Finish") return dt_uint8;

  if (propertyName == "Stage 02 Break Door State") return dt_uint8;

  if (propertyName == "Stage 31 TriDoor State") return dt_uint16;
  if (propertyName == "Stage 31 Wall State") return dt_uint16;

  if (propertyName == "Stage 33 Pool Wall State") return dt_uint16;
  if (propertyName == "Stage 33 Bat Active") return dt_uint16;
  if (propertyName == "Stage 33 Big Rock State") return dt_uint16;
  if (propertyName == "Stage 33 Room 204 Vine 1 Pos Y") return dt_uint16;
  if (propertyName == "Stage 33 Water Wall") return dt_uint16;
  if (propertyName == "Stage 33 Water Push") return dt_uint16;

  if (propertyName == "Stage 37 Progress State") return dt_uint16;

  if (propertyName == "Stage 64 Progress State") return dt_uint16;
  if (propertyName == "Stage 64 Enemy 1 Dead State") return dt_uint16;
  if (propertyName == "Stage 64 Enemy 2 Dead State") return dt_uint16;
  if (propertyName == "Stage 64 Guard Door State") return dt_uint16;

  if (propertyName == "Stage 50 Panel Progress") return dt_uint16;
  if (propertyName == "Stage 50 Button 1") return dt_uint16;
  if (propertyName == "Stage 50 Button 2") return dt_uint16;
  if (propertyName == "Stage 50 Button 3") return dt_uint16;
  if (propertyName == "Stage 50 Button 4") return dt_uint16;
  if (propertyName == "Stage 50 Escape") return dt_uint16;

  EXIT_WITH_ERROR("[Error] Rule %lu, unrecognized property: %s\n", _label, propertyName.c_str());

  return dt_uint8;
}

void* GameRule::getPropertyPointer(const nlohmann::json& condition, GameInstance* gameInstance)
{
  std::string propertyName = condition["Property"].get<std::string>();

  if (propertyName == "Game Timer") return gameInstance->gameTimer;
  if (propertyName == "Animation Frame") return gameInstance->animationFrame;
  if (propertyName == "Game Mode") return gameInstance->gameMode;
  if (propertyName == "Current Stage") return gameInstance->currentStage;
  if (propertyName == "Lester Position X") return gameInstance->lesterPosX;
  if (propertyName == "Lester Absolute Position X") return &gameInstance->lesterAbsolutePosX;
  if (propertyName == "Lester Position Y") return gameInstance->lesterPosY;
  if (propertyName == "Lester Room") return gameInstance->lesterRoom;
  if (propertyName == "Lester Next Room") return gameInstance->lesterNextRoom;
  if (propertyName == "Lester Frame") return gameInstance->lesterFrame;
  if (propertyName == "Lester Has Gun") return gameInstance->lesterHasGun;
  if (propertyName == "Lester Gun Charge") return gameInstance->lesterGunCharge;
  if (propertyName == "Lester Gun Power Load") return gameInstance->lesterGunPowerLoad;
  if (propertyName == "Lester Dead Flag") return gameInstance->lesterDeadFlag;
  if (propertyName == "Lift Status") return gameInstance->liftStatus;

  if (propertyName == "Shield 1 Pos X") return gameInstance->shield1PosX;
  if (propertyName == "Shield 1 Duration") return gameInstance->shield1Duration;
  if (propertyName == "Shield 2 Duration") return gameInstance->shield1Duration;
  if (propertyName == "Shield 3 Duration") return gameInstance->shield1Duration;
  if (propertyName == "Shield 4 Duration") return gameInstance->shield1Duration;
  if (propertyName == "Shield 5 Duration") return gameInstance->shield1Duration;

  if (propertyName == "Alien Dead Flag") return gameInstance->alienDeadFlag;
  if (propertyName == "Alien Position X") return gameInstance->alienPosX;
  if (propertyName == "Alien Room") return gameInstance->alienRoom;

  if (propertyName == "Stage 01 Appear Time") return gameInstance->stage01AppearTimer;
  if (propertyName == "Stage 01 Vine State") return gameInstance->stage01VineState;
  if (propertyName == "Stage 01 Skip Monster Flag") return gameInstance->stage01SkipMonsterFlag;
  if (propertyName == "Stage 01 Finish") return gameInstance->stage01Finish;

  if (propertyName == "Stage 02 Break Door State") return gameInstance->stage02BreakDoorState;

  if (propertyName == "Stage 31 TriDoor State") return gameInstance->stage31TriDoorState;
  if (propertyName == "Stage 31 Wall State") return gameInstance->stage31WallState;

  if (propertyName == "Stage 33 Pool Wall State") return gameInstance->stage33PoolWallState;
  if (propertyName == "Stage 33 Bat Active") return gameInstance->stage33BatActive;
  if (propertyName == "Stage 33 Big Rock State") return gameInstance->stage33BigRockState;
  if (propertyName == "Stage 33 Room 204 Vine 1 Pos Y") return gameInstance->stage33Room204Vine1PosY;
  if (propertyName == "Stage 33 Water Wall") return gameInstance->stage33WaterWall;
  if (propertyName == "Stage 33 Water Push") return gameInstance->stage33WaterPush;

  if (propertyName == "Stage 37 Progress State") return gameInstance->stage37ProgressState;

  if (propertyName == "Stage 64 Progress State") return gameInstance->stage64ProgressState;
  if (propertyName == "Stage 64 Enemy 1 Dead State") return gameInstance->stage64Enemy1DeadState;
  if (propertyName == "Stage 64 Enemy 2 Dead State") return gameInstance->stage64Enemy2DeadState;
  if (propertyName == "Stage 64 Guard Door State") return gameInstance->stage64GuardDoorState;

  if (propertyName == "Stage 50 Panel Progress") return gameInstance->stage50PanelProgress;
  if (propertyName == "Stage 50 Button 1") return gameInstance->stage50Button1;
  if (propertyName == "Stage 50 Button 2") return gameInstance->stage50Button2;
  if (propertyName == "Stage 50 Button 3") return gameInstance->stage50Button3;
  if (propertyName == "Stage 50 Button 4") return gameInstance->stage50Button4;
  if (propertyName == "Stage 50 Escape")  return gameInstance->stage50Escape;

  EXIT_WITH_ERROR("[Error] Rule %lu, unrecognized property: %s\n", _label, propertyName.c_str());

  return NULL;
}
