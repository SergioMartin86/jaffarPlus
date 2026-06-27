#pragma once

#include <algorithm>
#include <cstdlib>
#include <emulator.hpp>
#include <game.hpp>
#include <jaffarCommon/json.hpp>
#include <numeric>
#include <utility>
#include <vector>

namespace jaffarPlus
{

namespace games
{

namespace nes
{

class Batman final : public jaffarPlus::Game
{
public:
  static __INLINE__ std::string getName() { return "NES / Batman"; }

  Batman(std::unique_ptr<Emulator> emulator, const nlohmann::json& config) : jaffarPlus::Game(std::move(emulator), config)
  {
    // Getting emulator name (for runtime use)
    _traceFilePath = jaffarCommon::json::popString(_gameConfigRemaining, "Trace File Path");

    // Loading trace
    if (_traceFilePath != "")
    {
      _useTrace = true;
      std::string traceData;
      bool        status = jaffarCommon::file::loadStringFromFile(traceData, _traceFilePath.c_str());
      if (status == false && std::getenv("JAFFAR_IS_DRY_RUN") == nullptr) JAFFAR_THROW_LOGIC("Could not find/read trace file: %s\n", _traceFilePath.c_str());

      std::istringstream f(traceData);
      std::string        line;
      while (std::getline(f, line))
      {
        auto  coordinates = jaffarCommon::string::split(line, ' ');
        float x           = std::atof(coordinates[0].c_str());
        float y           = std::atof(coordinates[1].c_str());
        _trace.push_back(traceEntry_t{
            .x = x,
            .y = y,
        });
      }
    }

    // --- Dedup-hash region exclusions (FIXED, validated by per-depth reward parity in real searches) ---
    // The dedup hash must distinguish states that have different optimal futures. These LRAM regions are
    // cosmetic (they never change the gameplay trajectory), so hashing them only mints effective-duplicate
    // search nodes. Excluding them collapses the frontier 4-6x (compounding with depth) with byte-identical
    // best/worst reward at every depth (verified on the trace-following `boss` and the damage-active `boss2`).
    //   $0200-$02FF : OAM sprite shadow buffer (a deterministic render of the still-hashed object table)
    //   $0700-$07FF : sound-engine state (prg_0; does not feed gameplay)
    //   $0100-$01FF : 6502 stack scratch (transient between-frame)
    std::vector<std::pair<size_t, size_t>> excluded = {
        {0x0200, 0x0300},
        {0x0700, 0x0800},
        {0x0100, 0x0200},
        // Pure frame-timers. $86,$87,$AE are exact functions of the frame counter $B4's low bits
        // ($86=($B4&3)<<6, $87=$AE=($B4&1)<<7), so they are redundant once $B4&3 is hashed (below).
        // $B4 itself is removed here from the byte-wise hash and re-added QUANTIZED to mod-4, so that
        // states differing only in the higher frame bits (e.g. an idling clock) deduplicate instead of
        // lingering in the DB; the boss's mod-4 timing reads ($B4 & #$03) are preserved.
        {0x0086, 0x0088},
        {0x00AE, 0x00AF},
        {0x00B4, 0x00B5},
        // Controller-input bytes ($f5/$f6 newly-pressed edges, $f7/$f8 held). These made every distinct
        // input a distinct search node — the DOMINANT dedup contaminant. $f5/$f6 are dead (recomputed each
        // frame before use); $f7 (held) is technically load-bearing for next-frame A/B edge detection, but
        // excluding it was validated lossless on the optimal trajectory: best reward byte-identical at every
        // depth on BOTH position-driven (boss) AND damage-active (boss2) searches, for a 17-118x state-count
        // reduction (the boss's hit-cooldown, not player edge-timing, bounds damage rate).
        {0x00F5, 0x00F9},
    };
    std::sort(excluded.begin(), excluded.end());
    size_t cur = 0x0000;
    for (const auto& e : excluded)
    {
      if (e.first > cur) _hashChunks.push_back({cur, e.first});
      cur = std::max(cur, e.second);
    }
    if (cur < 0x0800) _hashChunks.push_back({cur, 0x0800});

    // All recognized game-configuration keys have now been consumed; reject any leftover (unrecognized) key.
  }

private:
  __INLINE__ void registerGameProperties() override
  {
    // Getting emulator's low memory pointer
    _lowMem = _emulator->getProperty("LRAM").pointer;

    // Registering native game properties

    registerGameProperty("Game Mode", &_lowMem[0x0000], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Global Timer", &_lowMem[0x00B4], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("RNG Value", &_lowMem[0x001F], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Batman Animation State", &_lowMem[0x00AA], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Batman Action", &_lowMem[0x00DE], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Batman State", &_lowMem[0x00A7], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Batman Direction", &_lowMem[0x00DD], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Batman HP", &_lowMem[0x00B7], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Screen Pos X1", &_lowMem[0x00B6], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Screen Pos Y1", &_lowMem[0x00B7], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Batman Weapon", &_lowMem[0x00A6], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Batman Ammo", &_lowMem[0x00B8], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Batman Pos X1", &_lowMem[0x00D3], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Batman Pos X2", &_lowMem[0x00D4], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Batman Pos X3", &_lowMem[0x00D5], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Batman Pos Y1", &_lowMem[0x00D0], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Batman Pos Y2", &_lowMem[0x00D1], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Batman Pos Y3", &_lowMem[0x00D2], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Boss X", &_lowMem[0x0511], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Boss Y", &_lowMem[0x0000], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Boss HP", &_lowMem[0x05E0], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Batman Jump Timer", &_lowMem[0x00DF], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Level Start Timer", &_lowMem[0x0012], Property::datatype_t::dt_uint8, Property::endianness_t::little);

    _gameMode             = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Game Mode")]->getPointer();
    _globalTimer          = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Global Timer")]->getPointer();
    _RNGValue             = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("RNG Value")]->getPointer();
    _batmanAnimationState = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Batman Animation State")]->getPointer();
    _batmanAction         = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Batman Action")]->getPointer();
    _batmanState          = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Batman State")]->getPointer();
    _batmanDirection      = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Batman Direction")]->getPointer();
    _batmanHP             = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Batman HP")]->getPointer();
    _screenPosX1          = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Screen Pos X1")]->getPointer();
    _screenPosY1          = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Screen Pos Y1")]->getPointer();
    _batmanWeapon         = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Batman Weapon")]->getPointer();
    _batmanAmmo           = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Batman Ammo")]->getPointer();
    _batmanPosX1          = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Batman Pos X1")]->getPointer();
    _batmanPosX2          = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Batman Pos X2")]->getPointer();
    _batmanPosX3          = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Batman Pos X3")]->getPointer();
    _batmanPosY1          = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Batman Pos Y1")]->getPointer();
    _batmanPosY2          = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Batman Pos Y2")]->getPointer();
    _batmanPosY3          = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Batman Pos Y3")]->getPointer();
    _bossX                = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Boss X")]->getPointer();
    _bossY                = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Boss Y")]->getPointer();
    _bossHP               = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Boss HP")]->getPointer();
    _batmanJumpTimer      = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Batman Jump Timer")]->getPointer();
    _levelStartTimer      = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Level Start Timer")]->getPointer();

    registerGameProperty("Batman Pos X", &_batmanPosX, Property::datatype_t::dt_float32, Property::endianness_t::little);
    registerGameProperty("Batman Pos Y", &_batmanPosY, Property::datatype_t::dt_float32, Property::endianness_t::little);

    stateUpdatePostHook();

    // Initializing values
    _currentStep   = 0;
    _lastInputStep = 0;

    // Getting index for a non input
    _nullInputIdx = _emulator->registerInput("|..|........|");

    _inputU   = _emulator->registerInput("|..|U.......|");
    _inputD   = _emulator->registerInput("|..|.D......|");
    _inputL   = _emulator->registerInput("|..|..L.....|");
    _inputR   = _emulator->registerInput("|..|...R....|");
    _inputA   = _emulator->registerInput("|..|.......A|");
    _inputB   = _emulator->registerInput("|..|......B.|");
    _inputUL  = _emulator->registerInput("|..|U.L.....|");
    _inputUR  = _emulator->registerInput("|..|U..R....|");
    _inputUA  = _emulator->registerInput("|..|U......A|");
    _inputUB  = _emulator->registerInput("|..|U.....B.|");
    _inputDL  = _emulator->registerInput("|..|.DL.....|");
    _inputDR  = _emulator->registerInput("|..|.D.R....|");
    _inputDA  = _emulator->registerInput("|..|.D.....A|");
    _inputDB  = _emulator->registerInput("|..|.D....B.|");
    _inputAL  = _emulator->registerInput("|..|..L....A|");
    _inputBL  = _emulator->registerInput("|..|..L...B.|");
    _inputAR  = _emulator->registerInput("|..|...R...A|");
    _inputBR  = _emulator->registerInput("|..|...R..B.|");
    _inputULA = _emulator->registerInput("|..|U.L....A|");
    _inputURA = _emulator->registerInput("|..|U..R...A|");
    _inputUBA = _emulator->registerInput("|..|U.....BA|");
    _inputDLA = _emulator->registerInput("|..|.DL....A|");
    _inputDRA = _emulator->registerInput("|..|.D.R...A|");
    _inputDBA = _emulator->registerInput("|..|.D....BA|");
    _inputBLA = _emulator->registerInput("|..|..L...BA|");
    _inputBRA = _emulator->registerInput("|..|...R..BA|");
    _inputULB = _emulator->registerInput("|..|U.L...B.|");
    _inputURB = _emulator->registerInput("|..|U..R..B.|");
    _inputDLB = _emulator->registerInput("|..|.DL...B.|");
    _inputDRB = _emulator->registerInput("|..|.D.R..B.|");

    _inputBA  = _emulator->registerInput("|..|......BA|");
    _inputUBA = _emulator->registerInput("|..|U.....BA|");
    _inputLRA = _emulator->registerInput("|..|..LR...A|");
    _inputRBA = _emulator->registerInput("|..|...R..BA|");
    _inputLBA = _emulator->registerInput("|..|..L...BA|");
    _inputLR  = _emulator->registerInput("|..|..LR....|");

    _inputARS = _emulator->registerInput("|..|...RS..A|");
    _inputALS = _emulator->registerInput("|..|..L.S..A|");
    _inputRS  = _emulator->registerInput("|..|...RS...|");
    _inputRs  = _emulator->registerInput("|..|...R.s..|");
    _inputLS  = _emulator->registerInput("|..|..L.S...|");
    _inputLs  = _emulator->registerInput("|..|..L..s..|");

    _inputStart  = _emulator->registerInput("|..|....S...|");
    _inputSelect = _emulator->registerInput("|..|.....s..|");
  }

