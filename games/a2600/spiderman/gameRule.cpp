#include "gameRule.hpp"

GameRule::GameRule() : Rule()
{
}

bool GameRule::parseGameAction(nlohmann::json actionJs, size_t actionId)
{
 bool recognizedActionType = false;
 std::string actionType = actionJs["Type"].get<std::string>();

 if (actionType == "Set Vertical Scroll Magnet")
 {
   if (isDefined(actionJs, "Intensity") == false) EXIT_WITH_ERROR("[ERROR] Magnet in Rule %lu Action %lu missing 'Intensity' key.\n", _label, actionId);
   if (isDefined(actionJs, "Center") == false) EXIT_WITH_ERROR("[ERROR] Magnet in Rule %lu Action %lu missing 'Center' key.\n", _label, actionId);
   _magnets.verticalScrollMagnet = genericMagnet_t { .intensity = actionJs["Intensity"].get<float>(), .center= actionJs["Center"].get<float>() };
   recognizedActionType = true;
 }

 return recognizedActionType;
}

datatype_t GameRule::getPropertyType(const nlohmann::json& condition)
{
 std::string propertyName = condition["Property"].get<std::string>();

 if (propertyName == "Game Timer") return dt_uint8;
 if (propertyName == "Game State") return dt_uint8;
 if (propertyName == "Player State") return dt_uint8;
 if (propertyName == "Vertical Scroll") return dt_float;

 EXIT_WITH_ERROR("[Error] Rule %lu, unrecognized property: %s\n", _label, propertyName.c_str());

 return dt_uint8;
}

void* GameRule::getPropertyPointer(const nlohmann::json& condition, GameInstance* gameInstance)
{
 std::string propertyName = condition["Property"].get<std::string>();

 if (propertyName == "Game Timer") return gameInstance->gameTimer;
 if (propertyName == "Game State") return gameInstance->gameState;
 if (propertyName == "Player State") return gameInstance->playerState;
 if (propertyName == "Vertical Scroll") return &gameInstance->verticalScroll;

 EXIT_WITH_ERROR("[Error] Rule %lu, unrecognized property: %s\n", _label, propertyName.c_str());

 return NULL;
}
