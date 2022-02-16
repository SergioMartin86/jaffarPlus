#pragma once

#include "gameInstance.hpp"
#include "rule.hpp"

class GameRule : public Rule
{
 public:

 magnetSet_t _magnets;

 GameRule();
 ~GameRule() = default;

 bool parseGameAction(nlohmann::json actionJs, size_t actionId) override;
 datatype_t getPropertyType(const std::string &property) override;
 void *getPropertyPointer(const std::string &property, GameInstance* gameInstance) override;

};

