#pragma once

#include "gameInstance.hpp"
#include "rule.hpp"

// Contains the value of the magnets for a given character
struct magnet_t
{
  byte room;
  float intensity;
  float center;
};

class GameRule : public Rule
{
 public:

 uint8_t _customValue = 0;
 bool _customValueActive = false;
 magnetSet_t _magnets[ROOM_COUNT];

 GameRule();
 ~GameRule() = default;

 bool parseGameAction(nlohmann::json actionJs, size_t actionId) override;
 datatype_t getPropertyType(const nlohmann::json& condition) override;
 void *getPropertyPointer(const nlohmann::json& condition, GameInstance* gameInstance) override;

};

