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

   if (actionType == "Set Boss Horizontal Magnet")
   {
    if (isDefined(actionJs, "Intensity") == false) EXIT_WITH_ERROR("[ERROR] Magnet in Rule %lu Action %lu missing 'Intensity' key.\n", _label, actionId);
    if (isDefined(actionJs, "Min") == false) EXIT_WITH_ERROR("[ERROR] Magnet in Rule %lu Action %lu missing 'Min' key.\n", _label, actionId);
    if (isDefined(actionJs, "Max") == false) EXIT_WITH_ERROR("[ERROR] Magnet in Rule %lu Action %lu missing 'Max' key.\n", _label, actionId);
    _magnets.bossHorizontalMagnet = genericMagnet_t { .intensity = actionJs["Intensity"].get<float>(), .center= actionJs["Center"].get<float>(), .min = actionJs["Min"].get<float>(), .max = actionJs["Max"].get<float>() };
     recognizedActionType = true;
   }

   if (actionType == "Set Boss Vertical Magnet")
   {
    if (isDefined(actionJs, "Intensity") == false) EXIT_WITH_ERROR("[ERROR] Magnet in Rule %lu Action %lu missing 'Intensity' key.\n", _label, actionId);
    if (isDefined(actionJs, "Min") == false) EXIT_WITH_ERROR("[ERROR] Magnet in Rule %lu Action %lu missing 'Min' key.\n", _label, actionId);
    if (isDefined(actionJs, "Max") == false) EXIT_WITH_ERROR("[ERROR] Magnet in Rule %lu Action %lu missing 'Max' key.\n", _label, actionId);
    _magnets.bossVerticalMagnet = genericMagnet_t { .intensity = actionJs["Intensity"].get<float>(), .center= actionJs["Center"].get<float>(), .min = actionJs["Min"].get<float>(), .max = actionJs["Max"].get<float>() };
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

   if (actionType == "Set Ninja Weapon Magnet")
   {
    if (isDefined(actionJs, "Reward") == false) EXIT_WITH_ERROR("[ERROR] Weapon Magnet in Rule %lu Action %lu missing 'Reward' key.\n", _label, actionId);
    if (isDefined(actionJs, "Weapon") == false) EXIT_WITH_ERROR("[ERROR] Weapon Magnet in Rule %lu Action %lu missing 'Weapon' key.\n", _label, actionId);
    _magnets.ninjaWeaponMagnet = weaponMagnet_t { .reward = actionJs["Reward"].get<float>(), .weapon = actionJs["Weapon"].get<uint8_t>()};
    recognizedActionType = true;
   }

   if (actionType == "Set Ninja/Boss Distance Magnet")
   {
    if (isDefined(actionJs, "Intensity") == false) EXIT_WITH_ERROR("[ERROR] Ninja/Boss Distance Magnet in Rule %lu Action %lu missing 'Intensity' key.\n", _label, actionId);
    _magnets.ninjaBossDistanceMagnet = actionJs["Intensity"].get<float>();
    recognizedActionType = true;
   }

   if (actionType == "Set Ninja Speed X Magnet")
   {
    if (isDefined(actionJs, "Intensity") == false) EXIT_WITH_ERROR("[ERROR] Ninja Speed X Magnet in Rule %lu Action %lu missing 'Intensity' key.\n", _label, actionId);
    _magnets.ninjaSpeedXMagnet = actionJs["Intensity"].get<float>();
    recognizedActionType = true;
   }

   if (actionType == "Set Ninja Speed Y Magnet")
   {
    if (isDefined(actionJs, "Intensity") == false) EXIT_WITH_ERROR("[ERROR] Ninja Speed Y Magnet in Rule %lu Action %lu missing 'Intensity' key.\n", _label, actionId);
    _magnets.ninjaSpeedYMagnet = actionJs["Intensity"].get<float>();
    recognizedActionType = true;
   }

   if (actionType == "Set Enemy HP Magnet")
   {
    if (isDefined(actionJs, "Intensity") == false) EXIT_WITH_ERROR("[ERROR] Enemy HP Magnet in Rule %lu Action %lu missing 'Intensity' key.\n", _label, actionId);
    if (isDefined(actionJs, "Index") == false) EXIT_WITH_ERROR("[ERROR] Enemy HP Magnet in Rule %lu Action %lu missing 'Index' key.\n", _label, actionId);
    _magnets.enemyHPMagnet.intensity = actionJs["Intensity"].get<float>();
    _magnets.enemyHPMagnet.index = actionJs["Index"].get<uint8_t>();
    recognizedActionType = true;
   }

   if (actionType == "Set Boss Health Magnet")
   {
    if (isDefined(actionJs, "Intensity") == false) EXIT_WITH_ERROR("[ERROR] Boss Health Magnet in Rule %lu Action %lu missing 'Intensity' key.\n", _label, actionId);
    _magnets.bossHealthMagnet = actionJs["Intensity"].get<float>();
    recognizedActionType = true;
   }

  return recognizedActionType;
}

