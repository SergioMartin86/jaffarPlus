#include "gameRule.hpp"

GameRule::GameRule() : Rule()
{
}

bool GameRule::parseGameAction(nlohmann::json actionJs, size_t actionId)
{
  bool recognizedActionType = false;
  std::string actionType = actionJs["Type"].get<std::string>();

  if (actionType == "Set Ninja Horizontal Magnet")
   {
    if (isDefined(actionJs, "Intensity") == false) EXIT_WITH_ERROR("[ERROR] Magnet in Rule %lu Action %lu missing 'Intensity' key.\n", _label, actionId);
    if (isDefined(actionJs, "Center") == false) EXIT_WITH_ERROR("[ERROR] Magnet in Rule %lu Action %lu missing 'Center' key.\n", _label, actionId);
    if (isDefined(actionJs, "Min") == false) EXIT_WITH_ERROR("[ERROR] Magnet in Rule %lu Action %lu missing 'Min' key.\n", _label, actionId);
    if (isDefined(actionJs, "Max") == false) EXIT_WITH_ERROR("[ERROR] Magnet in Rule %lu Action %lu missing 'Max' key.\n", _label, actionId);
    _magnets.ninjaHorizontalMagnet = genericMagnet_t { .intensity = actionJs["Intensity"].get<float>(), .center= actionJs["Center"].get<float>(), .min = actionJs["Min"].get<float>(), .max = actionJs["Max"].get<float>() };
     recognizedActionType = true;
   }

   if (actionType == "Set Ninja Vertical Magnet")
   {
    if (isDefined(actionJs, "Intensity") == false) EXIT_WITH_ERROR("[ERROR] Magnet in Rule %lu Action %lu missing 'Intensity' key.\n", _label, actionId);
    if (isDefined(actionJs, "Min") == false) EXIT_WITH_ERROR("[ERROR] Magnet in Rule %lu Action %lu missing 'Min' key.\n", _label, actionId);
    if (isDefined(actionJs, "Max") == false) EXIT_WITH_ERROR("[ERROR] Magnet in Rule %lu Action %lu missing 'Max' key.\n", _label, actionId);
    _magnets.ninjaVerticalMagnet = genericMagnet_t { .intensity = actionJs["Intensity"].get<float>(), .center= actionJs["Center"].get<float>(), .min = actionJs["Min"].get<float>(), .max = actionJs["Max"].get<float>() };
    recognizedActionType = true;
   }

   if (actionType == "Set Ninja Power Magnet")
   {
    if (isDefined(actionJs, "Intensity") == false) EXIT_WITH_ERROR("[ERROR] Magnet in Rule %lu Action %lu missing 'Intensity' key.\n", _label, actionId);
    if (isDefined(actionJs, "Min") == false) EXIT_WITH_ERROR("[ERROR] Magnet in Rule %lu Action %lu missing 'Min' key.\n", _label, actionId);
    if (isDefined(actionJs, "Max") == false) EXIT_WITH_ERROR("[ERROR] Magnet in Rule %lu Action %lu missing 'Max' key.\n", _label, actionId);
    _magnets.ninjaPowerMagnet = genericMagnet_t { .intensity = actionJs["Intensity"].get<float>(), .center= actionJs["Center"].get<float>(), .min = actionJs["Min"].get<float>(), .max = actionJs["Max"].get<float>() };
    recognizedActionType = true;
   }

   if (actionType == "Set Ninja HP Magnet")
   {
    if (isDefined(actionJs, "Intensity") == false) EXIT_WITH_ERROR("[ERROR] Magnet in Rule %lu Action %lu missing 'Intensity' key.\n", _label, actionId);
    if (isDefined(actionJs, "Min") == false) EXIT_WITH_ERROR("[ERROR] Magnet in Rule %lu Action %lu missing 'Min' key.\n", _label, actionId);
    if (isDefined(actionJs, "Max") == false) EXIT_WITH_ERROR("[ERROR] Magnet in Rule %lu Action %lu missing 'Max' key.\n", _label, actionId);
    _magnets.ninjaHPMagnet = genericMagnet_t { .intensity = actionJs["Intensity"].get<float>(), .center= actionJs["Center"].get<float>(), .min = actionJs["Min"].get<float>(), .max = actionJs["Max"].get<float>() };
    recognizedActionType = true;
   }


   if (actionType == "Set Ninja Weapon Magnet")
   {
    if (isDefined(actionJs, "Reward") == false) EXIT_WITH_ERROR("[ERROR] Weapon Magnet in Rule %lu Action %lu missing 'Reward' key.\n", _label, actionId);
    if (isDefined(actionJs, "Weapon") == false) EXIT_WITH_ERROR("[ERROR] Weapon Magnet in Rule %lu Action %lu missing 'Weapon' key.\n", _label, actionId);
    _magnets.ninjaWeaponMagnet = weaponMagnet_t { .reward = actionJs["Reward"].get<float>(), .weapon = actionJs["Weapon"].get<uint8_t>()};
    recognizedActionType = true;
   }

   if (actionType == "Set Boss HP Magnet")
   {
    if (isDefined(actionJs, "Intensity") == false) EXIT_WITH_ERROR("[ERROR] Magnet in Rule %lu Action %lu missing 'Intensity' key.\n", _label, actionId);
    if (isDefined(actionJs, "Min") == false) EXIT_WITH_ERROR("[ERROR] Magnet in Rule %lu Action %lu missing 'Min' key.\n", _label, actionId);
    if (isDefined(actionJs, "Max") == false) EXIT_WITH_ERROR("[ERROR] Magnet in Rule %lu Action %lu missing 'Max' key.\n", _label, actionId);
    _magnets.bossHPMagnet = genericMagnet_t { .intensity = actionJs["Intensity"].get<float>(), .center= actionJs["Center"].get<float>(), .min = actionJs["Min"].get<float>(), .max = actionJs["Max"].get<float>() };
    recognizedActionType = true;
   }

   if (actionType == "Set Ninja/Boss Distance Magnet")
   {
    if (isDefined(actionJs, "Intensity") == false) EXIT_WITH_ERROR("[ERROR] Ninja/Boss Distance Magnet in Rule %lu Action %lu missing 'Intensity' key.\n", _label, actionId);
    _magnets.ninjaBossDistanceMagnet = actionJs["Intensity"].get<float>();
    recognizedActionType = true;
   }

   if (actionType == "Set Trace Magnet")
   {
    if (isDefined(actionJs, "Intensity") == false) EXIT_WITH_ERROR("[ERROR] Trace Magnet in Rule %lu Action %lu missing 'Intensity' key.\n", _label, actionId);
    _magnets.traceMagnet = actionJs["Intensity"].get<float>();
    recognizedActionType = true;
   }

  return recognizedActionType;
}

