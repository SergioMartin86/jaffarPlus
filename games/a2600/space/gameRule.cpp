#include "gameRule.hpp"

GameRule::GameRule() : Rule()
{
}

bool GameRule::parseGameAction(nlohmann::json actionJs, size_t actionId)
{
 bool recognizedActionType = false;
 std::string actionType = actionJs["Type"].get<std::string>();

 return recognizedActionType;
}

datatype_t GameRule::getPropertyType(const nlohmann::json& condition)
{
 std::string propertyName = condition["Property"].get<std::string>();

 if (propertyName == "Score") return dt_float;

 EXIT_WITH_ERROR("[Error] Rule %lu, unrecognized property: %s\n", _label, propertyName.c_str());

 return dt_uint8;
}

void* GameRule::getPropertyPointer(const nlohmann::json& condition, GameInstance* gameInstance)
{
 std::string propertyName = condition["Property"].get<std::string>();

 if (propertyName == "Score") return &gameInstance->score;

 EXIT_WITH_ERROR("[Error] Rule %lu, unrecognized property: %s\n", _label, propertyName.c_str());

 return NULL;
}