datatype_t GameRule::getPropertyType(const nlohmann::json& condition)
{
  std::string propertyName = condition["Property"].get<std::string>();

  if (propertyName == "Game Mode") return dt_uint8;
  if (propertyName == "Current Stage") return dt_uint8;
  if (propertyName == "Current Substage") return dt_uint8;
  if (propertyName == "Ninja Lives") return dt_uint8;
  if (propertyName == "Ninja Power") return dt_uint8;
  if (propertyName == "Ninja HP") return dt_uint8;
  if (propertyName == "Boss HP") return dt_uint8;
  if (propertyName == "Ninja State Flags") return dt_uint8;
  if (propertyName == "Ninja Position X") return dt_uint8;
  if (propertyName == "Ninja Position Y") return dt_uint8;
  if (propertyName == "Ninja Speed X") return dt_uint8;
  if (propertyName == "Ninja Speed Y") return dt_uint8;
  if (propertyName == "Ninja Collision Flags") return dt_uint8;
  if (propertyName == "Ninja Flinch Direction") return dt_uint8;
  if (propertyName == "Ninja Invincibility Timer") return dt_uint8;
  if (propertyName == "Ninja Weapon") return dt_uint8;
  if (propertyName == "Ninja Animation Type") return dt_uint8;
  if (propertyName == "Ninja Animation Offset") return dt_uint8;
  if (propertyName == "Ninja Animation Timer") return dt_uint8;
  if (propertyName == "Ninja Animation Frame") return dt_uint8;

  if (propertyName == "Ninja Projectile 1 Pos X") return dt_uint8;
  if (propertyName == "Ninja Projectile 2 Pos X") return dt_uint8;
  if (propertyName == "Ninja Projectile 3 Pos X") return dt_uint8;

  if (propertyName == "Ninja Projectile 1 Pos Y") return dt_uint8;
  if (propertyName == "Ninja Projectile 2 Pos Y") return dt_uint8;
  if (propertyName == "Ninja Projectile 3 Pos Y") return dt_uint8;

  if (propertyName == "Enemy Count") return dt_uint8;
  if (propertyName == "Timeout Seconds") return dt_uint8;
  if (propertyName == "Timeout Fractions") return dt_uint8;
  if (propertyName == "Screen Scroll 1") return dt_uint8;
  if (propertyName == "Screen Scroll 2") return dt_uint8;
  if (propertyName == "Screen Scroll 3") return dt_uint8;
  if (propertyName == "Absolute Pos X") return dt_double;
  if (propertyName == "PPU Indicator Bit 6") return dt_uint8;
  if (propertyName == "Enemy Flags") return dt_uint8;
  if (propertyName == "Enemy Type") return dt_uint8;

  EXIT_WITH_ERROR("[Error] Rule %lu, unrecognized property: %s\n", _label, propertyName.c_str());

  return dt_uint8;
}

