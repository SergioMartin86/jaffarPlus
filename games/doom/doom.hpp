#pragma once

#include <emulator.hpp>
#include <game.hpp>
#include <jaffarCommon/hash.hpp>
#include <jaffarCommon/json.hpp>
#include <jaffarCommon/logger.hpp>
#include <r_defs.h>

// Players information
extern "C" void                        P_SetThingPosition(mobj_t* thing, int refind);
extern "C" __STORAGE_MODIFIER int      enableOutput;
extern "C" __STORAGE_MODIFIER player_t players[MAX_MAXPLAYERS];
extern "C" __STORAGE_MODIFIER int      preventLevelExit;
extern "C" __STORAGE_MODIFIER int      preventGameEnd;
extern "C" __STORAGE_MODIFIER int      reachedLevelExit;
extern "C" __STORAGE_MODIFIER int      reachedGameEnd;
extern "C" __STORAGE_MODIFIER int      gamemap;
extern "C" __STORAGE_MODIFIER int      gametic;
extern "C" __STORAGE_MODIFIER int      numlines;
extern "C" __STORAGE_MODIFIER line_t*  lines;

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

  Doom(std::unique_ptr<Emulator> emulator, const nlohmann::json& config) : jaffarPlus::Game(std::move(emulator), config)
  {
    _pointMagnet.intensity = 0;
    _pointMagnet.x         = 0;
    _pointMagnet.y         = 0;
    _pointMagnet.z         = 0;

    _playerPosX              = 0;
    _playerPosY              = 0;
    _playerPosZ              = 0;
    _playerMomentumX         = 0;
    _playerMomentumY         = 0;
    _playerMomentumZ         = 0;
    _player1DistanceToPointX = 0;
    _player1DistanceToPointY = 0;
    _player1DistanceToPointZ = 0;
    _player1DistanceToPoint  = 0;
    _momentumMagnet          = 0;
  }

