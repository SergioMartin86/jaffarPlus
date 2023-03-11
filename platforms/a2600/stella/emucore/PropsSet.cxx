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

#include <map>

#include "bspf.hxx"
#include "FSNode.hxx"
#include "Logger.hxx"
#include "DefProps.hxx"
#include "Props.hxx"
#include "PropsSet.hxx"

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
bool PropertiesSet::getMD5(string_view md5, Properties& properties,
                           bool useDefaults) const
{
  bool found = false;

  return found;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void PropertiesSet::insert(const Properties& properties, bool save)
{
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void PropertiesSet::loadPerROM(const FSNode& rom, string_view md5)
{
  Properties props;

  // Handle ROM properties, do some error checking
  // Only add to the database when necessary
  bool toInsert = false;

  // First, does this ROM have a per-ROM properties entry?
  // If so, load it into the database
  const FSNode propsNode(rom.getPathWithExt(".pro"));
  if (propsNode.exists()) {
  }

  // Next, make sure we have a valid md5 and name
  if(!getMD5(md5, props))
  {
    props.set(PropType::Cart_MD5, md5);
    toInsert = true;
  }
  if(toInsert || props.get(PropType::Cart_Name) == EmptyString)
  {
    props.set(PropType::Cart_Name, rom.getNameWithExt(""));
    toInsert = true;
  }

  // Finally, insert properties if any info was missing
  if(toInsert)
    insert(props, false);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void PropertiesSet::print() const
{
  // We only look at the external properties and the built-in ones;
  // the temp properties are ignored
  // Also, any properties entries in the external file override the built-in
  // ones
  // The easiest way to merge the lists is to create another temporary one
  // This isn't fast, but I suspect this method isn't used too often (or at all)

  // First insert all external props
  PropsList list = myExternalProps;

  // Now insert all the built-in ones
  // Note that if we try to insert a duplicate, the insertion will fail
  // This is fine, since a duplicate in the built-in list means it should
  // be overrided anyway (and insertion shouldn't be done)
  Properties properties;
  for(uInt32 i = 0; i < DEF_PROPS_SIZE; ++i)
  {
    properties.setDefaults();
    for(uInt8 p = 0; p < static_cast<uInt8>(PropType::NumTypes); ++p)
      if(DefProps[i][p][0] != 0)
        properties.set(PropType{p}, DefProps[i][p]);

    list.emplace(DefProps[i][static_cast<uInt8>(PropType::Cart_MD5)], properties);
  }

  // Now, print the resulting list
  Properties::printHeader();
  for(const auto& i: list)
    i.second.print();
}