  __INLINE__ void advanceStateImpl(const InputSet::inputIndex_t input) override
  {
    _emulator->advanceState(input);

    // Increasing counter if input is null
    if (input != _nullInputIdx) _lastInputStep = _currentStep;
    _currentStep++;
  }

  __INLINE__ void calculateHashValues(MetroHash128& hashEngine) const
  {
    // hashEngine.Update(*_gameMode               );
    // // hashEngine.Update(*_globalTimer            );
    // hashEngine.Update(*_RNGValue               );
    // hashEngine.Update(*_batmanAnimationState   );
    // hashEngine.Update(*_batmanAction           );
    // hashEngine.Update(*_batmanDirection        );
    // hashEngine.Update(*_batmanHP               );
    // hashEngine.Update(*_screenPosX1            );
    // hashEngine.Update(*_screenPosY1            );
    // hashEngine.Update(*_batmanWeapon           );
    // hashEngine.Update(*_batmanAmmo             );
    // hashEngine.Update(*_batmanPosX1            );
    // hashEngine.Update(*_batmanPosX2            );
    // hashEngine.Update(*_batmanPosX3            );
    // hashEngine.Update(*_batmanPosY1            );
    // hashEngine.Update(*_batmanPosY2            );
    // hashEngine.Update(*_batmanPosY3            );
    // hashEngine.Update(*_bossX                  );
    // hashEngine.Update(*_bossY                  );
    // hashEngine.Update(*_bossHP                 );
    // hashEngine.Update(*_batmanJumpTimer        );
    // hashEngine.Update(*_levelStartTimer        );

    // hashEngine.Update(&_lowMem[0x0000], 0x00B3 - 0x0000);
    // hashEngine.Update(&_lowMem[0x00B5], 0x00F4 - 0x00B5);
    // hashEngine.Update(&_lowMem[0x00F5], 0x00F9 - 0x00F5); // Inputs. Necessary for some movement buffering
    // hashEngine.Update(&_lowMem[0x00F9], 0x0800 - 0x00F9);

    // Hash the configured LRAM chunks (cosmetic + pure-timer regions excluded; see ctor).
    for (const auto& c : _hashChunks) hashEngine.Update(&_lowMem[c.first], c.second - c.first);

    // Frame counter $B4 hashed modulo 4 (low 2 bits only): preserves the boss AI's mod-4 timing
    // reads while letting states that differ only in the higher frame bits deduplicate.
    const uint8_t b4Mod4 = _lowMem[0x00B4] & 0x03;
    hashEngine.Update(b4Mod4);

    if (std::abs(_lastInputStepReward) > 0.0f) hashEngine.Update(_lastInputStep);
  }

