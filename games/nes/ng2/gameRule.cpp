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

  return recognizedActionType;
}

datatype_t GameRule::getPropertyType(const nlohmann::json& condition)
{
  std::string propertyName = condition["Property"].get<std::string>();

  if (propertyName == "Current Level") return dt_uint8;
  if (propertyName == "Ninja Power") return dt_uint8;
  if (propertyName == "Ninja Power Max") return dt_uint8;
  if (propertyName == "Ninja HP") return dt_uint8;
  if (propertyName == "Ninja Position X") return dt_uint8;
  if (propertyName == "Ninja Position Y") return dt_uint8;
  if (propertyName == "Ninja Speed X") return dt_uint8;
  if (propertyName == "Ninja Speed Y") return dt_uint8;
  if (propertyName == "Ninja Weapon") return dt_uint8;
  if (propertyName == "Absolute Pos X") return dt_double;
  if (propertyName == "Screen Scroll 1") return dt_uint8;
  if (propertyName == "Screen Scroll 2") return dt_uint8;
  if (propertyName == "Screen Scroll 3") return dt_uint8;
  if (propertyName == "Object Active") return dt_uint8;
  if (propertyName == "Object State") return dt_uint8;
  if (propertyName == "Game Mode") return dt_uint8;
  if (propertyName == "Boss HP") return dt_uint8;

  EXIT_WITH_ERROR("[Error] Rule %lu, unrecognized property: %s\n", _label, propertyName.c_str());

  return dt_uint8;
}

void* GameRule::getPropertyPointer(const nlohmann::json& condition, GameInstance* gameInstance)
{
  std::string propertyName = condition["Property"].get<std::string>();

  if (propertyName == "Current Level") return gameInstance->currentLevel;
  if (propertyName == "Ninja Lives") return gameInstance->ninjaLives;
  if (propertyName == "Ninja Power") return gameInstance->ninjaPower;
  if (propertyName == "Ninja Power Max") return gameInstance->ninjaPowerMax;
  if (propertyName == "Ninja HP") return gameInstance->ninjaHP;
  if (propertyName == "Ninja Weapon") return gameInstance->ninjaWeapon;
  if (propertyName == "Ninja Position X") return gameInstance->ninjaPosX;
  if (propertyName == "Ninja Position Y") return gameInstance->ninjaPosY;
  if (propertyName == "Ninja Speed X") return gameInstance->ninjaSpeedX;
  if (propertyName == "Ninja Speed Y") return gameInstance->ninjaSpeedY;
  if (propertyName == "Absolute Pos X") return &gameInstance->absolutePosX;
  if (propertyName == "Screen Scroll 1") return gameInstance->screenScroll1;
  if (propertyName == "Screen Scroll 2") return gameInstance->screenScroll2;
  if (propertyName == "Screen Scroll 3") return gameInstance->screenScroll3;
  if (propertyName == "Game Mode") return gameInstance->gameMode;
  if (propertyName == "Boss HP") return gameInstance->bossHP;

  if (propertyName == "Object Active")
  {
   if (isDefined(condition, "Index") == false) EXIT_WITH_ERROR("[ERROR] Object missing 'Index' key.\n");
   return &gameInstance->ObjectActivationFlags[condition["Index"].get<uint8_t>()];
  }

  if (propertyName == "Object State")
  {
   if (isDefined(condition, "Index") == false) EXIT_WITH_ERROR("[ERROR] Object missing 'Index' key.\n");
   return &gameInstance->ninjaState[condition["Index"].get<uint8_t>()];
  }

  EXIT_WITH_ERROR("[Error] Rule %lu, unrecognized property: %s\n", _label, propertyName.c_str());

  return NULL;
}
