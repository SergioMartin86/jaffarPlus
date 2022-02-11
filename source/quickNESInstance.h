#pragma once

#include <utils.h>
#include <Nes_Emu.h>
#include <Nes_State.h>
#include <blargg_errors.h>
#include <string>
#include "metrohash64.h"

class quickNESInstance
{
  public:

  // Initializes the miniPop instance
  quickNESInstance(const std::string& romFilePath);

  // Loading/Saving state files
  void loadStateFile(const std::string& stateFilePath);
  void saveStateFile(const std::string& stateFilePath);

  // Advance a frame
  void advanceFrame(const uint8_t &move);

  // Print information about the current frame
  void printFrameInfo();

  // Function to update values that are derived from raw game data
  void updateDerivedValues();

  // Minimal serialization functions
  void serializeState(uint8_t* state) const;
  void deserializeState(const uint8_t* state);
  void serializeStateSmall(uint8_t* state) const;
  void deserializeStateSmall(const uint8_t* state);

  // This function computes the hash for the current state
  inline uint64_t computeHash() const
  {
    // Storage for hash calculation
    MetroHash64 hash;

    // Adding fixed hash elements
    hash.Update(*_screenScroll);
    hash.Update(*_marioAnimation);
    hash.Update(*_marioState);

    hash.Update(*_marioBasePosX);
    hash.Update(*_marioRelPosX);
    hash.Update(*_marioSubpixelPosX);

    hash.Update(*_marioPosY);
    hash.Update(*_marioSubpixelPosY);

    hash.Update(*_marioXMoveForce);
    hash.Update(*_marioFacingDirection);
    hash.Update(*_marioMovingDirection);
    hash.Update(*_marioFloatingMode);
    hash.Update(*_marioWalkingMode);
    hash.Update(*_marioWalkingDelay);
    hash.Update(*_marioWalkingFrame);
    hash.Update(*_marioMaxVelLeft);
    hash.Update(*_marioMaxVelRight);
    hash.Update(*_marioVelX);
    hash.Update(*_marioVelY);
    hash.Update(*_marioFracVelY);
    hash.Update(*_marioGravity);
    hash.Update(*_marioFriction);

    hash.Update(*_screenBasePosX);
    hash.Update(*_screenRelPosX);

    hash.Update(*_currentWorldRaw);
    hash.Update(*_currentStageRaw);
    hash.Update(*_levelEntryFlag);
    hash.Update(*_gameMode);

    hash.Update(*_enemy1Active);
    hash.Update(*_enemy2Active);
    hash.Update(*_enemy3Active);
    hash.Update(*_enemy4Active);
    hash.Update(*_enemy5Active);

    hash.Update(*_enemy1State);
    hash.Update(*_enemy2State);
    hash.Update(*_enemy3State);
    hash.Update(*_enemy4State);
    hash.Update(*_enemy5State);

    hash.Update(*_enemy1Type);
    hash.Update(*_enemy2Type);
    hash.Update(*_enemy3Type);
    hash.Update(*_enemy4Type);
    hash.Update(*_enemy5Type);

    hash.Update(*_marioCollision);
    hash.Update(*_enemyCollision);
    hash.Update(*_hitDetectionFlag);

    // To Reduce timer pressure on hash, have 0, 1, and >1 as possibilities only
    hash.Update(*_animationTimer < 2 ? *_animationTimer : (uint8_t)2);
    hash.Update(*_jumpSwimTimer < 2 ? *_jumpSwimTimer : (uint8_t)2);
    hash.Update(*_runningTimer < 2 ? *_runningTimer : (uint8_t)2);
    hash.Update(*_blockBounceTimer < 2 ? *_blockBounceTimer : (uint8_t)2);
    hash.Update(*_sideCollisionTimer);
    hash.Update(*_jumpspringTimer);
    hash.Update(*_gameControlTimer);
    hash.Update(*_climbSideTimer);
    hash.Update(*_enemyFrameTimer);
    hash.Update(*_frenzyEnemyTimer);
    hash.Update(*_bowserFireTimer);
    hash.Update(*_stompTimer);
    hash.Update(*_airBubbleTimer);
    hash.Update(*_fallPitTimer);
    hash.Update(*_multiCoinBlockTimer);
    hash.Update(*_invincibleTimer);
    hash.Update(*_starTimer);
    hash.Update(*_powerUpActive);


    hash.Update(*_warpAreaOffset);

//    hash.Update(*_player1Input);
//    hash.Update(*_player1Buttons);
//    hash.Update(*_player1GamePad1);
//    hash.Update(*_player1GamePad2);

    uint64_t result;
    hash.Finalize(reinterpret_cast<uint8_t *>(&result));
    return result;
  }