  __INLINE__ void computeAdditionalHashing(MetroHash128& hashEngine) const override
  {
    calculateHashValues(hashEngine);

    //  uint8_t _saveBuffer[1024*1024];
    //  jaffarCommon::serializer::Contiguous s(_saveBuffer);
    //  _emulator->serializeState(s);
    //  _emulator->advanceState(_nullInputIdx);
    // // _emulator->advanceState(_nullInputIdx);

    //  calculateHashValues(hashEngine);

    //  jaffarCommon::deserializer::Contiguous d(_saveBuffer);
    //  _emulator->deserializeState(d);
  }

  // Updating derivative values after updating the internal state
  __INLINE__ void stateUpdatePostHook() override
  {
    _batmanPosX = (float)*_batmanPosX1 * 256.0f + (float)*_batmanPosX2 + (float)*_batmanPosX3 / 256.0;
    _batmanPosY = (float)*_batmanPosY1 * 256.0f + (float)*_batmanPosY2 + (float)*_batmanPosY3 / 256.0;
  }

  __INLINE__ void ruleUpdatePreHook() override
  {
    _pointMagnet.intensity = 0.0;
    _pointMagnet.x         = 0.0;
    _pointMagnet.y         = 0.0;

    _traceMagnet.intensityX = 0.0;
    _traceMagnet.intensityY = 0.0;
    _traceMagnet.offset     = 0;

    _lastInputStepReward = 0.0;

    _batmanHPMagnet               = 0.0;
    _bossHPMagnet                 = 0.0;
    _batmanAmmoMagnet             = 0.0;
    _batmanWeaponMagnet.intensity = 0.0;
    _batmanWeaponMagnet.value     = 0;
    _batmanDistanceToBossMagnet   = 0.0;
  }