void* GameRule::getPropertyPointer(const nlohmann::json& condition, GameInstance* gameInstance)
{
  std::string propertyName = condition["Property"].get<std::string>();

  if (propertyName == "Game Mode") return gameInstance->gameMode;
  if (propertyName == "Current Stage") return gameInstance->currentStage;
  if (propertyName == "Current Substage") return gameInstance->currentSubStage;
  if (propertyName == "Ninja Lives") return gameInstance->ninjalives;
  if (propertyName == "Ninja Power") return gameInstance->ninjaPower;
  if (propertyName == "Ninja HP") return gameInstance->ninjaHP;
  if (propertyName == "Boss HP") return gameInstance->bossHP;
  if (propertyName == "Ninja State Flags") return gameInstance->ninjaStateFlags;
  if (propertyName == "Ninja Position X") return gameInstance->ninjaPosX;
  if (propertyName == "Ninja Position Y") return gameInstance->ninjaPosY;
  if (propertyName == "Ninja Speed X") return gameInstance->ninjaSpeedX;
  if (propertyName == "Ninja Speed Y") return gameInstance->ninjaSpeedY;
  if (propertyName == "Ninja Collision Flags") return gameInstance->ninjaCollisionFlags;
  if (propertyName == "Ninja Flinch Direction") return gameInstance->ninjaFlinchDirection;
  if (propertyName == "Ninja Invincibility Timer") return gameInstance->ninjaInvincibilityTimer;
  if (propertyName == "Ninja Weapon") return gameInstance->ninjaWeapon;
  if (propertyName == "Ninja Animation Type") return gameInstance->ninjaAnimationType;
  if (propertyName == "Ninja Animation Offset") return gameInstance->ninjaAnimationOffset;
  if (propertyName == "Ninja Animation Timer") return gameInstance->ninjaAnimationTimer;
  if (propertyName == "Ninja Animation Frame") return gameInstance->ninjaAnimationFrame;

  if (propertyName == "Ninja Projectile 1 Pos X") return gameInstance->ninjaProjectile1PosX;
  if (propertyName == "Ninja Projectile 2 Pos X") return gameInstance->ninjaProjectile2PosX;
  if (propertyName == "Ninja Projectile 3 Pos X") return gameInstance->ninjaProjectile3PosX;

  if (propertyName == "Ninja Projectile 1 Pos Y") return gameInstance->ninjaProjectile1PosY;
  if (propertyName == "Ninja Projectile 2 Pos Y") return gameInstance->ninjaProjectile2PosY;
  if (propertyName == "Ninja Projectile 3 Pos Y") return gameInstance->ninjaProjectile3PosY;

  if (propertyName == "Enemy Count") return gameInstance->enemyCount;
  if (propertyName == "Timeout Seconds") return gameInstance->timeoutSeconds1;
  if (propertyName == "Timeout Fractions") return gameInstance->timeoutSeconds60;
  if (propertyName == "Screen Scroll 1") return gameInstance->screenScroll1;
  if (propertyName == "Screen Scroll 2") return gameInstance->screenScroll2;
  if (propertyName == "Screen Scroll 3") return gameInstance->screenScroll3;
  if (propertyName == "Absolute Pos X") return &gameInstance->absolutePosX;
  if (propertyName == "PPU Indicator Bit 6") return &gameInstance->ppuIndicatorBit6;

  if (propertyName == "Enemy Flags")
  {
   if (isDefined(condition, "Index") == false) EXIT_WITH_ERROR("[ERROR] Enemy missing 'Index' key.\n");
   return &gameInstance->enemyFlags[condition["Index"].get<uint8_t>()];
  }

  if (propertyName == "Enemy Type")
  {
   if (isDefined(condition, "Index") == false) EXIT_WITH_ERROR("[ERROR] Enemy missing 'Index' key.\n");
   return &gameInstance->enemyType[condition["Index"].get<uint8_t>()];
  }

  EXIT_WITH_ERROR("[Error] Rule %lu, unrecognized property: %s\n", _label, propertyName.c_str());

  return NULL;
}
