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

class Gimmick final : public jaffarPlus::Game
{
public:
  static __INLINE__ std::string getName() { return "NES / Gimmick!"; }

  Gimmick(std::unique_ptr<Emulator> emulator, const nlohmann::json& config) : jaffarPlus::Game(std::move(emulator), config)
  {
    // Getting emulator name (for runtime use)
    _traceFilePath = jaffarCommon::json::getString(config, "Trace File Path");

    // Loading trace
    if (_traceFilePath != "")
    {
      _useTrace = true;
      std::string traceData;
      bool        status = jaffarCommon::file::loadStringFromFile(traceData, _traceFilePath.c_str());
      if (status == false) JAFFAR_THROW_LOGIC("Could not find/read trace file: %s\n", _traceFilePath.c_str());

      std::istringstream f(traceData);
      std::string line;
      while (std::getline(f, line))
      {
        auto coordinates = jaffarCommon::string::split(line, ' ');
        float playerPosX = std::atof(coordinates[0].c_str());
        float playerPosY = std::atof(coordinates[1].c_str());
        float starPosX = std::atof(coordinates[2].c_str());
        float starPosY = std::atof(coordinates[3].c_str());
        _trace.push_back(traceEntry_t{
          .playerPosX = playerPosX,
          .playerPosY = playerPosY,
          .starPosX = starPosX,
          .starPosY = starPosY
        });
      }
    }
  }

private:

  struct traceEntry_t
  {
    float playerPosX;
    float playerPosY;
    float starPosX;
    float starPosY;
  };