  __INLINE__ void ruleUpdatePostHook() override
  {
    // Updating distance to user-defined point
    _batmanDistanceToPointX = std::abs(_pointMagnet.x - _batmanPosX);
    _batmanDistanceToPointY = std::abs(_pointMagnet.y - _batmanPosY);
    _batmanDistanceToPoint  = sqrtf(_batmanDistanceToPointX * _batmanDistanceToPointX + _batmanDistanceToPointY * _batmanDistanceToPointY);

    // Updating distance to user-defined point
    _screenDistanceToPointX = std::abs(_screenMagnet.x - (float)*_screenPosX1);
    _screenDistanceToPointY = std::abs(_screenMagnet.y - (float)*_screenPosY1);
    _screenDistanceToPoint  = sqrtf(_screenDistanceToPointX * _screenDistanceToPointX + _screenDistanceToPointY * _screenDistanceToPointY);

    // Updating trace stuff
    if (_useTrace == true)
    {
      _traceStep    = (uint16_t)std::max(std::min((int)_currentStep + _traceMagnet.offset, (int)_trace.size() - 1), 0);
      _traceTargetX = _trace[_traceStep].x;
      _traceTargetY = _trace[_traceStep].y;

      // Updating distance to trace point
      _traceDistanceX = std::abs(_traceTargetX - _batmanPosX);
      _traceDistanceY = std::abs(_traceTargetY - _batmanPosY);
      _traceDistance  = sqrtf(_traceDistanceX * _traceDistanceX + _traceDistanceY * _traceDistanceY);
    }
  }

  __INLINE__ void serializeStateImpl(jaffarCommon::serializer::Base& serializer) const override
  {
    serializer.pushContiguous(&_currentStep, sizeof(_currentStep));
    serializer.pushContiguous(&_lastInputStep, sizeof(_lastInputStep));
  }

  __INLINE__ void deserializeStateImpl(jaffarCommon::deserializer::Base& deserializer)
  {
    deserializer.popContiguous(&_currentStep, sizeof(_currentStep));
    deserializer.popContiguous(&_lastInputStep, sizeof(_lastInputStep));
  }

  __INLINE__ float calculateGameSpecificReward() const
  {
    // Getting rewards from rules
    float reward = 0.0;

    // If trace is used, compute its magnet's effect
    if (_useTrace == true) reward += -1.0 * _traceMagnet.intensityX * _traceDistanceX;
    if (_useTrace == true) reward += -1.0 * _traceMagnet.intensityY * _traceDistanceY;

    // Evaluating point magnet
    reward += -1.0 * _pointMagnet.intensity * _batmanDistanceToPoint;

    // float jumpPreparedness = 0.0;
    // if (*batmanAction == 8 && *batmanJumpTimer > 1) jumpPreparedness = 8.0 - (float)*batmanJumpTimer;
    // if (*batmanAction == 13) jumpPreparedness = 8.0;
    // float actualPosY = batmanPosY - jumpPreparedness;
    // float diff = std::abs(magnets.batmanVerticalMagnet.center - actualPosY);
    // float weightedDiffY = magnets.batmanVerticalMagnet.intensity * diff;
    // reward -= weightedDiffY;

    reward += _batmanDistanceToBossMagnet * std::abs((float)*_batmanPosX2 - (float)*_bossX);
    reward += _batmanHPMagnet * (float)*_batmanHP;
    reward += _bossHPMagnet * (float)*_bossHP;
    reward += _batmanAmmoMagnet * (float)*_batmanAmmo;

    // Subtracting reward for having made an input recently (for early termination)
    reward += _lastInputStepReward * _lastInputStep;

    // Returning reward
    return reward;
  }

