#pragma once

#include "gameInstance.hpp"
#include "rule.hpp"

// Contains the value of the magnets for a given character
struct magnet_t
{
  byte room;
  float value;
};

class GameRule : public Rule
{
 public:

 // Stores magnet information
 std::vector<magnet_t> _kidMagnetPositionX;
 std::vector<magnet_t> _kidMagnetIntensityX;
 std::vector<magnet_t> _kidMagnetIntensityY;
 std::vector<magnet_t> _guardMagnetPositionX;
 std::vector<magnet_t> _guardMagnetIntensityX;
 std::vector<magnet_t> _guardMagnetIntensityY;

 GameRule();
 ~GameRule() = default;

 bool parseGameAction(nlohmann::json actionJs, size_t actionId) override;
 datatype_t getPropertyType(const nlohmann::json& condition) override;
 void *getPropertyPointer(const nlohmann::json& condition, GameInstance* gameInstance) override;

};

