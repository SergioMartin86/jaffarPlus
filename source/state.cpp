#include "state.hpp"
#include "utils.hpp"

#ifdef NCURSES
 #include <ncurses.h>
 #define LOG printw
#else
 #define LOG printf
#endif

extern nlohmann::json _scriptJs;

State::State(const std::string romFile, const std::string stateFile)
{
  // Creating quickNES instance
  _emu = new EmuInstance(romFile);
  _emu->loadStateFile(stateFile);

  // Setting game-specific value pointers
  setGameValuePointers();

  // Updating initial derived values
  updateDerivedValues();
}

State::State(EmuInstance* emu)
{
  // Copying emu instance pointer
  _emu = emu;

  // Setting game-specific value pointers
  setGameValuePointers();

  // Updating initial derived values
  updateDerivedValues();
}


void State::parseRules(const nlohmann::json rulesConfig)
{
 // Processing rules
 for (size_t ruleId = 0; ruleId < rulesConfig.size(); ruleId++)
  _rules.push_back(new Rule(rulesConfig[ruleId], this));

 // Setting global rule count
 _ruleCount = _rules.size();

 if (_ruleCount > _MAX_RULE_COUNT) EXIT_WITH_ERROR("[ERROR] Configured Jaffar to run %lu rules, but the specified script contains %lu. Modify frame.h and rebuild to run this script.\n", _MAX_RULE_COUNT, _ruleCount);

 // Checking for repeated rule labels
 std::set<size_t> ruleLabelSet;
 for (size_t ruleId = 0; ruleId < _ruleCount; ruleId++)
 {
  size_t label = _rules[ruleId]->_label;
  ruleLabelSet.insert(label);
  if (ruleLabelSet.size() < ruleId + 1)
   EXIT_WITH_ERROR("[ERROR] Rule label %lu is repeated in the configuration file.\n", label);
 }

 // Looking for rule satisfied sub-rules indexes that match their labels
 for (size_t ruleId = 0; ruleId < _ruleCount; ruleId++)
  for (size_t satisfiedId = 0; satisfiedId < _rules[ruleId]->_satisfiesLabels.size(); satisfiedId++)
  {
   bool foundLabel = false;
   size_t label = _rules[ruleId]->_satisfiesLabels[satisfiedId];
   if (label == _rules[ruleId]->_label) EXIT_WITH_ERROR("[ERROR] Rule %lu references itself in satisfied vector.\n", label);

   for (size_t subRuleId = 0; subRuleId < _ruleCount; subRuleId++)
    if (_rules[subRuleId]->_label == label)
    {
     _rules[ruleId]->_satisfiesIndexes[satisfiedId] = subRuleId;
     foundLabel = true;
     break;
    }
   if (foundLabel == false) EXIT_WITH_ERROR("[ERROR] Could not find rule label %lu, specified as satisfied by rule %lu.\n", label, satisfiedId);
  }
}

void State::printRuleStatus(const bool* rulesStatus)
{
 printf("[Jaffar]  + Rule Status: ");
 for (size_t i = 0; i < _ruleCount; i++)
 {
   if (i > 0 && i % 60 == 0) printf("\n                         ");
   printf("%d", rulesStatus[i] ? 1 : 0);
 }
 printf("\n");

 // Getting magnet values
 auto magnets = getMagnetValues(rulesStatus);

 printf("[Jaffar]  + Screen Horizontal Magnet   - Intensity: %.1f, Max: %f\n", magnets.screenHorizontalMagnet.intensity, magnets.screenHorizontalMagnet.max);
 printf("[Jaffar]  + Mario Screen Offset Magnet - Intensity: %.1f, Max: %f\n", magnets.marioScreenOffsetMagnet.intensity, magnets.marioScreenOffsetMagnet.max);
 printf("[Jaffar]  + Mario Horizontal Magnet    - Intensity: %.1f, Max: %f\n", magnets.marioHorizontalMagnet.intensity, magnets.marioHorizontalMagnet.max);
 printf("[Jaffar]  + Mario Vertical Magnet      - Intensity: %.1f, Max: %f\n", magnets.marioVerticalMagnet.intensity, magnets.marioVerticalMagnet.max);
}