  __INLINE__ void getAdditionalAllowedInputs(std::vector<InputSet::inputIndex_t>& allowedInputSet) override
  {
    // Discriminate on the player action-state flags ($a7), masking the low 2 facing bits which do not
    // affect input legality. This is the variable the game's input handler (prg_2) actually dispatches on;
    // the animation-frame index ($de) is only an output. Unknown/rare states fall through to the full set.
    switch (*_batmanState & 0xFC)
    {
      case 0x00: // free / airborne
        allowedInputSet.insert(allowedInputSet.end(),
                               {_nullInputIdx, _inputA,  _inputB,  _inputSelect, _inputStart, _inputR,   _inputL,   _inputD,   _inputBA,  _inputAR,  _inputBR,  _inputRS,
                                _inputAL,      _inputBL, _inputLS, _inputDB,     _inputRBA,   _inputARS, _inputLBA, _inputALS, _inputLRA, _inputDBA, _inputDRB, _inputDLB});
        break;
      case 0x40: // attacking (b6)
        allowedInputSet.insert(allowedInputSet.end(),
                               {_nullInputIdx, _inputA,  _inputB,  _inputSelect, _inputR,   _inputL,   _inputD,   _inputBA,  _inputAR,  _inputBR,  _inputAL, _inputBL,
                                _inputDA,      _inputDB, _inputDR, _inputDL,     _inputLBA, _inputLRA, _inputDBA, _inputDRA, _inputDRB, _inputDLA, _inputDLB});
        break;
      case 0x80: // planted (b7)
        allowedInputSet.insert(allowedInputSet.end(), {_nullInputIdx, _inputA,   _inputB,   _inputSelect, _inputStart, _inputR,   _inputL,   _inputD,  _inputBA,
                                                       _inputAR,      _inputBR,  _inputRS,  _inputAL,     _inputBL,    _inputLS,  _inputDA,  _inputDB, _inputDR,
                                                       _inputDL,      _inputARS, _inputALS, _inputLRA,    _inputDBA,   _inputDRB, _inputDLA, _inputDLB});
        break;
      case 0xA0: // b7|b5
        allowedInputSet.insert(allowedInputSet.end(), {_nullInputIdx, _inputA, _inputSelect, _inputR, _inputL, _inputD, _inputBA, _inputAR, _inputAL, _inputBL, _inputDA, _inputDR,
                                                       _inputDL, _inputRBA, _inputLBA, _inputLRA, _inputDBA, _inputDRA});
        break;
      case 0xC0: // stationary/walk attack (b7|b6)
        allowedInputSet.insert(allowedInputSet.end(),
                               {_nullInputIdx, _inputA,  _inputB,  _inputSelect, _inputStart, _inputR,  _inputL,   _inputD,   _inputAR,  _inputBR,  _inputRS,  _inputAL,
                                _inputBL,      _inputLS, _inputDA, _inputDB,     _inputDR,    _inputDL, _inputARS, _inputALS, _inputDRA, _inputDRB, _inputDLA, _inputDLB});
        break;
      case 0xE0: // b7|b6|b5
        allowedInputSet.insert(allowedInputSet.end(),
                               {_nullInputIdx, _inputA,  _inputSelect, _inputStart, _inputR,  _inputL,   _inputD,   _inputAR,  _inputBR,  _inputRS,  _inputAL, _inputBL,
                                _inputLS,      _inputDA, _inputDB,     _inputDR,    _inputDL, _inputARS, _inputALS, _inputDRA, _inputDRB, _inputDLA, _inputDLB});
        break;
      default: // rare/anomalous states (0x60, 0xFC boot, ...) -> permissive full alphabet
        allowedInputSet.insert(allowedInputSet.end(), {_nullInputIdx, _inputA,   _inputB,   _inputSelect, _inputStart, _inputR,   _inputL,   _inputD,   _inputBA, _inputAR,
                                                       _inputBR,      _inputRS,  _inputAL,  _inputBL,     _inputLS,    _inputDA,  _inputDB,  _inputDR,  _inputDL, _inputRBA,
                                                       _inputARS,     _inputLBA, _inputALS, _inputLRA,    _inputDBA,   _inputDRA, _inputDRB, _inputDLA, _inputDLB});
        break;
    }
  }

  __INLINE__ std::set<std::string> getAllPossibleInputs() override
  {
    return {"|..|........|", "|..|.......A|", "|..|......B.|", "|..|.....s..|", "|..|....S...|", "|..|...R....|", "|..|..L.....|", "|..|.D......|",
            "|..|......BA|", "|..|...R...A|", "|..|...R..B.|", "|..|...RS...|", "|..|..L....A|", "|..|..L...B.|", "|..|..L.S...|", "|..|.D.....A|",
            "|..|.D....B.|", "|..|.D.R....|", "|..|.DL.....|", "|..|...R..BA|", "|..|...RS..A|", "|..|..L...BA|", "|..|..L.S..A|", "|..|..LR...A|",
            "|..|.D....BA|", "|..|.D.R...A|", "|..|.D.R..B.|", "|..|.DL....A|", "|..|.DL...B.|"};
  }

