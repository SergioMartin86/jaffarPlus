#pragma once

#include <emulator.hpp>
#include <game.hpp>
#include <jaffarCommon/json.hpp>
#include <jaffarCommon/logger.hpp>
#include <memory>
#include <quickerArkBot/quickerArkBot.hpp>

namespace jaffarPlus
{

namespace games
{

namespace arkbot
{

class Arkanoid final : public jaffarPlus::Game
{
public:
  static __INLINE__ std::string getName() { return "ArkBot / Arkanoid"; }

  Arkanoid(std::unique_ptr<Emulator> emulator, const nlohmann::json& config) : jaffarPlus::Game(std::move(emulator), config)
  {
    _quickerArkBot = dynamic_cast<jaffarPlus::emulator::QuickerArkBot*>(_emulator.get());
    _arkState      = _quickerArkBot->getGameState();

    // Registering input index LUT
    for (size_t i = 0; i < 161; i++) paddlePositionIndexLUT[i] = 255;
    for (size_t i = 16; i < 161; i++) paddlePositionIndexLUT[i] = _emulator->registerInput(paddlePositionStringLUT[i]);

    registerGameProperty("Operation State", &_arkState->opState, Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Paddle X", &_arkState->paddleX, Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Paddle Collision", &_arkState->paddleX, Property::datatype_t::dt_bool, Property::endianness_t::little);

    registerGameProperty("Ball 0 Pos X", &_arkState->ball[0].pos.x, Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Ball 0 Pos Y", &_arkState->ball[0].pos.y, Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Ball 0 Vel X", &_arkState->ball[0].vel.vx, Property::datatype_t::dt_int8, Property::endianness_t::little);
    registerGameProperty("Ball 0 Vel Y", &_arkState->ball[0].vel.vy, Property::datatype_t::dt_int8, Property::endianness_t::little);
    registerGameProperty("Ball 0 Vel Sign X", &_arkState->ball[0].vSign.vx, Property::datatype_t::dt_int8, Property::endianness_t::little);
    registerGameProperty("Ball 0 Vel Sign Y", &_arkState->ball[0].vSign.vy, Property::datatype_t::dt_int8, Property::endianness_t::little);
    registerGameProperty("Ball 0 Angle", &_arkState->ball[0].angle, Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Ball 0 Cycle", &_arkState->ball[0].cycle, Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Ball 0 Exists", &_arkState->ball[0].exists, Property::datatype_t::dt_bool, Property::endianness_t::little);
    registerGameProperty("Ball 0 Collision X", &_arkState->ball[0].xCollis, Property::datatype_t::dt_bool, Property::endianness_t::little);
    registerGameProperty("Ball 0 Collision Y", &_arkState->ball[0].yCollis, Property::datatype_t::dt_bool, Property::endianness_t::little);
    registerGameProperty("Ball 0 Speed Multiplier", &_arkState->ball[0].speedMult, Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Ball 0 Speed Stage", &_arkState->ball[0].speedStage, Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Ball 0 Speed Stage M", &_arkState->ball[0].speedStageM, Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Ball 0 Speed Row Idx", &_arkState->ball[0].speedRowIdx, Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Ball 0 Paddle Collision", &_arkState->ball[0]._paddleCollis, Property::datatype_t::dt_bool, Property::endianness_t::little);

    registerGameProperty("Ball 1 Pos X", &_arkState->ball[1].pos.x, Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Ball 1 Pos Y", &_arkState->ball[1].pos.y, Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Ball 1 Vel X", &_arkState->ball[1].vel.vx, Property::datatype_t::dt_int8, Property::endianness_t::little);
    registerGameProperty("Ball 1 Vel Y", &_arkState->ball[1].vel.vy, Property::datatype_t::dt_int8, Property::endianness_t::little);
    registerGameProperty("Ball 1 Vel Sign X", &_arkState->ball[1].vSign.vx, Property::datatype_t::dt_int8, Property::endianness_t::little);
    registerGameProperty("Ball 1 Vel Sign Y", &_arkState->ball[1].vSign.vy, Property::datatype_t::dt_int8, Property::endianness_t::little);
    registerGameProperty("Ball 1 Angle", &_arkState->ball[1].angle, Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Ball 1 Cycle", &_arkState->ball[1].cycle, Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Ball 1 Exists", &_arkState->ball[1].exists, Property::datatype_t::dt_bool, Property::endianness_t::little);
    registerGameProperty("Ball 1 Collision X", &_arkState->ball[1].xCollis, Property::datatype_t::dt_bool, Property::endianness_t::little);
    registerGameProperty("Ball 1 Collision Y", &_arkState->ball[1].yCollis, Property::datatype_t::dt_bool, Property::endianness_t::little);
    registerGameProperty("Ball 1 Speed Multiplier", &_arkState->ball[1].speedMult, Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Ball 1 Speed Stage", &_arkState->ball[1].speedStage, Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Ball 1 Speed Stage M", &_arkState->ball[1].speedStageM, Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Ball 1 Speed Row Idx", &_arkState->ball[1].speedRowIdx, Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Ball 1 Paddle Collision", &_arkState->ball[1]._paddleCollis, Property::datatype_t::dt_bool, Property::endianness_t::little);

    registerGameProperty("Ball 2 Pos X", &_arkState->ball[2].pos.x, Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Ball 2 Pos Y", &_arkState->ball[2].pos.y, Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Ball 2 Vel X", &_arkState->ball[2].vel.vx, Property::datatype_t::dt_int8, Property::endianness_t::little);
    registerGameProperty("Ball 2 Vel Y", &_arkState->ball[2].vel.vy, Property::datatype_t::dt_int8, Property::endianness_t::little);
    registerGameProperty("Ball 2 Vel Sign X", &_arkState->ball[2].vSign.vx, Property::datatype_t::dt_int8, Property::endianness_t::little);
    registerGameProperty("Ball 2 Vel Sign Y", &_arkState->ball[2].vSign.vy, Property::datatype_t::dt_int8, Property::endianness_t::little);
    registerGameProperty("Ball 2 Angle", &_arkState->ball[2].angle, Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Ball 2 Cycle", &_arkState->ball[2].cycle, Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Ball 2 Exists", &_arkState->ball[2].exists, Property::datatype_t::dt_bool, Property::endianness_t::little);
    registerGameProperty("Ball 2 Collision X", &_arkState->ball[2].xCollis, Property::datatype_t::dt_bool, Property::endianness_t::little);
    registerGameProperty("Ball 2 Collision Y", &_arkState->ball[2].yCollis, Property::datatype_t::dt_bool, Property::endianness_t::little);
    registerGameProperty("Ball 2 Speed Multiplier", &_arkState->ball[2].speedMult, Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Ball 2 Speed Stage", &_arkState->ball[2].speedStage, Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Ball 2 Speed Stage M", &_arkState->ball[2].speedStageM, Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Ball 2 Speed Row Idx", &_arkState->ball[2].speedRowIdx, Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Ball 2 Paddle Collision", &_arkState->ball[2]._paddleCollis, Property::datatype_t::dt_bool, Property::endianness_t::little);

    for (size_t i = 0; i < GameConsts::BlockTableSize - 33; i++)
      registerGameProperty(std::string("Block[") + std::to_string(i) + std::string("]"), &_arkState->blocks[i], Property::datatype_t::dt_uint8, Property::endianness_t::little);

    registerGameProperty("Total Blocks", &_arkState->totalBlocks, Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Current Blocks", &_arkState->currentBlocks, Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Block Collision Count", &_arkState->blockCollisCount, Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Block Collision Side (This Block)", &_arkState->blockCollisSide.thisBlock, Property::datatype_t::dt_bool, Property::endianness_t::little);
    registerGameProperty("Block Collision Side (One Right)", &_arkState->blockCollisSide.oneRight, Property::datatype_t::dt_bool, Property::endianness_t::little);
    registerGameProperty("Block Collision Side (One Down)", &_arkState->blockCollisSide.oneDown, Property::datatype_t::dt_bool, Property::endianness_t::little);
    registerGameProperty("Block Collision Side (One Down Right)", &_arkState->blockCollisSide.oneDownRight, Property::datatype_t::dt_bool, Property::endianness_t::little);

    registerGameProperty("Just Hit Block 0", &_arkState->justHitBlock[0], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Just Hit Block 1", &_arkState->justHitBlock[1], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Just Hit Block 2", &_arkState->justHitBlock[2], Property::datatype_t::dt_uint8, Property::endianness_t::little);

    registerGameProperty("Just Hit Block 0 Cell X", &_arkState->justHitBlockCell[0].x, Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Just Hit Block 0 Cell Y", &_arkState->justHitBlockCell[0].y, Property::datatype_t::dt_uint8, Property::endianness_t::little);

    registerGameProperty("Just Hit Block 1 Cell X", &_arkState->justHitBlockCell[1].x, Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Just Hit Block 1 Cell Y", &_arkState->justHitBlockCell[1].y, Property::datatype_t::dt_uint8, Property::endianness_t::little);

    registerGameProperty("Just Hit Block 2 Cell X", &_arkState->justHitBlockCell[2].x, Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Just Hit Block 2 Cell Y", &_arkState->justHitBlockCell[2].y, Property::datatype_t::dt_uint8, Property::endianness_t::little);

    registerGameProperty("Just Destroyed Block 0", &_arkState->justDestrBlock[0], Property::datatype_t::dt_bool, Property::endianness_t::little);
    registerGameProperty("Just Destroyed Block 1", &_arkState->justDestrBlock[1], Property::datatype_t::dt_bool, Property::endianness_t::little);
    registerGameProperty("Just Destroyed Block 2", &_arkState->justDestrBlock[2], Property::datatype_t::dt_bool, Property::endianness_t::little);

    registerGameProperty("Calculated Cell X", &_arkState->calculatedCell.x, Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Calculated Cell Y", &_arkState->calculatedCell.y, Property::datatype_t::dt_uint8, Property::endianness_t::little);

    registerGameProperty("Spawned PowerUp", &_arkState->spawnedPowerup, Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Owned PowerUp", &_arkState->ownedPowerup, Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("PowerUp Pos X", &_arkState->powerupPos.x, Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("PowerUp Pos Y", &_arkState->powerupPos.y, Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Just Spawned PowerUp", &_arkState->justSpawnedPowerup, Property::datatype_t::dt_bool, Property::endianness_t::little);

    registerGameProperty("Score", &_arkState->score, Property::datatype_t::dt_uint32, Property::endianness_t::little);
    registerGameProperty("Pending Score", &_arkState->pendingScore, Property::datatype_t::dt_uint32, Property::endianness_t::little);

    registerGameProperty("Enemy 0 Active", &_arkState->enemies[0].active, Property::datatype_t::dt_bool, Property::endianness_t::little);
    registerGameProperty("Enemy 0 Exiting", &_arkState->enemies[0].exiting, Property::datatype_t::dt_bool, Property::endianness_t::little);
    registerGameProperty("Enemy 0 Destruction Frame", &_arkState->enemies[0].destrFrame, Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Enemy 0 Pos X", &_arkState->enemies[0].pos.x, Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Enemy 0 Pos Y", &_arkState->enemies[0].pos.y, Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Enemy 0 Movement Type", &_arkState->enemies[0].movementType, Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Enemy 0 Move Timer", &_arkState->enemies[0].moveTimer, Property::datatype_t::dt_uint16, Property::endianness_t::little);
    registerGameProperty("Enemy 0 Descent Timer", &_arkState->enemies[0].descentTimer, Property::datatype_t::dt_uint16, Property::endianness_t::little);
    registerGameProperty("Enemy 0 Animation Timer", &_arkState->enemies[0].animTimer, Property::datatype_t::dt_uint16, Property::endianness_t::little);
    registerGameProperty("Enemy 0 Circle Stage", &_arkState->enemies[0].circleStage, Property::datatype_t::dt_uint16, Property::endianness_t::little);
    registerGameProperty("Enemy 0 Circle Half", &_arkState->enemies[0].circleHalf, Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Enemy 0 Circle Direction", &_arkState->enemies[0].circleDir, Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Enemy 0 Id", &_arkState->enemies[0]._id, Property::datatype_t::dt_uint16, Property::endianness_t::little);

    registerGameProperty("Enemy 1 Active", &_arkState->enemies[1].active, Property::datatype_t::dt_bool, Property::endianness_t::little);
    registerGameProperty("Enemy 1 Exiting", &_arkState->enemies[1].exiting, Property::datatype_t::dt_bool, Property::endianness_t::little);
    registerGameProperty("Enemy 1 Destruction Frame", &_arkState->enemies[1].destrFrame, Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Enemy 1 Pos X", &_arkState->enemies[1].pos.x, Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Enemy 1 Pos Y", &_arkState->enemies[1].pos.y, Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Enemy 1 Movement Type", &_arkState->enemies[1].movementType, Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Enemy 1 Move Timer", &_arkState->enemies[1].moveTimer, Property::datatype_t::dt_uint16, Property::endianness_t::little);
    registerGameProperty("Enemy 1 Descent Timer", &_arkState->enemies[1].descentTimer, Property::datatype_t::dt_uint16, Property::endianness_t::little);
    registerGameProperty("Enemy 1 Animation Timer", &_arkState->enemies[1].animTimer, Property::datatype_t::dt_uint16, Property::endianness_t::little);
    registerGameProperty("Enemy 1 Circle Stage", &_arkState->enemies[1].circleStage, Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Enemy 1 Circle Half", &_arkState->enemies[1].circleHalf, Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Enemy 1 Circle Direction", &_arkState->enemies[1].circleDir, Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Enemy 1 Id", &_arkState->enemies[1]._id, Property::datatype_t::dt_uint8, Property::endianness_t::little);

    registerGameProperty("Enemy 2 Active", &_arkState->enemies[2].active, Property::datatype_t::dt_bool, Property::endianness_t::little);
    registerGameProperty("Enemy 2 Exiting", &_arkState->enemies[2].exiting, Property::datatype_t::dt_bool, Property::endianness_t::little);
    registerGameProperty("Enemy 2 Destruction Frame", &_arkState->enemies[2].destrFrame, Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Enemy 2 Pos X", &_arkState->enemies[2].pos.x, Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Enemy 2 Pos Y", &_arkState->enemies[2].pos.y, Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Enemy 2 Movement Type", &_arkState->enemies[2].movementType, Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Enemy 2 Move Timer", &_arkState->enemies[2].moveTimer, Property::datatype_t::dt_uint16, Property::endianness_t::little);
    registerGameProperty("Enemy 2 Descent Timer", &_arkState->enemies[2].descentTimer, Property::datatype_t::dt_uint16, Property::endianness_t::little);
    registerGameProperty("Enemy 2 Animation Timer", &_arkState->enemies[2].animTimer, Property::datatype_t::dt_uint16, Property::endianness_t::little);
    registerGameProperty("Enemy 2 Circle Stage", &_arkState->enemies[2].circleStage, Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Enemy 2 Circle Half", &_arkState->enemies[2].circleHalf, Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Enemy 2 Circle Direction", &_arkState->enemies[2].circleDir, Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Enemy 2 Id", &_arkState->enemies[2]._id, Property::datatype_t::dt_uint8, Property::endianness_t::little);

    registerGameProperty("Enemy Spawn Timer 0", &_arkState->enemySpawnTimers[0], Property::datatype_t::dt_uint16, Property::endianness_t::little);
    registerGameProperty("Enemy Spawn Timer 1", &_arkState->enemySpawnTimers[1], Property::datatype_t::dt_uint16, Property::endianness_t::little);
    registerGameProperty("Enemy Spawn Timer 2", &_arkState->enemySpawnTimers[2], Property::datatype_t::dt_uint16, Property::endianness_t::little);
    registerGameProperty("Enemy Spawn Index", &_arkState->enemySpawnIndex, Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Enemy Gate State", &_arkState->enemyGateState, Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Enemy Gate Intex", &_arkState->enemyGateIndex, Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Enemy Gate Timer", &_arkState->enemyGateTimer, Property::datatype_t::dt_uint16, Property::endianness_t::little);

    registerGameProperty("Overall Speed Stage", &_arkState->overallSpeedStage, Property::datatype_t::dt_uint16, Property::endianness_t::little);
    registerGameProperty("Overall Speed Stage M", &_arkState->overallSpeedStageM, Property::datatype_t::dt_uint16, Property::endianness_t::little);
    registerGameProperty("Speed Stage Counter", &_arkState->speedStageCounter, Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Speed Reduction", &_arkState->speedReduction, Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Level", &_arkState->level, Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Mystery Input", &_arkState->mysteryInput, Property::datatype_t::dt_uint16, Property::endianness_t::little);
    registerGameProperty("Boss Hits", &_arkState->bossHits, Property::datatype_t::dt_uint8, Property::endianness_t::little);

    registerGameProperty("Ceil Collision", &_arkState->ceilCollis, Property::datatype_t::dt_bool, Property::endianness_t::little);
    registerGameProperty("RNG Was Used 1", &_arkState->_wasRNGUsed1, Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("RNG Was Used 2", &_arkState->_wasRNGUsed2, Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("RNG Was Used 3", &_arkState->_wasRNGUsed3, Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("RNG Was Used 4", &_arkState->_wasRNGUsed4, Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Enemy Gate Active", &_arkState->_enemyGateActive, Property::datatype_t::dt_uint8, Property::endianness_t::little);

    registerGameProperty("Just Moved Enemy", &_arkState->_justMovedEnemy, Property::datatype_t::dt_int8, Property::endianness_t::little);
    registerGameProperty("Enemy Mystery Input", &_arkState->_enemyMysteryInput, Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Enemy Move Options", &_arkState->_enemyMoveOptions, Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Queue Enemy Destruction", &_arkState->_enemyMoveOptions, Property::datatype_t::dt_int8, Property::endianness_t::little);
    registerGameProperty("Ball Hits Remaining", &_ballHitsRemaining, Property::datatype_t::dt_uint16, Property::endianness_t::little);

    for (size_t i = 0; i <= GameConsts::BlockTableSize - 33; i++)
    {
      auto propertyName = std::string("Block Value [") + std::to_string(i) + std::string("]");
      registerGameProperty(propertyName, &_arkState->blocks[i], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    }
  }

private:
  void initializeImpl() override { _quickerArkBot->doSoftReset(); }

  __INLINE__ void registerGameProperties() override { registerGameProperty("Score", &_arkState->score, Property::datatype_t::dt_uint32, Property::endianness_t::little); }

  __INLINE__ void advanceStateImpl(const InputSet::inputIndex_t input) override
  {
    // Running emulator
    _emulator->advanceState(input);
  }

  __INLINE__ void computeAdditionalHashing(MetroHash128& hashEngine) const override
  {
    hashEngine.Update(*_arkState);

    // hashEngine.Update(_arkState->opState);
    // hashEngine.Update(_arkState->paddleCollis);
    // hashEngine.Update(_arkState->ball);
    // hashEngine.Update(_arkState->blocks);
    // hashEngine.Update(_arkState->totalBlocks         );
    // hashEngine.Update(_arkState->currentBlocks       );
    // hashEngine.Update(_arkState->blockCollisCount    );
    // hashEngine.Update(_arkState->blockCollisSide     );
    // hashEngine.Update(_arkState->justHitBlock);
    // hashEngine.Update(_arkState->justHitBlockCell );
    // hashEngine.Update(_arkState->justDestrBlock   );
    // hashEngine.Update(_arkState->calculatedCell      );
    // hashEngine.Update(_arkState->spawnedPowerup      );
    // hashEngine.Update(_arkState->ownedPowerup        );
    // hashEngine.Update(_arkState->powerupPos          );
    // hashEngine.Update(_arkState->justSpawnedPowerup  );
    // hashEngine.Update(_arkState->score        );
    // hashEngine.Update(_arkState->pendingScore );
    // hashEngine.Update(_arkState->enemies);
    // hashEngine.Update(_arkState->enemySpawnTimers);
    // hashEngine.Update(_arkState->enemySpawnIndex    );
    // hashEngine.Update(_arkState->enemyGateState     );
    // hashEngine.Update(_arkState->enemyGateIndex     );
    // hashEngine.Update(_arkState->enemyGateTimer     );
    // hashEngine.Update(_arkState->overallSpeedStage  );
    // hashEngine.Update(_arkState->overallSpeedStageM );
    // hashEngine.Update(_arkState->speedStageCounter  );
    // hashEngine.Update(_arkState->speedReduction     );
    // hashEngine.Update(_arkState->level              );
    // // hashEngine.Update(_arkState->mysteryInput       );
    // hashEngine.Update(_arkState->bossHits           );
    // hashEngine.Update(_arkState->ceilCollis         );
  }

  // Updating derivative values after updating the internal state
  __INLINE__ void stateUpdatePostHook() override
  {
    _ballInRange0 = ballInRange(_arkState->ball[0]);
    _ballInRange1 = ballInRange(_arkState->ball[1]);
    _ballInRange2 = ballInRange(_arkState->ball[2]);

    _ballHitsRemaining = 0;
    for (size_t i = 0; i < GameConsts::BlockTableSize - 33; i++)
    {
      switch (_arkState->blocks[i])
      {
        case 0xE6: _ballHitsRemaining += 6; break;
        case 0xE5: _ballHitsRemaining += 5; break;
        case 0xE4: _ballHitsRemaining += 4; break;
        case 0xE3: _ballHitsRemaining += 3; break;
        case 0xE2: _ballHitsRemaining += 2; break;
        case 0xF0: break; // Gold Brick
        case 0x00: break;
        default: _ballHitsRemaining++;
      }
    }
  }

  __INLINE__ void ruleUpdatePreHook() override {}

  __INLINE__ void ruleUpdatePostHook() override {}

  __INLINE__ void serializeStateImpl(jaffarCommon::serializer::Base& serializer) const override {}

  __INLINE__ void deserializeStateImpl(jaffarCommon::deserializer::Base& deserializer) {}

  __INLINE__ void getAdditionalAllowedInputs(std::vector<InputSet::inputIndex_t>& allowedInputSet)
  {
    ///// First of all, we add possible inputs for things that matter in the present.
    // For example, place the paddle to hit a ball in the next frame or kill an emeny with the paddle

    // { DecisionPoint::LaunchBall, "Launch Ball" },
    // Handled at configuration

    // { DecisionPoint::BounceBall1, "Bounce Ball 1" },
    // { DecisionPoint::BounceBall2, "Bounce Ball 2" },
    // { DecisionPoint::BounceBall3, "Bounce Ball 3" },

    addBallInputs(_arkState->ball[0], allowedInputSet);
    addBallInputs(_arkState->ball[1], allowedInputSet);
    addBallInputs(_arkState->ball[2], allowedInputSet);

    // { DecisionPoint::CollectPowerup, "Collect Powerup" },
    if (powerUpInRange())
    {
      auto powerUpPaddlePos = (uint8_t)std::max(std::min(158, (int)_arkState->powerupPos.x - 2), 16);
      allowedInputSet.push_back(paddlePositionIndexLUT[powerUpPaddlePos]);

      // Adding options to not take the power up now
      if (powerUpPaddlePos <= 80) allowedInputSet.push_back(paddlePositionIndexLUT[158]);
      if (powerUpPaddlePos > 80) allowedInputSet.push_back(paddlePositionIndexLUT[16]);
    }

    // { DecisionPoint::HitEnemy1WithPaddle, "Hit Enemy 1 With Paddle" },
    // { DecisionPoint::HitEnemy2WithPaddle, "Hit Enemy 2 With Paddle" },
    // { DecisionPoint::HitEnemy3WithPaddle, "Hit Enemy 3 With Paddle" },

    if (enemyInRange(_arkState->enemies[0]))
    {
      auto enemyPosX = (uint8_t)std::max(std::min(158, (int)_arkState->enemies[0].pos.x), 16);
      allowedInputSet.push_back(paddlePositionIndexLUT[enemyPosX]);

      // Adding options to not hit the enemy now
      if (enemyPosX <= 80) allowedInputSet.push_back(paddlePositionIndexLUT[158]);
      if (enemyPosX > 80) allowedInputSet.push_back(paddlePositionIndexLUT[16]);
    }

    if (enemyInRange(_arkState->enemies[1]))
    {
      auto enemyPosX = (uint8_t)std::max(std::min(158, (int)_arkState->enemies[1].pos.x), 16);
      allowedInputSet.push_back(paddlePositionIndexLUT[enemyPosX]);

      // Adding options to not hit the enemy now
      if (enemyPosX <= 80) allowedInputSet.push_back(paddlePositionIndexLUT[158]);
      if (enemyPosX > 80) allowedInputSet.push_back(paddlePositionIndexLUT[16]);
    }

    if (enemyInRange(_arkState->enemies[2]))
    {
      auto enemyPosX = (uint8_t)std::max(std::min(158, (int)_arkState->enemies[2].pos.x), 16);
      allowedInputSet.push_back(paddlePositionIndexLUT[enemyPosX]);

      // Adding options to not hit the enemy now
      if (enemyPosX <= 80) allowedInputSet.push_back(paddlePositionIndexLUT[158]);
      if (enemyPosX > 80) allowedInputSet.push_back(paddlePositionIndexLUT[16]);
    }

    // { DecisionPoint::None_LevelEnded, "Level End" },
    // { DecisionPoint::None_LimitReached, "Limit Reached" },
    // { DecisionPoint::None_BallLost, "Ball Lost" },
    // { DecisionPoint::Invalid, "Invalid" }

    ////// Second, add inputs for things that will happen in the future
    // This includes manipulating the RNG with the paddle that will be processed in the next frame
    // For this, we need to save the current state, advance the state twice, check some flags and load back the current state
    GameState tempState;
    memcpy(&tempState, _arkState, sizeof(GameState));
    _emulator->advanceState(0);
    _detectRNGManipulationInputs = true;
    _emulator->advanceState(0);

    // Adding  paddle x input candidates
    for (const auto paddleX : _RNfGManipulationPaddleXCandidates) allowedInputSet.push_back(paddlePositionIndexLUT[paddleX - 2]);
    _detectRNGManipulationInputs = false;

    if (_arkState->_enemyGateActive)
    {
      allowedInputSet.push_back(paddlePositionIndexLUT[16]);
      allowedInputSet.push_back(paddlePositionIndexLUT[158]);
    }
    memcpy(_arkState, &tempState, sizeof(GameState));

    // If no other decisions were made, add wildcard
    if (allowedInputSet.empty()) allowedInputSet.push_back(paddlePositionIndexLUT[16]);
  };

  __INLINE__ void addBallInputs(const Ball& ball, std::vector<InputSet::inputIndex_t>& allowedInputSet) const
  {
    if (ballInRange(ball))
    {
      const auto ballPosX = ball.pos.x;

      auto input0 = (uint8_t)std::max(std::min(158, (int)ballPosX - 31 - 2), 16);
      auto input1 = (uint8_t)std::max(std::min(158, (int)ballPosX - 26 - 2), 16);
      auto input2 = (uint8_t)std::max(std::min(158, (int)ballPosX - 17 - 2), 16);
      auto input3 = (uint8_t)std::max(std::min(158, (int)ballPosX - 10 - 2), 16);
      auto input4 = (uint8_t)std::max(std::min(158, (int)ballPosX - 2 - 2), 16);
      auto input5 = (uint8_t)std::max(std::min(158, (int)ballPosX + 0 - 2), 16);

      allowedInputSet.push_back(paddlePositionIndexLUT[input0]);
      allowedInputSet.push_back(paddlePositionIndexLUT[input1]);
      allowedInputSet.push_back(paddlePositionIndexLUT[input2]);
      allowedInputSet.push_back(paddlePositionIndexLUT[input3]);
      allowedInputSet.push_back(paddlePositionIndexLUT[input4]);
      allowedInputSet.push_back(paddlePositionIndexLUT[input5]);

      // Adding options to not hit the ball now
      if (ballPosX <= 80) allowedInputSet.push_back(paddlePositionIndexLUT[158]);
      if (ballPosX > 80) allowedInputSet.push_back(paddlePositionIndexLUT[16]);
    }
  };

  __INLINE__ float calculateGameSpecificReward() const
  {
    // Getting rewards from rules
    float reward = 0.0;

    // Subtracting the number of remaining hits
    reward -= _ballHitsRemaining * 1000.0f;

    // Subtracting the number of remaining blocks
    reward -= _arkState->currentBlocks * 20.0f;

    // Subtracting pending score
    reward -= (float)_arkState->pendingScore * 0.001f;

    // Adding boss hits
    reward += (float)_arkState->bossHits * 1.0f;

    // Rewarding spawning powerup
    if (_arkState->spawnedPowerup == Powerup::Multiball) reward += 10000.0f;

    // Rewarding getting powerup
    if (_arkState->ownedPowerup == Powerup::Multiball) reward += 20000.0f;

    // Returning reward
    return reward;
  }

  void printInfoImpl() const override
  {
    jaffarCommon::logger::log("[J+]    + Is Ball 0 In Range                         %s\n", _ballInRange0 ? "True" : "False");
    jaffarCommon::logger::log("[J+]    + Is Ball 1 In Range                         %s\n", _ballInRange1 ? "True" : "False");
    jaffarCommon::logger::log("[J+]    + Is Ball 2 In Range                         %s\n", _ballInRange2 ? "True" : "False");

    jaffarCommon::logger::log("[J+]    + Ball 0 Pos Y                               %03u\n", _arkState->ball[0].pos.y);
    jaffarCommon::logger::log("[J+]    + Ball 1 Pos Y                               %03u\n", _arkState->ball[1].pos.y);
    jaffarCommon::logger::log("[J+]    + Ball 2 Pos Y                               %03u\n", _arkState->ball[2].pos.y);

    jaffarCommon::logger::log("[J+]    + Ball 0 VSign Y                             %03d\n", _arkState->ball[0].vSign.vy);
    jaffarCommon::logger::log("[J+]    + Ball 1 VSign Y                             %03d\n", _arkState->ball[1].vSign.vy);
    jaffarCommon::logger::log("[J+]    + Ball 2 VSign Y                             %03d\n", _arkState->ball[2].vSign.vy);
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

  // QuickerArkBot pointer
  jaffarPlus::emulator::QuickerArkBot* _quickerArkBot;

  // Game state
  GameState* _arkState;

  // Are any of the balls in range=
  bool _ballInRange0;
  bool _ballInRange1;
  bool _ballInRange2;

  __INLINE__ bool enemyInRange(const Enemy& enemy) const { return enemy.active && GameConsts::PaddleTop + 4 >= enemy.pos.y && enemy.pos.y + 0xe >= GameConsts::PaddleTop; }

  // State evaluation operations
  __INLINE__ bool ballInRange(const Ball& ball) const
  {
    return (ball.pos.y + 7 >= GameConsts::PaddleTop && GameConsts::PaddleTop + 3 >= ball.pos.y && ball.vSign.vy == 1 && !ball._paddleCollis);
  };

  __INLINE__ bool powerUpInRange() const
  {
    return _arkState->spawnedPowerup == Powerup::Multiball && _arkState->powerupPos.y + 8 >= GameConsts::PaddleTop && GameConsts::PaddleTop + 4 >= _arkState->powerupPos.y;
  }

public:
  // Hits remaining counter
  uint16_t _ballHitsRemaining = 0;

  // Lookup table from paddle position to input index
  InputSet::inputIndex_t paddlePositionIndexLUT[161];

  // Lookup table for target position and its corresponding input
  const char* paddlePositionStringLUT[161] = {
      "|..|    0,.|", "|..|    1,.|", "|..|    2,.|", "|..|    3,.|", "|..|    4,.|", "|..|    5,.|", "|..|    6,.|", "|..|    7,.|", "|..|    8,.|", "|..|    9,.|",
      "|..|   10,.|", "|..|   11,.|", "|..|   12,.|", "|..|   13,.|", "|..|   14,.|", "|..|   15,.|", "|..|   16,.|", "|..|   17,.|", "|..|   18,.|", "|..|   19,.|",
      "|..|   20,.|", "|..|   21,.|", "|..|   22,.|", "|..|   23,.|", "|..|   24,.|", "|..|   25,.|", "|..|   26,.|", "|..|   27,.|", "|..|   28,.|", "|..|   29,.|",
      "|..|   30,.|", "|..|   31,.|", "|..|   32,.|", "|..|   33,.|", "|..|   34,.|", "|..|   35,.|", "|..|   36,.|", "|..|   37,.|", "|..|   38,.|", "|..|   39,.|",
      "|..|   40,.|", "|..|   41,.|", "|..|   42,.|", "|..|   43,.|", "|..|   44,.|", "|..|   45,.|", "|..|   46,.|", "|..|   47,.|", "|..|   48,.|", "|..|   49,.|",
      "|..|   50,.|", "|..|   51,.|", "|..|   52,.|", "|..|   53,.|", "|..|   54,.|", "|..|   55,.|", "|..|   56,.|", "|..|   57,.|", "|..|   58,.|", "|..|   59,.|",
      "|..|   60,.|", "|..|   61,.|", "|..|   62,.|", "|..|   63,.|", "|..|   64,.|", "|..|   65,.|", "|..|   66,.|", "|..|   67,.|", "|..|   68,.|", "|..|   69,.|",
      "|..|   70,.|", "|..|   71,.|", "|..|   72,.|", "|..|   73,.|", "|..|   74,.|", "|..|   75,.|", "|..|   76,.|", "|..|   77,.|", "|..|   78,.|", "|..|   79,.|",
      "|..|   80,.|", "|..|   81,.|", "|..|   82,.|", "|..|   83,.|", "|..|   84,.|", "|..|   85,.|", "|..|   86,.|", "|..|   87,.|", "|..|   88,.|", "|..|   89,.|",
      "|..|   90,.|", "|..|   91,.|", "|..|   92,.|", "|..|   93,.|", "|..|   94,.|", "|..|   95,.|", "|..|   96,.|", "|..|   97,.|", "|..|   98,.|", "|..|   99,.|",
      "|..|  100,.|", "|..|  101,.|", "|..|  102,.|", "|..|  103,.|", "|..|  104,.|", "|..|  105,.|", "|..|  106,.|", "|..|  107,.|", "|..|  108,.|", "|..|  109,.|",
      "|..|  110,.|", "|..|  111,.|", "|..|  112,.|", "|..|  113,.|", "|..|  114,.|", "|..|  115,.|", "|..|  116,.|", "|..|  117,.|", "|..|  118,.|", "|..|  119,.|",
      "|..|  120,.|", "|..|  121,.|", "|..|  122,.|", "|..|  123,.|", "|..|  124,.|", "|..|  125,.|", "|..|  126,.|", "|..|  127,.|", "|..|  128,.|", "|..|  129,.|",
      "|..|  130,.|", "|..|  131,.|", "|..|  132,.|", "|..|  133,.|", "|..|  134,.|", "|..|  135,.|", "|..|  136,.|", "|..|  137,.|", "|..|  138,.|", "|..|  139,.|",
      "|..|  140,.|", "|..|  141,.|", "|..|  142,.|", "|..|  143,.|", "|..|  144,.|", "|..|  145,.|", "|..|  146,.|", "|..|  147,.|", "|..|  148,.|", "|..|  149,.|",
      "|..|  150,.|", "|..|  151,.|", "|..|  152,.|", "|..|  153,.|", "|..|  154,.|", "|..|  155,.|", "|..|  156,.|", "|..|  157,.|", "|..|  158,.|", "|..|  159,.|",
      "|..|  160,.|"};
};

} // namespace arkbot

} // namespace games

} // namespace jaffarPlus