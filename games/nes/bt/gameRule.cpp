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

  if (propertyName == "Current Level") return dt_uint8;
  if (propertyName == "Key Address 1") return dt_uint8;
  if (propertyName == "Key Address 2") return dt_uint8;
  if (propertyName == "Key Address 3") return dt_uint8;

  EXIT_WITH_ERROR("[Error] Rule %lu, unrecognized property: %s\n", _label, propertyName.c_str());

  return dt_uint8;
}

void* GameRule::getPropertyPointer(const nlohmann::json& condition, GameInstance* gameInstance)
{
  std::string propertyName = condition["Property"].get<std::string>();

  if (propertyName == "Current Level") return gameInstance->currentLevel;
  if (propertyName == "Key Address 1") return gameInstance->keyAddress1;
  if (propertyName == "Key Address 2") return gameInstance->keyAddress2;
  if (propertyName == "Key Address 3") return gameInstance->keyAddress3;

  EXIT_WITH_ERROR("[Error] Rule %lu, unrecognized property: %s\n", _label, propertyName.c_str());

  return NULL;
}