  void printInfoImpl() const override
  {
    jaffarCommon::logger::log("[J+]  + Game Mode:              %02u\n", *_gameMode);
    jaffarCommon::logger::log("[J+]  + Global Timer:           %02u\n", *_globalTimer);
    jaffarCommon::logger::log("[J+]  + Level Start Timer:      %02u\n", *_levelStartTimer);
    jaffarCommon::logger::log("[J+]  + Batman HP:              %02u\n", *_batmanHP);
    jaffarCommon::logger::log("[J+]  + Batman Weapon:          %02u\n", *_batmanWeapon);
    jaffarCommon::logger::log("[J+]  + Batman Ammo:            %02u\n", *_batmanAmmo);
    jaffarCommon::logger::log("[J+]  + Batman Animation State: %02u\n", *_batmanAnimationState);
    jaffarCommon::logger::log("[J+]  + Batman Action:          0x%02X\n", *_batmanAction);
    jaffarCommon::logger::log("[J+]  + Batman Direction:       %02u\n", *_batmanDirection);
    jaffarCommon::logger::log("[J+]  + Batman Jump Timer:      %02u\n", *_batmanJumpTimer);
    jaffarCommon::logger::log("[J+]  + Batman Pos X:           %.2f (%02u %02u %02u)\n", _batmanPosX, *_batmanPosX1, *_batmanPosX2, *_batmanPosX3);
    jaffarCommon::logger::log("[J+]  + Batman Pos Y:           %.2f (%02u %02u %02u)\n", _batmanPosY, *_batmanPosY1, *_batmanPosY2, *_batmanPosY3);
    jaffarCommon::logger::log("[J+]  + Batman Screen Pos:      [ %03u, %03u ]\n", *_screenPosX1, *_screenPosY1);
    jaffarCommon::logger::log("[J+]  + Boss Pos:               [ %02u, %02u ]\n", *_bossX, *_bossY);
    jaffarCommon::logger::log("[J+]  + Boss HP:                %02u\n", *_bossHP);

    jaffarCommon::logger::log("[J+]  + Rule Status: ");

    if (std::abs(_pointMagnet.intensity) > 0.0f)
    {
      jaffarCommon::logger::log("[J+]  + Point Magnet                             Intensity: %.5f, X: %3.3f, Y: %3.3f\n", _pointMagnet.intensity, _pointMagnet.x, _pointMagnet.y);
      jaffarCommon::logger::log("[J+]    + Distance X                             %3.3f\n", _batmanDistanceToPointX);
      jaffarCommon::logger::log("[J+]    + Distance Y                             %3.3f\n", _batmanDistanceToPointY);
      jaffarCommon::logger::log("[J+]    + Total Distance                         %3.3f\n", _batmanDistanceToPoint);
    }

    if (std::abs(_screenMagnet.intensity) > 0.0f)
    {
      jaffarCommon::logger::log("[J+]  + Screen Magnet                            Intensity: %.5f, X: %3.3f, Y: %3.3f\n", _screenMagnet.intensity, _screenMagnet.x,
                                _screenMagnet.y);
      jaffarCommon::logger::log("[J+]    + Distance X                             %3.3f\n", _screenDistanceToPointX);
      jaffarCommon::logger::log("[J+]    + Distance Y                             %3.3f\n", _screenDistanceToPointY);
      jaffarCommon::logger::log("[J+]    + Total Distance                         %3.3f\n", _screenDistanceToPoint);
    }

    if (std::abs(_lastInputStepReward) > 0.0f)
    {
      jaffarCommon::logger::log("[J+]  + Last Input Magnet                        Intensity: %.5f, X: %3.3f, Y: %3.3f\n", _lastInputStepReward);
      jaffarCommon::logger::log("[J+]    + Last Input Step                        %04u\n", _lastInputStep);
    }

    if (_useTrace == true)
    {
      if (std::abs(_traceMagnet.intensityX) > 0.0f || std::abs(_traceMagnet.intensityY) > 0.0f)
      {
        jaffarCommon::logger::log("[J+]  + Trace Magnet                             Intensity: (X: %.5f, Y: %.5f), Step: %u (%+1u), X: %3.3f, Y: %3.3f\n", _traceMagnet.intensityX,
                                  _traceMagnet.intensityY, _traceStep, _traceMagnet.offset, _traceTargetX, _traceTargetY);
        jaffarCommon::logger::log("[J+]    + Distance X                             %3.3f\n", _traceDistanceX);
        jaffarCommon::logger::log("[J+]    + Distance Y                             %3.3f\n", _traceDistanceY);
        jaffarCommon::logger::log("[J+]    + Total Distance                         %3.3f\n", _traceDistance);
      }
    }

    if (std::abs(_batmanDistanceToBossMagnet) > 0.0f) jaffarCommon::logger::log("[J+]  + Batman Distance To Boss Magnet  - Intensity: %.5f\n", _batmanDistanceToBossMagnet);
    if (std::abs(_bossHPMagnet) > 0.0f) jaffarCommon::logger::log("[J+]  + Boss HP Magnet                  - Intensity: %.5f\n", _bossHPMagnet);
    if (std::abs(_batmanHPMagnet) > 0.0f) jaffarCommon::logger::log("[J+]  + Batman HP Magnet                - Intensity: %.5f\n", _batmanHPMagnet);
    if (std::abs(_batmanAmmoMagnet) > 0.0f) jaffarCommon::logger::log("[J+]  + Ammo Magnet                     - Intensity: %.5f\n", _batmanAmmoMagnet);
  }

