#pragma once

#include <nlohmann/json.hpp>
#include "logger.hpp"

namespace jaffarPlus
{

#define JSON_GET_STRING(ARG, ENTRY) jaffarPlus::jsonGetString(ARG, ENTRY);
static inline const std::string jsonGetString(const nlohmann::json& json, const std::string& entry)
{
  if (json.is_object() == false) EXIT_WITH_ERROR("[Error] JSON passed is not a key/value object. Happened when trying to obtain string entry '%s'\n", entry.c_str());
  if (json.contains(entry) == false) EXIT_WITH_ERROR("[Error] JSON contains no field called '%s'\n", entry.c_str());
  if (json[entry].is_string() == false) EXIT_WITH_ERROR("[Error] Configuration entry '%s' is not a string\n", entry.c_str());
  return json.at(entry).get<std::string>();
}

#define JSON_GET_OBJECT(ARG, ENTRY) jaffarPlus::jsonGetObject(ARG, ENTRY)
static inline const nlohmann::json& jsonGetObject(const nlohmann::json& json, const std::string& entry)
{
  if (json.is_object() == false) EXIT_WITH_ERROR("[Error] JSON passed is not a key/value object. Happened when trying to obtain string entry '%s'\n", entry.c_str());
  if (json.contains(entry) == false) EXIT_WITH_ERROR("[Error] JSON contains no field called '%s'\n", entry.c_str());
  if (json[entry].is_object() == false) EXIT_WITH_ERROR("[Error] Configuration entry '%s' is not a key/value object\n", entry.c_str());
  return json.at(entry);
}

}