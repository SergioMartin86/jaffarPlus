#include "gameRule.hpp"

GameRule::GameRule() : Rule()
{
 _magnets.simonHorizontalMagnet = genericMagnet_t { .intensity = 0.0f, .center = 0.0f, .min = 0.0f, .max = 0.0f };
 _magnets.simonVerticalMagnet = genericMagnet_t { .intensity = 0.0f, .center = 0.0f, .min = 0.0f, .max = 0.0f };
 _magnets.bossHorizontalMagnet = genericMagnet_t { .intensity = 0.0f, .center = 0.0f, .min = 0.0f, .max = 0.0f };
 _magnets.bossVerticalMagnet = genericMagnet_t { .intensity = 0.0f, .center = 0.0f, .min = 0.0f, .max = 0.0f };
 _magnets.batMedusaHorizontalMagnet = genericMagnet_t { .intensity = 0.0f, .center = 0.0f, .min = 0.0f, .max = 0.0f };
 _magnets.batMedusaVerticalMagnet = genericMagnet_t { .intensity = 0.0f, .center = 0.0f, .min = 0.0f, .max = 0.0f };
 _magnets.skeletonHorizontalMagnet = genericMagnet_t { .intensity = 0.0f, .center = 0.0f, .min = 0.0f, .max = 0.0f };
 _magnets.simonStairMagnet = stairMagnet_t { .reward = 0.0f, .mode = 0};
 _magnets.simonWeaponMagnet = weaponMagnet_t { .reward = 0.0f, .weapon = 0};
 _magnets.simonHeartMagnet = 0.0f;
 _magnets.bossHealthMagnet = 0.0f;
 _magnets.bossStateTimerMagnet = 0.0f;
 _magnets.freezeTimeMagnet = 0.0f;
 _magnets.scrollTileMagnets.clear();
}