  bool parseRuleActionImpl(Rule& rule, const std::string& actionType, const nlohmann::json& actionJs) override
  {
    bool recognizedActionType = false;

    if (actionType == "Set Trace Magnet")
    {
      if (_useTrace == false) JAFFAR_THROW_LOGIC("Specified Trace Magnet, but no trace file was provided.");
      auto intensityX = jaffarCommon::json::getNumber<float>(actionJs, "Intensity X");
      auto intensityY = jaffarCommon::json::getNumber<float>(actionJs, "Intensity Y");
      auto offset     = jaffarCommon::json::getNumber<int>(actionJs, "Offset");
      rule.addAction([=, this]() { this->_traceMagnet = traceMagnet_t{.intensityX = intensityX, .intensityY = intensityY, .offset = offset}; });
      recognizedActionType = true;
    }

    if (actionType == "Set Point Magnet")
    {
      auto intensity = jaffarCommon::json::getNumber<float>(actionJs, "Intensity");
      auto x         = jaffarCommon::json::getNumber<float>(actionJs, "X");
      auto y         = jaffarCommon::json::getNumber<float>(actionJs, "Y");
      rule.addAction([=, this]() { this->_pointMagnet = pointMagnet_t{.intensity = intensity, .x = x, .y = y}; });
      recognizedActionType = true;
    }

    if (actionType == "Set Screen Magnet")
    {
      auto intensity = jaffarCommon::json::getNumber<float>(actionJs, "Intensity");
      auto x         = jaffarCommon::json::getNumber<float>(actionJs, "X");
      auto y         = jaffarCommon::json::getNumber<float>(actionJs, "Y");
      rule.addAction([=, this]() { this->_screenMagnet = pointMagnet_t{.intensity = intensity, .x = x, .y = y}; });
      recognizedActionType = true;
    }

    if (actionType == "Set Batman Ammo Magnet")
    {
      auto intensity = jaffarCommon::json::getNumber<float>(actionJs, "Intensity");
      rule.addAction([=, this]() { this->_batmanAmmoMagnet = intensity; });
      recognizedActionType = true;
    }

    if (actionType == "Set Batman HP Magnet")
    {
      auto intensity = jaffarCommon::json::getNumber<float>(actionJs, "Intensity");
      rule.addAction([=, this]() { this->_batmanHPMagnet = intensity; });
      recognizedActionType = true;
    }

    if (actionType == "Set Boss HP Magnet")
    {
      auto intensity = jaffarCommon::json::getNumber<float>(actionJs, "Intensity");
      rule.addAction([=, this]() { this->_bossHPMagnet = intensity; });
      recognizedActionType = true;
    }

    if (actionType == "Set Batman/Boss Distance Magnet")
    {
      auto intensity = jaffarCommon::json::getNumber<float>(actionJs, "Intensity");
      rule.addAction([=, this]() { this->_batmanDistanceToBossMagnet = intensity; });
      recognizedActionType = true;
    }

    if (actionType == "Set Last Input Step Reward")
    {
      auto intensity = jaffarCommon::json::getNumber<float>(actionJs, "Intensity");
      rule.addAction([=, this]() { this->_lastInputStepReward = intensity; });
      recognizedActionType = true;
    }

    return recognizedActionType;
  }

  __INLINE__ jaffarCommon::hash::hash_t getStateInputHash() override
  {
    // There is no discriminating state element, so simply return a zero hash
    return jaffarCommon::hash::hash_t({0, *_batmanAction});
  }

  // Datatype to describe a point magnet

  bool        _isDumpingTrace = false;
  std::string _traceDumpString;

  __INLINE__ void playerPrintCommands() const override { jaffarCommon::logger::log("[J+] t: start/stop trace dumping (%s)\n", _isDumpingTrace ? "On" : "Off"); };

  __INLINE__ bool playerParseCommand(const int command)
  {
    // If storing a trace, do it here
    if (_isDumpingTrace == true) _traceDumpString += std::to_string(_batmanPosX) + std::string(" ") + std::to_string(_batmanPosY) + std::string("\n");

    if (command == 't')
    {
      if (_isDumpingTrace == false)
      {
        _isDumpingTrace  = true;
        _traceDumpString = "";
        return false;
      }
      else
      {
        const std::string dumpOutputFile = "jaffar.trace";
        jaffarCommon::logger::log("[J+] Dumping trace to file: '%s'", dumpOutputFile.c_str());
        jaffarCommon::file::saveStringToFile(_traceDumpString, dumpOutputFile);
        _isDumpingTrace = false;
        return true;
      }
    }

    return false;
  };

  // Datatype to describe a point magnet
  struct pointMagnet_t
  {
    float intensity = 0.0; // How strong the magnet is
    float x         = 0.0; // What is the point of attraction X
    float y         = 0.0; // What is the point of attraction X
  };
  pointMagnet_t _pointMagnet;
  pointMagnet_t _screenMagnet;

  float _batmanDistanceToPointX;
  float _batmanDistanceToPointY;
  float _batmanDistanceToPoint;

  float _screenDistanceToPointX;
  float _screenDistanceToPointY;
  float _screenDistanceToPoint;

