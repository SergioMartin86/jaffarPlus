#pragma once

#include "gameInstance.hpp"
#include "rule.hpp"

class GameRule : public Rule
{
 public:


 GameRule();
 ~GameRule() = default;
 magnetSet_t _magnets[_ROOM_COUNT_];

 bool parseGameAction(nlohmann::json actionJs, size_t actionId) override;
 datatype_t getPropertyType(const nlohmann::json& condition) override;
 void *getPropertyPointer(const nlohmann::json& condition, GameInstance* gameInstance) override;

};

