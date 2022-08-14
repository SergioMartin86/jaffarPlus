#include "gameRule.hpp"

GameRule::GameRule() : Rule()
{
}

bool GameRule::parseGameAction(nlohmann::json actionJs, size_t actionId)
{
  bool recognizedActionType = false;
  std::string actionType = actionJs["Type"].get<std::string>();

  if (actionType == "Set Hero Horizontal Magnet")
   {
    if (isDefined(actionJs, "Intensity") == false) EXIT_WITH_ERROR("[ERROR] Magnet in Rule %lu Action %lu missing 'Intensity' key.\n", _label, actionId);
    if (isDefined(actionJs, "Center") == false) EXIT_WITH_ERROR("[ERROR] Magnet in Rule %lu Action %lu missing 'Center' key.\n", _label, actionId);
    if (isDefined(actionJs, "Min") == false) EXIT_WITH_ERROR("[ERROR] Magnet in Rule %lu Action %lu missing 'Min' key.\n", _label, actionId);
    if (isDefined(actionJs, "Max") == false) EXIT_WITH_ERROR("[ERROR] Magnet in Rule %lu Action %lu missing 'Max' key.\n", _label, actionId);
    _magnets.heroHorizontalMagnet = genericMagnet_t { .intensity = actionJs["Intensity"].get<float>(), .center= actionJs["Center"].get<float>(), .min = actionJs["Min"].get<float>(), .max = actionJs["Max"].get<float>() };
     recognizedActionType = true;
   }

   if (actionType == "Set Hero Vertical Magnet")
   {
    if (isDefined(actionJs, "Intensity") == false) EXIT_WITH_ERROR("[ERROR] Magnet in Rule %lu Action %lu missing 'Intensity' key.\n", _label, actionId);
    if (isDefined(actionJs, "Min") == false) EXIT_WITH_ERROR("[ERROR] Magnet in Rule %lu Action %lu missing 'Min' key.\n", _label, actionId);
    if (isDefined(actionJs, "Max") == false) EXIT_WITH_ERROR("[ERROR] Magnet in Rule %lu Action %lu missing 'Max' key.\n", _label, actionId);
    _magnets.heroVerticalMagnet = genericMagnet_t { .intensity = actionJs["Intensity"].get<float>(), .center= actionJs["Center"].get<float>(), .min = actionJs["Min"].get<float>(), .max = actionJs["Max"].get<float>() };
    recognizedActionType = true;
   }

   if (actionType == "Set Boss Health Magnet")
   {
    if (isDefined(actionJs, "Intensity") == false) EXIT_WITH_ERROR("[ERROR] Boss Health Magnet in Rule %lu Action %lu missing 'Intensity' key.\n", _label, actionId);
    _magnets.bossHealthMagnet = actionJs["Intensity"].get<float>();
    recognizedActionType = true;
   }

   if (actionType == "Set Boss 2 Health Magnet")
   {
    if (isDefined(actionJs, "Intensity") == false) EXIT_WITH_ERROR("[ERROR] Boss Health Magnet in Rule %lu Action %lu missing 'Intensity' key.\n", _label, actionId);
    _magnets.boss2HealthMagnet = actionJs["Intensity"].get<float>();
    recognizedActionType = true;
   }


   if (actionType == "Set Hero Magic Magnet")
   {
    if (isDefined(actionJs, "Intensity") == false) EXIT_WITH_ERROR("[ERROR] Magnet in Rule %lu Action %lu missing 'Intensity' key.\n", _label, actionId);
    _magnets.heroMagicMagnet = actionJs["Intensity"].get<float>();
    recognizedActionType = true;
   }

  return recognizedActionType;
}

datatype_t GameRule::getPropertyType(const nlohmann::json& condition)
{
  std::string propertyName = condition["Property"].get<std::string>();

  if (propertyName == "Hero Pos X") return dt_float;
  if (propertyName == "Hero Pos Y") return dt_float;
  if (propertyName == "Hero Money") return dt_float;
  if (propertyName == "Hero Keys") return dt_uint8;
  if (propertyName == "Hero Life") return dt_uint8;
  if (propertyName == "Inventory Has Egg") return dt_uint8;

  if (propertyName == "Hero Score") return dt_float;
  if (propertyName == "Boss Health") return dt_float;
  if (propertyName == "Attack X") return dt_float;
  if (propertyName == "Attack Y") return dt_float;

  if (propertyName == "Game Mode") return dt_uint8;

  EXIT_WITH_ERROR("[Error] Rule %lu, unrecognized property: %s\n", _label, propertyName.c_str());

  return dt_uint8;
}

void* GameRule::getPropertyPointer(const nlohmann::json& condition, GameInstance* gameInstance)
{
  std::string propertyName = condition["Property"].get<std::string>();

  if (propertyName == "Hero Pos X") return &gameInstance->heroPosX;
  if (propertyName == "Hero Pos Y") return &gameInstance->heroPosY;
  if (propertyName == "Hero Money") return &gameInstance->heroMoney;
  if (propertyName == "Hero Keys") return gameInstance->heroKeys;
  if (propertyName == "Hero Score") return &gameInstance->heroScore;
  if (propertyName == "Hero Life") return &gameInstance->heroLife;
  if (propertyName == "Inventory Has Egg") return gameInstance->inventoryHasEgg;
  if (propertyName == "Boss Health") return &gameInstance->bossHealth;
  if (propertyName == "Attack X") return &gameInstance->attackX;
  if (propertyName == "Attack Y") return &gameInstance->attackY;
  if (propertyName == "Game Mode") return gameInstance->gameMode;

  EXIT_WITH_ERROR("[Error] Rule %lu, unrecognized property: %s\n", _label, propertyName.c_str());

  return NULL;
}