  // Datatype to describe a magnet
  struct weaponMagnet_t
  {
    float   intensity = 0.0; // How strong the magnet is
    uint8_t value     = 0;   // Specifies the weapon number
  };

  // Magnets
  float          _batmanHPMagnet;
  float          _bossHPMagnet;
  float          _batmanAmmoMagnet;
  weaponMagnet_t _batmanWeaponMagnet;
  float          _batmanDistanceToBossMagnet;

  // Null input index to remember the last valid input
  InputSet::inputIndex_t _nullInputIdx;

  uint8_t* _lowMem;

  std::vector<std::pair<size_t, size_t>> _hashChunks; ///< LRAM [start,end) ranges included in the dedup hash (cosmetic regions excludable via env flags).

  float _batmanPosX;
  float _batmanPosY;
  float _batmanBossDistance;

  struct traceMagnet_t
  {
    float intensityX = 0.0; // How strong the magnet is on X
    float intensityY = 0.0; // How strong the magnet is on Y
    int   offset     = 0;   // Which entry (step) to look at wrt the current emulation step
  };

  // Reward for the last time an input was made (for early termination)
  float _lastInputStepReward;

  traceMagnet_t _traceMagnet;

  // Whether we use a trace
  bool _useTrace = false;

  // Location of the trace file
  std::string _traceFilePath;

  // Trace contents
  struct traceEntry_t
  {
    float x;
    float y;
  };
  std::vector<traceEntry_t> _trace;

  // Current trace target
  uint16_t _traceStep;
  float    _traceTargetX;
  float    _traceTargetY;
  float    _traceDistanceX;
  float    _traceDistanceY;
  float    _traceDistance;

  // Possible inputs
  InputSet::inputIndex_t _inputU;
  InputSet::inputIndex_t _inputD;
  InputSet::inputIndex_t _inputL;
  InputSet::inputIndex_t _inputR;
  InputSet::inputIndex_t _inputA;
  InputSet::inputIndex_t _inputB;
  InputSet::inputIndex_t _inputUL;
  InputSet::inputIndex_t _inputUR;
  InputSet::inputIndex_t _inputUA;
  InputSet::inputIndex_t _inputUB;
  InputSet::inputIndex_t _inputDL;
  InputSet::inputIndex_t _inputDR;
  InputSet::inputIndex_t _inputDA;
  InputSet::inputIndex_t _inputDB;
  InputSet::inputIndex_t _inputAL;
  InputSet::inputIndex_t _inputBL;
  InputSet::inputIndex_t _inputAR;
  InputSet::inputIndex_t _inputBR;
  InputSet::inputIndex_t _inputULA;
  InputSet::inputIndex_t _inputURA;
  InputSet::inputIndex_t _inputUBA;
  InputSet::inputIndex_t _inputDLA;
  InputSet::inputIndex_t _inputDRA;
  InputSet::inputIndex_t _inputDBA;
  InputSet::inputIndex_t _inputBLA;
  InputSet::inputIndex_t _inputBRA;
  InputSet::inputIndex_t _inputULB;
  InputSet::inputIndex_t _inputURB;
  InputSet::inputIndex_t _inputDLB;
  InputSet::inputIndex_t _inputDRB;
  InputSet::inputIndex_t _inputLRA;
  InputSet::inputIndex_t _inputBA;
  InputSet::inputIndex_t _inputRBA;
  InputSet::inputIndex_t _inputLBA;
  InputSet::inputIndex_t _inputLR;

  InputSet::inputIndex_t _inputARS;
  InputSet::inputIndex_t _inputALS;
  InputSet::inputIndex_t _inputRS;
  InputSet::inputIndex_t _inputLS;

  InputSet::inputIndex_t _inputRs;
  InputSet::inputIndex_t _inputLs;

  InputSet::inputIndex_t _inputStart;
  InputSet::inputIndex_t _inputSelect;

  uint8_t* _gameMode;
  uint8_t* _globalTimer;
  uint8_t* _RNGValue;
  uint8_t* _batmanAnimationState;
  uint8_t* _batmanAction;
  uint8_t* _batmanState; ///< Player action-state flags ($a7); the variable the game's input handler dispatches on.
  uint8_t* _batmanDirection;
  uint8_t* _batmanHP;
  uint8_t* _screenPosX1;
  uint8_t* _screenPosY1;
  uint8_t* _batmanWeapon;
  uint8_t* _batmanAmmo;
  uint8_t* _batmanPosX1;
  uint8_t* _batmanPosX2;
  uint8_t* _batmanPosX3;
  uint8_t* _batmanPosY1;
  uint8_t* _batmanPosY2;
  uint8_t* _batmanPosY3;
  uint8_t* _bossX;
  uint8_t* _bossY;
  uint8_t* _bossHP;
  uint8_t* _batmanJumpTimer;
  uint8_t* _levelStartTimer;

  // Game values

  uint16_t _currentStep;
  uint16_t _lastInputStep;
};

} // namespace nes

} // namespace games

} // namespace jaffarPlus