  // Emulator instance
  Nes_Emu _emu;

  // Base low-memory pointer
  uint8_t* _baseMem;

  // Game specific values
  uint16_t* _screenScroll; inline uint16_t getScreenScroll() { return swap_endian<uint16_t>(*_screenScroll); };
  uint8_t* _marioAnimation;
  uint8_t* _marioState;

  uint8_t* _marioBasePosX;
  uint8_t* _marioRelPosX;
  uint8_t* _marioSubpixelPosX;

  uint8_t* _marioPosY;
  uint8_t* _marioSubpixelPosY;

  uint8_t* _marioMovingDirection;
  uint8_t* _marioFacingDirection;
  uint8_t* _marioFloatingMode;
  uint8_t* _marioWalkingMode;
  uint8_t* _marioWalkingDelay;
  uint8_t* _marioWalkingFrame;
  int8_t* _marioMaxVelLeft;
  int8_t* _marioMaxVelRight;
  int8_t* _marioVelX;
  int8_t* _marioXMoveForce;
  int8_t* _marioVelY;
  int8_t* _marioFracVelY;
  uint8_t* _marioGravity;
  uint8_t* _marioFriction;
  uint8_t* _timeLeft100;
  uint8_t* _timeLeft10;
  uint8_t* _timeLeft1;

  uint8_t* _screenBasePosX;
  uint8_t* _screenRelPosX;

  uint8_t* _currentWorldRaw;
  uint8_t* _currentStageRaw;
  uint8_t* _levelEntryFlag;
  uint8_t* _gameMode;

  uint8_t* _enemy1Active;
  uint8_t* _enemy2Active;
  uint8_t* _enemy3Active;
  uint8_t* _enemy4Active;
  uint8_t* _enemy5Active;

  uint8_t* _enemy1State;
  uint8_t* _enemy2State;
  uint8_t* _enemy3State;
  uint8_t* _enemy4State;
  uint8_t* _enemy5State;

  uint8_t* _enemy1Type;
  uint8_t* _enemy2Type;
  uint8_t* _enemy3Type;
  uint8_t* _enemy4Type;
  uint8_t* _enemy5Type;

  uint8_t* _marioCollision;
  uint8_t* _enemyCollision;
  uint8_t* _hitDetectionFlag;

  uint8_t* _powerUpActive;

  uint8_t* _animationTimer;
  uint8_t* _jumpSwimTimer;
  uint8_t* _runningTimer;
  uint8_t* _blockBounceTimer;
  uint8_t* _sideCollisionTimer;
  uint8_t* _jumpspringTimer;
  uint8_t* _climbSideTimer;
  uint8_t* _gameControlTimer;
  uint8_t* _enemyFrameTimer;
  uint8_t* _frenzyEnemyTimer;
  uint8_t* _bowserFireTimer;
  uint8_t* _stompTimer;
  uint8_t* _airBubbleTimer;
  uint8_t* _fallPitTimer;
  uint8_t* _multiCoinBlockTimer;
  uint8_t* _invincibleTimer;
  uint8_t* _starTimer;

  uint8_t* _player1Input;
  uint8_t* _player1Buttons;
  uint8_t* _player1GamePad1;
  uint8_t* _player1GamePad2;

  uint16_t* _warpAreaOffset;

  // Derivative values
  uint16_t _marioPosX;
  uint16_t _screenPosX;
  int16_t _marioScreenOffset;
  uint8_t _currentWorld;
  uint8_t _currentStage;
};
