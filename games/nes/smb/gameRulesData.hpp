#pragma once

// Datatype to describe a magnet
struct magnet_t {
 float intensity; // How strong the magnet is
 float max;  // What is the maximum input value to the calculation.
 float min;  // What is the maximum input value to the calculation.
};

// Struct to hold all of the mario's magnet information
struct gameRuleData_t
{
 magnet_t marioScreenOffsetMagnet;
 magnet_t screenHorizontalMagnet;
 magnet_t marioHorizontalMagnet;
 magnet_t marioVerticalMagnet;
};