bool GameRule::parseGameAction(nlohmann::json actionJs, size_t actionId)
{
  bool recognizedActionType = false;
  std::string actionType = actionJs["Type"].get<std::string>();

  if (actionType == "Set Simon Horizontal Magnet")
  {
   if (isDefined(actionJs, "Intensity") == false) EXIT_WITH_ERROR("[ERROR] Magnet in Rule %lu Action %lu missing 'Intensity' key.\n", _label, actionId);
   if (isDefined(actionJs, "Center") == false) EXIT_WITH_ERROR("[ERROR] Magnet in Rule %lu Action %lu missing 'Center' key.\n", _label, actionId);
   if (isDefined(actionJs, "Min") == false) EXIT_WITH_ERROR("[ERROR] Magnet in Rule %lu Action %lu missing 'Min' key.\n", _label, actionId);
   if (isDefined(actionJs, "Max") == false) EXIT_WITH_ERROR("[ERROR] Magnet in Rule %lu Action %lu missing 'Max' key.\n", _label, actionId);
   _magnets.simonHorizontalMagnet = genericMagnet_t { .intensity = actionJs["Intensity"].get<float>(), .center= actionJs["Center"].get<float>(), .min = actionJs["Min"].get<float>(), .max = actionJs["Max"].get<float>() };
    recognizedActionType = true;
  }

  if (actionType == "Set Simon Vertical Magnet")
  {
   if (isDefined(actionJs, "Intensity") == false) EXIT_WITH_ERROR("[ERROR] Magnet in Rule %lu Action %lu missing 'Intensity' key.\n", _label, actionId);
   if (isDefined(actionJs, "Min") == false) EXIT_WITH_ERROR("[ERROR] Magnet in Rule %lu Action %lu missing 'Min' key.\n", _label, actionId);
   if (isDefined(actionJs, "Max") == false) EXIT_WITH_ERROR("[ERROR] Magnet in Rule %lu Action %lu missing 'Max' key.\n", _label, actionId);
   _magnets.simonVerticalMagnet = genericMagnet_t { .intensity = actionJs["Intensity"].get<float>(), .center= actionJs["Center"].get<float>(), .min = actionJs["Min"].get<float>(), .max = actionJs["Max"].get<float>() };
   recognizedActionType = true;
  }

  if (actionType == "Set Boss Horizontal Magnet")
  {
   if (isDefined(actionJs, "Intensity") == false) EXIT_WITH_ERROR("[ERROR] Magnet in Rule %lu Action %lu missing 'Intensity' key.\n", _label, actionId);
   if (isDefined(actionJs, "Min") == false) EXIT_WITH_ERROR("[ERROR] Magnet in Rule %lu Action %lu missing 'Min' key.\n", _label, actionId);
   if (isDefined(actionJs, "Max") == false) EXIT_WITH_ERROR("[ERROR] Magnet in Rule %lu Action %lu missing 'Max' key.\n", _label, actionId);
   _magnets.bossHorizontalMagnet = genericMagnet_t { .intensity = actionJs["Intensity"].get<float>(), .center= actionJs["Center"].get<float>(), .min = actionJs["Min"].get<float>(), .max = actionJs["Max"].get<float>() };
    recognizedActionType = true;
  }

  if (actionType == "Set Boss Vertical Magnet")
  {
   if (isDefined(actionJs, "Intensity") == false) EXIT_WITH_ERROR("[ERROR] Magnet in Rule %lu Action %lu missing 'Intensity' key.\n", _label, actionId);
   if (isDefined(actionJs, "Min") == false) EXIT_WITH_ERROR("[ERROR] Magnet in Rule %lu Action %lu missing 'Min' key.\n", _label, actionId);
   if (isDefined(actionJs, "Max") == false) EXIT_WITH_ERROR("[ERROR] Magnet in Rule %lu Action %lu missing 'Max' key.\n", _label, actionId);
   _magnets.bossVerticalMagnet = genericMagnet_t { .intensity = actionJs["Intensity"].get<float>(), .center= actionJs["Center"].get<float>(), .min = actionJs["Min"].get<float>(), .max = actionJs["Max"].get<float>() };
   recognizedActionType = true;
  }


  if (actionType == "Set Bat / Medusa Horizontal Magnet")
  {
   if (isDefined(actionJs, "Intensity") == false) EXIT_WITH_ERROR("[ERROR] Magnet in Rule %lu Action %lu missing 'Intensity' key.\n", _label, actionId);
   if (isDefined(actionJs, "Min") == false) EXIT_WITH_ERROR("[ERROR] Magnet in Rule %lu Action %lu missing 'Min' key.\n", _label, actionId);
   if (isDefined(actionJs, "Max") == false) EXIT_WITH_ERROR("[ERROR] Magnet in Rule %lu Action %lu missing 'Max' key.\n", _label, actionId);
   _magnets.batMedusaHorizontalMagnet = genericMagnet_t { .intensity = actionJs["Intensity"].get<float>(), .center= actionJs["Center"].get<float>(), .min = actionJs["Min"].get<float>(), .max = actionJs["Max"].get<float>() };
   recognizedActionType = true;
  }

  if (actionType == "Set Bat / Medusa Vertical Magnet")
  {
   if (isDefined(actionJs, "Intensity") == false) EXIT_WITH_ERROR("[ERROR] Magnet in Rule %lu Action %lu missing 'Intensity' key.\n", _label, actionId);
   if (isDefined(actionJs, "Min") == false) EXIT_WITH_ERROR("[ERROR] Magnet in Rule %lu Action %lu missing 'Min' key.\n", _label, actionId);
   if (isDefined(actionJs, "Max") == false) EXIT_WITH_ERROR("[ERROR] Magnet in Rule %lu Action %lu missing 'Max' key.\n", _label, actionId);
   _magnets.batMedusaVerticalMagnet = genericMagnet_t { .intensity = actionJs["Intensity"].get<float>(), .center= actionJs["Center"].get<float>(), .min = actionJs["Min"].get<float>(), .max = actionJs["Max"].get<float>() };
   recognizedActionType = true;
  }

  if (actionType == "Set Skeleton Horizontal Magnet")
  {
   if (isDefined(actionJs, "Intensity") == false) EXIT_WITH_ERROR("[ERROR] Magnet in Rule %lu Action %lu missing 'Intensity' key.\n", _label, actionId);
   if (isDefined(actionJs, "Min") == false) EXIT_WITH_ERROR("[ERROR] Magnet in Rule %lu Action %lu missing 'Min' key.\n", _label, actionId);
   if (isDefined(actionJs, "Max") == false) EXIT_WITH_ERROR("[ERROR] Magnet in Rule %lu Action %lu missing 'Max' key.\n", _label, actionId);
   _magnets.skeletonHorizontalMagnet = genericMagnet_t { .intensity = actionJs["Intensity"].get<float>(), .center= actionJs["Center"].get<float>(), .min = actionJs["Min"].get<float>(), .max = actionJs["Max"].get<float>() };
    recognizedActionType = true;
  }

  if (actionType == "Set Skeleton 2 Horizontal Magnet")
  {
   if (isDefined(actionJs, "Intensity") == false) EXIT_WITH_ERROR("[ERROR] Magnet in Rule %lu Action %lu missing 'Intensity' key.\n", _label, actionId);
   if (isDefined(actionJs, "Min") == false) EXIT_WITH_ERROR("[ERROR] Magnet in Rule %lu Action %lu missing 'Min' key.\n", _label, actionId);
   if (isDefined(actionJs, "Max") == false) EXIT_WITH_ERROR("[ERROR] Magnet in Rule %lu Action %lu missing 'Max' key.\n", _label, actionId);
   _magnets.skeleton2HorizontalMagnet = genericMagnet_t { .intensity = actionJs["Intensity"].get<float>(), .center= actionJs["Center"].get<float>(), .min = actionJs["Min"].get<float>(), .max = actionJs["Max"].get<float>() };
    recognizedActionType = true;
  }

  if (actionType == "Set Simon Stair Mode Magnet")
  {
   if (isDefined(actionJs, "Reward") == false) EXIT_WITH_ERROR("[ERROR] Stairs Magnet in Rule %lu Action %lu missing 'Reward' key.\n", _label, actionId);
   if (isDefined(actionJs, "Mode") == false) EXIT_WITH_ERROR("[ERROR] Stairs Magnet in Rule %lu Action %lu missing 'Mode' key.\n", _label, actionId);
   _magnets.simonStairMagnet = stairMagnet_t { .reward = actionJs["Reward"].get<float>(), .mode = actionJs["Mode"].get<uint8_t>()};
   recognizedActionType = true;
  }

  if (actionType == "Set Simon Weapon Magnet")
  {
   if (isDefined(actionJs, "Reward") == false) EXIT_WITH_ERROR("[ERROR] Weapon Magnet in Rule %lu Action %lu missing 'Reward' key.\n", _label, actionId);
   if (isDefined(actionJs, "Weapon") == false) EXIT_WITH_ERROR("[ERROR] Weapon Magnet in Rule %lu Action %lu missing 'Weapon' key.\n", _label, actionId);
   _magnets.simonWeaponMagnet = weaponMagnet_t { .reward = actionJs["Reward"].get<float>(), .weapon = actionJs["Weapon"].get<uint8_t>()};
   recognizedActionType = true;
  }

  if (actionType == "Set Scroll Tile Magnets")
  {
   for (const auto& tileMagnet : actionJs["Magnets"])
   {
    if (isDefined(tileMagnet, "Position") == false) EXIT_WITH_ERROR("[ERROR] Scroll Tile in Rule %lu Action %lu missing 'Position' key.\n", _label, actionId);
    if (isDefined(tileMagnet, "Reward") == false) EXIT_WITH_ERROR("[ERROR] Scroll Tile in Rule %lu Action %lu missing 'Reward' key.\n", _label, actionId);
    if (isDefined(tileMagnet, "Value") == false) EXIT_WITH_ERROR("[ERROR] Scroll Tile in Rule %lu Action %lu missing 'Value' key.\n", _label, actionId);
    _magnets.scrollTileMagnets.push_back(nametableTileMagnet_t { .reward = tileMagnet["Reward"].get<float>(), .pos = (uint16_t)std::stoul(tileMagnet["Position"].get<std::string>(), 0, 16), .value = tileMagnet["Value"].get<uint8_t>()});
    recognizedActionType = true;
   }
  }

  if (actionType == "Set Simon Heart Magnet")
  {
   if (isDefined(actionJs, "Intensity") == false) EXIT_WITH_ERROR("[ERROR] Simon Heart Magnet in Rule %lu Action %lu missing 'Intensity' key.\n", _label, actionId);
   _magnets.simonHeartMagnet = actionJs["Intensity"].get<float>();
   recognizedActionType = true;
  }

  if (actionType == "Set Boss/Simon Distance Magnet")
  {
   if (isDefined(actionJs, "Intensity") == false) EXIT_WITH_ERROR("[ERROR] Boss/Simon Distance Magnet in Rule %lu Action %lu missing 'Intensity' key.\n", _label, actionId);
   _magnets.bossSimonDistanceMagnet = actionJs["Intensity"].get<float>();
   recognizedActionType = true;
  }

  if (actionType == "Set Mummies Distance Magnet")
  {
   if (isDefined(actionJs, "Intensity") == false) EXIT_WITH_ERROR("[ERROR] Boss/Simon Distance Magnet in Rule %lu Action %lu missing 'Intensity' key.\n", _label, actionId);
   _magnets.mummiesDistanceMagnet = actionJs["Intensity"].get<float>();
   recognizedActionType = true;
  }

  if (actionType == "Set Boss/Weapon Distance Magnet")
  {
   if (isDefined(actionJs, "Intensity") == false) EXIT_WITH_ERROR("[ERROR] Boss/Weapon Distance Magnet in Rule %lu Action %lu missing 'Intensity' key.\n", _label, actionId);
   _magnets.bossWeaponDistanceMagnet = actionJs["Intensity"].get<float>();
   recognizedActionType = true;
  }

  if (actionType == "Set Subweapon Hit Count Magnet")
  {
   if (isDefined(actionJs, "Intensity") == false) EXIT_WITH_ERROR("[ERROR] Magnet in Rule %lu Action %lu missing 'Intensity' key.\n", _label, actionId);
   if (isDefined(actionJs, "Min") == false) EXIT_WITH_ERROR("[ERROR] Magnet in Rule %lu Action %lu missing 'Min' key.\n", _label, actionId);
   if (isDefined(actionJs, "Max") == false) EXIT_WITH_ERROR("[ERROR] Magnet in Rule %lu Action %lu missing 'Max' key.\n", _label, actionId);
   _magnets.subweaponHitCountMagnet = genericMagnet_t { .intensity = actionJs["Intensity"].get<float>(), .center= actionJs["Center"].get<float>(), .min = actionJs["Min"].get<float>(), .max = actionJs["Max"].get<float>() };
   recognizedActionType = true;
  }

  if (actionType == "Set Subweapon 1 Active Magnet")
  {
   if (isDefined(actionJs, "Intensity") == false) EXIT_WITH_ERROR("[ERROR] Magnet in Rule %lu Action %lu missing 'Intensity' key.\n", _label, actionId);
   _magnets.subweapon1ActiveMagnet = actionJs["Intensity"].get<float>();
   recognizedActionType = true;
  }

  if (actionType == "Set Subweapon 2 Active Magnet")
  {
   if (isDefined(actionJs, "Intensity") == false) EXIT_WITH_ERROR("[ERROR] Magnet in Rule %lu Action %lu missing 'Intensity' key.\n", _label, actionId);
   _magnets.subweapon2ActiveMagnet = actionJs["Intensity"].get<float>();
   recognizedActionType = true;
  }

  if (actionType == "Set Subweapon 3 Active Magnet")
  {
   if (isDefined(actionJs, "Intensity") == false) EXIT_WITH_ERROR("[ERROR] Magnet in Rule %lu Action %lu missing 'Intensity' key.\n", _label, actionId);
   _magnets.subweapon3ActiveMagnet = actionJs["Intensity"].get<float>();
   recognizedActionType = true;
  }

  if (actionType == "Set Freeze Timer Magnet")
  {
   if (isDefined(actionJs, "Intensity") == false) EXIT_WITH_ERROR("[ERROR] Freeze Timer Magnet in Rule %lu Action %lu missing 'Intensity' key.\n", _label, actionId);
   _magnets.freezeTimeMagnet = actionJs["Intensity"].get<float>();
   recognizedActionType = true;
  }

  if (actionType == "Set Boss State Timer Magnet")
  {
   if (isDefined(actionJs, "Intensity") == false) EXIT_WITH_ERROR("[ERROR] Boss State Timer Magnet in Rule %lu Action %lu missing 'Intensity' key.\n", _label, actionId);
   _magnets.bossStateTimerMagnet = actionJs["Intensity"].get<float>();
   recognizedActionType = true;
  }

  if (actionType == "Set Boss Health Magnet")
  {
   if (isDefined(actionJs, "Intensity") == false) EXIT_WITH_ERROR("[ERROR] Boss Health Magnet in Rule %lu Action %lu missing 'Intensity' key.\n", _label, actionId);
   _magnets.bossHealthMagnet = actionJs["Intensity"].get<float>();
   recognizedActionType = true;
  }


  return recognizedActionType;
}