void State::printStateInfo() const
{
  LOG("[Jaffar]  + Current World-Stage:    %1u-%1u\n", _currentWorld, _currentStage);
  LOG("[Jaffar]  + Time Left:              %1u%1u%1u\n", *_timeLeft100, *_timeLeft10, *_timeLeft1);
  LOG("[Jaffar]  + Mario Animation:        %02u\n", *_marioAnimation);
  LOG("[Jaffar]  + Mario State:            %02u\n", *_marioState);
  LOG("[Jaffar]  + Screen Pos X:           %04u (%02u * 256 = %04u + %02u)\n", _screenPosX, *_screenBasePosX, (uint16_t)*_screenBasePosX * 255, *_screenRelPosX);
  LOG("[Jaffar]  + Mario Pos X:            %04u (%02u * 256 = %04u + %02u)\n", _marioPosX, *_marioBasePosX, (uint16_t)*_marioBasePosX * 255, *_marioRelPosX);
  LOG("[Jaffar]  + Mario / Screen Offset:  %04d\n", _marioScreenOffset);
  LOG("[Jaffar]  + Mario Pos Y:            %02u\n", *_marioPosY);
  LOG("[Jaffar]  + Mario SubPixel X/Y:     %02u / %02u\n", *_marioSubpixelPosX, *_marioSubpixelPosY);
  LOG("[Jaffar]  + Mario Vel X:            %02d (Force: %02d, MaxL: %02d, MaxR: %02d)\n", *_marioVelX, *_marioXMoveForce, *_marioMaxVelLeft, *_marioMaxVelRight);
  LOG("[Jaffar]  + Mario Vel Y:            %02d (%02d)\n", *_marioVelY, *_marioFracVelY);
  LOG("[Jaffar]  + Mario Gravity:          %02u\n", *_marioGravity);
  LOG("[Jaffar]  + Mario Friction:         %02u\n", *_marioFriction);
  LOG("[Jaffar]  + Mario Moving Direction: %s\n", *_marioMovingDirection == 1 ? "Right" : "Left");
  LOG("[Jaffar]  + Mario Facing Direction: %s\n", *_marioFacingDirection == 1 ? "Right" : "Left");
  LOG("[Jaffar]  + Mario Floating Mode:    %02u\n", *_marioFloatingMode);
  LOG("[Jaffar]  + Mario Walking:          %02u %02u %02u\n", *_marioWalkingMode, *_marioWalkingDelay, *_marioWalkingFrame);
  LOG("[Jaffar]  + Player 1 Inputs:        %02u %02u %02u %02u\n", *_player1Input, *_player1Buttons, *_player1GamePad1, *_player1GamePad2);
  LOG("[Jaffar]  + Powerup Active:         %1u\n", *_powerUpActive);
  LOG("[Jaffar]  + Enemy Active:           %1u%1u%1u%1u%1u\n", *_enemy1Active, *_enemy2Active, *_enemy3Active, *_enemy4Active, *_enemy5Active);
  LOG("[Jaffar]  + Enemy State:            %02u %02u %02u %02u %02u\n", *_enemy1State, *_enemy2State, *_enemy3State, *_enemy4State, *_enemy5State);
  LOG("[Jaffar]  + Enemy Type:             %02u %02u %02u %02u %02u\n", *_enemy1Type, *_enemy2Type, *_enemy3Type, *_enemy4Type, *_enemy5Type);
  LOG("[Jaffar]  + Hit Detection Flags:    %02u %02u %02u\n", *_marioCollision, *_enemyCollision, *_hitDetectionFlag);
  LOG("[Jaffar]  + LevelEntry / GameMode:  %02u / %02u\n", *_levelEntryFlag, *_gameMode);
  LOG("[Jaffar]  + Warp Area Offset:       %04u\n", *_warpAreaOffset);
  LOG("[Jaffar]  + Timers:                 %02u %02u %02u %02u %02u %02u %02u %02u %02u %02u %02u %02u %02u %02u %02u %02u\n", *_animationTimer, *_jumpSwimTimer, *_runningTimer, *_blockBounceTimer, *_sideCollisionTimer, *_jumpspringTimer, *_gameControlTimer, *_climbSideTimer, *_enemyFrameTimer, *_frenzyEnemyTimer, *_bowserFireTimer, *_stompTimer, *_airBubbleTimer, *_multiCoinBlockTimer, *_invincibleTimer, *_starTimer);
}
