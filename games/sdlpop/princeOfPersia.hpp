#pragma once

#include <set>
#include <jaffarCommon/json.hpp>
#include <emulator.hpp>
#include <game.hpp>

namespace jaffarPlus
{

namespace games
{

namespace sdlpop
{

const uint8_t roomCount = 24;

class PrinceOfPersia final : public jaffarPlus::Game
{
  public:

  static __INLINE__ std::string getName() { return "SDLPoP / Prince of Persia"; }

  PrinceOfPersia(std::unique_ptr<Emulator> emulator, const nlohmann::json &config)
    : jaffarPlus::Game(std::move(emulator), config)
  {
    // // Getting watch mobs indexes
    // auto watchMobsIndexesJs = jaffarCommon::json::getArray<nlohmann::json>(config, "Watch Moving Object Indexes");
    // for (const auto& mobJs : watchMobsIndexesJs)
    // {
    //   uint8_t room = jaffarCommon::json::getNumber<uint8_t>(mobJs, "Room");
    //   uint8_t column = jaffarCommon::json::getNumber<uint8_t>(mobJs, "Column");
    //   uint16_t index = room * 10 + column;
    //   _watchMobsIndexes[index] = mobsIndex_t{.room = room, .column = column };
    // }
  }

  private:

  __INLINE__ void registerGameProperties() override
  {
    _gameState = (quicker::sdlPopState_t *)_emulator->getProperty("Game State").pointer;

    registerGameProperty("Checkpoint Reached", &_gameState->checkpoint, Property::datatype_t::dt_uint16, Property::endianness_t::little);
    registerGameProperty("Is Upside Down", &_gameState->upside_down, Property::datatype_t::dt_uint16, Property::endianness_t::little);
    registerGameProperty("Is Drawn Room", &_gameState->drawn_room, Property::datatype_t::dt_uint16, Property::endianness_t::little);
    registerGameProperty("Current Level", &_gameState->current_level, Property::datatype_t::dt_uint16, Property::endianness_t::little);
    registerGameProperty("Next Level", &_gameState->next_level, Property::datatype_t::dt_uint16, Property::endianness_t::little);
    registerGameProperty("Mobs Count", &_gameState->mobs_count, Property::datatype_t::dt_int16, Property::endianness_t::little);
    registerGameProperty("Trobs Count", &_gameState->trobs_count, Property::datatype_t::dt_int16, Property::endianness_t::little);
    registerGameProperty("Level Door Open", &_gameState->leveldoor_open, Property::datatype_t::dt_uint16, Property::endianness_t::little);
    registerGameProperty("Kid Frame", &_gameState->Kid.frame, Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Kid Pos X Raw", &_gameState->Kid.x, Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Kid Pos Y Raw", &_gameState->Kid.y, Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Kid Direction", &_gameState->Kid.direction, Property::datatype_t::dt_int8, Property::endianness_t::little);
    registerGameProperty("Kid Current Col", &_gameState->Kid.curr_col, Property::datatype_t::dt_int8, Property::endianness_t::little);
    registerGameProperty("Kid Current Row", &_gameState->Kid.curr_row, Property::datatype_t::dt_int8, Property::endianness_t::little);
    registerGameProperty("Kid Action", &_gameState->Kid.action, Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Kid Fall Speed X", &_gameState->Kid.fall_x, Property::datatype_t::dt_int8, Property::endianness_t::little);
    registerGameProperty("Kid Fall Speed Y", &_gameState->Kid.fall_y, Property::datatype_t::dt_int8, Property::endianness_t::little);
    registerGameProperty("Kid Room", &_gameState->Kid.room, Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Kid Repeat", &_gameState->Kid.repeat, Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Kid Char Id", &_gameState->Kid.charid, Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Kid Sword", &_gameState->Kid.sword, Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Kid Is Alive", &_gameState->Kid.alive, Property::datatype_t::dt_int8, Property::endianness_t::little);
    registerGameProperty("Kid Current Sequence", &_gameState->Kid.curr_seq, Property::datatype_t::dt_uint16, Property::endianness_t::little);
    registerGameProperty("Kid Current HP", &_gameState->hitp_curr, Property::datatype_t::dt_uint16, Property::endianness_t::little);
    registerGameProperty("Kid Max HP", &_gameState->hitp_max, Property::datatype_t::dt_uint16, Property::endianness_t::little);
    registerGameProperty("Kid Initial HP", &_gameState->hitp_beg_lev, Property::datatype_t::dt_uint16, Property::endianness_t::little);
    registerGameProperty("Grab Timer", &_gameState->grab_timer, Property::datatype_t::dt_uint16, Property::endianness_t::little);
    registerGameProperty("Holding Sword", &_gameState->holding_sword, Property::datatype_t::dt_uint16, Property::endianness_t::little);
    registerGameProperty("United With Shadow", &_gameState->united_with_shadow, Property::datatype_t::dt_int16, Property::endianness_t::little);
    registerGameProperty("Have Sword", &_gameState->have_sword, Property::datatype_t::dt_uint16, Property::endianness_t::little);
    registerGameProperty("Kid Sword Strike", &_gameState->kid_sword_strike, Property::datatype_t::dt_uint16, Property::endianness_t::little);
    registerGameProperty("Pickup Object Type", &_gameState->pickup_obj_type, Property::datatype_t::dt_int16, Property::endianness_t::little);
    registerGameProperty("Off Guard", &_gameState->offguard, Property::datatype_t::dt_uint16, Property::endianness_t::little);
    registerGameProperty("Guard Frame", &_gameState->Guard.frame, Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Guard Pos X Raw", &_gameState->Guard.x, Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Guard Pos Y Raw", &_gameState->Guard.y, Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Guard Direction", &_gameState->Guard.direction, Property::datatype_t::dt_int8, Property::endianness_t::little);
    registerGameProperty("Guard Current Col", &_gameState->Guard.curr_col, Property::datatype_t::dt_int8, Property::endianness_t::little);
    registerGameProperty("Guard Current Row", &_gameState->Guard.curr_row, Property::datatype_t::dt_int8, Property::endianness_t::little);
    registerGameProperty("Guard Action", &_gameState->Guard.action, Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Guard Fall Speed X", &_gameState->Guard.fall_x, Property::datatype_t::dt_int8, Property::endianness_t::little);
    registerGameProperty("Guard Fall Speed Y", &_gameState->Guard.fall_y, Property::datatype_t::dt_int8, Property::endianness_t::little);
    registerGameProperty("Guard Room", &_gameState->Guard.room, Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Guard Repeat", &_gameState->Guard.repeat, Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Guard Char Id", &_gameState->Guard.charid, Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Guard Sword", &_gameState->Guard.sword, Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Guard Is Alive", &_gameState->Guard.alive, Property::datatype_t::dt_int8, Property::endianness_t::little);
    registerGameProperty("Guard Current Sequence", &_gameState->Guard.curr_seq, Property::datatype_t::dt_uint16, Property::endianness_t::little);
    registerGameProperty("Guard Current HP", &_gameState->guardhp_curr, Property::datatype_t::dt_uint16, Property::endianness_t::little);
    registerGameProperty("Guard Max HP", &_gameState->guardhp_max, Property::datatype_t::dt_uint16, Property::endianness_t::little);
    registerGameProperty("Demo Index", &_gameState->demo_index, Property::datatype_t::dt_uint16, Property::endianness_t::little);
    registerGameProperty("Demo Time", &_gameState->demo_time, Property::datatype_t::dt_int16, Property::endianness_t::little);
    registerGameProperty("Current Guard Color", &_gameState->curr_guard_color, Property::datatype_t::dt_uint16, Property::endianness_t::little);
    registerGameProperty("Guard Notice Timer", &_gameState->guard_notice_timer, Property::datatype_t::dt_int16, Property::endianness_t::little);
    registerGameProperty("Guard Skill", &_gameState->guard_skill, Property::datatype_t::dt_uint16, Property::endianness_t::little);
    registerGameProperty("Shadow Initialized", &_gameState->shadow_initialized, Property::datatype_t::dt_uint16, Property::endianness_t::little);
    registerGameProperty("Guard Refrac", &_gameState->guard_refrac, Property::datatype_t::dt_uint16, Property::endianness_t::little);
    registerGameProperty("Just Blocked", &_gameState->justblocked, Property::datatype_t::dt_uint16, Property::endianness_t::little);
    registerGameProperty("Dropped Out", &_gameState->droppedout, Property::datatype_t::dt_uint16, Property::endianness_t::little);
    registerGameProperty("Previous Collision Row", &_gameState->prev_collision_row, Property::datatype_t::dt_int8, Property::endianness_t::little);
    registerGameProperty("Flash Color", &_gameState->flash_color, Property::datatype_t::dt_uint16, Property::endianness_t::little);
    registerGameProperty("Flash Time", &_gameState->flash_time, Property::datatype_t::dt_uint16, Property::endianness_t::little);
    registerGameProperty("Need Level 1 Music", &_gameState->need_level1_music, Property::datatype_t::dt_uint16, Property::endianness_t::little);
    registerGameProperty("Is Screaming", &_gameState->is_screaming, Property::datatype_t::dt_uint16, Property::endianness_t::little);
    registerGameProperty("Is Feather Fall", &_gameState->is_feather_fall, Property::datatype_t::dt_uint16, Property::endianness_t::little);
    registerGameProperty("Last Loose Sound", &_gameState->last_loose_sound, Property::datatype_t::dt_uint16, Property::endianness_t::little);
    registerGameProperty("Random Seed", &_gameState->random_seed, Property::datatype_t::dt_uint32, Property::endianness_t::little);
    registerGameProperty("Exit Room Timer", &_gameState->exit_room_timer, Property::datatype_t::dt_uint16, Property::endianness_t::little);
    registerGameProperty("Is Guard Notice", &_gameState->is_guard_notice, Property::datatype_t::dt_uint16, Property::endianness_t::little);
    registerGameProperty("Can Guard See Kid", &_gameState->can_guard_see_kid, Property::datatype_t::dt_int16, Property::endianness_t::little);
    registerGameProperty("Collision Row", &_gameState->collision_row, Property::datatype_t::dt_int8, Property::endianness_t::little);
    registerGameProperty("Jumped Through Mirror", &_gameState->jumped_through_mirror, Property::datatype_t::dt_int16, Property::endianness_t::little);
    registerGameProperty("Kid Previous Frame", &_gameState->kidPreviousFrame, Property::datatype_t::dt_uint8, Property::endianness_t::little);

    // Calculated values
    registerGameProperty("Kid Pos X", &_kidPosX, Property::datatype_t::dt_float32, Property::endianness_t::little);
    registerGameProperty("Kid Pos Y", &_kidPosY, Property::datatype_t::dt_float32, Property::endianness_t::little);

    // Registering element properties
    std::string propertyName;

    for (size_t i = 1; i <= 24; i++)
      for (size_t j = 1; j <= 30; j++)
        {
          propertyName = std::string("Foreground Element[") + std::to_string(i) + std::string("][") + std::to_string(j) + std::string("]");
          registerGameProperty(propertyName, &_gameState->level.fg[i - 1][j - 1], Property::datatype_t::dt_uint8, Property::endianness_t::little);

          propertyName = std::string("Background Element[") + std::to_string(i) + std::string("][") + std::to_string(j) + std::string("]");
          registerGameProperty(propertyName, &_gameState->level.bg[i - 1][j - 1], Property::datatype_t::dt_uint8, Property::endianness_t::little);
        }

    // Registering room properties
    for (size_t i = 0; i < 24; i++)
      {
        propertyName = std::string("Room Guard Tile[") + std::to_string(i) + std::string("]");
        registerGameProperty(propertyName, &_gameState->level.guards_tile[i], Property::datatype_t::dt_uint8, Property::endianness_t::little);

        propertyName = std::string("Room Guard Direction[") + std::to_string(i) + std::string("]");
        registerGameProperty(propertyName, &_gameState->level.guards_dir[i], Property::datatype_t::dt_uint8, Property::endianness_t::little);

        propertyName = std::string("Room Guard Pos X[") + std::to_string(i) + std::string("]");
        registerGameProperty(propertyName, &_gameState->level.guards_x[i], Property::datatype_t::dt_uint8, Property::endianness_t::little);

        propertyName = std::string("Room Guard Sequence Low[") + std::to_string(i) + std::string("]");
        registerGameProperty(propertyName, &_gameState->level.guards_skill[i], Property::datatype_t::dt_uint8, Property::endianness_t::little);

        propertyName = std::string("Room Guard Skill[") + std::to_string(i) + std::string("]");
        registerGameProperty(propertyName, &_gameState->level.guards_seq_hi[i], Property::datatype_t::dt_uint8, Property::endianness_t::little);

        propertyName = std::string("Room Guard Sequence High[") + std::to_string(i) + std::string("]");
        registerGameProperty(propertyName, &_gameState->level.guards_seq_hi[i], Property::datatype_t::dt_uint8, Property::endianness_t::little);
      }

    // Registering Moving Objects
    for (size_t i = 0; i < 24; i++)
      {
        propertyName = std::string("Moving Object[") + std::to_string(i) + std::string("] Pos X");
        registerGameProperty(propertyName, &_gameState->mobs[i].xh, Property::datatype_t::dt_uint8, Property::endianness_t::little);

        propertyName = std::string("Moving Object[") + std::to_string(i) + std::string("] Pos Y");
        registerGameProperty(propertyName, &_gameState->mobs[i].y, Property::datatype_t::dt_uint8, Property::endianness_t::little);

        propertyName = std::string("Moving Object[") + std::to_string(i) + std::string("] Room");
        registerGameProperty(propertyName, &_gameState->mobs[i].room, Property::datatype_t::dt_uint8, Property::endianness_t::little);

        propertyName = std::string("Moving Object[") + std::to_string(i) + std::string("] Speed");
        registerGameProperty(propertyName, &_gameState->mobs[i].speed, Property::datatype_t::dt_int8, Property::endianness_t::little);

        propertyName = std::string("Moving Object[") + std::to_string(i) + std::string("] Type");
        registerGameProperty(propertyName, &_gameState->mobs[i].type, Property::datatype_t::dt_uint8, Property::endianness_t::little);

        propertyName = std::string("Moving Object[") + std::to_string(i) + std::string("] Row");
        registerGameProperty(propertyName, &_gameState->mobs[i].row, Property::datatype_t::dt_uint8, Property::endianness_t::little);
      }

    // Registering Transformable Objects
    for (size_t i = 0; i < 30; i++)
      {
        propertyName = std::string("Transformable Object[") + std::to_string(i) + std::string("] Tile");
        registerGameProperty(propertyName, &_gameState->trobs[i].tilepos, Property::datatype_t::dt_uint8, Property::endianness_t::little);

        propertyName = std::string("Transformable Object[") + std::to_string(i) + std::string("] Room");
        registerGameProperty(propertyName, &_gameState->trobs[i].room, Property::datatype_t::dt_uint8, Property::endianness_t::little);

        propertyName = std::string("Transformable Object[") + std::to_string(i) + std::string("] Type");
        registerGameProperty(propertyName, &_gameState->trobs[i].type, Property::datatype_t::dt_int8, Property::endianness_t::little);
      }
  }

  __INLINE__ void advanceStateImpl(const InputSet::inputIndex_t input) override { _emulator->advanceState(input); }

  __INLINE__ void computeAdditionalHashing(MetroHash128 &hashEngine) const override
  {
    hashEngine.Update(_gameState->checkpoint);
    hashEngine.Update(_gameState->upside_down);
    hashEngine.Update(_gameState->drawn_room);
    hashEngine.Update(_gameState->current_level);
    hashEngine.Update(_gameState->next_level);
    hashEngine.Update(_gameState->mobs_count);
    hashEngine.Update(_gameState->trobs_count);
    hashEngine.Update(_gameState->leveldoor_open);
    hashEngine.Update(_gameState->Kid);
    hashEngine.Update(_gameState->hitp_curr);
    hashEngine.Update(_gameState->hitp_max);
    hashEngine.Update(_gameState->hitp_beg_lev);
    hashEngine.Update(_gameState->grab_timer);
    hashEngine.Update(_gameState->holding_sword);
    hashEngine.Update(_gameState->united_with_shadow);
    hashEngine.Update(_gameState->have_sword);
    hashEngine.Update(_gameState->kid_sword_strike);
    hashEngine.Update(_gameState->pickup_obj_type);
    hashEngine.Update(_gameState->offguard);
    hashEngine.Update(_gameState->Guard);
    hashEngine.Update(_gameState->guardhp_curr);
    hashEngine.Update(_gameState->guardhp_max);
    hashEngine.Update(_gameState->demo_index);
    hashEngine.Update(_gameState->demo_time);
    hashEngine.Update(_gameState->curr_guard_color);
    hashEngine.Update(_gameState->guard_notice_timer);
    hashEngine.Update(_gameState->guard_skill);
    hashEngine.Update(_gameState->shadow_initialized);
    hashEngine.Update(_gameState->guard_refrac);
    hashEngine.Update(_gameState->justblocked);
    hashEngine.Update(_gameState->droppedout);
    hashEngine.Update(_gameState->prev_collision_row);
    hashEngine.Update(_gameState->flash_color);
    hashEngine.Update(_gameState->flash_time);
    hashEngine.Update(_gameState->need_level1_music);
    hashEngine.Update(_gameState->is_screaming);
    hashEngine.Update(_gameState->is_feather_fall);
    hashEngine.Update(_gameState->exit_room_timer);
    hashEngine.Update(_gameState->is_guard_notice);
    hashEngine.Update(_gameState->can_guard_see_kid);
    hashEngine.Update(_gameState->collision_row);
    hashEngine.Update(_gameState->jumped_through_mirror);
    hashEngine.Update(_gameState->kidPreviousFrame);

    // Hashing all mobs by default
    hashEngine.Update(_gameState->mobs);
  }

  // Updating derivative values after updating the internal state
  __INLINE__ void stateUpdatePostHook() override
  {
    uint8_t kidFrameDiff = _gameState->Kid.frame - _gameState->kidPreviousFrame;
    _kidPosY             = (float)_gameState->Kid.y;

    // If climbing down, add pos y. Otherwise subtract
    if (_gameState->Kid.frame >= 0x8D && _gameState->Kid.frame <= 0x94 && kidFrameDiff < 0) _kidPosY += (0x94 - _gameState->Kid.frame);
    if (_gameState->Kid.frame >= 0x8D && _gameState->Kid.frame <= 0x94 && kidFrameDiff > 0) _kidPosY += 7.0f - (_gameState->Kid.frame - 0x8D);

    // If jumpclimb up, subtract pos y
    if (_gameState->Kid.frame >= 0x43 && _gameState->Kid.frame <= 0x4F) _kidPosY -= 16.0f - (0x4F - _gameState->Kid.frame);

    // If hanging, subtract pos y
    if (_gameState->Kid.frame == 0x50) _kidPosY -= 20.0f;
    if (_gameState->Kid.frame >= 0x57 && _gameState->Kid.frame <= 0x5B) _kidPosY -= 25.0f - (0x5B - _gameState->Kid.frame);

    // Climbing up
    if (_gameState->Kid.frame >= 0x87 && _gameState->Kid.frame < 0x8D) _kidPosY -= 32.0f + 7.0f - (0x8D - _gameState->Kid.frame);
  }

  __INLINE__ void ruleUpdatePreHook() override
  {
    _kidDirectionMagnet     = 0.0;
    _levelDoorOpenMagnet    = 0.0;
    _unitedWithShadowMagnet = 0.0;
    _guardHPMagnet          = 0.0;

    _kidPosXMagnet.intensity   = 0.0;
    _kidPosYMagnet.intensity   = 0.0;
    _guardPosXMagnet.intensity = 0.0;
    _guardPosYMagnet.intensity = 0.0;
  }

  __INLINE__ void ruleUpdatePostHook() override {}

  __INLINE__ void serializeStateImpl(jaffarCommon::serializer::Base &serializer) const override
  {
    // Nothing additional to serialize
  }

  __INLINE__ void deserializeStateImpl(jaffarCommon::deserializer::Base &deserializer)
  {
    // Nothing additional to serialize
  }

  __INLINE__ float calculateGameSpecificReward() const
  {
    // Getting rewards from rules
    float reward = 0.0;

    // Container for bounded value and difference with center
    float diff = 0.0;

    // Evaluating kid magnet's reward on position X
    diff = -255.0f + std::abs(_kidPosXMagnet.position - (float)_gameState->Kid.x);
    reward += _kidPosXMagnet.intensity * -diff;

    // Evaluating kid magnet's reward on position Y
    diff = -255.0f + std::abs(_kidPosYMagnet.position - (float)_gameState->Kid.y);
    reward += _kidPosYMagnet.intensity * -diff;

    // Evaluating guard magnet's reward on position X
    diff = -255.0f + std::abs(_guardPosXMagnet.position - (float)_gameState->Guard.x);
    reward += _guardPosXMagnet.intensity * -diff;

    // Evaluating guard magnet's reward on position Y
    diff = -255.0f + std::abs(_guardPosYMagnet.position - (float)_gameState->Guard.y);
    reward += _guardPosYMagnet.intensity * -diff;

    // Kid Direction Magnet
    reward += _gameState->Kid.direction == 0 ? 1.0 : -1.0 * _kidDirectionMagnet;

    // Rewarding climb stairs
    if (_gameState->Kid.frame >= 217 && _gameState->Kid.frame <= 228) reward += (_gameState->Kid.frame - 217 + 1) * 1.f;
    //if (_gameState->Kid.frame == 0 && _gameState->hitp_curr > 0) reward += 12000.0f;

    // Rewarding level door open
    reward += (float)_gameState->leveldoor_open * _levelDoorOpenMagnet;

    // Rewarding united with shadow
    reward += (float)_gameState->united_with_shadow * _unitedWithShadowMagnet;

    // Rewarding united with shadow
    reward += (float)(_gameState->guardhp_max - _gameState->guardhp_curr) * _guardHPMagnet;

    // Returning reward
    return reward;
  }

  void printInfoImpl() const override
  {
    // Calculating timing
    size_t curMins      = _gameState->globalStepCount / 720;
    size_t curSecs      = (_gameState->globalStepCount % 720) / 12;
    size_t curMilliSecs = floor((double)(_gameState->globalStepCount % 12) / 0.012);

    jaffarCommon::logger::log("[J+]  + Global Step Counter:  %05d\n", _gameState->globalStepCount);
    jaffarCommon::logger::log("[J+]  + Current/Next Level:   %2d / %2d\n", _gameState->current_level, _gameState->next_level);
    jaffarCommon::logger::log("[J+]  + Timer:                %2lu:%02lu.%03lu\n", curMins, curSecs, curMilliSecs);
    jaffarCommon::logger::log("[J+]  + [Kid]                 Room: %d, Pos.x: %3d, Pos.y: %f (%3d), Frame: %3d, Action: %2d, Dir: %d, HP: %d/%d, Alive: %d\n",
                              int(_gameState->Kid.room),
                              int(_gameState->Kid.x),
                              _kidPosY,
                              int(_gameState->Kid.y),
                              int(_gameState->Kid.frame),
                              int(_gameState->Kid.action),
                              int(_gameState->Kid.direction),
                              int(_gameState->hitp_curr),
                              int(_gameState->hitp_max),
                              int(_gameState->Kid.alive));
    jaffarCommon::logger::log("[J+]  + [Guard]               Room: %d, Pos.x: %3d, Pos.y: %3d, Frame: %3d, Action: %2d, Color: %3u, Dir: %d, HP: %d/%d, Alive: %d\n",
                              int(_gameState->Guard.room),
                              int(_gameState->Guard.x),
                              int(_gameState->Guard.y),
                              int(_gameState->Guard.frame),
                              int(_gameState->Guard.action),
                              int(_gameState->curr_guard_color),
                              int(_gameState->Guard.direction),
                              int(_gameState->guardhp_curr),
                              int(_gameState->guardhp_max),
                              int(_gameState->Guard.alive));
    jaffarCommon::logger::log("[J+]  + Exit Room Timer:      %d\n", _gameState->exit_room_timer);
    jaffarCommon::logger::log("[J+]  + Kid Has Sword:        %d\n", _gameState->have_sword);

    if (_gameState->current_level == 1) jaffarCommon::logger::log("[J+]  + Level 1 Need Music:   %d\n", _gameState->need_level1_music);
    if (_gameState->current_level == 8) jaffarCommon::logger::log("[J+]  + Level Door Open:      %d\n", _gameState->leveldoor_open);
    if (_gameState->current_level == 3) jaffarCommon::logger::log("[J+]  + Reached Checkpoint:   %s\n", _gameState->checkpoint ? "Yes" : "No");
    if (_gameState->current_level == 7) jaffarCommon::logger::log("[J+]  + Feather Fall:         %d\n", _gameState->is_feather_fall);
    if (_gameState->current_level == 12) jaffarCommon::logger::log("[J+]  + United With Shadow:   %d\n", _gameState->united_with_shadow);

    auto prevRNG1 = SDLPoPInstance::reverseRNGState(_gameState->random_seed);
    auto prevRNG2 = SDLPoPInstance::reverseRNGState(prevRNG1);
    auto prevRNG3 = SDLPoPInstance::reverseRNGState(prevRNG2);
    auto prevRNG4 = SDLPoPInstance::reverseRNGState(prevRNG3);
    auto prevRNG5 = SDLPoPInstance::reverseRNGState(prevRNG4);

    auto postRNG1 = SDLPoPInstance::advanceRNGState(_gameState->random_seed);
    auto postRNG2 = SDLPoPInstance::advanceRNGState(postRNG1);
    auto postRNG3 = SDLPoPInstance::advanceRNGState(postRNG2);
    auto postRNG4 = SDLPoPInstance::advanceRNGState(postRNG3);
    auto postRNG5 = SDLPoPInstance::advanceRNGState(postRNG4);

    jaffarCommon::logger::log("[J+]  + RNG: [ 0x%08X 0x%08X 0x%08X 0x%08X 0x%08X ] 0x%08X [ 0x%08X 0x%08X 0x%08X 0x%08X 0x%08X ]\n",
                              prevRNG5,
                              prevRNG4,
                              prevRNG3,
                              prevRNG2,
                              prevRNG1,
                              _gameState->random_seed,
                              postRNG1,
                              postRNG2,
                              postRNG3,
                              postRNG4,
                              postRNG5);

    jaffarCommon::logger::log("[J+]  + Moving Objects:\n");
    for (int i = 0; i < _gameState->mobs_count; ++i)
      {
        const auto &mob = (_gameState->mobs)[i];
        jaffarCommon::logger::log("[J+]    + Room: %d, X: %d, Y: %d, Speed: %d, Type: %d, Row: %d\n", mob.room, mob.xh, mob.y, mob.speed, mob.type, mob.row);
      }

    jaffarCommon::logger::log("[J+]  + Transformable Objects:\n");
    for (int i = 0; i < _gameState->trobs_count; i++)
      {
        const auto &trob    = _gameState->trobs[i];
        const auto  roomIdx = trob.room - 1;
        const auto  tileIdx = trob.tilepos - 1;

        jaffarCommon::logger::log("[J+]    + Type: %u, Room: %u, Pos: %u, FG: %u, BG: %u\n",
                                  trob.type,
                                  trob.room,
                                  trob.tilepos,
                                  _gameState->level.fg[roomIdx][tileIdx],
                                  _gameState->level.bg[roomIdx][tileIdx]);
      }

    if (std::abs(_kidPosXMagnet.intensity) > 0.0f)
      jaffarCommon::logger::log("[J+]  + Kid Pos X Magnet          - Intensity: %.1f, Center: %u\n", _kidPosXMagnet.intensity, _kidPosXMagnet.position);
    if (std::abs(_kidPosYMagnet.intensity) > 0.0f)
      jaffarCommon::logger::log("[J+]  + Kid Pos Y Magnet          - Intensity: %.1f, Center: %u\n", _kidPosYMagnet.intensity, _kidPosYMagnet.position);
    if (std::abs(_kidDirectionMagnet) > 0.0f) jaffarCommon::logger::log("[J+]  + Kid Direction Magnet      - Intensity: %.1f\n", _kidDirectionMagnet);

    if (std::abs(_guardPosXMagnet.intensity) > 0.0f)
      jaffarCommon::logger::log("[J+]  + Guard Pos X Magnet        - Intensity: %.1f, Center: %u\n", _guardPosXMagnet.intensity, _guardPosXMagnet.position);
    if (std::abs(_guardPosYMagnet.intensity) > 0.0f)
      jaffarCommon::logger::log("[J+]  + Guard Pos Y Magnet        - Intensity: %.1f, Center: %u\n", _guardPosYMagnet.intensity, _guardPosYMagnet.position);

    if (std::abs(_levelDoorOpenMagnet) > 0.0f) jaffarCommon::logger::log("[J+]  + Level Door Open Magnet    - Intensity: %.1f\n", _levelDoorOpenMagnet);
    if (std::abs(_unitedWithShadowMagnet) > 0.0f) jaffarCommon::logger::log("[J+]  + United With Shadow Magnet - Intensity: %.1f\n", _unitedWithShadowMagnet);
    if (std::abs(_guardHPMagnet) > 0.0f) jaffarCommon::logger::log("[J+]  + Guard HP Magnet           - Intensity: %.1f\n", _guardHPMagnet);
  }

  bool parseRuleActionImpl(Rule &rule, const std::string &actionType, const nlohmann::json &actionJs) override
  {
    bool recognizedActionType = false;

    if (actionType == "Set Kid Pos X Magnet")
      {
        auto intensity = jaffarCommon::json::getNumber<float>(actionJs, "Intensity");
        auto position  = jaffarCommon::json::getNumber<uint8_t>(actionJs, "Position");
        auto room      = jaffarCommon::json::getNumber<uint8_t>(actionJs, "Room");
        auto action    = [=, this]() { _kidPosXMagnet = pointMagnet_t{.intensity = _gameState->Kid.room == room ? intensity : 0.0f, .position = position}; };
        rule.addAction(action);
        recognizedActionType = true;
    }

    if (actionType == "Set Kid Pos Y Magnet")
      {
        auto intensity = jaffarCommon::json::getNumber<float>(actionJs, "Intensity");
        auto position  = jaffarCommon::json::getNumber<uint8_t>(actionJs, "Position");
        auto room      = jaffarCommon::json::getNumber<uint8_t>(actionJs, "Room");
        auto action    = [=, this]() { _kidPosYMagnet = pointMagnet_t{.intensity = _gameState->Kid.room == room ? intensity : 0.0f, .position = position}; };
        rule.addAction(action);
        recognizedActionType = true;
    }

    if (actionType == "Set Guard Pos X Magnet")
      {
        auto intensity = jaffarCommon::json::getNumber<float>(actionJs, "Intensity");
        auto position  = jaffarCommon::json::getNumber<uint8_t>(actionJs, "Position");
        auto room      = jaffarCommon::json::getNumber<uint8_t>(actionJs, "Room");
        auto action    = [=, this]() { _guardPosXMagnet = pointMagnet_t{.intensity = _gameState->Guard.room == room ? intensity : 0.0f, .position = position}; };
        rule.addAction(action);
        recognizedActionType = true;
    }

    if (actionType == "Set Guard Pos Y Magnet")
      {
        auto intensity = jaffarCommon::json::getNumber<float>(actionJs, "Intensity");
        auto position  = jaffarCommon::json::getNumber<uint8_t>(actionJs, "Position");
        auto room      = jaffarCommon::json::getNumber<uint8_t>(actionJs, "Room");
        auto action    = [=, this]() { _guardPosYMagnet = pointMagnet_t{.intensity = _gameState->Guard.room == room ? intensity : 0.0f, .position = position}; };
        rule.addAction(action);
        recognizedActionType = true;
    }

    if (actionType == "Set Kid Direction Magnet")
      {
        auto intensity = jaffarCommon::json::getNumber<float>(actionJs, "Intensity");
        auto action    = [=, this]() { _kidDirectionMagnet = intensity; };
        rule.addAction(action);
        recognizedActionType = true;
    }

    if (actionType == "Set Level Door Open Magnet")
      {
        auto intensity = jaffarCommon::json::getNumber<float>(actionJs, "Intensity");
        auto action    = [=, this]() { _levelDoorOpenMagnet = intensity; };
        rule.addAction(action);
        recognizedActionType = true;
    }

    if (actionType == "Set United With Shadow Magnet")
      {
        auto intensity = jaffarCommon::json::getNumber<float>(actionJs, "Intensity");
        auto action    = [=, this]() { _unitedWithShadowMagnet = intensity; };
        rule.addAction(action);
        recognizedActionType = true;
    }

    if (actionType == "Set Guard HP Magnet")
      {
        auto intensity = jaffarCommon::json::getNumber<float>(actionJs, "Intensity");
        auto action    = [=, this]() { _guardHPMagnet = intensity; };
        rule.addAction(action);
        recognizedActionType = true;
    }

    return recognizedActionType;
  }

  __INLINE__ jaffarCommon::hash::hash_t getStateInputHash() override { return jaffarCommon::hash::calculateMetroHash(&_gameState->Kid.frame, sizeof(uint8_t)); }

  private:

  quicker::sdlPopState_t *_gameState;

  // Values artificially added to the state
  uint8_t _kidPreviousFrame;

  // Calculated values
  float _kidPosX;
  float _kidPosY;

  // Datatype to describe a point magnet
  struct pointMagnet_t
  {
    float   intensity = 0.0; // How strong the magnet is
    uint8_t position  = 0;   // Position
  };

  pointMagnet_t _kidPosXMagnet;
  pointMagnet_t _kidPosYMagnet;
  pointMagnet_t _guardPosXMagnet;
  pointMagnet_t _guardPosYMagnet;

  float _kidDirectionMagnet;
  float _levelDoorOpenMagnet;
  float _unitedWithShadowMagnet;
  float _guardHPMagnet;

  //  struct mobsIndex_t
  //  {
  //    uint8_t room;
  //    uint8_t column;
  //  };

  //  std::map<uint16_t, mobsIndex_t> _watchMobsIndexes;
};

} // namespace sdlpop

} // namespace games

} // namespace jaffarPlus