datatype_t GameRule::getPropertyType(const nlohmann::json& condition)
{
  std::string propertyName = condition["Property"].get<std::string>();

  if (propertyName == "Game Mode") return dt_uint8;
  if (propertyName == "Game SubMode") return dt_uint8;
  if (propertyName == "Current Stage") return dt_uint8;
  if (propertyName == "Current SubStage") return dt_uint8;
  if (propertyName == "Simon Lives") return dt_uint8;
  if (propertyName == "Is Lag Frame") return dt_uint8;
  if (propertyName == "Stage Timer") return dt_uint8;
  if (propertyName == "Screen Offset") return dt_uint16;
  if (propertyName == "Simon Stair Mode") return dt_uint8;
  if (propertyName == "Simon Position Y") return dt_uint8;
  if (propertyName == "Simon Position X") return dt_uint16;
  if (propertyName == "Simon Relative Position X") return dt_uint8;
  if (propertyName == "Simon Health") return dt_uint8;
  if (propertyName == "Simon Invulnerability") return dt_uint8;
  if (propertyName == "Simon Kneeling Mode") return dt_uint8;
  if (propertyName == "Subweapon Shot Count") return dt_uint8;
  if (propertyName == "Subweapon 1 Position Y") return dt_uint8;
  if (propertyName == "Subweapon 1 Position X") return dt_uint8;
  if (propertyName == "Subweapon 2 Position X") return dt_uint8;
  if (propertyName == "Subweapon 3 Position X") return dt_uint8;
  if (propertyName == "Whip Length") return dt_uint8;
  if (propertyName == "Simon Heart Count") return dt_uint8;
  if (propertyName == "Simon Image") return dt_uint8;
  if (propertyName == "Subweapon Number") return dt_uint8;
  if (propertyName == "Subweapon Hit Count") return dt_uint8;
  if (propertyName == "Simon Facing Direction") return dt_uint8;
  if (propertyName == "Simon State") return dt_uint8;
  if (propertyName == "Simon SubState") return dt_uint8;
  if (propertyName == "Simon Vertical Speed") return dt_uint8;
  if (propertyName == "Simon Vertical Direction") return dt_uint8;
  if (propertyName == "Boss Health") return dt_uint8;
  if (propertyName == "Boss Is Active") return dt_uint8;
  if (propertyName == "Boss State") return dt_uint8;
  if (propertyName == "Boss Position X") return dt_uint8;
  if (propertyName == "Boss Position Y") return dt_uint8;
  if (propertyName == "Boss State Timer") return dt_uint8;
  if (propertyName == "Freeze Time Timer") return dt_uint8;
  if (propertyName == "Enemy 0 State") return dt_uint8;
  if (propertyName == "Enemy 1 State") return dt_uint8;
  if (propertyName == "Enemy 2 State") return dt_uint8;
  if (propertyName == "Enemy 3 State") return dt_uint8;
  if (propertyName == "Enemy 4 State") return dt_uint8;
  if (propertyName == "Enemy 5 State") return dt_uint8;
  if (propertyName == "Enemy 6 State") return dt_uint8;
  if (propertyName == "Enemy 7 State") return dt_uint8;
  if (propertyName == "Bat / Medusa 1 State") return dt_uint8;
  if (propertyName == "Bat / Medusa 2 State") return dt_uint8;
  if (propertyName == "Bat / Medusa 3 State") return dt_uint8;
  if (propertyName == "Bat / Medusa 4 State") return dt_uint8;
  if (propertyName == "Bat / Medusa 5 State") return dt_uint8;
  if (propertyName == "Bat / Medusa 1 Position Y") return dt_uint8;
  if (propertyName == "Bat / Medusa 1 Position X") return dt_uint8;
  if (propertyName == "Enemy 1 Holy Water Lock State") return dt_uint8;
  if (propertyName == "Enemy 1 Holy Water Lock State") return dt_uint8;
  if (propertyName == "Skeleton Pos X") return dt_uint8;
  if (propertyName == "Skeleton Pos X2") return dt_uint8;
  if (propertyName == "Mummies Distance") return dt_uint8;
  if (propertyName == "Tile State") return dt_uint8;
  if (propertyName == "Candelabrum State") return dt_uint8;
  if (propertyName == "Is Candelabrum Broken") return dt_uint8;
  if (propertyName == "Bat Shot 1 Pos X") return dt_uint8;
  if (propertyName == "Bat Shot 1 Pos Y") return dt_uint8;

  EXIT_WITH_ERROR("[Error] Rule %lu, unrecognized property: %s\n", _label, propertyName.c_str());

  return dt_uint8;
}

