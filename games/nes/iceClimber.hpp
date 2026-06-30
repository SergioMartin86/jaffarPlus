#pragma once

#include <emulator.hpp>
#include <game.hpp>
#include <jaffarCommon/json.hpp>
#include <numeric>

namespace jaffarPlus
{

namespace games
{

namespace nes
{

class IceClimber final : public jaffarPlus::Game
{
public:
  static __INLINE__ std::string getName() { return "NES / Ice Climber"; }

  IceClimber(std::unique_ptr<Emulator> emulator, const nlohmann::json& config) : jaffarPlus::Game(std::move(emulator), config)
  {
    // No game-specific configuration keys; reject any leftover (unrecognized) key in the game configuration.
  }

private:
  __INLINE__ void registerGameProperties() override
  {
    // Getting emulator's low memory pointer
    _lowMem = _emulator->getProperty("LRAM").pointer;

    // Registering native game properties

    registerGameProperty("Current Level", &_lowMem[0x0059], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Player Pos X", &_lowMem[0x0064], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Player Pos Y", &_lowMem[0x0066], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Screen Scroll Y", &_lowMem[0x0013], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Screen Scroll Y1", &_lowMem[0x008A], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Screen Scroll Y2", &_lowMem[0x008D], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Screen Scroll Y3", &_lowMem[0x0090], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Player Floor", &_lowMem[0x0343], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Blocks Hit", &_lowMem[0x0361], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Vegetables Collected", &_lowMem[0x0362], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Nitpickers Hit", &_lowMem[0x0363], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Bricks Broken", &_lowMem[0x0364], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Player Inputs 1", &_lowMem[0x0014], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Player Inputs 2", &_lowMem[0x0015], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Global Timer", &_lowMem[0x0050], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("SubTimer", &_lowMem[0x0031], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("SubTimer", &_lowMem[0x0031], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Bonus Stage Start", &_lowMem[0x0027], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Grabbed Bird", &_lowMem[0x004D], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Player Jumping", &_lowMem[0x00E0], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Player Is Dead", &_lowMem[0x0382], Property::datatype_t::dt_uint8, Property::endianness_t::little);

    registerGameProperty("Bird State", &_lowMem[0x00D3], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Bird Pos X", &_lowMem[0x0243], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Bird Direction", &_lowMem[0x00D5], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Is Bird Detected", &_isBirdDetected, Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Is Condor Active", &_isCondorActive, Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Min Bird Dist", &_minBirdDist, Property::datatype_t::dt_float32, Property::endianness_t::little);

    registerGameProperty("Player Real Pos Y", &_playerRealPosY, Property::datatype_t::dt_float32, Property::endianness_t::little);
    registerGameProperty("Player Best Pos Y", &_playerBestPosY, Property::datatype_t::dt_float32, Property::endianness_t::little);
    registerGameProperty("Player Real/Best Pos Y Diff", &_playerRealBestPosYDiff, Property::datatype_t::dt_float32, Property::endianness_t::little);

    _currentLevel        = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Current Level")]->getPointer();
    _playerPosX          = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Player Pos X")]->getPointer();
    _playerPosY          = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Player Pos Y")]->getPointer();
    _screenScrollY       = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Screen Scroll Y")]->getPointer();
    _screenScrollY1      = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Screen Scroll Y1")]->getPointer();
    _screenScrollY2      = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Screen Scroll Y2")]->getPointer();
    _screenScrollY3      = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Screen Scroll Y3")]->getPointer();
    _playerFloor         = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Player Floor")]->getPointer();
    _playerJumping       = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Player Jumping")]->getPointer();
    _blocksHit           = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Blocks Hit")]->getPointer();
    _vegetablesCollected = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Vegetables Collected")]->getPointer();
    _nitpickersHit       = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Nitpickers Hit")]->getPointer();
    _bricksBroken        = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Bricks Broken")]->getPointer();
    _playerInputs1       = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Player Inputs 1")]->getPointer();
    _playerInputs2       = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Player Inputs 2")]->getPointer();
    _globalTimer         = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Global Timer")]->getPointer();
    _subTimer            = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("SubTimer")]->getPointer();
    _birdState           = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Bird State")]->getPointer();
    _birdPosX            = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Bird Pos X")]->getPointer();
    _birdDirection       = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Bird Direction")]->getPointer();

    _pastBricksBroken  = 0;
    _isBirdDetected    = 0;
    _isCondorActive    = 0;
    _minBirdDist       = std::numeric_limits<float>::infinity();
    _fullScreenScrolls = 0;
    _playerBestPosY    = std::numeric_limits<float>::infinity();
    _bestReachY        = std::numeric_limits<float>::infinity();
    updateRealPosY();

    // Initializing values
    _currentStep   = 0;
    _lastInputStep = 0;

    // Getting index for a non input
    _nullInputIdx = _emulator->registerInput("|..|........|");
  }

  __INLINE__ void updateRealPosY()
  {
    _playerRealPosY = (float)*_playerPosY;
    if (_playerRealPosY > 245.0) _playerRealPosY -= 255.0;
    _playerRealPosY += (float)*_screenScrollY;
    _playerRealPosY -= (float)_fullScreenScrolls * 240.0; // vertical scroll wraps at 240 (0..239), verified empirically: scroll 3->239 across a full screen
    _playerRealBestPosYDiff = _playerRealPosY - _playerBestPosY;

    // Airborne-inclusive best height ever reached. This is the dense climb gradient: to gain a
    // floor the player must jump up, bonk/break the ceiling, fall back, and jump through. A pure
    // *standing* best is flat through that whole maneuver, giving the search no signal. The jump
    // peak, in contrast, rises every time the ceiling opens a little further, so rewarding the
    // best reach pulls the search toward making the hole (and through it).
    if (_playerRealPosY < _bestReachY) _bestReachY = _playerRealPosY;

    // Standing best (secured height). Kept SEPARATE from _bestReachY so the fall-back prune
    // (realY - standingBest) tolerates the dip back to the current floor during hole-making.
    if (*_playerJumping == 0)
      if (_playerRealPosY < _playerBestPosY)
      {
        _playerBestPosY   = _playerRealPosY;
        _pastBricksBroken = *_bricksBroken;
      }
  }

  __INLINE__ void advanceStateImpl(const InputSet::inputIndex_t input) override
  {
    // Remembering current screen scroll
    auto curScroll = *_screenScrollY;

    _emulator->advanceState(input);

    // Increasing counter if input is null
    if (input != _nullInputIdx) _lastInputStep = _currentStep;
    _currentStep++;

    // Getting new scroll
    auto newScroll = *_screenScrollY;

    // If new scroll is larger than current, a new cycle is made
    if (newScroll > curScroll) _fullScreenScrolls++;
  }

  __INLINE__ void computeAdditionalHashing(MetroHash128& hashEngine) const override
  {
    hashEngine.Update(&_lowMem[0x0000], 0x000A - 0x0000); //
    hashEngine.Update(_lowMem[0x000F]);                   // No idea but seems important
    hashEngine.Update(&_lowMem[0x0051], 0x009D - 0x0051); // Skipping timers
    hashEngine.Update(&_lowMem[0x00A9], 0x00F6 - 0x00A9);
    hashEngine.Update(&_lowMem[0x0100], 0x01F7 - 0x0100);
    hashEngine.Update(&_lowMem[0x01FD], 0x0217 - 0x01FD);
    hashEngine.Update(&_lowMem[0x0400], 0x04C0 - 0x0400); // Brick states

    // Bonus-stage: hash ONLY the cloud immediately above the player (the one to land on next). Without
    // any cloud in the hash, states differing only in cloud position dedup together, so the search
    // can't represent "wait for the cloud to arrive" and can't time a landing. But hashing ALL clouds
    // explodes the state space with positions of clouds we've already passed or are far above -- which
    // is irrelevant noise. Hashing just the next cloud up lets the search time THAT landing without the
    // blowup. Gated on the bonus stage so the climb (where these bytes are static layout) is untouched.
    if (_isCondorActive == 1)
    {
      int pY = (int)_lowMem[0x0066];
      int tgt = -1, tgtY = -1000;
      for (int i = 0; i < 4; i++)
      {
        if (_lowMem[0x0786 + i] == 0) continue;
        int cY = (int)_lowMem[0x06BE + i] + 32;
        if (cY < pY - 1 && cY > tgtY) { tgtY = cY; tgt = i; }
      }
      // Hash the next cloud's X QUANTIZED to 8px buckets: the cloud is ~48px wide so the search only
      // needs its general screen position to time a landing, not the exact pixel -- this collapses
      // 256 cloud positions to 32, keeping the timing-relevant state space small.
      if (tgt >= 0) { uint8_t q = (uint8_t)(_lowMem[0x0682 + tgt] >> 3); hashEngine.Update(q); }
    }
  }

  // Updating derivative values after updating the internal state
  __INLINE__ void stateUpdatePostHook() override
  {
    updateRealPosY();

    // The condor ("giant bird") is the grab target. Its position is $D6 (X) / $D7 (Y); once the
    // player nears the top it flies at a fixed top-screen altitude ($D7 == 21), circling and
    // wrapping around in X. Player ($64,$66) and condor ($D6,$D7) are BOTH in the same settled
    // top-screen coordinate frame, so the grab distance is computed in SCREEN coords -- NOT the
    // player's continuous climb-Y. (The old logic used $0243, which stays 112 until the grab
    // itself, and $D3, which only animates at the grab -- both useless for guiding the chase.)
    _isCondorActive = (_lowMem[0x00D7] == 21) ? 1 : 0;
    if (_isCondorActive) _isBirdDetected = 1; // latched flag kept for the config's reward rule
    if (_isCondorActive)
    {
      _distanceToBirdX = std::abs((float)*_playerPosX - (float)_lowMem[0x00D6]);
      if (_distanceToBirdX > 128.0f) _distanceToBirdX = 256.0f - _distanceToBirdX; // condor X wraps
      _distanceToBirdY = std::abs((float)*_playerPosY - (float)_lowMem[0x00D7]);
      _distanceToBird  = sqrt(_distanceToBirdX * _distanceToBirdX + _distanceToBirdY * _distanceToBirdY);

      // MONOTONIC closest approach. The condor circles continuously and outruns the player, so a
      // raw distance reward makes the beam chase it forever. Rewarding the closest-ever distance
      // (like the climb's _bestReachY) locks in progress: the beam keeps the best interception
      // states and drives the min toward 0 (overlap = grab), instead of following the condor around.
      if (_distanceToBird < _minBirdDist) _minBirdDist = _distanceToBird;
    }
    else { _distanceToBird = _distanceToBirdX = _distanceToBirdY = 0.0f; }
  }

  __INLINE__ void ruleUpdatePreHook() override {}

  __INLINE__ void ruleUpdatePostHook() override {}

  __INLINE__ void serializeStateImpl(jaffarCommon::serializer::Base& serializer) const override
  {
    serializer.pushContiguous(&_currentStep, sizeof(_currentStep));
    serializer.pushContiguous(&_lastInputStep, sizeof(_lastInputStep));
    serializer.pushContiguous(&_isBirdDetected, sizeof(_isBirdDetected));
    serializer.pushContiguous(&_playerRealPosY, sizeof(_playerRealPosY));
    serializer.pushContiguous(&_fullScreenScrolls, sizeof(_fullScreenScrolls));
    serializer.pushContiguous(&_playerBestPosY, sizeof(_playerBestPosY));
    serializer.pushContiguous(&_pastBricksBroken, sizeof(_pastBricksBroken));
    serializer.pushContiguous(&_playerRealBestPosYDiff, sizeof(_playerRealBestPosYDiff));
    serializer.pushContiguous(&_bestReachY, sizeof(_bestReachY));
    serializer.pushContiguous(&_minBirdDist, sizeof(_minBirdDist));
  }

  __INLINE__ void deserializeStateImpl(jaffarCommon::deserializer::Base& deserializer)
  {
    deserializer.popContiguous(&_currentStep, sizeof(_currentStep));
    deserializer.popContiguous(&_lastInputStep, sizeof(_lastInputStep));
    deserializer.popContiguous(&_isBirdDetected, sizeof(_isBirdDetected));
    deserializer.popContiguous(&_playerRealPosY, sizeof(_playerRealPosY));
    deserializer.popContiguous(&_fullScreenScrolls, sizeof(_fullScreenScrolls));
    deserializer.popContiguous(&_playerBestPosY, sizeof(_playerBestPosY));
    deserializer.popContiguous(&_pastBricksBroken, sizeof(_pastBricksBroken));
    deserializer.popContiguous(&_playerRealBestPosYDiff, sizeof(_playerRealBestPosYDiff));
    deserializer.popContiguous(&_bestReachY, sizeof(_bestReachY));
    deserializer.popContiguous(&_minBirdDist, sizeof(_minBirdDist));
  }

  __INLINE__ float calculateGameSpecificReward() const
  {
    // Getting rewards from rules
    float reward = 0.0;

    // Climb reward = SECURED (standing) height as the primary term, plus a smaller airborne-reach
    // bonus. Why the split: a pure airborne reward rewards jumping above the top of the current
    // screen into the void (posY wraps past 0) -- a dead end, since the camera only scrolls from a
    // STANDING position at posY<96. That void-peak then out-scores the land-and-scroll progression
    // and traps the beam (mountain 2 stalled at the 2nd-screen top). Making secured height dominant
    // commits the search to landing on real higher floors / triggering the next scroll, while the
    // airborne bonus still supplies the hole-making gradient (a jump that opens the ceiling reaches
    // higher). Small current-Y term breaks ties toward being high right now.
    // Climb reward: SECURED (standing) height dominant + a smaller airborne-reach bonus (the dense
    // hole-making gradient -- without it the climb stalls almost immediately). This is the recipe
    // that climbed mountains 1-2 to the top and mountain 3 to ~-365 (the grab, not the climb, is
    // the open problem there).
    // (Tried dropping the airborne term in the bonus stage to force align-then-jump; it removed the
    // drive to ascend and made things worse -- dist 69 vs 30 -- so the airborne bonus stays.)
    reward -= 1.0f * _playerBestPosY + 0.3f * _bestReachY + 0.01f * _playerRealPosY;

    // Reward making holes in the ceiling (hole-making progress).
    reward += 2.0 * (float)*_bricksBroken;

    // Once the condor has been seen, reward the closest the player has EVER come to it (monotonic,
    // so the beam converges toward overlap instead of chasing the circling condor). The climb term
    // plateaus at the summit, so this drives the endgame; overlap fires the grab/win.
    if (_isBirdDetected == 1) reward += 256.0f - _minBirdDist;

    // BONUS-STAGE CLOUD LADDER (secondary driver). The top of each mountain is a bonus stage where
    // the player must ascend by landing on moving cloud platforms before reaching the condor. There
    // are up to 4 clouds (slot i): screen span [ $0682+i , $06A0+i ] (~48px wide), screen Y =
    // $06BE+i + 32, active = $0786+i (all reverse-engineered; sub_E33C advances them each frame).
    // A pure height/condor reward gives no gradient to MEET a moving cloud, so the search jumps into
    // gaps. Here we reward horizontal alignment with the NEXT cloud above the player (the one it must
    // land on next). Kept a SIGNIFICANT SECONDARY term -- it differentiates states within a cloud
    // level (where height is ~constant) without overriding the primary height/condor drive.
    if (_isCondorActive == 1)
    {
      float pX = (float)*_playerPosX, pY = (float)*_playerPosY;
      int   tgt = -1; float tgtY = -1.0e9f;
      for (int i = 0; i < 4; i++)
      {
        if (_lowMem[0x0786 + i] == 0) continue;             // inactive slot
        float cY = (float)_lowMem[0x06BE + i] + 32.0f;       // cloud screen Y
        if (cY < pY - 1.0f && cY > tgtY) { tgtY = cY; tgt = i; } // closest cloud ABOVE the player
      }
      if (tgt >= 0)
      {
        // General "head toward the next cloud" signal (NOT mountain-specific): reward horizontal
        // alignment with the next cloud's span. NOTE: this is greedy and on some mountains (e.g. mtn3)
        // the nearest cloud is a trap requiring a non-greedy wait-for-wrap -- which a reward-priority
        // beam can't generally express (delayed-reward). Kept as the best general guidance we have.
        float L = (float)_lowMem[0x0682 + tgt], R = (float)_lowMem[0x06A0 + tgt];
        float dx;
        if (R >= L) dx = (pX < L) ? (L - pX) : (pX > R ? pX - R : 0.0f);            // [L,R] span
        else        dx = (pX <= R || pX >= L) ? 0.0f : std::min(L - pX, pX - R + 256.0f); // wrapped
        reward -= 0.5f * dx;
      }
      else
      {
        // No cloud above -> the player has ridden the clouds up to the top (solid) platform, and the
        // only thing left is to grab the condor. FOLLOW THE CONDOR by tracking its PREDICTED position:
        // it moves -1/+1 px per frame in direction $D5, and a grab jump takes ~LEAD frames to reach
        // its altitude, during which it slides. Reward aligning with where it WILL be, so the jump
        // intercepts it (instead of aiming where it is now and arriving late). Significant driver.
        float v     = (_lowMem[0x00D5] != 0) ? 1.0f : -1.0f;
        float predX = (float)_lowMem[0x00D6] + v * 10.0f; // lead by ~jump-rise time
        float dxc   = std::abs(pX - predX);
        if (dxc > 128.0f) dxc = 256.0f - dxc;             // condor X wraps
        reward -= 1.0f * dxc;
      }
    }

    // Huge bonus on the grab (= level complete), so a winning line is the global best-reward state
    // (=> best.sol is the win). Detected exactly like the win rule: grab timer $4D>0 with the
    // condor moved off-center ($0243 != 112).
    if (_lowMem[0x004D] > 0 && _lowMem[0x0243] != 112) reward += 1000000.0f;

    // Per-step time cost (=> faster solutions score higher; also orders dedup collisions toward the
    // earlier path). PHASE-CONDITIONAL, because in a reward-priority beam this weight controls how
    // tightly the frontier focuses on the leading edge:
    //   - Climbing (condor not yet seen): heavy (1.0) so the beam drops trailing states and races
    //     up the mountain. A light weight here climbs ~3x slower (beam clogs with stale states).
    //   - Grab phase (condor seen): light (0.05) so the beam KEEPS the patient states needed to
    //     wait for the slowly-circling condor to come around and intercept it. A heavy weight here
    //     starves the grab (mountain 2 exhausted its frontier before grabbing).
    reward -= (_isBirdDetected ? 0.05f : 1.0f) * (float)_currentStep;

    // Returning reward
    return reward;
  }

  void printInfoImpl() const override
  {
    jaffarCommon::logger::log("[J+]  + Player Real Pos Y:                  %.0f\n", _playerRealPosY);
    jaffarCommon::logger::log("[J+]  + Player Best Pos Y:                  %.0f\n", _playerBestPosY);
    jaffarCommon::logger::log("[J+]  + Player Best Reach Y (airborne):     %.0f\n", _bestReachY);
    jaffarCommon::logger::log("[J+]  + Player Best / Real Pos Y Diff:      %.0f\n", _playerRealBestPosYDiff);
    jaffarCommon::logger::log("[J+]  + Past Bricks Broken:                 %u\n", _pastBricksBroken);
    jaffarCommon::logger::log("[J+]  + Full Screen Scrolls:                %u\n", _fullScreenScrolls);
    if (_isBirdDetected == 1) jaffarCommon::logger::log("[J+]  + Distance To Bird:                  %.2f (%.2f, %.2f)\n", _distanceToBird, _distanceToBirdX, _distanceToBirdY);
  }

  bool parseRuleActionImpl(Rule& rule, const std::string& actionType, const nlohmann::json& actionJs) override
  {
    bool recognizedActionType = false;

    return recognizedActionType;
  }

  __INLINE__ jaffarCommon::hash::hash_t getStateInputHash() override
  {
    // There is no discriminating state element, so simply return a zero hash
    return jaffarCommon::hash::hash_t();
  }

  uint8_t* _currentLevel;
  uint8_t* _playerPosX;
  uint8_t* _playerPosY;
  uint8_t* _screenScrollY;
  uint8_t* _screenScrollY1;
  uint8_t* _screenScrollY2;
  uint8_t* _screenScrollY3;
  uint8_t* _playerFloor;
  uint8_t* _blocksHit;
  uint8_t* _vegetablesCollected;
  uint8_t* _nitpickersHit;
  uint8_t* _bricksBroken;
  uint8_t* _playerInputs1;
  uint8_t* _playerInputs2;
  uint8_t* _globalTimer;
  uint8_t* _subTimer;
  uint8_t* _birdState;
  uint8_t* _birdPosX;
  uint8_t* _lowMem;
  uint8_t  _isBirdDetected;
  uint8_t  _isCondorActive; // condor flying at top this frame ($D7==21) — gates the screen-coord chase
  float    _minBirdDist;    // closest the player has EVER come to the condor (monotonic) — drives grab
  uint8_t* _playerJumping;
  uint8_t* _birdDirection;

  float _distanceToBirdX;
  float _distanceToBirdY;
  float _distanceToBird;

  // Remember frame in which the last input was made
  uint16_t _lastInputStep;
  uint16_t _currentStep;
  uint8_t  _fullScreenScrolls;
  uint8_t  _pastBricksBroken;
  float    _playerRealPosY;
  float    _playerBestPosY;   // best height reached while STANDING (secured) — drives fall-back prune
  float    _bestReachY;       // best height EVER reached incl. airborne (jump peaks) — drives reward
  float    _playerRealBestPosYDiff;

  // Null input index to remember the last valid input
  InputSet::inputIndex_t _nullInputIdx;
};

} // namespace nes

} // namespace games

} // namespace jaffarPlus