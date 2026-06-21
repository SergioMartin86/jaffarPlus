#pragma once

/**
 * @file inputHistoryNone.hpp
 * @brief Input-history strategy that records nothing but the step counter. Smallest possible per-state
 *        footprint; solutions cannot be reconstructed. Selected with `{"Type": "None"}`.
 */

#include "inputHistory.hpp"

namespace jaffarPlus
{

/// @brief Records only the step counter; smallest per-state footprint, but no solution reconstruction.
class InputHistoryNone final : public InputHistory
{
public:
  void   reset() override { _count = 0; }
  void   pushInput(const InputSet::inputIndex_t /*input*/) override { _count++; }
  size_t getInputCount() const override { return _count; }

  void serializeCold(jaffarCommon::serializer::Base& s) const override { s.pushContiguous(&_count, sizeof(_count)); }
  void deserializeCold(jaffarCommon::deserializer::Base& d) override { d.popContiguous(&_count, sizeof(_count)); }
  void serializeFull(jaffarCommon::serializer::Base& s) const override { serializeCold(s); }
  void deserializeFull(jaffarCommon::deserializer::Base& d) override { deserializeCold(d); }

  std::string toString(const std::map<InputSet::inputIndex_t, std::string>& /*m*/) const override { return ""; }

  size_t getColdSize() const override { return sizeof(_count); }
  size_t getFullSize() const override { return sizeof(_count); }

  void captureColdToFull(const void* cold, void* full) const override { memcpy(full, cold, sizeof(_count)); }

private:
  uint32_t _count = 0; ///< Number of inputs applied so far (the step counter).
};

} // namespace jaffarPlus