void* GameRule::getPropertyPointer(const nlohmann::json& condition, GameInstance* gameInstance)
{
  std::string propertyName = condition["Property"].get<std::string>();

  if (propertyName == "Game Mode") return gameInstance->gameMode;
  if (propertyName == "Game SubMode") return gameInstance->gameSubMode;
  if (propertyName == "Current Stage") return gameInstance->currentStage;
  if (propertyName == "Current SubStage") return gameInstance->currentSubStage;
  if (propertyName == "Is Lag Frame") return gameInstance->isLagFrame;
  if (propertyName == "Stage Timer") return gameInstance->stageTimer;
  if (propertyName == "Simon Lives") return gameInstance->simonLives;
  if (propertyName == "Screen Offset") return gameInstance->screenOffset;
  if (propertyName == "Simon Stair Mode") return gameInstance->simonStairMode;
  if (propertyName == "Simon Position Y") return gameInstance->simonPosY;
  if (propertyName == "Simon Position X") return gameInstance->simonPosX;
  if (propertyName == "Simon Relative Position X") return &gameInstance->simonRelativePosX;
  if (propertyName == "Simon Health") return gameInstance->simonHealth;
  if (propertyName == "Simon Invulnerability") return gameInstance->simonInvulnerability;
  if (propertyName == "Simon Kneeling Mode") return gameInstance->simonKneelingMode;
  if (propertyName == "Subweapon Shot Count") return gameInstance->subweaponShotCount;
  if (propertyName == "Whip Length") return gameInstance->whipLength;
  if (propertyName == "Simon Heart Count") return gameInstance->simonHeartCount;
  if (propertyName == "Simon Image") return gameInstance->simonImage;
  if (propertyName == "Subweapon Number") return gameInstance->subweaponNumber;
  if (propertyName == "Subweapon Hit Count") return gameInstance->subweaponHitCount;
  if (propertyName == "Subweapon 1 Position Y") return gameInstance->subweapon1PosY;
  if (propertyName == "Subweapon 1 Position X") return gameInstance->subweapon1PosX;
  if (propertyName == "Subweapon 2 Position X") return gameInstance->subweapon2PosX;
  if (propertyName == "Subweapon 3 Position X") return gameInstance->subweapon3PosX;
  if (propertyName == "Simon Facing Direction") return gameInstance->simonFacingDirection;
  if (propertyName == "Simon State") return gameInstance->simonState;
  if (propertyName == "Simon Vertical Speed") return gameInstance->simonVerticalSpeed;
  if (propertyName == "Simon Vertical Direction") return gameInstance->simonVerticalDirection;
  if (propertyName == "Boss Health") return gameInstance->bossHealth;
  if (propertyName == "Boss Is Active") return gameInstance->bossIsActive;
  if (propertyName == "Boss State") return gameInstance->bossState;
  if (propertyName == "Enemy 0 State") return gameInstance->enemy0State;
  if (propertyName == "Enemy 1 State") return gameInstance->enemy1State;
  if (propertyName == "Enemy 2 State") return gameInstance->enemy2State;
  if (propertyName == "Enemy 3 State") return gameInstance->enemy3State;
  if (propertyName == "Enemy 4 State") return gameInstance->enemy4State;
  if (propertyName == "Enemy 5 State") return gameInstance->enemy5State;
  if (propertyName == "Enemy 6 State") return gameInstance->enemy6State;
  if (propertyName == "Enemy 7 State") return gameInstance->enemy7State;
  if (propertyName == "Boss Position X") return gameInstance->bossPosX;
  if (propertyName == "Boss Position Y") return gameInstance->bossPosY;
  if (propertyName == "Boss State Timer") return gameInstance->bossStateTimer;
  if (propertyName == "Freeze Time Timer") return gameInstance->freezeTimeTimer;
  if (propertyName == "Bat / Medusa 1 State") return gameInstance->batMedusa1State;
  if (propertyName == "Bat / Medusa 2 State") return gameInstance->batMedusa2State;
  if (propertyName == "Bat / Medusa 3 State") return gameInstance->batMedusa3State;
  if (propertyName == "Bat / Medusa 4 State") return gameInstance->batMedusa4State;
  if (propertyName == "Bat / Medusa 5 State") return gameInstance->batMedusa5State;
  if (propertyName == "Bat / Medusa 1 Position Y") return gameInstance->batMedusa1PosY;
  if (propertyName == "Bat / Medusa 1 Position X") return gameInstance->batMedusa1PosX;
  if (propertyName == "Enemy 1 Holy Water Lock State") return gameInstance->enemy1HolyWaterLockState;
  if (propertyName == "Mummies Distance") return &gameInstance->mummiesDistance;
  if (propertyName == "Skeleton Pos X") return gameInstance->skeletonPosX;
  if (propertyName == "Skeleton Pos X2") return gameInstance->skeletonPosX2;
  if (propertyName == "Is Candelabrum Broken") return &gameInstance->isCandelabrumBroken;
  if (propertyName == "Bat Shot 1 Pos X") return &gameInstance->batShot1PosX;
  if (propertyName == "Bat Shot 1 Pos Y") return &gameInstance->batShot1PosY;

  if (propertyName == "Tile State")
  {
   if (isDefined(condition, "Position") == false) EXIT_WITH_ERROR("[ERROR] Tile State property missing 'Position' key.\n");
   return &gameInstance->_emu->_ppuNameTableMem[(uint16_t)std::stoul(condition["Position"].get<std::string>(), 0, 16)];
  }

  if (propertyName == "Candelabrum State")
  {
   if (isDefined(condition, "Position") == false) EXIT_WITH_ERROR("[ERROR] Candelabrum State property missing 'Position' key.\n");
    return &gameInstance->_emu->_baseMem[0x0191 + condition["Position"].get<size_t>()];
  }

  EXIT_WITH_ERROR("[Error] Rule %lu, unrecognized property: %s\n", _label, propertyName.c_str());

  return NULL;
}
