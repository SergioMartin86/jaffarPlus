#pragma once

#include <jaffarCommon/deserializers/base.hpp>
#include <jaffarCommon/hash.hpp>
#include <jaffarCommon/json.hpp>
#include <jaffarCommon/logger.hpp>
#include <jaffarCommon/serializers/base.hpp>
#include <emulator.hpp>
#include <gpgxInstance.hpp>

namespace jaffarPlus
{

namespace emulator
{

class QuickerGPGX final : public Emulator
{
  public:

  static std::string getName() { return "QuickerGPGX"; }

  // Constructor must only do configuration parsing
  QuickerGPGX(const nlohmann::json &config)
    : Emulator(config)
  {
  };

  void initializeImpl() override
  {
  }

  // State advancing function
  void advanceState(const std::string &input) override
  {
  }

  __INLINE__ void serializeState(jaffarCommon::serializer::Base &serializer) const override
  {
  };

  __INLINE__ void deserializeState(jaffarCommon::deserializer::Base &deserializer) override
  {
  };

  __INLINE__ void disableStateProperties()
  {
  }
  __INLINE__ void enableStateProperties()
  {
  }

  __INLINE__ void loadFullState(const std::string &state) override
  {
  }
  __INLINE__ void saveFullState(std::string &state) override
  {
  }

  size_t getFullStateSize() override
  {
    return 0;
  }

  __INLINE__ void printInfo() const override {}

  property_t getProperty(const std::string &propertyName) const override
  {
    return property_t();
  }

  __INLINE__ void enableStateProperty(const std::string &property) 
  {
  }

  __INLINE__ void disableStateProperty(const std::string &property)
  { 
  }

  // This function opens the video output (e.g., window)
  void initializeVideoOutput() override
  {
  }

  // This function closes the video output (e.g., window)
  void finalizeVideoOutput() override
  { 
  }

  __INLINE__ void enableRendering() override
  {
  }

  __INLINE__ void disableRendering() override
  {
  }

  __INLINE__ void updateRendererState(const size_t stepIdx, const std::string input) override
  {

  }

  __INLINE__ void serializeRendererState(jaffarCommon::serializer::Base &serializer) const override
  {
  }

  __INLINE__ void deserializeRendererState(jaffarCommon::deserializer::Base &deserializer) override
  {
  }

  __INLINE__ size_t getRendererStateSize() const
  {
    return 0;
  }

  __INLINE__ void showRender() override
  {
  }

  private:

  // Collection of state blocks to disable during engine run
  std::vector<std::string> _disabledStateProperties;
};

} // namespace emulator

} // namespace jaffarPlus