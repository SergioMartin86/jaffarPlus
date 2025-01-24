#pragma once

#include <jaffarCommon/logger.hpp>
#include <jaffarCommon/json.hpp>
#include <emulator.hpp>
#include <game.hpp>

// Players information
extern "C" __STORAGE_MODIFIER int enableOutput;
extern "C" __STORAGE_MODIFIER player_t players[MAX_MAXPLAYERS];
extern "C" __STORAGE_MODIFIER int preventLevelExit;
extern "C" __STORAGE_MODIFIER int preventGameEnd;
extern "C" __STORAGE_MODIFIER int reachedLevelExit;
extern "C" __STORAGE_MODIFIER int reachedGameEnd;
extern "C" __STORAGE_MODIFIER int gamemap;
extern "C" __STORAGE_MODIFIER int gametic;

namespace jaffarPlus
{

namespace games
{

namespace doom
{

class Doom final : public jaffarPlus::Game
{
  public:

  static __INLINE__ std::string getName() { return "Doom"; }

  Doom(std::unique_ptr<Emulator> emulator, const nlohmann::json &config)
    : jaffarPlus::Game(std::move(emulator), config)
  {
    _pointMagnet.intensity = 0;
    _pointMagnet.x = 0;
    _pointMagnet.y = 0;
    _pointMagnet.z = 0;

    _playerPosX = 0;
    _playerPosY = 0;
    _playerPosZ = 0;
    _playerMomentumX = 0;
    _playerMomentumY = 0;
    _playerMomentumZ = 0;
    _player1DistanceToPointX = 0;
    _player1DistanceToPointY = 0;
    _player1DistanceToPointZ = 0;
    _player1DistanceToPoint = 0;
  }

  private:

  __INLINE__ void registerGameProperties() override
  {
    // Registering native game properties
    registerGameProperty("Player Pos X", &_playerPosX, Property::datatype_t::dt_float32, Property::endianness_t::little);
    registerGameProperty("Player Pos Y", &_playerPosY, Property::datatype_t::dt_float32, Property::endianness_t::little);
    registerGameProperty("Player Pos Z", &_playerPosZ, Property::datatype_t::dt_float32, Property::endianness_t::little);
  }

  __INLINE__ void advanceStateImpl(const InputSet::inputIndex_t input) override
  {
    // Running emulator
    _emulator->advanceState(input);
  }

  __INLINE__ void computeAdditionalHashing(MetroHash128 &hashEngine) const override
   {
    // hash.Update(reachedLevelExit);
    // hash.Update(reachedGameEnd);
    // hashEngine.Update(gamemap);
    // hashEngine.Update(gametic);
    
    if (players[0].mo != nullptr)
    {
      hashEngine.Update(players[0].mo->x >> FRACBITS);
      hashEngine.Update(players[0].mo->y >> FRACBITS);
      // hashEngine.Update(players[0].mo->z);
      hashEngine.Update(players[0].mo->angle >> FRACBITS);
      hashEngine.Update(players[0].mo->momx >> FRACBITS);
      hashEngine.Update(players[0].mo->momy >> FRACBITS);
      // hashEngine.Update(players[0].mo->momz);
      // hashEngine.Update(players[0].mo->health);
    }
   }

  // Updating derivative values after updating the internal state
  __INLINE__ void stateUpdatePostHook() override
  {
    _playerPosX = jaffar::EmuInstance::getFloatFrom1616Fixed(players[0].mo->x);
    _playerPosY = jaffar::EmuInstance::getFloatFrom1616Fixed(players[0].mo->y);
    _playerPosZ = jaffar::EmuInstance::getFloatFrom1616Fixed(players[0].mo->z);

    _playerMomentumX = jaffar::EmuInstance::getFloatFrom1616Fixed(players[0].mo->momx);
    _playerMomentumY = jaffar::EmuInstance::getFloatFrom1616Fixed(players[0].mo->momy);
    _playerMomentumZ = jaffar::EmuInstance::getFloatFrom1616Fixed(players[0].mo->momz);

  }

  __INLINE__ void ruleUpdatePreHook() override
  {
    // Resetting magnets ahead of rule re-evaluation
    _pointMagnet.intensity = 0.0;
    _pointMagnet.x         = 0.0;
    _pointMagnet.y         = 0.0;
    _pointMagnet.z         = 0.0;
  }