datatype_t GameRule::getPropertyType(const nlohmann::json& condition)
{
  std::string propertyName = condition["Property"].get<std::string>();

  if (propertyName == "Current Stage") return dt_uint8;
  if (propertyName == "Ninja Power") return dt_uint8;
  if (propertyName == "Ninja Power Max") return dt_uint8;
  if (propertyName == "Ninja HP") return dt_uint8;
  if (propertyName == "Ninja Position X1") return dt_uint8;
  if (propertyName == "Ninja Position X2") return dt_uint8;
  if (propertyName == "Ninja Position Y1") return dt_uint8;
  if (propertyName == "Ninja Position Y2") return dt_uint8;
  if (propertyName == "Ninja Speed X1") return dt_uint8;
  if (propertyName == "Ninja Speed X2") return dt_uint8;
  if (propertyName == "Ninja Speed Y1") return dt_uint8;
  if (propertyName == "Ninja Speed Y2") return dt_uint8;
  if (propertyName == "Ninja Weapon") return dt_uint8;
  if (propertyName == "Absolute Pos X") return dt_float;
  if (propertyName == "Absolute Pos Y") return dt_float;
  if (propertyName == "Screen Pos X1") return dt_uint8;
  if (propertyName == "Screen Pos X2") return dt_uint8;
  if (propertyName == "Screen Pos X3") return dt_uint8;
  if (propertyName == "Screen Pos Y1") return dt_uint8;
  if (propertyName == "Screen Pos Y2") return dt_uint8;
  if (propertyName == "Screen Pos Y3") return dt_uint8;
  if (propertyName == "Object Active") return dt_uint8;
  if (propertyName == "Ninja Sword Type") return dt_uint8;
  if (propertyName == "Object State") return dt_uint8;
  if (propertyName == "Game Mode") return dt_uint8;
  if (propertyName == "Boss HP") return dt_uint8;
  if (propertyName == "Level Exit Flag") return dt_uint8;
  if (propertyName == "Level Exit Flag 2") return dt_uint8;

  EXIT_WITH_ERROR("[Error] Rule %lu, unrecognized property: %s\n", _label, propertyName.c_str());

  return dt_uint8;
}

void* GameRule::getPropertyPointer(const nlohmann::json& condition, GameInstance* gameInstance)
{
  std::string propertyName = condition["Property"].get<std::string>();

  if (propertyName == "Current Stage") return gameInstance->currentStage;
  if (propertyName == "Ninja Power") return gameInstance->ninjaPower;
  if (propertyName == "Ninja Power Max") return gameInstance->ninjaPowerMax;
  if (propertyName == "Ninja HP") return gameInstance->ninjaHP;
  if (propertyName == "Ninja Weapon") return gameInstance->ninjaWeapon;
  if (propertyName == "Ninja Position X1") return gameInstance->ninjaPosX1;
  if (propertyName == "Ninja Position X2") return gameInstance->ninjaPosX2;
  if (propertyName == "Ninja Position Y1") return gameInstance->ninjaPosY1;
  if (propertyName == "Ninja Position Y2") return gameInstance->ninjaPosY2;
  if (propertyName == "Ninja Speed X1") return gameInstance->ninjaSpeedX1;
  if (propertyName == "Ninja Speed X2") return gameInstance->ninjaSpeedX2;
  if (propertyName == "Ninja Speed Y1") return gameInstance->ninjaSpeedY1;
  if (propertyName == "Ninja Speed Y2") return gameInstance->ninjaSpeedY2;
  if (propertyName == "Ninja Sword Type") return gameInstance->ninjaSwordType;
  if (propertyName == "Absolute Pos X") return &gameInstance->absolutePosX;
  if (propertyName == "Absolute Pos Y") return &gameInstance->absolutePosY;
  if (propertyName == "Screen Pos X1") return gameInstance->screenPosX1;
  if (propertyName == "Screen Pos X2") return gameInstance->screenPosX2;
  if (propertyName == "Screen Pos X3") return gameInstance->screenPosX3;
  if (propertyName == "Screen Pos X1") return gameInstance->screenPosY1;
  if (propertyName == "Screen Pos X2") return gameInstance->screenPosY2;
  if (propertyName == "Screen Pos X3") return gameInstance->screenPosY3;
  if (propertyName == "Game Mode") return gameInstance->gameMode;
  if (propertyName == "Level Exit Flag") return gameInstance->levelExitFlag;
  if (propertyName == "Level Exit Flag 2") return gameInstance->levelExitFlag2;

  EXIT_WITH_ERROR("[Error] Rule %lu, unrecognized property: %s\n", _label, propertyName.c_str());

  return NULL;
}
