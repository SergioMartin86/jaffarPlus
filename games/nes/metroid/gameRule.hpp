#pragma once

#include "gameInstance.hpp"
#include "rule.hpp"

class GameRule : public Rule
{
 public:

 magnetSet_t _magnets;

 uint8_t _customValue = 0;
 bool _customValueActive = false;

 bool _disableBValue = false;
 bool _disableBActive = false;

 GameRule();
 ~GameRule() = default;

 bool parseGameAction(nlohmann::json actionJs, size_t actionId) override;
 datatype_t getPropertyType(const nlohmann::json& condition) override;
 void *getPropertyPointer(const nlohmann::json& condition, GameInstance* gameInstance) override;

};

