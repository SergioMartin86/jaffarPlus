//============================================================================
//
//   SSSS    tt          lll  lll
//  SS  SS   tt           ll   ll
//  SS     tttttt  eeee   ll   ll   aaaa
//   SSSS    tt   ee  ee  ll   ll      aa
//      SS   tt   eeeeee  ll   ll   aaaaa  --  "An Atari 2600 VCS Emulator"
//  SS  SS   tt   ee      ll   ll  aa  aa
//   SSSS     ttt  eeeee llll llll  aaaaa
//
// Copyright (c) 1995-2023 by Bradford W. Mott, Stephen Anthony
// and the Stella Team
//
// See the file "License.txt" for information on usage and redistribution of
// this file, and for a DISCLAIMER OF ALL WARRANTIES.
//============================================================================

#include "KeyMap.hxx"
#include "Logger.hxx"
#include <map>


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void KeyMap::add(const Event::Type event, const Mapping& mapping)
{
  myMap[convertMod(mapping)] = event;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void KeyMap::add(const Event::Type event, const EventMode mode, const int key, const int mod)
{
  add(event, Mapping(mode, key, mod));
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void KeyMap::erase(const Mapping& mapping)
{
  myMap.erase(convertMod(mapping));
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void KeyMap::erase(const EventMode mode, const int key, const int mod)
{
  erase(Mapping(mode, key, mod));
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
Event::Type KeyMap::get(const Mapping& mapping) const
{
  Mapping m = convertMod(mapping);

  if(myModEnabled)
  {
    const auto find = myMap.find(m);
    if(find != myMap.end())
      return find->second;
  }

  // mapping not found, try without modifiers
  m.mod = static_cast<StellaMod>(0);

  const auto find = myMap.find(m);
  if(find != myMap.end())
    return find->second;

  return Event::Type::NoType;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
Event::Type KeyMap::get(const EventMode mode, const int key, const int mod) const
{
  return get(Mapping(mode, key, mod));
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
bool KeyMap::check(const Mapping& mapping) const
{
  const auto find = myMap.find(convertMod(mapping));

  return (find != myMap.end());
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
bool KeyMap::check(const EventMode mode, const int key, const int mod) const
{
  return check(Mapping(mode, key, mod));
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
string KeyMap::getDesc(const Mapping& mapping)
{
  ostringstream buf;
#if defined(BSPF_MACOS) || defined(MACOS_KEYS)
  const string mod2 = "Option";
  constexpr int MOD2 = KBDM_ALT;
  constexpr int LMOD2 = KBDM_LALT;
  constexpr int RMOD2 = KBDM_RALT;
  const string mod3 = "Cmd";
  constexpr int MOD3 = KBDM_GUI;
  constexpr int LMOD3 = KBDM_LGUI;
  constexpr int RMOD3 = KBDM_RGUI;
#else
  const string mod2 = "Windows";
  constexpr int MOD2 = KBDM_GUI;
  constexpr int LMOD2 = KBDM_LGUI;
  constexpr int RMOD2 = KBDM_RGUI;
  const string mod3 = "Alt";
  constexpr int MOD3 = KBDM_ALT;
  constexpr int LMOD3 = KBDM_LALT;
  constexpr int RMOD3 = KBDM_RALT;
#endif

  if((mapping.mod & KBDM_CTRL) == KBDM_CTRL) buf << "Ctrl";
  else if(mapping.mod & KBDM_LCTRL) buf << "Left Ctrl";
  else if(mapping.mod & KBDM_RCTRL) buf << "Right Ctrl";

  if((mapping.mod & (MOD2)) && buf.tellp()) buf << "-";
  if((mapping.mod & MOD2) == MOD2) buf << mod2;
  else if(mapping.mod & LMOD2) buf << "Left " << mod2;
  else if(mapping.mod & RMOD2) buf << "Right " << mod2;

  if((mapping.mod & (MOD3)) && buf.tellp()) buf << "-";
  if((mapping.mod & MOD3) == MOD3) buf << mod3;
  else if(mapping.mod & LMOD3) buf << "Left " << mod3;
  else if(mapping.mod & RMOD3) buf << "Right " << mod3;

  if((mapping.mod & (KBDM_SHIFT)) && buf.tellp()) buf << "-";
  if((mapping.mod & KBDM_SHIFT) == KBDM_SHIFT) buf << "Shift";
  else if(mapping.mod & KBDM_LSHIFT) buf << "Left Shift";
  else if(mapping.mod & KBDM_RSHIFT) buf << "Right Shift";

  if(buf.tellp()) buf << "+";
  buf << StellaKeyName::forKey(mapping.key);

  return buf.str();
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
string KeyMap::getDesc(const EventMode mode, const int key, const int mod)
{
  return getDesc(Mapping(mode, key, mod));
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
string KeyMap::getEventMappingDesc(const Event::Type event, const EventMode mode) const
{
  ostringstream buf;

  for (const auto& [_mapping, _event]: myMap)
  {
    if (_event == event && _mapping.mode == mode)
    {
      if(!buf.str().empty())
        buf << ", ";
      buf << getDesc(_mapping);
    }
  }
  return buf.str();
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
KeyMap::MappingArray KeyMap::getEventMapping(const Event::Type event,
                                             const EventMode mode) const
{
  MappingArray ma;

  for (const auto& [_mapping, _event]: myMap)
    if (_event == event && _mapping.mode == mode)
      ma.push_back(_mapping);

  return ma;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void KeyMap::eraseMode(const EventMode mode)
{
  for(auto item = myMap.begin(); item != myMap.end();)
    if(item->first.mode == mode) {
      const auto _item = item++;
      erase(_item->first);
    }
    else item++;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void KeyMap::eraseEvent(const Event::Type event, const EventMode mode)
{
  for(auto item = myMap.begin(); item != myMap.end();)
    if(item->second == event && item->first.mode == mode) {
      const auto _item = item++;
      erase(_item->first);
    }
    else item++;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
KeyMap::Mapping KeyMap::convertMod(const Mapping& mapping)
{
  Mapping m = mapping;

  if(m.key >= KBDK_LCTRL && m.key <= KBDK_RGUI)
    // handle solo modifier keys differently
    m.mod = KBDM_NONE;
  else
  {
    // limit to modifiers we want to support
    m.mod = static_cast<StellaMod>(m.mod & (KBDM_SHIFT | KBDM_CTRL | KBDM_ALT | KBDM_GUI));
  }

  return m;
}