private:
  __INLINE__ void registerGameProperties() override
  {
    // Registering native game properties
    registerGameProperty("Player Pos X", &_playerPosX, Property::datatype_t::dt_float32, Property::endianness_t::little);
    registerGameProperty("Player Pos Y", &_playerPosY, Property::datatype_t::dt_float32, Property::endianness_t::little);
    registerGameProperty("Player Pos Z", &_playerPosZ, Property::datatype_t::dt_float32, Property::endianness_t::little);

    for (int i = 0; i < numlines; i++)
    {
      std::string propertyName = std::string("Line[") + std::to_string(lines[i].iLineID) + std::string("] Activations");
      registerGameProperty(propertyName, &lines[i].player_activations, Property::datatype_t::dt_int32, Property::endianness_t::little);
    }
  }

  __INLINE__ void advanceStateImpl(const InputSet::inputIndex_t input) override
  {
    // Running emulator
    _emulator->advanceState(input);
  }

  __INLINE__ jaffarCommon::hash::hash_t getDirectStateHash() const override
  {
    jaffarCommon::hash::hash_t value = {0, 0};
    if (players[0].mo != nullptr)
    {
      value.first |= ((uint64_t)((uint16_t)(players[0].mo->x >> FRACBITS))) << 0;
      value.first |= ((uint64_t)((uint16_t)(players[0].mo->y >> FRACBITS))) << 16;
      value.first |= ((uint64_t)((uint16_t)(players[0].mo->momx >> FRACBITS))) << 32;
      value.first |= ((uint64_t)((uint16_t)(players[0].mo->momy >> FRACBITS))) << 48;
      value.second |= ((uint64_t)((uint16_t)(players[0].mo->angle >> FRACBITS))) << 0;
    }
    return value;
  }

  __INLINE__ void computeAdditionalHashing(MetroHash128& hashEngine) const override
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

    _totalMomentum = sqrt(_playerMomentumX * _playerMomentumX + _playerMomentumY * _playerMomentumY + _playerMomentumZ * _playerMomentumZ);
  }

  __INLINE__ void ruleUpdatePreHook() override
  {
    // Resetting magnets ahead of rule re-evaluation
    _pointMagnet.intensity = 0.0;
    _pointMagnet.x         = 0.0;
    _pointMagnet.y         = 0.0;
    _pointMagnet.z         = 0.0;

    _momentumMagnet = 0.0;
  }

  __INLINE__ void ruleUpdatePostHook() override
  {
    // Updating distance to user-defined point
    _player1DistanceToPointX = std::abs(_pointMagnet.x - _playerPosX);
    _player1DistanceToPointY = std::abs(_pointMagnet.y - _playerPosY);
    _player1DistanceToPointZ = std::abs(_pointMagnet.z - _playerPosZ);
    _player1DistanceToPoint =
        sqrtf(_player1DistanceToPointX * _player1DistanceToPointX + _player1DistanceToPointY * _player1DistanceToPointY + _player1DistanceToPointZ * _player1DistanceToPointZ);
  }

  __INLINE__ void serializeStateImpl(jaffarCommon::serializer::Base& serializer) const override
  {
    serializer.push(&gametic, sizeof(gametic));
    // serializer.push(&players[0].mo->x, sizeof(players[0].mo->x));
    // serializer.push(&players[0].mo->y, sizeof(players[0].mo->y));
    // serializer.push(&players[0].mo->z, sizeof(players[0].mo->z));
    // serializer.push(&players[0].mo->angle, sizeof(players[0].mo->angle));
    // serializer.push(&players[0].mo->momx, sizeof(players[0].mo->momx));
    // serializer.push(&players[0].mo->momy, sizeof(players[0].mo->momy));
    // serializer.push(&players[0].mo->momz, sizeof(players[0].mo->momz));

    // serializer.push(&players[0].mo->sprite, sizeof(players[0].mo->sprite));
    // serializer.push(&players[0].mo->frame, sizeof(players[0].mo->frame));
    // serializer.push(&players[0].mo->floorz, sizeof(players[0].mo->floorz));
    // serializer.push(&players[0].mo->ceilingz, sizeof(players[0].mo->ceilingz));
    // serializer.push(&players[0].mo->dropoffz, sizeof(players[0].mo->dropoffz));
    // serializer.push(&players[0].mo->radius, sizeof(players[0].mo->radius));
    // serializer.push(&players[0].mo->height, sizeof(players[0].mo->height));
    // serializer.push(&players[0].mo->validcount, sizeof(players[0].mo->validcount));
    // serializer.push(&players[0].mo->type, sizeof(players[0].mo->type));
    // serializer.push(&players[0].mo->tics, sizeof(players[0].mo->tics));
    // serializer.push(&players[0].mo->flags, sizeof(players[0].mo->flags));
    // serializer.push(&players[0].mo->intflags, sizeof(players[0].mo->intflags));
    // serializer.push(&players[0].mo->health, sizeof(players[0].mo->health));
    // serializer.push(&players[0].mo->movedir, sizeof(players[0].mo->movedir));
    // serializer.push(&players[0].mo->movecount, sizeof(players[0].mo->movecount));
    // serializer.push(&players[0].mo->strafecount, sizeof(players[0].mo->strafecount));
    // serializer.push(&players[0].mo->reactiontime, sizeof(players[0].mo->reactiontime));
    // serializer.push(&players[0].mo->threshold, sizeof(players[0].mo->threshold));
    // serializer.push(&players[0].mo->pursuecount, sizeof(players[0].mo->pursuecount));
    // serializer.push(&players[0].mo->gear, sizeof(players[0].mo->gear));
    // serializer.push(&players[0].mo->lastlook, sizeof(players[0].mo->lastlook));
    // serializer.push(&players[0].mo->friction, sizeof(players[0].mo->friction));
    // serializer.push(&players[0].mo->movefactor, sizeof(players[0].mo->movefactor));
    // serializer.push(&players[0].mo->pitch, sizeof(players[0].mo->pitch));
    // serializer.push(&players[0].mo->index, sizeof(players[0].mo->index));
    // serializer.push(&players[0].mo->patch_width, sizeof(players[0].mo->patch_width));
    // serializer.push(&players[0].mo->iden_nums, sizeof(players[0].mo->iden_nums));
    // serializer.push(&players[0].mo->gravity, sizeof(players[0].mo->gravity));
  }

  __INLINE__ void deserializeStateImpl(jaffarCommon::deserializer::Base& deserializer)
  {
    deserializer.pop(&gametic, sizeof(gametic));
    // deserializer.pop(&players[0].mo->x, sizeof(players[0].mo->x));
    // deserializer.pop(&players[0].mo->y, sizeof(players[0].mo->y));
    // deserializer.pop(&players[0].mo->z, sizeof(players[0].mo->z));
    // deserializer.pop(&players[0].mo->angle, sizeof(players[0].mo->angle));
    // deserializer.pop(&players[0].mo->momx, sizeof(players[0].mo->momx));
    // deserializer.pop(&players[0].mo->momy, sizeof(players[0].mo->momy));
    // deserializer.pop(&players[0].mo->momz, sizeof(players[0].mo->momz));

    // deserializer.pop(&players[0].mo->sprite, sizeof(players[0].mo->sprite));
    // deserializer.pop(&players[0].mo->frame, sizeof(players[0].mo->frame));
    // deserializer.pop(&players[0].mo->floorz, sizeof(players[0].mo->floorz));
    // deserializer.pop(&players[0].mo->ceilingz, sizeof(players[0].mo->ceilingz));
    // deserializer.pop(&players[0].mo->dropoffz, sizeof(players[0].mo->dropoffz));
    // deserializer.pop(&players[0].mo->radius, sizeof(players[0].mo->radius));
    // deserializer.pop(&players[0].mo->height, sizeof(players[0].mo->height));
    // deserializer.pop(&players[0].mo->validcount, sizeof(players[0].mo->validcount));
    // deserializer.pop(&players[0].mo->type, sizeof(players[0].mo->type));
    // deserializer.pop(&players[0].mo->tics, sizeof(players[0].mo->tics));
    // deserializer.pop(&players[0].mo->flags, sizeof(players[0].mo->flags));
    // deserializer.pop(&players[0].mo->intflags, sizeof(players[0].mo->intflags));
    // deserializer.pop(&players[0].mo->health, sizeof(players[0].mo->health));
    // deserializer.pop(&players[0].mo->movedir, sizeof(players[0].mo->movedir));
    // deserializer.pop(&players[0].mo->movecount, sizeof(players[0].mo->movecount));
    // deserializer.pop(&players[0].mo->strafecount, sizeof(players[0].mo->strafecount));
    // deserializer.pop(&players[0].mo->reactiontime, sizeof(players[0].mo->reactiontime));
    // deserializer.pop(&players[0].mo->threshold, sizeof(players[0].mo->threshold));
    // deserializer.pop(&players[0].mo->pursuecount, sizeof(players[0].mo->pursuecount));
    // deserializer.pop(&players[0].mo->gear, sizeof(players[0].mo->gear));
    // deserializer.pop(&players[0].mo->lastlook, sizeof(players[0].mo->lastlook));
    // deserializer.pop(&players[0].mo->friction, sizeof(players[0].mo->friction));
    // deserializer.pop(&players[0].mo->movefactor, sizeof(players[0].mo->movefactor));
    // deserializer.pop(&players[0].mo->pitch, sizeof(players[0].mo->pitch));
    // deserializer.pop(&players[0].mo->index, sizeof(players[0].mo->index));
    // deserializer.pop(&players[0].mo->patch_width, sizeof(players[0].mo->patch_width));
    // deserializer.pop(&players[0].mo->iden_nums, sizeof(players[0].mo->iden_nums));
    // deserializer.pop(&players[0].mo->gravity, sizeof(players[0].mo->gravity));
  }

  __INLINE__ float calculateGameSpecificReward() const
  {
    // Getting rewards from rules
    float reward = 0.0;

    // Reward advancing always
    if (std::abs(_pointMagnet.intensity) > 0.0f) reward += -1.0 * _pointMagnet.intensity * _player1DistanceToPoint;

    // Reward total momentum
    if (std::abs(_momentumMagnet) > 0.0f) reward += _totalMomentum * _momentumMagnet;

    // Returning reward
    return reward;
  }

  void printInfoImpl() const override
  {
    char mapName[512];
    headlessGetMapName(mapName);
    jaffarCommon::logger::log("[J+] Map:        %s\n", mapName);
    jaffarCommon::logger::log("[J+] Game Tic:   %d\n", gametic);
    jaffarCommon::logger::log("[J+] Level Exit: %s\n", reachedLevelExit == 1 ? "Yes" : "No");
    jaffarCommon::logger::log("[J+] Game End:   %s\n", reachedGameEnd == 1 ? "Yes" : "No");

    if (players[0].mo != nullptr)
    {
      jaffarCommon::logger::log("[J+] Player 1 Coordinates:    (%f, %f, %f)\n", _playerPosX, _playerPosY, _playerPosZ);
      jaffarCommon::logger::log("[J+] Player 1 Angle:           %lu\n", players[0].mo->angle);
      jaffarCommon::logger::log("[J+] Player 1 Momentum:        %f (%f, %f, %f)\n", _totalMomentum, _playerMomentumX, _playerMomentumY, _playerMomentumZ);
      jaffarCommon::logger::log("[J+] Player 1 Health:          %d\n", players[0].mo->health);
    }

    // jaffarCommon::logger::log("[] Line Information: \n");
    // for (int i = 0; i < numlines; i++)
    // {
    //   jaffarCommon::logger::log("[] Line %3d: Flags: %d, Activations: %d\n", lines[i].iLineID, lines[i].flags, lines[i].player_activations);
    // }

    if (std::abs(_momentumMagnet) > 0.0f) jaffarCommon::logger::log("[J+]  + Momentum Magnet                          Intensity: %.5f\n", _momentumMagnet);

    if (std::abs(_pointMagnet.intensity) > 0.0f)
    {
      jaffarCommon::logger::log("[J+]  + Point Magnet                             Intensity: %.5f, X: %3.3f, Y: %3.3f, Z: %3.3f\n", _pointMagnet.intensity, _pointMagnet.x,
                                _pointMagnet.y, _pointMagnet.z);
      jaffarCommon::logger::log("[J+]    + Distance X                             %3.3f\n", _player1DistanceToPointX);
      jaffarCommon::logger::log("[J+]    + Distance Y                             %3.3f\n", _player1DistanceToPointY);
      jaffarCommon::logger::log("[J+]    + Distance Z                             %3.3f\n", _player1DistanceToPointZ);
      jaffarCommon::logger::log("[J+]    + Total Distance                         %3.3f\n", _player1DistanceToPoint);
    }
  }

  bool parseRuleActionImpl(Rule& rule, const std::string& actionType, const nlohmann::json& actionJs) override
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

    if (actionType == "Set Momentum Magnet")
    {
      auto intensity = jaffarCommon::json::getNumber<float>(actionJs, "Intensity");
      rule.addAction([=, this]() { this->_momentumMagnet = intensity; });
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
  float         _momentumMagnet;
  float         _totalMomentum;

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