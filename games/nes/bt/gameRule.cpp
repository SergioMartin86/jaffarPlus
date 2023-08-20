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
  if (propertyName == "Player 1 Lives") return dt_uint8;
  if (propertyName == "Key Event 1 Triggered") return dt_uint8;
  if (propertyName == "Key Event 2 Triggered") return dt_uint8;
  if (propertyName == "Lag Frame") return dt_uint8;

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
  if (propertyName == "Player 1 Lives") return gameInstance->player1Lives;
  if (propertyName == "Key Event 1 Triggered") return gameInstance->keyEvent1Triggered;
  if (propertyName == "Key Event 2 Triggered") return gameInstance->keyEvent2Triggered;
  if (propertyName == "Lag Frame") return gameInstance->lagFrame;

  EXIT_WITH_ERROR("[Error] Rule %lu, unrecognized property: %s\n", _label, propertyName.c_str());

  return NULL;
}