  __INLINE__ void registerGameProperties() override
  {
    // Getting emulator's low memory pointer
    _lowMem = _emulator->getProperty("LRAM").pointer;

    // Registering native game properties
    registerGameProperty("Screen X 1"                   , &_lowMem[0x0015], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Screen X 2"                   , &_lowMem[0x0042], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Screen X 3"                   , &_lowMem[0x0014], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Screen Y"                     , &_lowMem[0x0016], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Object Iterator"              , &_lowMem[0x0019], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Current Level"                , &_lowMem[0x001B], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Current Checkpoint"           , &_lowMem[0x001C], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Enemy Data Pointer 1"         , &_lowMem[0x0021], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Enemy Data Pointer 2"         , &_lowMem[0x0022], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("CHR Timer"                    , &_lowMem[0x0026], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("CHR Index"                    , &_lowMem[0x0027], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("IRQ Index"                    , &_lowMem[0x0028], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Screen Update"                , &_lowMem[0x0029], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Player Max HP"                , &_lowMem[0x002B], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Score Increase 1"             , &_lowMem[0x002C], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Score Increase 2"             , &_lowMem[0x002D], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Score 1"                      , &_lowMem[0x002E], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Score 2"                      , &_lowMem[0x002F], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Score 3"                      , &_lowMem[0x0030], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Score 4"                      , &_lowMem[0x0031], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Score 5"                      , &_lowMem[0x0032], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Score 6"                      , &_lowMem[0x0033], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Score 7"                      , &_lowMem[0x0034], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Enemy Kills"                  , &_lowMem[0x0035], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Item 1"                       , &_lowMem[0x0036], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Item 2"                       , &_lowMem[0x0037], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Item 3"                       , &_lowMem[0x0038], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Item Amount"                  , &_lowMem[0x0039], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("PRG Bank 1"                   , &_lowMem[0x0040], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("PRG Bank 2"                   , &_lowMem[0x0040], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Screen Locked"                , &_lowMem[0x0046], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Player Jumping"               , &_lowMem[0x004B], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Cutscene State"               , &_lowMem[0x004F], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Music Song"                   , &_lowMem[0x0052], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Yumetaro State"               , &_lowMem[0x0053], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Skip Map"                     , &_lowMem[0x0054], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Weapon Use Timer"             , &_lowMem[0x0055], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Music State"                  , &_lowMem[0x0071], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Player Status"                , &_lowMem[0x0090], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Player Pos Y 2"               , &_lowMem[0x009A], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Player Pos Y 1"               , &_lowMem[0x00A4], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Player Pos X 2"               , &_lowMem[0x00AE], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Player Pos X 1"               , &_lowMem[0x00B8], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Player Speed Y"               , &_lowMem[0x00C2], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Player Speed X"               , &_lowMem[0x00CC], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Player Stand"                 , &_lowMem[0x00D6], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Weapon Status"                , &_lowMem[0x0091], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Weapon Pos Y 2"               , &_lowMem[0x009B], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Weapon Pos Y 1"               , &_lowMem[0x00A5], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Weapon Pos X 2"               , &_lowMem[0x00AF], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Weapon Pos X 1"               , &_lowMem[0x00B9], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Weapon Speed Y"               , &_lowMem[0x00C3], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Weapon Speed X"               , &_lowMem[0x00CD], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Weapon Stand"                 , &_lowMem[0x00D7], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Object 1 Status"              , &_lowMem[0x0092], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Object 1 Pos Y 2"             , &_lowMem[0x009C], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Object 1 Pos Y 1"             , &_lowMem[0x00A6], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Object 1 Pos X 2"             , &_lowMem[0x00B0], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Object 1 Pos X 1"             , &_lowMem[0x00BA], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Object 1 Speed Y"             , &_lowMem[0x00C4], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Object 1 Speed X"             , &_lowMem[0x00CE], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Object 1 Stand"               , &_lowMem[0x00D8], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Object 2 Status"              , &_lowMem[0x0092], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Object 2 Pos Y 2"             , &_lowMem[0x009C], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Object 2 Pos Y 1"             , &_lowMem[0x00A6], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Object 2 Pos X 2"             , &_lowMem[0x00B0], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Object 2 Pos X 1"             , &_lowMem[0x00BA], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Object 2 Speed Y"             , &_lowMem[0x00C4], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Object 2 Speed X"             , &_lowMem[0x00CE], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Object 2 Stand"               , &_lowMem[0x00D8], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Object 3 Status"              , &_lowMem[0x0093], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Object 3 Pos Y 2"             , &_lowMem[0x009D], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Object 3 Pos Y 1"             , &_lowMem[0x00A7], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Object 3 Pos X 2"             , &_lowMem[0x00B1], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Object 3 Pos X 1"             , &_lowMem[0x00BB], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Object 3 Speed Y"             , &_lowMem[0x00C5], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Object 3 Speed X"             , &_lowMem[0x00CF], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Object 3 Stand"               , &_lowMem[0x00D9], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Object 4 Status"              , &_lowMem[0x0094], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Object 4 Pos Y 2"             , &_lowMem[0x009E], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Object 4 Pos Y 1"             , &_lowMem[0x00A8], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Object 4 Pos X 2"             , &_lowMem[0x00B2], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Object 4 Pos X 1"             , &_lowMem[0x00BC], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Object 4 Speed Y"             , &_lowMem[0x00C6], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Object 4 Speed X"             , &_lowMem[0x00D0], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Object 4 Stand"               , &_lowMem[0x00DA], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Object 5 Status"              , &_lowMem[0x0095], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Object 5 Pos Y 2"             , &_lowMem[0x009F], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Object 5 Pos Y 1"             , &_lowMem[0x00A9], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Object 5 Pos X 2"             , &_lowMem[0x00B3], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Object 5 Pos X 1"             , &_lowMem[0x00BD], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Object 5 Speed Y"             , &_lowMem[0x00C7], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Object 5 Speed X"             , &_lowMem[0x00D1], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Object 5 Stand"               , &_lowMem[0x00DB], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Object 6 Status"              , &_lowMem[0x0096], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Object 6 Pos Y 2"             , &_lowMem[0x00A0], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Object 6 Pos Y 1"             , &_lowMem[0x00AA], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Object 6 Pos X 2"             , &_lowMem[0x00B4], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Object 6 Pos X 1"             , &_lowMem[0x00BE], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Object 6 Speed Y"             , &_lowMem[0x00C8], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Object 6 Speed X"             , &_lowMem[0x00D2], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Object 6 Stand"               , &_lowMem[0x00DC], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Object 7 Status"              , &_lowMem[0x0097], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Object 7 Pos Y 2"             , &_lowMem[0x00A1], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Object 7 Pos Y 1"             , &_lowMem[0x00AB], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Object 7 Pos X 2"             , &_lowMem[0x00B5], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Object 7 Pos X 1"             , &_lowMem[0x00BF], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Object 7 Speed Y"             , &_lowMem[0x00C9], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Object 7 Speed X"             , &_lowMem[0x00D3], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Object 7 Stand"               , &_lowMem[0x00DD], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Object 8 Status"              , &_lowMem[0x0098], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Object 8 Pos Y 2"             , &_lowMem[0x00A2], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Object 8 Pos Y 1"             , &_lowMem[0x00AC], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Object 8 Pos X 2"             , &_lowMem[0x00B6], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Object 8 Pos X 1"             , &_lowMem[0x00C0], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Object 8 Speed Y"             , &_lowMem[0x00CA], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Object 8 Speed X"             , &_lowMem[0x00D4], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Object 8 Stand"               , &_lowMem[0x00DE], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Object 9 Status"              , &_lowMem[0x0099], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Object 9 Pos Y 2"             , &_lowMem[0x00A3], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Object 9 Pos Y 1"             , &_lowMem[0x00AD], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Object 9 Pos X 2"             , &_lowMem[0x00B7], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Object 9 Pos X 1"             , &_lowMem[0x00C1], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Object 9 Speed Y"             , &_lowMem[0x00CB], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Object 9 Speed X"             , &_lowMem[0x00D5], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Object 9 Stand"               , &_lowMem[0x00DF], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Respawn Timer"                , &_lowMem[0x00EA], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Global timer 1"               , &_lowMem[0x00ED], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Global timer 2"               , &_lowMem[0x00EE], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Player Inputs 1"              , &_lowMem[0x00F5], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Player Inputs 2"              , &_lowMem[0x00F6], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Player Buttons 1"             , &_lowMem[0x00F7], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Player Buttons 2"             , &_lowMem[0x00F8], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("PPU Scroll Y"                 , &_lowMem[0x00FC], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("PPU Scroll X"                 , &_lowMem[0x00FD], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("PPU Mask Mirror"              , &_lowMem[0x00FE], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Player Lives"                 , &_lowMem[0x0104], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Secret Items Collected 1"     , &_lowMem[0x0105], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Secret Items Collected 2"     , &_lowMem[0x0106], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Secret Items Collected 3"     , &_lowMem[0x0107], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Secret Items Collected 4"     , &_lowMem[0x0108], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Secret Items Collected 5"     , &_lowMem[0x0109], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Secret Items Collected 6"     , &_lowMem[0x010A], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Player Animation State"       , &_lowMem[0x0300], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Player Direction"             , &_lowMem[0x030A], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Player Hitbox Y 2"            , &_lowMem[0x0314], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Player Hitbox Y 1"            , &_lowMem[0x031E], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Player Hitbox X 2"            , &_lowMem[0x0328], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Player Hitbox X 1"            , &_lowMem[0x0332], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Player Health"                , &_lowMem[0x0346], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Player Object Type"           , &_lowMem[0x0350], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Player Is Stood"              , &_lowMem[0x0364], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Player Is Standing"           , &_lowMem[0x036E], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Player On Wall"               , &_lowMem[0x0378], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Player Timer 1"               , &_lowMem[0x0382], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Player Timer 2"               , &_lowMem[0x038C], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Player Drop Item"             , &_lowMem[0x03B4], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Weapon Animation State"       , &_lowMem[0x0301], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Weapon Direction"             , &_lowMem[0x030B], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Weapon Hitbox Y 2"            , &_lowMem[0x0315], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Weapon Hitbox Y 1"            , &_lowMem[0x031F], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Weapon Hitbox X 2"            , &_lowMem[0x0329], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Weapon Hitbox X 1"            , &_lowMem[0x0333], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Weapon Health"                , &_lowMem[0x0347], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Weapon Object Type"           , &_lowMem[0x0351], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Weapon Is Stood"              , &_lowMem[0x0365], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Weapon Is Standing"           , &_lowMem[0x036F], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Weapon On Wall"               , &_lowMem[0x0379], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Weapon Timer 1"               , &_lowMem[0x0383], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Weapon Timer 2"               , &_lowMem[0x038D], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Weapon Drop Item"             , &_lowMem[0x03B5], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Object 1 Animation State"     , &_lowMem[0x0302], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Object 1 Direction"           , &_lowMem[0x030C], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Object 1 Hitbox Y 2"          , &_lowMem[0x0316], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Object 1 Hitbox Y 1"          , &_lowMem[0x0320], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Object 1 Hitbox X 2"          , &_lowMem[0x032A], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Object 1 Hitbox X 1"          , &_lowMem[0x0334], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Object 1 Health"              , &_lowMem[0x0348], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Object 1 Object Type"         , &_lowMem[0x0352], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Object 1 Is Stood"            , &_lowMem[0x0366], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Object 1 Is Standing"         , &_lowMem[0x0370], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Object 1 On Wall"             , &_lowMem[0x037A], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Object 1 Timer 1"             , &_lowMem[0x0384], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Object 1 Timer 2"             , &_lowMem[0x038E], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Object 1 Drop Item"           , &_lowMem[0x03B6], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Object 2 Animation State"     , &_lowMem[0x0303], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Object 2 Direction"           , &_lowMem[0x030D], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Object 2 Hitbox Y 2"          , &_lowMem[0x0317], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Object 2 Hitbox Y 1"          , &_lowMem[0x0321], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Object 2 Hitbox X 2"          , &_lowMem[0x032B], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Object 2 Hitbox X 1"          , &_lowMem[0x0335], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Object 2 Health"              , &_lowMem[0x0349], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Object 2 Object Type"         , &_lowMem[0x0353], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Object 2 Is Stood"            , &_lowMem[0x0367], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Object 2 Is Standing"         , &_lowMem[0x0371], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Object 2 On Wall"             , &_lowMem[0x037B], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Object 2 Timer 1"             , &_lowMem[0x0385], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Object 2 Timer 2"             , &_lowMem[0x038F], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Object 2 Drop Item"           , &_lowMem[0x03B7], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Object 3 Animation State"     , &_lowMem[0x0304], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Object 3 Direction"           , &_lowMem[0x030E], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Object 3 Hitbox Y 2"          , &_lowMem[0x0318], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Object 3 Hitbox Y 1"          , &_lowMem[0x0322], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Object 3 Hitbox X 2"          , &_lowMem[0x032C], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Object 3 Hitbox X 1"          , &_lowMem[0x0336], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Object 3 Health"              , &_lowMem[0x034A], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Object 3 Object Type"         , &_lowMem[0x0354], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Object 3 Is Stood"            , &_lowMem[0x0368], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Object 3 Is Standing"         , &_lowMem[0x0372], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Object 3 On Wall"             , &_lowMem[0x037C], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Object 3 Timer 1"             , &_lowMem[0x0386], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Object 3 Timer 2"             , &_lowMem[0x0390], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Object 3 Drop Item"           , &_lowMem[0x03B8], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Object 4 Animation State"     , &_lowMem[0x0305], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Object 4 Direction"           , &_lowMem[0x030F], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Object 4 Hitbox Y 2"          , &_lowMem[0x0319], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Object 4 Hitbox Y 1"          , &_lowMem[0x0323], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Object 4 Hitbox X 2"          , &_lowMem[0x032D], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Object 4 Hitbox X 1"          , &_lowMem[0x0337], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Object 4 Health"              , &_lowMem[0x034B], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Object 4 Object Type"         , &_lowMem[0x0355], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Object 4 Is Stood"            , &_lowMem[0x0369], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Object 4 Is Standing"         , &_lowMem[0x0373], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Object 4 On Wall"             , &_lowMem[0x037D], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Object 4 Timer 1"             , &_lowMem[0x0387], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Object 4 Timer 2"             , &_lowMem[0x0391], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Object 4 Drop Item"           , &_lowMem[0x03B9], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Object 5 Animation State"     , &_lowMem[0x0306], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Object 5 Direction"           , &_lowMem[0x0310], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Object 5 Hitbox Y 2"          , &_lowMem[0x031A], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Object 5 Hitbox Y 1"          , &_lowMem[0x0324], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Object 5 Hitbox X 2"          , &_lowMem[0x032E], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Object 5 Hitbox X 1"          , &_lowMem[0x0338], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Object 5 Health"              , &_lowMem[0x0349], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Object 5 Object Type"         , &_lowMem[0x0356], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Object 5 Is Stood"            , &_lowMem[0x036A], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Object 5 Is Standing"         , &_lowMem[0x0374], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Object 5 On Wall"             , &_lowMem[0x037E], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Object 5 Timer 1"             , &_lowMem[0x0388], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Object 5 Timer 2"             , &_lowMem[0x0392], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Object 5 Drop Item"           , &_lowMem[0x03BA], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Object 6 Animation State"     , &_lowMem[0x0307], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Object 6 Direction"           , &_lowMem[0x0311], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Object 6 Hitbox Y 2"          , &_lowMem[0x031B], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Object 6 Hitbox Y 1"          , &_lowMem[0x0325], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Object 6 Hitbox X 2"          , &_lowMem[0x032F], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Object 6 Hitbox X 1"          , &_lowMem[0x0339], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Object 6 Health"              , &_lowMem[0x034A], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Object 6 Object Type"         , &_lowMem[0x0357], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Object 6 Is Stood"            , &_lowMem[0x036B], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Object 6 Is Standing"         , &_lowMem[0x0375], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Object 6 On Wall"             , &_lowMem[0x037F], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Object 6 Timer 1"             , &_lowMem[0x0389], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Object 6 Timer 2"             , &_lowMem[0x0393], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Object 6 Drop Item"           , &_lowMem[0x03BB], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Object 7 Animation State"     , &_lowMem[0x0308], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Object 7 Direction"           , &_lowMem[0x0312], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Object 7 Hitbox Y 2"          , &_lowMem[0x031C], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Object 7 Hitbox Y 1"          , &_lowMem[0x0326], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Object 7 Hitbox X 2"          , &_lowMem[0x0330], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Object 7 Hitbox X 1"          , &_lowMem[0x033A], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Object 7 Health"              , &_lowMem[0x034B], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Object 7 Object Type"         , &_lowMem[0x0358], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Object 7 Is Stood"            , &_lowMem[0x036C], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Object 7 Is Standing"         , &_lowMem[0x0376], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Object 7 On Wall"             , &_lowMem[0x0380], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Object 7 Timer 1"             , &_lowMem[0x038A], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Object 7 Timer 2"             , &_lowMem[0x0394], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Object 7 Drop Items"          , &_lowMem[0x03BC], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Object 8 Animation State"     , &_lowMem[0x0309], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Object 8 Direction"           , &_lowMem[0x0313], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Object 8 Hitbox Y 2"          , &_lowMem[0x031D], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Object 8 Hitbox Y 1"          , &_lowMem[0x0327], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Object 8 Hitbox X 2"          , &_lowMem[0x0331], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Object 8 Hitbox X 1"          , &_lowMem[0x033B], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Object 8 Health"              , &_lowMem[0x034C], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Object 8 Object Type"         , &_lowMem[0x0359], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Object 8 Is Stood"            , &_lowMem[0x036D], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Object 8 Is Standing"         , &_lowMem[0x0377], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Object 8 On Wall"             , &_lowMem[0x0381], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Object 8 Timer 1"             , &_lowMem[0x038B], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Object 8 Timer 2"             , &_lowMem[0x0395], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Object 8 Drop Items"          , &_lowMem[0x03BD], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Star 5 Pos Y 2"               , &_lowMem[0x0420], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Star 5 Pos Y 1"               , &_lowMem[0x0425], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Star 5 Pos X 1"               , &_lowMem[0x042A], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Star 5 Pos X 2"               , &_lowMem[0x042F], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Star 5 Status"                , &_lowMem[0x0439], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Star 4 Pos Y 2"               , &_lowMem[0x0421], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Star 4 Pos Y 1"               , &_lowMem[0x0426], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Star 4 Pos X 1"               , &_lowMem[0x042B], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Star 4 Pos X 2"               , &_lowMem[0x0430], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Star 4 Status"                , &_lowMem[0x043A], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Star 3 Pos Y 2"               , &_lowMem[0x0422], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Star 3 Pos Y 1"               , &_lowMem[0x0427], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Star 3 Pos X 1"               , &_lowMem[0x042C], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Star 3 Pos X 2"               , &_lowMem[0x0431], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Star 3 Status"                , &_lowMem[0x043B], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Star 2 Pos Y 2"               , &_lowMem[0x0423], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Star 2 Pos Y 1"               , &_lowMem[0x0428], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Star 2 Pos X 1"               , &_lowMem[0x042D], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Star 2 Pos X 2"               , &_lowMem[0x0432], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Star 2 Status"                , &_lowMem[0x043C], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Star 1 Pos Y 2"               , &_lowMem[0x0424], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Star 1 Pos Y 1"               , &_lowMem[0x0429], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Star 1 Pos X 1"               , &_lowMem[0x042E], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Star 1 Pos X 2"               , &_lowMem[0x0433], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Star 1 Status"                , &_lowMem[0x043D], Property::datatype_t::dt_uint8, Property::endianness_t::little);


    _screenX1                       = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Screen X 1"               )]->getPointer();   
    _screenX2                       = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Screen X 2"               )]->getPointer();   
    _screenX3                       = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Screen X 3"               )]->getPointer();   
    _screenY                        = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Screen Y"                 )]->getPointer();   
    _objectIterator                 = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Object Iterator"          )]->getPointer();   
    _currentLevel                   = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Current Level"            )]->getPointer();   
    _currentCheckpoint              = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Current Checkpoint"       )]->getPointer();   
    _enemyDataPointer1              = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Enemy Data Pointer 1"     )]->getPointer();   
    _enemyDataPointer2              = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Enemy Data Pointer 2"     )]->getPointer();   
    _CHRTimer                       = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("CHR Timer"                )]->getPointer();   
    _CHRIndex                       = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("CHR Index"                )]->getPointer();   
    _IRQIndex                       = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("IRQ Index"                )]->getPointer();   
    _screenUpdate                   = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Screen Update"            )]->getPointer();   
    _playerMaxHP                    = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Player Max HP"            )]->getPointer();   
    _scoreIncrease1                 = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Score Increase 1"         )]->getPointer();   
    _scoreIncrease2                 = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Score Increase 2"         )]->getPointer();   
    _score1                         = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Score 1"                  )]->getPointer();   
    _score2                         = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Score 2"                  )]->getPointer();   
    _score3                         = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Score 3"                  )]->getPointer();   
    _score4                         = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Score 4"                  )]->getPointer();   
    _score5                         = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Score 5"                  )]->getPointer();   
    _score6                         = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Score 6"                  )]->getPointer();   
    _score7                         = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Score 7"                  )]->getPointer();   
    _enemyKills                     = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Enemy Kills"              )]->getPointer();   
    _item1                          = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Item 1"                   )]->getPointer();   
    _item2                          = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Item 2"                   )]->getPointer();   
    _item3                          = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Item 3"                   )]->getPointer();   
    _itemAmount                     = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Item Amount"              )]->getPointer();   
    _PRGBank1                       = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("PRG Bank 1"               )]->getPointer();   
    _PRGBank2                       = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("PRG Bank 2"               )]->getPointer();   
    _screenLocked                   = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Screen Locked"            )]->getPointer();   
    _playerJumping                  = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Player Jumping"           )]->getPointer();   
    _cutsceneState                  = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Cutscene State"           )]->getPointer();   
    _musicSong                      = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Music Song"               )]->getPointer();   
    _yumetaroState                  = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Yumetaro State"           )]->getPointer();   
    _skipMap                        = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Skip Map"                 )]->getPointer();   
    _weaponUseTimer                 = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Weapon Use Timer"         )]->getPointer();   
    _musicState                     = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Music State"              )]->getPointer();   
    _playerStatus                   = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Player Status"            )]->getPointer();   
    _playerPosY2                    = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Player Pos Y 2"           )]->getPointer();   
    _playerPosY1                    = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Player Pos Y 1"           )]->getPointer();   
    _playerPosX2                    = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Player Pos X 2"           )]->getPointer();   
    _playerPosX1                    = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Player Pos X 1"           )]->getPointer();   
    _playerSpeedY                   = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Player Speed Y"           )]->getPointer();   
    _playerSpeedX                   = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Player Speed X"           )]->getPointer();   
    _playerStand                    = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Player Stand"             )]->getPointer();   
    _weaponStatus                   = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Weapon Status"            )]->getPointer();   
    _weaponPosY2                    = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Weapon Pos Y 2"           )]->getPointer();   
    _weaponPosY1                    = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Weapon Pos Y 1"           )]->getPointer();   
    _weaponPosX2                    = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Weapon Pos X 2"           )]->getPointer();   
    _weaponPosX1                    = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Weapon Pos X 1"           )]->getPointer();   
    _weaponSpeedY                   = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Weapon Speed Y"           )]->getPointer();   
    _weaponSpeedX                   = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Weapon Speed X"           )]->getPointer();   
    _weaponStand                    = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Weapon Stand"             )]->getPointer();   
    _object1Status                  = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Object 1 Status"          )]->getPointer();   
    _object1PosY2                   = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Object 1 Pos Y 2"         )]->getPointer();   
    _object1PosY1                   = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Object 1 Pos Y 1"         )]->getPointer();   
    _object1PosX2                   = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Object 1 Pos X 2"         )]->getPointer();   
    _object1PosX1                   = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Object 1 Pos X 1"         )]->getPointer();   
    _object1SpeedY                  = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Object 1 Speed Y"         )]->getPointer();   
    _object1SpeedX                  = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Object 1 Speed X"         )]->getPointer();   
    _object1Stand                   = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Object 1 Stand"           )]->getPointer();   
    _object2Status                  = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Object 2 Status"          )]->getPointer();   
    _object2PosY2                   = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Object 2 Pos Y 2"         )]->getPointer();   
    _object2PosY1                   = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Object 2 Pos Y 1"         )]->getPointer();   
    _object2PosX2                   = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Object 2 Pos X 2"         )]->getPointer();   
    _object2PosX1                   = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Object 2 Pos X 1"         )]->getPointer();   
    _object2SpeedY                  = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Object 2 Speed Y"         )]->getPointer();   
    _object2SpeedX                  = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Object 2 Speed X"         )]->getPointer();   
    _object2Stand                   = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Object 2 Stand"           )]->getPointer();   
    _object3Status                  = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Object 3 Status"          )]->getPointer();   
    _object3PosY2                   = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Object 3 Pos Y 2"         )]->getPointer();   
    _object3PosY1                   = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Object 3 Pos Y 1"         )]->getPointer();   
    _object3PosX2                   = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Object 3 Pos X 2"         )]->getPointer();   
    _object3PosX1                   = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Object 3 Pos X 1"         )]->getPointer();   
    _object3SpeedY                  = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Object 3 Speed Y"         )]->getPointer();   
    _object3SpeedX                  = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Object 3 Speed X"         )]->getPointer();   
    _object3Stand                   = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Object 3 Stand"           )]->getPointer();   
    _object4Status                  = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Object 4 Status"          )]->getPointer();   
    _object4PosY2                   = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Object 4 Pos Y 2"         )]->getPointer();   
    _object4PosY1                   = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Object 4 Pos Y 1"         )]->getPointer();   
    _object4PosX2                   = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Object 4 Pos X 2"         )]->getPointer();   
    _object4PosX1                   = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Object 4 Pos X 1"         )]->getPointer();   
    _object4SpeedY                  = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Object 4 Speed Y"         )]->getPointer();   
    _object4SpeedX                  = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Object 4 Speed X"         )]->getPointer();   
    _object4Stand                   = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Object 4 Stand"           )]->getPointer();   
    _object5Status                  = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Object 5 Status"          )]->getPointer();   
    _object5PosY2                   = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Object 5 Pos Y 2"         )]->getPointer();   
    _object5PosY1                   = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Object 5 Pos Y 1"         )]->getPointer();   
    _object5PosX2                   = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Object 5 Pos X 2"         )]->getPointer();   
    _object5PosX1                   = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Object 5 Pos X 1"         )]->getPointer();   
    _object5SpeedY                  = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Object 5 Speed Y"         )]->getPointer();   
    _object5SpeedX                  = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Object 5 Speed X"         )]->getPointer();   
    _object5Stand                   = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Object 5 Stand"           )]->getPointer();   
    _object6Status                  = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Object 6 Status"          )]->getPointer();   
    _object6PosY2                   = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Object 6 Pos Y 2"         )]->getPointer();   
    _object6PosY1                   = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Object 6 Pos Y 1"         )]->getPointer();   
    _object6PosX2                   = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Object 6 Pos X 2"         )]->getPointer();   
    _object6PosX1                   = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Object 6 Pos X 1"         )]->getPointer();   
    _object6SpeedY                  = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Object 6 Speed Y"         )]->getPointer();   
    _object6SpeedX                  = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Object 6 Speed X"         )]->getPointer();   
    _object6Stand                   = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Object 6 Stand"           )]->getPointer();   
    _object7Status                  = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Object 7 Status"          )]->getPointer();   
    _object7PosY2                   = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Object 7 Pos Y 2"         )]->getPointer();   
    _object7PosY1                   = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Object 7 Pos Y 1"         )]->getPointer();   
    _object7PosX2                   = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Object 7 Pos X 2"         )]->getPointer();   
    _object7PosX1                   = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Object 7 Pos X 1"         )]->getPointer();   
    _object7SpeedY                  = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Object 7 Speed Y"         )]->getPointer();   
    _object7SpeedX                  = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Object 7 Speed X"         )]->getPointer();   
    _object7Stand                   = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Object 7 Stand"           )]->getPointer();   
    _object8Status                  = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Object 8 Status"          )]->getPointer();   
    _object8PosY2                   = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Object 8 Pos Y 2"         )]->getPointer();   
    _object8PosY1                   = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Object 8 Pos Y 1"         )]->getPointer();   
    _object8PosX2                   = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Object 8 Pos X 2"         )]->getPointer();   
    _object8PosX1                   = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Object 8 Pos X 1"         )]->getPointer();   
    _object8SpeedY                  = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Object 8 Speed Y"         )]->getPointer();   
    _object8SpeedX                  = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Object 8 Speed X"         )]->getPointer();   
    _object8Stand                   = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Object 8 Stand"           )]->getPointer();   
    _object9Status                  = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Object 9 Status"          )]->getPointer();   
    _object9PosY2                   = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Object 9 Pos Y 2"         )]->getPointer();   
    _object9PosY1                   = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Object 9 Pos Y 1"         )]->getPointer();   
    _object9PosX2                   = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Object 9 Pos X 2"         )]->getPointer();   
    _object9PosX1                   = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Object 9 Pos X 1"         )]->getPointer();   
    _object9SpeedY                  = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Object 9 Speed Y"         )]->getPointer();   
    _object9SpeedX                  = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Object 9 Speed X"         )]->getPointer();   
    _object9Stand                   = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Object 9 Stand"           )]->getPointer();   
    _respawnTimer                   = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Respawn Timer"            )]->getPointer();   
    _globaltimer1                   = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Global timer 1"           )]->getPointer();   
    _globaltimer2                   = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Global timer 2"           )]->getPointer();   
    _playerInputs1                  = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Player Inputs 1"          )]->getPointer();   
    _playerInputs2                  = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Player Inputs 2"          )]->getPointer();   
    _playerButtons1                 = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Player Buttons 1"         )]->getPointer();   
    _playerButtons2                 = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Player Buttons 2"         )]->getPointer();   
    _PPUScrollY                     = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("PPU Scroll Y"             )]->getPointer();   
    _PPUScrollX                     = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("PPU Scroll X"             )]->getPointer();   
    _PPUMaskMirror                  = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("PPU Mask Mirror"          )]->getPointer();   
    _playerLives                    = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Player Lives"             )]->getPointer();   
    _secretItemsCollected1          = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Secret Items Collected 1" )]->getPointer();   
    _secretItemsCollected2          = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Secret Items Collected 2" )]->getPointer();   
    _secretItemsCollected3          = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Secret Items Collected 3" )]->getPointer();   
    _secretItemsCollected4          = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Secret Items Collected 4" )]->getPointer();   
    _secretItemsCollected5          = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Secret Items Collected 5" )]->getPointer();   
    _secretItemsCollected6          = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Secret Items Collected 6" )]->getPointer();   
    _playerAnimationState           = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Player Animation State"   )]->getPointer();   
    _playerDirection                = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Player Direction"         )]->getPointer();   
    _playerHitboxY2                 = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Player Hitbox Y 2"        )]->getPointer();   
    _playerHitboxY1                 = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Player Hitbox Y 1"        )]->getPointer();   
    _playerHitboxX2                 = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Player Hitbox X 2"        )]->getPointer();   
    _playerHitboxX1                 = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Player Hitbox X 1"        )]->getPointer();   
    _playerHealth                   = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Player Health"            )]->getPointer();   
    _playerObjectType               = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Player Object Type"       )]->getPointer();   
    _playerIsStood                  = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Player Is Stood"          )]->getPointer();   
    _playerIsStanding               = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Player Is Standing"       )]->getPointer();   
    _playerOnWall                   = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Player On Wall"           )]->getPointer();   
    _playerTimer1                   = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Player Timer 1"           )]->getPointer();   
    _playerTimer2                   = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Player Timer 2"           )]->getPointer();   
    _playerDropItem                 = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Player Drop Item"         )]->getPointer();   
    _weaponAnimationState           = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Weapon Animation State"   )]->getPointer();   
    _weaponDirection                = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Weapon Direction"         )]->getPointer();   
    _weaponHitboxY2                 = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Weapon Hitbox Y 2"        )]->getPointer();   
    _weaponHitboxY1                 = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Weapon Hitbox Y 1"        )]->getPointer();   
    _weaponHitboxX2                 = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Weapon Hitbox X 2"        )]->getPointer();   
    _weaponHitboxX1                 = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Weapon Hitbox X 1"        )]->getPointer();   
    _weaponHealth                   = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Weapon Health"            )]->getPointer();   
    _weaponObjectType               = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Weapon Object Type"       )]->getPointer();   
    _weaponIsStood                  = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Weapon Is Stood"          )]->getPointer();   
    _weaponIsStanding               = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Weapon Is Standing"       )]->getPointer();   
    _weaponOnWall                   = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Weapon On Wall"           )]->getPointer();   
    _weaponTimer1                   = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Weapon Timer 1"           )]->getPointer();   
    _weaponTimer2                   = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Weapon Timer 2"           )]->getPointer();   
    _weaponDropItem                 = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Weapon Drop Item"         )]->getPointer();   
    _object1AnimationState          = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Object 1 Animation State" )]->getPointer();   
    _object1Direction               = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Object 1 Direction"       )]->getPointer();   
    _object1HitboxY2                = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Object 1 Hitbox Y 2"      )]->getPointer();   
    _object1HitboxY1                = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Object 1 Hitbox Y 1"      )]->getPointer();   
    _object1HitboxX2                = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Object 1 Hitbox X 2"      )]->getPointer();   
    _object1HitboxX1                = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Object 1 Hitbox X 1"      )]->getPointer();   
    _object1Health                  = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Object 1 Health"          )]->getPointer();   
    _object1ObjectType              = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Object 1 Object Type"     )]->getPointer();   
    _object1IsStood                 = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Object 1 Is Stood"        )]->getPointer();   
    _object1IsStanding              = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Object 1 Is Standing"     )]->getPointer();   
    _object1OnWall                  = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Object 1 On Wall"         )]->getPointer();   
    _object1Timer1                  = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Object 1 Timer 1"         )]->getPointer();   
    _object1Timer2                  = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Object 1 Timer 2"         )]->getPointer();   
    _object1DropItem                = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Object 1 Drop Item"       )]->getPointer();   
    _object2AnimationState          = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Object 2 Animation State" )]->getPointer();   
    _object2Direction               = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Object 2 Direction"       )]->getPointer();   
    _object2HitboxY2                = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Object 2 Hitbox Y 2"      )]->getPointer();   
    _object2HitboxY1                = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Object 2 Hitbox Y 1"      )]->getPointer();   
    _object2HitboxX2                = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Object 2 Hitbox X 2"      )]->getPointer();   
    _object2HitboxX1                = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Object 2 Hitbox X 1"      )]->getPointer();   
    _object2Health                  = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Object 2 Health"          )]->getPointer();   
    _object2ObjectType              = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Object 2 Object Type"     )]->getPointer();   
    _object2IsStood                 = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Object 2 Is Stood"        )]->getPointer();   
    _object2IsStanding              = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Object 2 Is Standing"     )]->getPointer();   
    _object2OnWall                  = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Object 2 On Wall"         )]->getPointer();   
    _object2Timer1                  = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Object 2 Timer 1"         )]->getPointer();   
    _object2Timer2                  = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Object 2 Timer 2"         )]->getPointer();   
    _object2DropItem                = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Object 2 Drop Item"       )]->getPointer();   
    _object3AnimationState          = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Object 3 Animation State" )]->getPointer();   
    _object3Direction               = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Object 3 Direction"       )]->getPointer();   
    _object3HitboxY2                = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Object 3 Hitbox Y 2"      )]->getPointer();   
    _object3HitboxY1                = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Object 3 Hitbox Y 1"      )]->getPointer();   
    _object3HitboxX2                = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Object 3 Hitbox X 2"      )]->getPointer();   
    _object3HitboxX1                = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Object 3 Hitbox X 1"      )]->getPointer();   
    _object3Health                  = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Object 3 Health"          )]->getPointer();   
    _object3ObjectType              = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Object 3 Object Type"     )]->getPointer();   
    _object3IsStood                 = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Object 3 Is Stood"        )]->getPointer();   
    _object3IsStanding              = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Object 3 Is Standing"     )]->getPointer();   
    _object3OnWall                  = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Object 3 On Wall"         )]->getPointer();   
    _object3Timer1                  = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Object 3 Timer 1"         )]->getPointer();   
    _object3Timer2                  = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Object 3 Timer 2"         )]->getPointer();   
    _object3DropItem                = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Object 3 Drop Item"       )]->getPointer();   
    _object4AnimationState          = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Object 4 Animation State" )]->getPointer();   
    _object4Direction               = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Object 4 Direction"       )]->getPointer();   
    _object4HitboxY2                = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Object 4 Hitbox Y 2"      )]->getPointer();   
    _object4HitboxY1                = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Object 4 Hitbox Y 1"      )]->getPointer();   
    _object4HitboxX2                = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Object 4 Hitbox X 2"      )]->getPointer();   
    _object4HitboxX1                = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Object 4 Hitbox X 1"      )]->getPointer();   
    _object4Health                  = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Object 4 Health"          )]->getPointer();   
    _object4ObjectType              = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Object 4 Object Type"     )]->getPointer();   
    _object4IsStood                 = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Object 4 Is Stood"        )]->getPointer();   
    _object4IsStanding              = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Object 4 Is Standing"     )]->getPointer();   
    _object4OnWall                  = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Object 4 On Wall"         )]->getPointer();   
    _object4Timer1                  = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Object 4 Timer 1"         )]->getPointer();   
    _object4Timer2                  = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Object 4 Timer 2"         )]->getPointer();   
    _object4DropItem                = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Object 4 Drop Item"       )]->getPointer();   
    _object5AnimationState          = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Object 5 Animation State" )]->getPointer();   
    _object5Direction               = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Object 5 Direction"       )]->getPointer();   
    _object5HitboxY2                = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Object 5 Hitbox Y 2"      )]->getPointer();   
    _object5HitboxY1                = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Object 5 Hitbox Y 1"      )]->getPointer();   
    _object5HitboxX2                = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Object 5 Hitbox X 2"      )]->getPointer();   
    _object5HitboxX1                = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Object 5 Hitbox X 1"      )]->getPointer();   
    _object5Health                  = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Object 5 Health"          )]->getPointer();   
    _object5ObjectType              = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Object 5 Object Type"     )]->getPointer();   
    _object5IsStood                 = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Object 5 Is Stood"        )]->getPointer();   
    _object5IsStanding              = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Object 5 Is Standing"     )]->getPointer();   
    _object5OnWall                  = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Object 5 On Wall"         )]->getPointer();   
    _object5Timer1                  = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Object 5 Timer 1"         )]->getPointer();   
    _object5Timer2                  = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Object 5 Timer 2"         )]->getPointer();   
    _object5DropItem                = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Object 5 Drop Item"       )]->getPointer();   
    _object6AnimationState          = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Object 6 Animation State" )]->getPointer();   
    _object6Direction               = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Object 6 Direction"       )]->getPointer();   
    _object6HitboxY2                = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Object 6 Hitbox Y 2"      )]->getPointer();   
    _object6HitboxY1                = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Object 6 Hitbox Y 1"      )]->getPointer();   
    _object6HitboxX2                = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Object 6 Hitbox X 2"      )]->getPointer();   
    _object6HitboxX1                = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Object 6 Hitbox X 1"      )]->getPointer();   
    _object6Health                  = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Object 6 Health"          )]->getPointer();   
    _object6ObjectType              = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Object 6 Object Type"     )]->getPointer();   
    _object6IsStood                 = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Object 6 Is Stood"        )]->getPointer();   
    _object6IsStanding              = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Object 6 Is Standing"     )]->getPointer();   
    _object6OnWall                  = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Object 6 On Wall"         )]->getPointer();   
    _object6Timer1                  = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Object 6 Timer 1"         )]->getPointer();   
    _object6Timer2                  = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Object 6 Timer 2"         )]->getPointer();   
    _object6DropItem                = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Object 6 Drop Item"       )]->getPointer();   
    _object7AnimationState          = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Object 7 Animation State" )]->getPointer();   
    _object7Direction               = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Object 7 Direction"       )]->getPointer();   
    _object7HitboxY2                = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Object 7 Hitbox Y 2"      )]->getPointer();   
    _object7HitboxY1                = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Object 7 Hitbox Y 1"      )]->getPointer();   
    _object7HitboxX2                = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Object 7 Hitbox X 2"      )]->getPointer();   
    _object7HitboxX1                = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Object 7 Hitbox X 1"      )]->getPointer();   
    _object7Health                  = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Object 7 Health"          )]->getPointer();   
    _object7ObjectType              = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Object 7 Object Type"     )]->getPointer();   
    _object7IsStood                 = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Object 7 Is Stood"        )]->getPointer();   
    _object7IsStanding              = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Object 7 Is Standing"     )]->getPointer();   
    _object7OnWall                  = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Object 7 On Wall"         )]->getPointer();   
    _object7Timer1                  = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Object 7 Timer 1"         )]->getPointer();   
    _object7Timer2                  = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Object 7 Timer 2"         )]->getPointer();   
    _object7DropItems               = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Object 7 Drop Items"      )]->getPointer();   
    _object8AnimationState          = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Object 8 Animation State" )]->getPointer();   
    _object8Direction               = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Object 8 Direction"       )]->getPointer();   
    _object8HitboxY2                = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Object 8 Hitbox Y 2"      )]->getPointer();   
    _object8HitboxY1                = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Object 8 Hitbox Y 1"      )]->getPointer();   
    _object8HitboxX2                = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Object 8 Hitbox X 2"      )]->getPointer();   
    _object8HitboxX1                = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Object 8 Hitbox X 1"      )]->getPointer();   
    _object8Health                  = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Object 8 Health"          )]->getPointer();   
    _object8ObjectType              = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Object 8 Object Type"     )]->getPointer();   
    _object8IsStood                 = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Object 8 Is Stood"        )]->getPointer();   
    _object8IsStanding              = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Object 8 Is Standing"     )]->getPointer();   
    _object8OnWall                  = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Object 8 On Wall"         )]->getPointer();   
    _object8Timer1                  = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Object 8 Timer 1"         )]->getPointer();   
    _object8Timer2                  = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Object 8 Timer 2"         )]->getPointer();   
    _object8DropItems               = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Object 8 Drop Items"      )]->getPointer();   
    _star5PosY2                     = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Star 5 Pos Y 2"           )]->getPointer();   
    _star5PosY1                     = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Star 5 Pos Y 1"           )]->getPointer();   
    _star5PosX1                     = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Star 5 Pos X 1"           )]->getPointer();   
    _star5PosX2                     = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Star 5 Pos X 2"           )]->getPointer();   
    _star5Status                    = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Star 5 Status"            )]->getPointer();   
    _star4PosY2                     = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Star 4 Pos Y 2"           )]->getPointer();   
    _star4PosY1                     = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Star 4 Pos Y 1"           )]->getPointer();   
    _star4PosX1                     = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Star 4 Pos X 1"           )]->getPointer();   
    _star4PosX2                     = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Star 4 Pos X 2"           )]->getPointer();   
    _star4Status                    = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Star 4 Status"            )]->getPointer();   
    _star3PosY2                     = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Star 3 Pos Y 2"           )]->getPointer();   
    _star3PosY1                     = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Star 3 Pos Y 1"           )]->getPointer();   
    _star3PosX1                     = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Star 3 Pos X 1"           )]->getPointer();   
    _star3PosX2                     = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Star 3 Pos X 2"           )]->getPointer();   
    _star3Status                    = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Star 3 Status"            )]->getPointer();   
    _star2PosY2                     = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Star 2 Pos Y 2"           )]->getPointer();   
    _star2PosY1                     = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Star 2 Pos Y 1"           )]->getPointer();   
    _star2PosX1                     = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Star 2 Pos X 1"           )]->getPointer();   
    _star2PosX2                     = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Star 2 Pos X 2"           )]->getPointer();   
    _star2Status                    = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Star 2 Status"            )]->getPointer();   
    _star1PosY2                     = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Star 1 Pos Y 2"           )]->getPointer();   
    _star1PosY1                     = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Star 1 Pos Y 1"           )]->getPointer();   
    _star1PosX1                     = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Star 1 Pos X 1"           )]->getPointer();   
    _star1PosX2                     = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Star 1 Pos X 2"           )]->getPointer();   
    _star1Status                    = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Star 1 Status"            )]->getPointer();   

    registerGameProperty("Player Pos X"                , &_playerPosX, Property::datatype_t::dt_float32, Property::endianness_t::little);
    registerGameProperty("Player Pos Y"                , &_playerPosY, Property::datatype_t::dt_float32, Property::endianness_t::little);

    // Initializing values
    _currentStep   = 0;
    _lastInputStep = 0;

    // Getting index for a non input
    _nullInputIdx = _emulator->registerInput("|..|........|");

    stateUpdatePostHook();
  }


  __INLINE__ void advanceStateImpl(const InputSet::inputIndex_t input) override
  {
    _emulator->advanceState(input);

    // Increasing counter if input is null
    if (input != _nullInputIdx) _lastInputStep = _currentStep;
    _currentStep++;
  }

  __INLINE__ void computeAdditionalHashing(MetroHash128& hashEngine) const override
  {
    hashEngine.Update(_screenX1             , sizeof(_screenX1               ));         
    hashEngine.Update(_screenX2             , sizeof(_screenX2               ));         
    hashEngine.Update(_screenX3             , sizeof(_screenX3               ));         
    hashEngine.Update(_screenY              , sizeof(_screenY                ));         
    hashEngine.Update(_objectIterator       , sizeof(_objectIterator         ));         
    hashEngine.Update(_currentLevel         , sizeof(_currentLevel           ));         
    hashEngine.Update(_currentCheckpoint    , sizeof(_currentCheckpoint      ));         
    hashEngine.Update(_enemyDataPointer1    , sizeof(_enemyDataPointer1      ));         
    hashEngine.Update(_enemyDataPointer2    , sizeof(_enemyDataPointer2      ));         
    // hashEngine.Update(_CHRTimer             , sizeof(_CHRTimer               ));         
    // hashEngine.Update(_CHRIndex             , sizeof(_CHRIndex               ));         
    hashEngine.Update(_IRQIndex             , sizeof(_IRQIndex               ));         
    hashEngine.Update(_screenUpdate         , sizeof(_screenUpdate           ));         
    hashEngine.Update(_playerMaxHP          , sizeof(_playerMaxHP            ));         
    hashEngine.Update(_scoreIncrease1       , sizeof(_scoreIncrease1         ));         
    hashEngine.Update(_scoreIncrease2       , sizeof(_scoreIncrease2         ));         
    hashEngine.Update(_score1               , sizeof(_score1                 ));         
    hashEngine.Update(_score2               , sizeof(_score2                 ));         
    hashEngine.Update(_score3               , sizeof(_score3                 ));         
    hashEngine.Update(_score4               , sizeof(_score4                 ));         
    hashEngine.Update(_score5               , sizeof(_score5                 ));         
    hashEngine.Update(_score6               , sizeof(_score6                 ));         
    hashEngine.Update(_score7               , sizeof(_score7                 ));         
    hashEngine.Update(_enemyKills           , sizeof(_enemyKills             ));         
    hashEngine.Update(_item1                , sizeof(_item1                  ));         
    hashEngine.Update(_item2                , sizeof(_item2                  ));         
    hashEngine.Update(_item3                , sizeof(_item3                  ));         
    hashEngine.Update(_itemAmount           , sizeof(_itemAmount             ));         
    hashEngine.Update(_PRGBank1             , sizeof(_PRGBank1               ));         
    hashEngine.Update(_PRGBank2             , sizeof(_PRGBank2               ));         
    hashEngine.Update(_screenLocked         , sizeof(_screenLocked           ));         
    hashEngine.Update(_playerJumping        , sizeof(_playerJumping          ));         
    hashEngine.Update(_cutsceneState        , sizeof(_cutsceneState          ));         
    hashEngine.Update(_musicSong            , sizeof(_musicSong              ));         
    hashEngine.Update(_yumetaroState        , sizeof(_yumetaroState          ));         
    hashEngine.Update(_skipMap              , sizeof(_skipMap                ));         
    hashEngine.Update(_weaponUseTimer       , sizeof(_weaponUseTimer         ));         
    hashEngine.Update(_musicState           , sizeof(_musicState             ));         
    hashEngine.Update(_playerStatus         , sizeof(_playerStatus           ));         
    hashEngine.Update(_playerPosY2          , sizeof(_playerPosY2            ));         
    hashEngine.Update(_playerPosY1          , sizeof(_playerPosY1            ));         
    hashEngine.Update(_playerPosX2          , sizeof(_playerPosX2            ));         
    hashEngine.Update(_playerPosX1          , sizeof(_playerPosX1            ));         
    hashEngine.Update(_playerSpeedY         , sizeof(_playerSpeedY           ));         
    hashEngine.Update(_playerSpeedX         , sizeof(_playerSpeedX           ));         
    hashEngine.Update(_playerStand          , sizeof(_playerStand            ));         
    hashEngine.Update(_weaponStatus         , sizeof(_weaponStatus           ));         
    hashEngine.Update(_weaponPosY2          , sizeof(_weaponPosY2            ));         
    hashEngine.Update(_weaponPosY1          , sizeof(_weaponPosY1            ));         
    hashEngine.Update(_weaponPosX2          , sizeof(_weaponPosX2            ));         
    hashEngine.Update(_weaponPosX1          , sizeof(_weaponPosX1            ));         
    hashEngine.Update(_weaponSpeedY         , sizeof(_weaponSpeedY           ));         
    hashEngine.Update(_weaponSpeedX         , sizeof(_weaponSpeedX           ));         
    hashEngine.Update(_weaponStand          , sizeof(_weaponStand            ));         
    hashEngine.Update(_object1Status        , sizeof(_object1Status          ));         
    hashEngine.Update(_object1PosY2         , sizeof(_object1PosY2           ));         
    hashEngine.Update(_object1PosY1         , sizeof(_object1PosY1           ));         
    hashEngine.Update(_object1PosX2         , sizeof(_object1PosX2           ));         
    hashEngine.Update(_object1PosX1         , sizeof(_object1PosX1           ));         
    hashEngine.Update(_object1SpeedY        , sizeof(_object1SpeedY          ));         
    hashEngine.Update(_object1SpeedX        , sizeof(_object1SpeedX          ));         
    hashEngine.Update(_object1Stand         , sizeof(_object1Stand           ));         
    hashEngine.Update(_object2Status        , sizeof(_object2Status          ));         
    hashEngine.Update(_object2PosY2         , sizeof(_object2PosY2           ));         
    hashEngine.Update(_object2PosY1         , sizeof(_object2PosY1           ));         
    hashEngine.Update(_object2PosX2         , sizeof(_object2PosX2           ));         
    hashEngine.Update(_object2PosX1         , sizeof(_object2PosX1           ));         
    hashEngine.Update(_object2SpeedY        , sizeof(_object2SpeedY          ));         
    hashEngine.Update(_object2SpeedX        , sizeof(_object2SpeedX          ));         
    hashEngine.Update(_object2Stand         , sizeof(_object2Stand           ));         
    hashEngine.Update(_object3Status        , sizeof(_object3Status          ));         
    hashEngine.Update(_object3PosY2         , sizeof(_object3PosY2           ));         
    hashEngine.Update(_object3PosY1         , sizeof(_object3PosY1           ));         
    hashEngine.Update(_object3PosX2         , sizeof(_object3PosX2           ));         
    hashEngine.Update(_object3PosX1         , sizeof(_object3PosX1           ));         
    hashEngine.Update(_object3SpeedY        , sizeof(_object3SpeedY          ));         
    hashEngine.Update(_object3SpeedX        , sizeof(_object3SpeedX          ));         
    hashEngine.Update(_object3Stand         , sizeof(_object3Stand           ));         
    hashEngine.Update(_object4Status        , sizeof(_object4Status          ));         
    hashEngine.Update(_object4PosY2         , sizeof(_object4PosY2           ));         
    hashEngine.Update(_object4PosY1         , sizeof(_object4PosY1           ));         
    hashEngine.Update(_object4PosX2         , sizeof(_object4PosX2           ));         
    hashEngine.Update(_object4PosX1         , sizeof(_object4PosX1           ));         
    hashEngine.Update(_object4SpeedY        , sizeof(_object4SpeedY          ));         
    hashEngine.Update(_object4SpeedX        , sizeof(_object4SpeedX          ));         
    hashEngine.Update(_object4Stand         , sizeof(_object4Stand           ));         
    hashEngine.Update(_object5Status        , sizeof(_object5Status          ));         
    hashEngine.Update(_object5PosY2         , sizeof(_object5PosY2           ));         
    hashEngine.Update(_object5PosY1         , sizeof(_object5PosY1           ));         
    hashEngine.Update(_object5PosX2         , sizeof(_object5PosX2           ));         
    hashEngine.Update(_object5PosX1         , sizeof(_object5PosX1           ));         
    hashEngine.Update(_object5SpeedY        , sizeof(_object5SpeedY          ));         
    hashEngine.Update(_object5SpeedX        , sizeof(_object5SpeedX          ));         
    hashEngine.Update(_object5Stand         , sizeof(_object5Stand           ));         
    hashEngine.Update(_object6Status        , sizeof(_object6Status          ));         
    hashEngine.Update(_object6PosY2         , sizeof(_object6PosY2           ));         
    hashEngine.Update(_object6PosY1         , sizeof(_object6PosY1           ));         
    hashEngine.Update(_object6PosX2         , sizeof(_object6PosX2           ));         
    hashEngine.Update(_object6PosX1         , sizeof(_object6PosX1           ));         
    hashEngine.Update(_object6SpeedY        , sizeof(_object6SpeedY          ));         
    hashEngine.Update(_object6SpeedX        , sizeof(_object6SpeedX          ));         
    hashEngine.Update(_object6Stand         , sizeof(_object6Stand           ));         
    hashEngine.Update(_object7Status        , sizeof(_object7Status          ));         
    hashEngine.Update(_object7PosY2         , sizeof(_object7PosY2           ));         
    hashEngine.Update(_object7PosY1         , sizeof(_object7PosY1           ));         
    hashEngine.Update(_object7PosX2         , sizeof(_object7PosX2           ));         
    hashEngine.Update(_object7PosX1         , sizeof(_object7PosX1           ));         
    hashEngine.Update(_object7SpeedY        , sizeof(_object7SpeedY          ));         
    hashEngine.Update(_object7SpeedX        , sizeof(_object7SpeedX          ));         
    hashEngine.Update(_object7Stand         , sizeof(_object7Stand           ));         
    hashEngine.Update(_object8Status        , sizeof(_object8Status          ));         
    hashEngine.Update(_object8PosY2         , sizeof(_object8PosY2           ));         
    hashEngine.Update(_object8PosY1         , sizeof(_object8PosY1           ));         
    hashEngine.Update(_object8PosX2         , sizeof(_object8PosX2           ));         
    hashEngine.Update(_object8PosX1         , sizeof(_object8PosX1           ));         
    hashEngine.Update(_object8SpeedY        , sizeof(_object8SpeedY          ));         
    hashEngine.Update(_object8SpeedX        , sizeof(_object8SpeedX          ));         
    hashEngine.Update(_object8Stand         , sizeof(_object8Stand           ));         
    hashEngine.Update(_object9Status        , sizeof(_object9Status          ));         
    hashEngine.Update(_object9PosY2         , sizeof(_object9PosY2           ));         
    hashEngine.Update(_object9PosY1         , sizeof(_object9PosY1           ));         
    hashEngine.Update(_object9PosX2         , sizeof(_object9PosX2           ));         
    hashEngine.Update(_object9PosX1         , sizeof(_object9PosX1           ));         
    hashEngine.Update(_object9SpeedY        , sizeof(_object9SpeedY          ));         
    hashEngine.Update(_object9SpeedX        , sizeof(_object9SpeedX          ));         
    hashEngine.Update(_object9Stand         , sizeof(_object9Stand           ));         
    hashEngine.Update(_respawnTimer         , sizeof(_respawnTimer           ));         
    // hashEngine.Update(_globaltimer1         , sizeof(_globaltimer1           ));         
    // hashEngine.Update(_globaltimer2         , sizeof(_globaltimer2           ));         
    // hashEngine.Update(_playerInputs1        , sizeof(_playerInputs1          ));         
    // hashEngine.Update(_playerInputs2        , sizeof(_playerInputs2          ));         
    // hashEngine.Update(_playerButtons1       , sizeof(_playerButtons1         ));         
    // hashEngine.Update(_playerButtons2       , sizeof(_playerButtons2         ));         
    hashEngine.Update(_PPUScrollY           , sizeof(_PPUScrollY             ));         
    hashEngine.Update(_PPUScrollX           , sizeof(_PPUScrollX             ));         
    hashEngine.Update(_PPUMaskMirror        , sizeof(_PPUMaskMirror          ));         
    hashEngine.Update(_playerLives          , sizeof(_playerLives            ));         
    hashEngine.Update(_secretItemsCollected1, sizeof(_secretItemsCollected1  ));         
    hashEngine.Update(_secretItemsCollected2, sizeof(_secretItemsCollected2  ));         
    hashEngine.Update(_secretItemsCollected3, sizeof(_secretItemsCollected3  ));         
    hashEngine.Update(_secretItemsCollected4, sizeof(_secretItemsCollected4  ));         
    hashEngine.Update(_secretItemsCollected5, sizeof(_secretItemsCollected5  ));         
    hashEngine.Update(_secretItemsCollected6, sizeof(_secretItemsCollected6  ));         
    hashEngine.Update(_playerAnimationState , sizeof(_playerAnimationState   ));         
    hashEngine.Update(_playerDirection      , sizeof(_playerDirection        ));         
    hashEngine.Update(_playerHitboxY2       , sizeof(_playerHitboxY2         ));         
    hashEngine.Update(_playerHitboxY1       , sizeof(_playerHitboxY1         ));         
    hashEngine.Update(_playerHitboxX2       , sizeof(_playerHitboxX2         ));         
    hashEngine.Update(_playerHitboxX1       , sizeof(_playerHitboxX1         ));         
    hashEngine.Update(_playerHealth         , sizeof(_playerHealth           ));         
    hashEngine.Update(_playerObjectType     , sizeof(_playerObjectType       ));         
    hashEngine.Update(_playerIsStood        , sizeof(_playerIsStood          ));         
    hashEngine.Update(_playerIsStanding     , sizeof(_playerIsStanding       ));         
    hashEngine.Update(_playerOnWall         , sizeof(_playerOnWall           ));         
    hashEngine.Update(_playerTimer1         , sizeof(_playerTimer1           ));         
    hashEngine.Update(_playerTimer2         , sizeof(_playerTimer2           ));         
    hashEngine.Update(_playerDropItem       , sizeof(_playerDropItem         ));         
    hashEngine.Update(_weaponAnimationState , sizeof(_weaponAnimationState   ));         
    hashEngine.Update(_weaponDirection      , sizeof(_weaponDirection        ));         
    hashEngine.Update(_weaponHitboxY2       , sizeof(_weaponHitboxY2         ));         
    hashEngine.Update(_weaponHitboxY1       , sizeof(_weaponHitboxY1         ));         
    hashEngine.Update(_weaponHitboxX2       , sizeof(_weaponHitboxX2         ));         
    hashEngine.Update(_weaponHitboxX1       , sizeof(_weaponHitboxX1         ));         
    hashEngine.Update(_weaponHealth         , sizeof(_weaponHealth           ));         
    hashEngine.Update(_weaponObjectType     , sizeof(_weaponObjectType       ));         
    hashEngine.Update(_weaponIsStood        , sizeof(_weaponIsStood          ));         
    hashEngine.Update(_weaponIsStanding     , sizeof(_weaponIsStanding       ));         
    hashEngine.Update(_weaponOnWall         , sizeof(_weaponOnWall           ));         
    hashEngine.Update(_weaponTimer1         , sizeof(_weaponTimer1           ));         
    hashEngine.Update(_weaponTimer2         , sizeof(_weaponTimer2           ));         
    hashEngine.Update(_weaponDropItem       , sizeof(_weaponDropItem         ));         
    hashEngine.Update(_object1AnimationState, sizeof(_object1AnimationState  ));         
    hashEngine.Update(_object1Direction     , sizeof(_object1Direction       ));         
    hashEngine.Update(_object1HitboxY2      , sizeof(_object1HitboxY2        ));         
    hashEngine.Update(_object1HitboxY1      , sizeof(_object1HitboxY1        ));         
    hashEngine.Update(_object1HitboxX2      , sizeof(_object1HitboxX2        ));         
    hashEngine.Update(_object1HitboxX1      , sizeof(_object1HitboxX1        ));         
    hashEngine.Update(_object1Health        , sizeof(_object1Health          ));         
    hashEngine.Update(_object1ObjectType    , sizeof(_object1ObjectType      ));         
    hashEngine.Update(_object1IsStood       , sizeof(_object1IsStood         ));         
    hashEngine.Update(_object1IsStanding    , sizeof(_object1IsStanding      ));         
    hashEngine.Update(_object1OnWall        , sizeof(_object1OnWall          ));         
    hashEngine.Update(_object1Timer1        , sizeof(_object1Timer1          ));         
    hashEngine.Update(_object1Timer2        , sizeof(_object1Timer2          ));         
    hashEngine.Update(_object1DropItem      , sizeof(_object1DropItem        ));         
    hashEngine.Update(_object2AnimationState, sizeof(_object2AnimationState  ));         
    hashEngine.Update(_object2Direction     , sizeof(_object2Direction       ));         
    hashEngine.Update(_object2HitboxY2      , sizeof(_object2HitboxY2        ));         
    hashEngine.Update(_object2HitboxY1      , sizeof(_object2HitboxY1        ));         
    hashEngine.Update(_object2HitboxX2      , sizeof(_object2HitboxX2        ));         
    hashEngine.Update(_object2HitboxX1      , sizeof(_object2HitboxX1        ));         
    hashEngine.Update(_object2Health        , sizeof(_object2Health          ));         
    hashEngine.Update(_object2ObjectType    , sizeof(_object2ObjectType      ));         
    hashEngine.Update(_object2IsStood       , sizeof(_object2IsStood         ));         
    hashEngine.Update(_object2IsStanding    , sizeof(_object2IsStanding      ));         
    hashEngine.Update(_object2OnWall        , sizeof(_object2OnWall          ));         
    hashEngine.Update(_object2Timer1        , sizeof(_object2Timer1          ));         
    hashEngine.Update(_object2Timer2        , sizeof(_object2Timer2          ));         
    hashEngine.Update(_object2DropItem      , sizeof(_object2DropItem        ));         
    hashEngine.Update(_object3AnimationState, sizeof(_object3AnimationState  ));         
    hashEngine.Update(_object3Direction     , sizeof(_object3Direction       ));         
    hashEngine.Update(_object3HitboxY2      , sizeof(_object3HitboxY2        ));         
    hashEngine.Update(_object3HitboxY1      , sizeof(_object3HitboxY1        ));         
    hashEngine.Update(_object3HitboxX2      , sizeof(_object3HitboxX2        ));         
    hashEngine.Update(_object3HitboxX1      , sizeof(_object3HitboxX1        ));         
    hashEngine.Update(_object3Health        , sizeof(_object3Health          ));         
    hashEngine.Update(_object3ObjectType    , sizeof(_object3ObjectType      ));         
    hashEngine.Update(_object3IsStood       , sizeof(_object3IsStood         ));         
    hashEngine.Update(_object3IsStanding    , sizeof(_object3IsStanding      ));         
    hashEngine.Update(_object3OnWall        , sizeof(_object3OnWall          ));         
    hashEngine.Update(_object3Timer1        , sizeof(_object3Timer1          ));         
    hashEngine.Update(_object3Timer2        , sizeof(_object3Timer2          ));         
    hashEngine.Update(_object3DropItem      , sizeof(_object3DropItem        ));         
    hashEngine.Update(_object4AnimationState, sizeof(_object4AnimationState  ));         
    hashEngine.Update(_object4Direction     , sizeof(_object4Direction       ));         
    hashEngine.Update(_object4HitboxY2      , sizeof(_object4HitboxY2        ));         
    hashEngine.Update(_object4HitboxY1      , sizeof(_object4HitboxY1        ));         
    hashEngine.Update(_object4HitboxX2      , sizeof(_object4HitboxX2        ));         
    hashEngine.Update(_object4HitboxX1      , sizeof(_object4HitboxX1        ));         
    hashEngine.Update(_object4Health        , sizeof(_object4Health          ));         
    hashEngine.Update(_object4ObjectType    , sizeof(_object4ObjectType      ));         
    hashEngine.Update(_object4IsStood       , sizeof(_object4IsStood         ));         
    hashEngine.Update(_object4IsStanding    , sizeof(_object4IsStanding      ));         
    hashEngine.Update(_object4OnWall        , sizeof(_object4OnWall          ));         
    hashEngine.Update(_object4Timer1        , sizeof(_object4Timer1          ));         
    hashEngine.Update(_object4Timer2        , sizeof(_object4Timer2          ));         
    hashEngine.Update(_object4DropItem      , sizeof(_object4DropItem        ));         
    hashEngine.Update(_object5AnimationState, sizeof(_object5AnimationState  ));         
    hashEngine.Update(_object5Direction     , sizeof(_object5Direction       ));         
    hashEngine.Update(_object5HitboxY2      , sizeof(_object5HitboxY2        ));         
    hashEngine.Update(_object5HitboxY1      , sizeof(_object5HitboxY1        ));         
    hashEngine.Update(_object5HitboxX2      , sizeof(_object5HitboxX2        ));         
    hashEngine.Update(_object5HitboxX1      , sizeof(_object5HitboxX1        ));         
    hashEngine.Update(_object5Health        , sizeof(_object5Health          ));         
    hashEngine.Update(_object5ObjectType    , sizeof(_object5ObjectType      ));         
    hashEngine.Update(_object5IsStood       , sizeof(_object5IsStood         ));         
    hashEngine.Update(_object5IsStanding    , sizeof(_object5IsStanding      ));         
    hashEngine.Update(_object5OnWall        , sizeof(_object5OnWall          ));         
    hashEngine.Update(_object5Timer1        , sizeof(_object5Timer1          ));         
    hashEngine.Update(_object5Timer2        , sizeof(_object5Timer2          ));         
    hashEngine.Update(_object5DropItem      , sizeof(_object5DropItem        ));         
    hashEngine.Update(_object6AnimationState, sizeof(_object6AnimationState  ));         
    hashEngine.Update(_object6Direction     , sizeof(_object6Direction       ));         
    hashEngine.Update(_object6HitboxY2      , sizeof(_object6HitboxY2        ));         
    hashEngine.Update(_object6HitboxY1      , sizeof(_object6HitboxY1        ));         
    hashEngine.Update(_object6HitboxX2      , sizeof(_object6HitboxX2        ));         
    hashEngine.Update(_object6HitboxX1      , sizeof(_object6HitboxX1        ));         
    hashEngine.Update(_object6Health        , sizeof(_object6Health          ));         
    hashEngine.Update(_object6ObjectType    , sizeof(_object6ObjectType      ));         
    hashEngine.Update(_object6IsStood       , sizeof(_object6IsStood         ));         
    hashEngine.Update(_object6IsStanding    , sizeof(_object6IsStanding      ));         
    hashEngine.Update(_object6OnWall        , sizeof(_object6OnWall          ));         
    hashEngine.Update(_object6Timer1        , sizeof(_object6Timer1          ));         
    hashEngine.Update(_object6Timer2        , sizeof(_object6Timer2          ));         
    hashEngine.Update(_object6DropItem      , sizeof(_object6DropItem        ));         
    hashEngine.Update(_object7AnimationState, sizeof(_object7AnimationState  ));         
    hashEngine.Update(_object7Direction     , sizeof(_object7Direction       ));         
    hashEngine.Update(_object7HitboxY2      , sizeof(_object7HitboxY2        ));         
    hashEngine.Update(_object7HitboxY1      , sizeof(_object7HitboxY1        ));         
    hashEngine.Update(_object7HitboxX2      , sizeof(_object7HitboxX2        ));         
    hashEngine.Update(_object7HitboxX1      , sizeof(_object7HitboxX1        ));         
    hashEngine.Update(_object7Health        , sizeof(_object7Health          ));         
    hashEngine.Update(_object7ObjectType    , sizeof(_object7ObjectType      ));         
    hashEngine.Update(_object7IsStood       , sizeof(_object7IsStood         ));         
    hashEngine.Update(_object7IsStanding    , sizeof(_object7IsStanding      ));         
    hashEngine.Update(_object7OnWall        , sizeof(_object7OnWall          ));         
    hashEngine.Update(_object7Timer1        , sizeof(_object7Timer1          ));         
    hashEngine.Update(_object7Timer2        , sizeof(_object7Timer2          ));         
    hashEngine.Update(_object7DropItems     , sizeof(_object7DropItems       ));         
    hashEngine.Update(_object8AnimationState, sizeof(_object8AnimationState  ));         
    hashEngine.Update(_object8Direction     , sizeof(_object8Direction       ));         
    hashEngine.Update(_object8HitboxY2      , sizeof(_object8HitboxY2        ));         
    hashEngine.Update(_object8HitboxY1      , sizeof(_object8HitboxY1        ));         
    hashEngine.Update(_object8HitboxX2      , sizeof(_object8HitboxX2        ));         
    hashEngine.Update(_object8HitboxX1      , sizeof(_object8HitboxX1        ));         
    hashEngine.Update(_object8Health        , sizeof(_object8Health          ));         
    hashEngine.Update(_object8ObjectType    , sizeof(_object8ObjectType      ));         
    hashEngine.Update(_object8IsStood       , sizeof(_object8IsStood         ));         
    hashEngine.Update(_object8IsStanding    , sizeof(_object8IsStanding      ));         
    hashEngine.Update(_object8OnWall        , sizeof(_object8OnWall          ));         
    hashEngine.Update(_object8Timer1        , sizeof(_object8Timer1          ));         
    hashEngine.Update(_object8Timer2        , sizeof(_object8Timer2          ));         
    hashEngine.Update(_object8DropItems     , sizeof(_object8DropItems       ));         
    hashEngine.Update(_star5PosY2           , sizeof(_star5PosY2             ));         
    hashEngine.Update(_star5PosY1           , sizeof(_star5PosY1             ));         
    hashEngine.Update(_star5PosX1           , sizeof(_star5PosX1             ));         
    hashEngine.Update(_star5PosX2           , sizeof(_star5PosX2             ));         
    hashEngine.Update(_star5Status          , sizeof(_star5Status            ));         
    hashEngine.Update(_star4PosY2           , sizeof(_star4PosY2             ));         
    hashEngine.Update(_star4PosY1           , sizeof(_star4PosY1             ));         
    hashEngine.Update(_star4PosX1           , sizeof(_star4PosX1             ));         
    hashEngine.Update(_star4PosX2           , sizeof(_star4PosX2             ));         
    hashEngine.Update(_star4Status          , sizeof(_star4Status            ));         
    hashEngine.Update(_star3PosY2           , sizeof(_star3PosY2             ));         
    hashEngine.Update(_star3PosY1           , sizeof(_star3PosY1             ));         
    hashEngine.Update(_star3PosX1           , sizeof(_star3PosX1             ));         
    hashEngine.Update(_star3PosX2           , sizeof(_star3PosX2             ));         
    hashEngine.Update(_star3Status          , sizeof(_star3Status            ));         
    hashEngine.Update(_star2PosY2           , sizeof(_star2PosY2             ));         
    hashEngine.Update(_star2PosY1           , sizeof(_star2PosY1             ));         
    hashEngine.Update(_star2PosX1           , sizeof(_star2PosX1             ));         
    hashEngine.Update(_star2PosX2           , sizeof(_star2PosX2             ));         
    hashEngine.Update(_star2Status          , sizeof(_star2Status            ));         
    hashEngine.Update(_star1PosY2           , sizeof(_star1PosY2             ));         
    hashEngine.Update(_star1PosY1           , sizeof(_star1PosY1             ));         
    hashEngine.Update(_star1PosX1           , sizeof(_star1PosX1             ));         
    hashEngine.Update(_star1PosX2           , sizeof(_star1PosX2             ));         
    hashEngine.Update(_star1Status          , sizeof(_star1Status            ));         
  }

  // Updating derivative values after updating the internal state
  __INLINE__ void stateUpdatePostHook() override
  {
   _playerPosX = (float)*_playerPosX1 + (float)*_playerPosX2 / 256.0;
   _playerPosY = (float)*_playerPosY1 + (float)*_playerPosY2 / 256.0;

   _starPosX = (float)*_star1PosX2 + (float)*_star1PosX1 / 256.0;
   _starPosY = (float)*_star1PosY1 + (float)*_star1PosY2 / 256.0;
  }

  __INLINE__ void ruleUpdatePreHook() override 
  {
    _playerXMagnet.intensity = 0.0; 
    _playerXMagnet.pos = 0.0; 
    _playerYMagnet.intensity = 0.0; 
    _playerYMagnet.pos = 0.0; 

    _traceMagnet.playerIntensity = 0.0;
    _traceMagnet.starIntensity = 0.0;
    _traceMagnet.offset = 0;
  }

  __INLINE__ void ruleUpdatePostHook() override
  {
    _playerDistanceToPointX = std::abs(_playerXMagnet.pos - _playerPosX);
    _playerDistanceToPointY = std::abs(_playerYMagnet.pos - _playerPosY);

    // Updating trace stuff
    if (_useTrace == true)
    {
      _traceStep = (uint16_t) std::max(std::min( (int)_currentStep + _traceMagnet.offset, (int) _trace.size() - 1), 0);
      
      _traceTargetPlayerPosX = _trace[_traceStep].playerPosX;
      _traceTargetPlayerPosY = _trace[_traceStep].playerPosY;
      _traceTargetStarPosX   = _trace[_traceStep].starPosX;
      _traceTargetStarPosY   = _trace[_traceStep].starPosY;

      // Updating distance to trace point
      _traceDistancePlayerPosX = std::abs(_traceTargetPlayerPosX - _playerPosX);
      _traceDistancePlayerPosY = std::abs(_traceTargetPlayerPosY - _playerPosY);
      _traceDistanceStarPosX   = std::abs(_traceTargetStarPosX   - _starPosX);
      _traceDistanceStarPosY   = std::abs(_traceTargetStarPosY   - _starPosY);

      _traceDistancePlayer  = sqrtf(
        _traceDistancePlayerPosX * _traceDistancePlayerPosX +
        _traceDistancePlayerPosY * _traceDistancePlayerPosY
      );

      _traceDistanceStar  = sqrtf(
        _traceDistanceStarPosX * _traceDistanceStarPosX +
        _traceDistanceStarPosY * _traceDistanceStarPosY 
      );
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

    // Distance to point magnet
    reward += -1.0 * _playerXMagnet.intensity * _playerDistanceToPointX;
    reward += -1.0 * _playerYMagnet.intensity * _playerDistanceToPointY;

    // If trace is used, compute its magnet's effect
    if (_useTrace == true)  reward += -1.0 * _traceMagnet.playerIntensity * _traceDistancePlayer;
    if (_useTrace == true)  reward += -1.0 * _traceMagnet.starIntensity * _traceDistanceStar;


    // Returning reward
    return reward;
  }

  void printInfoImpl() const override
  {
    jaffarCommon::logger::log("[J+]  + Player Pos X:                       %.3f\n", _playerPosX);
    jaffarCommon::logger::log("[J+]  + Player Pos Y:                       %.3f\n", _playerPosY);
    jaffarCommon::logger::log("[J+]  + Star Pos X:                         %.3f\n", _starPosX);
    jaffarCommon::logger::log("[J+]  + Star Pos Y:                         %.3f\n", _starPosY);

    if (std::abs(_playerXMagnet.intensity) > 0.0f)
    {
      jaffarCommon::logger::log("[J+]  + Point X Magnet                           Intensity: %.5f, Pos: %3.3f\n", _playerXMagnet.intensity, _playerXMagnet.pos);
      jaffarCommon::logger::log("[J+]    + Distance                               %3.3f\n", _playerDistanceToPointX);
    }

    if (std::abs(_playerYMagnet.intensity) > 0.0f)
    {
      jaffarCommon::logger::log("[J+]  + Point Y Magnet                           Intensity: %.5f, Pos: %3.3f\n", _playerYMagnet.intensity, _playerYMagnet.pos);
      jaffarCommon::logger::log("[J+]    + Distance                               %3.3f\n", _playerDistanceToPointY);
    }

    if (_useTrace == true)
    {
      if (std::abs(_traceMagnet.playerIntensity) > 0.0f)
      {
        jaffarCommon::logger::log("[J+]  + Player Trace Magnet                      Intensity: %.5f, Step: %u, X: %3.3f, Y: %3.3f\n",
           _traceMagnet.playerIntensity,
           _traceStep,
           _traceTargetPlayerPosX,
           _traceTargetPlayerPosY);

        jaffarCommon::logger::log("[J+]    + Distance Player Pos X                  %3.3f\n", _traceDistancePlayerPosX);
        jaffarCommon::logger::log("[J+]    + Distance Player Pos Y                  %3.3f\n", _traceDistancePlayerPosY);
        jaffarCommon::logger::log("[J+]    + Player Total Distance                  %3.3f\n", _traceDistancePlayer);
      }

      if (std::abs(_traceMagnet.starIntensity) > 0.0f)
      {
        jaffarCommon::logger::log("[J+]  + Star Trace Magnet                       Intensity: %.5f, Step: %u, X: %3.3f, Y: %3.3f\n",
           _traceMagnet.starIntensity,
           _traceStep,
           _traceTargetStarPosX,
           _traceTargetStarPosY);

        jaffarCommon::logger::log("[J+]    + Distance Star Pos X                    %3.3f\n", _traceDistanceStarPosX);
        jaffarCommon::logger::log("[J+]    + Distance Star Pos Y                    %3.3f\n", _traceDistanceStarPosY);
        jaffarCommon::logger::log("[J+]    + Star Total Distance                    %3.3f\n", _traceDistanceStar);
      }
    }
  }

  bool parseRuleActionImpl(Rule& rule, const std::string& actionType, const nlohmann::json& actionJs) override
  {
    bool recognizedActionType = false;

    if (actionType == "Set Point X Magnet")
    {
      auto intensity = jaffarCommon::json::getNumber<float>(actionJs, "Intensity");
      auto pos       = jaffarCommon::json::getNumber<float>(actionJs, "Pos");
      rule.addAction([=, this]() { this->_playerXMagnet = pointMagnet_t{.intensity = intensity, .pos = pos}; });
      recognizedActionType = true;
    }

    if (actionType == "Set Point Y Magnet")
    {
      auto intensity = jaffarCommon::json::getNumber<float>(actionJs, "Intensity");
      auto pos       = jaffarCommon::json::getNumber<float>(actionJs, "Pos");
      rule.addAction([=, this]() { this->_playerYMagnet = pointMagnet_t{.intensity = intensity, .pos = pos}; });
      recognizedActionType = true;
    }

    if (actionType == "Set Trace Magnet")
    {
      if (_useTrace == false) JAFFAR_THROW_LOGIC("Specified Trace Magnet, but no trace file was provided.");
      auto playerIntensity = jaffarCommon::json::getNumber<float>(actionJs, "Player Intensity");
      auto starIntensity = jaffarCommon::json::getNumber<float>(actionJs, "Star Intensity");
      auto offset    = jaffarCommon::json::getNumber<int>(actionJs, "Offset");
      rule.addAction([=, this]() { this->_traceMagnet = traceMagnet_t{.playerIntensity = playerIntensity, .starIntensity = starIntensity, .offset = offset }; });
      recognizedActionType = true;
    }

    return recognizedActionType;
  }

  __INLINE__ jaffarCommon::hash::hash_t getStateInputHash() override
  {
    // There is no discriminating state element, so simply return a zero hash
    return jaffarCommon::hash::hash_t();
  }

  bool _isDumpingTrace = false;
  std::string _traceDumpString;

  // Datatype to describe a point magnet
  struct traceMagnet_t
  {
    float playerIntensity = 0.0; // How strong the magnet is
    float starIntensity = 0.0; // How strong the magnet is
    int offset      = 0; // Which entry (step) to look at wrt the current emulation step
  };

  traceMagnet_t _traceMagnet;

  __INLINE__ void playerPrintCommands() const override
  {
    jaffarCommon::logger::log("[J+] t: start/stop trace dumping (%s)\n", _isDumpingTrace ? "On" : "Off");
  };

  __INLINE__ bool playerParseCommand(const int command)
  {
    // If storing a trace, do it here
    if (_isDumpingTrace == true) _traceDumpString +=
     std::to_string(_playerPosX) +
     std::string(" ") +
     std::to_string(_playerPosY) +
     std::string(" ") +
     std::to_string(_starPosX) +
     std::string(" ") +
     std::to_string(_starPosY) +
     std::string("\n");

     if (command == 't')
     {
      if (_isDumpingTrace == false)
      {
        _isDumpingTrace = true;
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

  uint8_t* _screenX1;
  uint8_t* _screenX2;
  uint8_t* _screenX3;
  uint8_t* _screenY;
  uint8_t* _objectIterator;
  uint8_t* _currentLevel;
  uint8_t* _currentCheckpoint;
  uint8_t* _enemyDataPointer1;
  uint8_t* _enemyDataPointer2;
  uint8_t* _CHRTimer;
  uint8_t* _CHRIndex;
  uint8_t* _IRQIndex;
  uint8_t* _screenUpdate;
  uint8_t* _playerMaxHP;
  uint8_t* _scoreIncrease1;
  uint8_t* _scoreIncrease2;
  uint8_t* _score1;
  uint8_t* _score2;
  uint8_t* _score3;
  uint8_t* _score4;
  uint8_t* _score5;
  uint8_t* _score6;
  uint8_t* _score7;
  uint8_t* _enemyKills;
  uint8_t* _item1;
  uint8_t* _item2;
  uint8_t* _item3;
  uint8_t* _itemAmount;
  uint8_t* _PRGBank1;
  uint8_t* _PRGBank2;
  uint8_t* _screenLocked;
  uint8_t* _playerJumping;
  uint8_t* _cutsceneState;
  uint8_t* _musicSong;
  uint8_t* _yumetaroState;
  uint8_t* _skipMap;
  uint8_t* _weaponUseTimer;
  uint8_t* _musicState;
  uint8_t* _playerStatus;
  uint8_t* _playerPosY2;
  uint8_t* _playerPosY1;
  uint8_t* _playerPosX2;
  uint8_t* _playerPosX1;
  uint8_t* _playerSpeedY;
  uint8_t* _playerSpeedX;
  uint8_t* _playerStand;
  uint8_t* _weaponStatus;
  uint8_t* _weaponPosY2;
  uint8_t* _weaponPosY1;
  uint8_t* _weaponPosX2;
  uint8_t* _weaponPosX1;
  uint8_t* _weaponSpeedY;
  uint8_t* _weaponSpeedX;
  uint8_t* _weaponStand;
  uint8_t* _object1Status;
  uint8_t* _object1PosY2;
  uint8_t* _object1PosY1;
  uint8_t* _object1PosX2;
  uint8_t* _object1PosX1;
  uint8_t* _object1SpeedY;
  uint8_t* _object1SpeedX;
  uint8_t* _object1Stand;
  uint8_t* _object2Status;
  uint8_t* _object2PosY2;
  uint8_t* _object2PosY1;
  uint8_t* _object2PosX2;
  uint8_t* _object2PosX1;
  uint8_t* _object2SpeedY;
  uint8_t* _object2SpeedX;
  uint8_t* _object2Stand;
  uint8_t* _object3Status;
  uint8_t* _object3PosY2;
  uint8_t* _object3PosY1;
  uint8_t* _object3PosX2;
  uint8_t* _object3PosX1;
  uint8_t* _object3SpeedY;
  uint8_t* _object3SpeedX;
  uint8_t* _object3Stand;
  uint8_t* _object4Status;
  uint8_t* _object4PosY2;
  uint8_t* _object4PosY1;
  uint8_t* _object4PosX2;
  uint8_t* _object4PosX1;
  uint8_t* _object4SpeedY;
  uint8_t* _object4SpeedX;
  uint8_t* _object4Stand;
  uint8_t* _object5Status;
  uint8_t* _object5PosY2;
  uint8_t* _object5PosY1;
  uint8_t* _object5PosX2;
  uint8_t* _object5PosX1;
  uint8_t* _object5SpeedY;
  uint8_t* _object5SpeedX;
  uint8_t* _object5Stand;
  uint8_t* _object6Status;
  uint8_t* _object6PosY2;
  uint8_t* _object6PosY1;
  uint8_t* _object6PosX2;
  uint8_t* _object6PosX1;
  uint8_t* _object6SpeedY;
  uint8_t* _object6SpeedX;
  uint8_t* _object6Stand;
  uint8_t* _object7Status;
  uint8_t* _object7PosY2;
  uint8_t* _object7PosY1;
  uint8_t* _object7PosX2;
  uint8_t* _object7PosX1;
  uint8_t* _object7SpeedY;
  uint8_t* _object7SpeedX;
  uint8_t* _object7Stand;
  uint8_t* _object8Status;
  uint8_t* _object8PosY2;
  uint8_t* _object8PosY1;
  uint8_t* _object8PosX2;
  uint8_t* _object8PosX1;
  uint8_t* _object8SpeedY;
  uint8_t* _object8SpeedX;
  uint8_t* _object8Stand;
  uint8_t* _object9Status;
  uint8_t* _object9PosY2;
  uint8_t* _object9PosY1;
  uint8_t* _object9PosX2;
  uint8_t* _object9PosX1;
  uint8_t* _object9SpeedY;
  uint8_t* _object9SpeedX;
  uint8_t* _object9Stand;
  uint8_t* _respawnTimer;
  uint8_t* _globaltimer1;
  uint8_t* _globaltimer2;
  uint8_t* _playerInputs1;
  uint8_t* _playerInputs2;
  uint8_t* _playerButtons1;
  uint8_t* _playerButtons2;
  uint8_t* _PPUScrollY;
  uint8_t* _PPUScrollX;
  uint8_t* _PPUMaskMirror;
  uint8_t* _playerLives;
  uint8_t* _secretItemsCollected1;
  uint8_t* _secretItemsCollected2;
  uint8_t* _secretItemsCollected3;
  uint8_t* _secretItemsCollected4;
  uint8_t* _secretItemsCollected5;
  uint8_t* _secretItemsCollected6;
  uint8_t* _playerAnimationState;
  uint8_t* _playerDirection;
  uint8_t* _playerHitboxY2;
  uint8_t* _playerHitboxY1;
  uint8_t* _playerHitboxX2;
  uint8_t* _playerHitboxX1;
  uint8_t* _playerHealth;
  uint8_t* _playerObjectType;
  uint8_t* _playerIsStood;
  uint8_t* _playerIsStanding;
  uint8_t* _playerOnWall;
  uint8_t* _playerTimer1;
  uint8_t* _playerTimer2;
  uint8_t* _playerDropItem;
  uint8_t* _weaponAnimationState;
  uint8_t* _weaponDirection;
  uint8_t* _weaponHitboxY2;
  uint8_t* _weaponHitboxY1;
  uint8_t* _weaponHitboxX2;
  uint8_t* _weaponHitboxX1;
  uint8_t* _weaponHealth;
  uint8_t* _weaponObjectType;
  uint8_t* _weaponIsStood;
  uint8_t* _weaponIsStanding;
  uint8_t* _weaponOnWall;
  uint8_t* _weaponTimer1;
  uint8_t* _weaponTimer2;
  uint8_t* _weaponDropItem;
  uint8_t* _object1AnimationState;
  uint8_t* _object1Direction;
  uint8_t* _object1HitboxY2;
  uint8_t* _object1HitboxY1;
  uint8_t* _object1HitboxX2;
  uint8_t* _object1HitboxX1;
  uint8_t* _object1Health;
  uint8_t* _object1ObjectType;
  uint8_t* _object1IsStood;
  uint8_t* _object1IsStanding;
  uint8_t* _object1OnWall;
  uint8_t* _object1Timer1;
  uint8_t* _object1Timer2;
  uint8_t* _object1DropItem;
  uint8_t* _object2AnimationState;
  uint8_t* _object2Direction;
  uint8_t* _object2HitboxY2;
  uint8_t* _object2HitboxY1;
  uint8_t* _object2HitboxX2;
  uint8_t* _object2HitboxX1;
  uint8_t* _object2Health;
  uint8_t* _object2ObjectType;
  uint8_t* _object2IsStood;
  uint8_t* _object2IsStanding;
  uint8_t* _object2OnWall;
  uint8_t* _object2Timer1;
  uint8_t* _object2Timer2;
  uint8_t* _object2DropItem;
  uint8_t* _object3AnimationState;
  uint8_t* _object3Direction;
  uint8_t* _object3HitboxY2;
  uint8_t* _object3HitboxY1;
  uint8_t* _object3HitboxX2;
  uint8_t* _object3HitboxX1;
  uint8_t* _object3Health;
  uint8_t* _object3ObjectType;
  uint8_t* _object3IsStood;
  uint8_t* _object3IsStanding;
  uint8_t* _object3OnWall;
  uint8_t* _object3Timer1;
  uint8_t* _object3Timer2;
  uint8_t* _object3DropItem;
  uint8_t* _object4AnimationState;
  uint8_t* _object4Direction;
  uint8_t* _object4HitboxY2;
  uint8_t* _object4HitboxY1;
  uint8_t* _object4HitboxX2;
  uint8_t* _object4HitboxX1;
  uint8_t* _object4Health;
  uint8_t* _object4ObjectType;
  uint8_t* _object4IsStood;
  uint8_t* _object4IsStanding;
  uint8_t* _object4OnWall;
  uint8_t* _object4Timer1;
  uint8_t* _object4Timer2;
  uint8_t* _object4DropItem;
  uint8_t* _object5AnimationState;
  uint8_t* _object5Direction;
  uint8_t* _object5HitboxY2;
  uint8_t* _object5HitboxY1;
  uint8_t* _object5HitboxX2;
  uint8_t* _object5HitboxX1;
  uint8_t* _object5Health;
  uint8_t* _object5ObjectType;
  uint8_t* _object5IsStood;
  uint8_t* _object5IsStanding;
  uint8_t* _object5OnWall;
  uint8_t* _object5Timer1;
  uint8_t* _object5Timer2;
  uint8_t* _object5DropItem;
  uint8_t* _object6AnimationState;
  uint8_t* _object6Direction;
  uint8_t* _object6HitboxY2;
  uint8_t* _object6HitboxY1;
  uint8_t* _object6HitboxX2;
  uint8_t* _object6HitboxX1;
  uint8_t* _object6Health;
  uint8_t* _object6ObjectType;
  uint8_t* _object6IsStood;
  uint8_t* _object6IsStanding;
  uint8_t* _object6OnWall;
  uint8_t* _object6Timer1;
  uint8_t* _object6Timer2;
  uint8_t* _object6DropItem;
  uint8_t* _object7AnimationState;
  uint8_t* _object7Direction;
  uint8_t* _object7HitboxY2;
  uint8_t* _object7HitboxY1;
  uint8_t* _object7HitboxX2;
  uint8_t* _object7HitboxX1;
  uint8_t* _object7Health;
  uint8_t* _object7ObjectType;
  uint8_t* _object7IsStood;
  uint8_t* _object7IsStanding;
  uint8_t* _object7OnWall;
  uint8_t* _object7Timer1;
  uint8_t* _object7Timer2;
  uint8_t* _object7DropItems;
  uint8_t* _object8AnimationState;
  uint8_t* _object8Direction;
  uint8_t* _object8HitboxY2;
  uint8_t* _object8HitboxY1;
  uint8_t* _object8HitboxX2;
  uint8_t* _object8HitboxX1;
  uint8_t* _object8Health;
  uint8_t* _object8ObjectType;
  uint8_t* _object8IsStood;
  uint8_t* _object8IsStanding;
  uint8_t* _object8OnWall;
  uint8_t* _object8Timer1;
  uint8_t* _object8Timer2;
  uint8_t* _object8DropItems;
  uint8_t* _star5PosY2;
  uint8_t* _star5PosY1;
  uint8_t* _star5PosX1;
  uint8_t* _star5PosX2;
  uint8_t* _star5Status;
  uint8_t* _star4PosY2;
  uint8_t* _star4PosY1;
  uint8_t* _star4PosX1;
  uint8_t* _star4PosX2;
  uint8_t* _star4Status;
  uint8_t* _star3PosY2;
  uint8_t* _star3PosY1;
  uint8_t* _star3PosX1;
  uint8_t* _star3PosX2;
  uint8_t* _star3Status;
  uint8_t* _star2PosY2;
  uint8_t* _star2PosY1;
  uint8_t* _star2PosX1;
  uint8_t* _star2PosX2;
  uint8_t* _star2Status;
  uint8_t* _star1PosY2;
  uint8_t* _star1PosY1;
  uint8_t* _star1PosX1;
  uint8_t* _star1PosX2;
  uint8_t* _star1Status;

  // Null input index to remember the last valid input
  InputSet::inputIndex_t _nullInputIdx;

  // Pointer to emulator's low memory storage
  uint8_t* _lowMem;
  uint16_t _currentStep;
  uint16_t _lastInputStep;

  // Derivative Values
  float _playerPosX;
  float _playerPosY;
  float _starPosX;
  float _starPosY;

  // Datatype to describe a point magnet
  struct pointMagnet_t
  {
    float intensity = 0.0; // How strong the magnet is
    float pos       = 0.0; // What is the point of attraction
  };

  pointMagnet_t _playerXMagnet; 
  pointMagnet_t _playerYMagnet; 
  float _playerDistanceToPointX;
  float _playerDistanceToPointY;

    // Whether we use a trace
  bool _useTrace = false;

  // Location of the trace file
  std::string _traceFilePath;

  // Trace contents
  std::vector<traceEntry_t> _trace;

  // Current trace target
  uint16_t _traceStep;
  float _traceTargetPlayerPosX;
  float _traceTargetPlayerPosY;
  float _traceTargetStarPosX;
  float _traceTargetStarPosY;

  float _traceDistancePlayerPosX;
  float _traceDistancePlayerPosY;
  float _traceDistancePlayer;

  float _traceDistanceStarPosX;
  float _traceDistanceStarPosY;
  float _traceDistanceStar;

};

} // namespace nes

} // namespace games

} // namespace jaffarPlus