  __INLINE__ void ruleUpdatePostHook() override
  {
    // Updating distance to user-defined point
    _player1DistanceToPointX = std::abs(_pointMagnet.x - _playerPosX);
    _player1DistanceToPointY = std::abs(_pointMagnet.y - _playerPosY);
    _player1DistanceToPointZ = std::abs(_pointMagnet.z - _playerPosZ);
    _player1DistanceToPoint  = sqrtf(_player1DistanceToPointX * _player1DistanceToPointX + _player1DistanceToPointY * _player1DistanceToPointY + _player1DistanceToPointZ * _player1DistanceToPointZ);
  }

  __INLINE__ void serializeStateImpl(jaffarCommon::serializer::Base &serializer) const override {}

  __INLINE__ void deserializeStateImpl(jaffarCommon::deserializer::Base &deserializer) {}

  __INLINE__ float calculateGameSpecificReward() const
  {
    // Getting rewards from rules
    float reward = 0.0;

    // Reward advancing always
    if (std::abs(_pointMagnet.intensity) > 0.0f) reward += -1.0 * _pointMagnet.intensity * _player1DistanceToPoint;

    // Returning reward
    return reward;
  }

  void printInfoImpl() const override
  {
    char mapName[512];
    headlessGetMapName(mapName);
    jaffarCommon::logger::log("[] Map:        %s\n", mapName);
    jaffarCommon::logger::log("[] Game Tic:   %d\n", gametic);
    jaffarCommon::logger::log("[] Level Exit: %s\n", reachedLevelExit == 1 ? "Yes" : "No");
    jaffarCommon::logger::log("[] Game End:   %s\n", reachedGameEnd   == 1 ? "Yes" : "No");
 
    if (players[0].mo != nullptr)
    {
      jaffarCommon::logger::log("[] Player 1 Coordinates:    (%f, %f, %f)\n", _playerPosX, _playerPosY, _playerPosZ);
      jaffarCommon::logger::log("[] Player 1 Angle:           %lu\n", players[0].mo->angle);
      jaffarCommon::logger::log("[] Player 1 Momenta:        (%f, %f, %f)\n", _playerMomentumX, _playerMomentumY, _playerMomentumZ);
      jaffarCommon::logger::log("[] Player 1 Health:          %d\n", players[0].mo->health);
    }

    if (std::abs(_pointMagnet.intensity) > 0.0f)
    {
        jaffarCommon::logger::log("[J+]  + Point Magnet                             Intensity: %.5f, X: %3.3f, Y: %3.3f, Z: %3.3f\n", _pointMagnet.intensity, _pointMagnet.x, _pointMagnet.y, _pointMagnet.z);
        jaffarCommon::logger::log("[J+]    + Distance X                             %3.3f\n", _player1DistanceToPointX);
        jaffarCommon::logger::log("[J+]    + Distance Y                             %3.3f\n", _player1DistanceToPointY);
        jaffarCommon::logger::log("[J+]    + Distance Z                             %3.3f\n", _player1DistanceToPointZ);
        jaffarCommon::logger::log("[J+]    + Total Distance                         %3.3f\n", _player1DistanceToPoint);
    }
  }

  bool parseRuleActionImpl(Rule &rule, const std::string &actionType, const nlohmann::json &actionJs) override
  {
    bool recognizedActionType = false;

    if (actionType == "Set Point Magnet")
      {
        auto intensity = jaffarCommon::json::getNumber<float>(actionJs, "Intensity");
        auto x         = jaffarCommon::json::getNumber<float>(actionJs, "X");
        auto y         = jaffarCommon::json::getNumber<float>(actionJs, "Y");
        auto z         = jaffarCommon::json::getNumber<float>(actionJs, "Z");
        rule.addAction([=, this]() { this->_pointMagnet = pointMagnet_t{.intensity = intensity, .x = x, .y = y, .z = z}; });
        recognizedActionType = true;
    }

    return recognizedActionType;
  }

  __INLINE__ jaffarCommon::hash::hash_t getStateInputHash() override
  {
    // There is no discriminating state element, so simply return a zero hash
    return jaffarCommon::hash::hash_t();
  }

  // Datatype to describe a point magnet
  struct pointMagnet_t
  {
    float intensity = 0.0; // How strong the magnet is
    float x         = 0.0; // What is the x point of attraction
    float y         = 0.0; // What is the y point of attraction
    float z         = 0.0; // What is the y point of attraction
  };

  // Derivative properties

  pointMagnet_t _pointMagnet;

  float _playerPosX;
  float _playerPosY;
  float _playerPosZ;
  float _playerMomentumX;
  float _playerMomentumY;
  float _playerMomentumZ;

  float _player1DistanceToPointX;
  float _player1DistanceToPointY;
  float _player1DistanceToPointZ;
  float _player1DistanceToPoint;

  size_t _maxStateSize;
};

} // namespace doom

} // namespace games

} // namespace jaffarPlus