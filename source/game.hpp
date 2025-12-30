#pragma once

#include "emulator.hpp"
#include "rule.hpp"
#include <jaffarCommon/bitwise.hpp>
#include <jaffarCommon/deserializers/base.hpp>
#include <jaffarCommon/hash.hpp>
#include <jaffarCommon/json.hpp>
#include <jaffarCommon/logger.hpp>
#include <jaffarCommon/serializers/base.hpp>
#include <map>
#include <memory>
#include <set>
#include <unordered_set>
#include <utility>
#include <vector>

namespace jaffarPlus
{

class Game
{
public:
  enum stateType_t
  {
    normal = 0,
    win    = 1,
    fail   = 2
  };

  // Constructor that takes an already created emulator
  Game(std::unique_ptr<Emulator> emulator, const nlohmann::json& config) : _emulator(std::move(emulator))
  {
    // Getting emulator name (for runtime use)
    _gameName = jaffarCommon::json::getString(config, "Game Name");

    // Parsing frame rate
    _frameRate = jaffarCommon::json::getNumber<float>(config, "Frame Rate");

    // Parsing whether to bypass emulator state load/saving
    _bypassEmulatorState = jaffarCommon::json::getBoolean(config, "Bypass Emulator State");

    // Marking printable properties
    const auto& printProperties = jaffarCommon::json::getArray<std::string>(config, "Print Properties");
    for (const auto& property : printProperties) _printablePropertyNames.push_back(property);

    // Parsing hashable game properties
    const auto& hashProperties = jaffarCommon::json::getArray<std::string>(config, "Hash Properties");
    for (const auto& property : hashProperties) _hashablePropertyNames.push_back(property);

    // Storing rules JSON for later parsing
    _rulesJs = jaffarCommon::json::getArray<nlohmann::json>(config, "Rules");
  };

  void initialize()
  {
    if (_isInitialized == true) JAFFAR_THROW_LOGIC("This game instance was already initialized");

    // Initializing emulator, if not already initialized
    if (_emulator->isInitialized() == false) _emulator->initialize();

    // Getting game-specific properties
    registerGameProperties();

    // Registering printable properties
    for (const auto& property : _printablePropertyNames)
    {
      // Getting property name hash
      const auto propertyHash = jaffarCommon::hash::hashString(property);

      // Checking the property is registered
      if (_propertyMap.contains(propertyHash) == false) JAFFAR_THROW_LOGIC("Property '%s' is not registered in this game", property.c_str());

      // If so, add its pointer to the print property vector
      _propertyPrintVector.push_back(_propertyMap.at(propertyHash).get());
    }

    // Registering hashable properties
    for (const auto& property : _hashablePropertyNames)
    {
      // Getting property name hash
      const auto propertyHash = jaffarCommon::hash::hashString(property);

      // Checking the property is registered
      if (_propertyMap.contains(propertyHash) == false) JAFFAR_THROW_LOGIC("Property '%s' is not registered in this game", property.c_str());

      // If so, add its pointer to the print property vector
      _propertyHashVector.push_back(_propertyMap.at(propertyHash).get());
    }

    // Now parsing rules
    parseRules(_rulesJs);

    // Update internals pre initialization (first state update)
    stateUpdatePreHook();

    // Calling game-specific initializer
    initializeImpl();

    // Update internals post initialization
    stateUpdatePostHook();

    // Set this as initialized
    _isInitialized = true;
  }

  Game()          = delete;
  virtual ~Game() = default;

  // Function to advance state.
  __INLINE__ void advanceState(const InputSet::inputIndex_t input)
  {
    // Calling the pre-update hook
    stateUpdatePreHook();

    // Update save solution last rule id
    _saveSolutionCurrentLastRuleIdx = _saveSolutionCurrentLastRuleId;

    // Performing the requested input
    advanceStateImpl(input);

    // Calling the post-update hook
    stateUpdatePostHook();
  }

  // Serialization routine
  __INLINE__ void serializeState(jaffarCommon::serializer::Base& serializer) const
  {
    // Serializing internal emulator state
    if (_bypassEmulatorState == false) _emulator->serializeState(serializer);

    // Storage for game-specific data
    serializeStateImpl(serializer);

    // Serializing reward
    serializer.push(&_reward, sizeof(_reward));

    // Serializing checkpoint level
    serializer.push(&_checkpointLevel, sizeof(_checkpointLevel));

    // Serializing the previous last rule id that activated a save solution
    serializer.push(&_saveSolutionCurrentLastRuleIdx, sizeof(_saveSolutionCurrentLastRuleIdx));

    // Serializing the current last rule id that activated a save solution
    serializer.push(&_saveSolutionCurrentLastRuleId, sizeof(_saveSolutionCurrentLastRuleId));

    // Serializing state type
    serializer.push(&_stateType, sizeof(_stateType));

    // Serializing rule states
    serializer.push(_rulesStatus.data(), _rulesStatus.size());
  }

  // Deserialization routine
  __INLINE__ void deserializeState(jaffarCommon::deserializer::Base& deserializer)
  {
    // Calling the pre-update hook
    stateUpdatePreHook();

    // Storage for the internal emulator state
    if (_bypassEmulatorState == false) _emulator->deserializeState(deserializer);

    // Storage for game-specific data
    deserializeStateImpl(deserializer);

    // Calling the post-update hook
    stateUpdatePostHook();

    // Deserializing reward
    deserializer.pop(&_reward, sizeof(_reward));

    // Deserializing checkpoint level
    deserializer.pop(&_checkpointLevel, sizeof(_checkpointLevel));

    // Deserializing the previous last rule id that activated a save solution
    deserializer.pop(&_saveSolutionCurrentLastRuleIdx, sizeof(_saveSolutionCurrentLastRuleIdx));

    // Deserializing the last rule id that activated a save solution
    deserializer.pop(&_saveSolutionCurrentLastRuleId, sizeof(_saveSolutionCurrentLastRuleId));

    // Deserializing state type
    deserializer.pop(&_stateType, sizeof(_stateType));

    // Calling the pre-rule update hook
    ruleUpdatePreHook();

    // Deserializing rules status
    deserializer.pop(_rulesStatus.data(), _rulesStatus.size());

    // Running game specific rule actions
    runGameSpecificRuleActions();

    // Calling the post-rule update hook
    ruleUpdatePostHook();
  }

  // This function computes the hash for the current state
  __INLINE__ void computeHash(MetroHash128& hashEngine) const
  {
    // Processing hashable game properties
    for (const auto& p : _propertyHashVector) hashEngine.Update(p->getPointer(), p->getSize());

    // Processing any additional game-specific hash
    computeAdditionalHashing(hashEngine);
  }

  // Function to print
  void printInfo() const
  {
    // Getting maximum printable property name, for formatting purposes
    const size_t separatorSize   = 4;
    size_t       maximumNameSize = 0;
    for (const auto& p : _propertyPrintVector) maximumNameSize = std::max(maximumNameSize, p->getName().size());

    // Printing game state
    jaffarCommon::logger::log("[J+]  + Game State Type: ");
    if (_stateType == stateType_t::normal) jaffarCommon::logger::log("Normal");
    if (_stateType == stateType_t::win) jaffarCommon::logger::log("Win");
    if (_stateType == stateType_t::fail) jaffarCommon::logger::log("Fail");
    jaffarCommon::logger::log("\n");

    // Printing game state
    jaffarCommon::logger::log("[J+]  + Game State Reward: %f\n", _reward);

    // Printing rule status
    jaffarCommon::logger::log("[J+]  + Rule Status: ");
    for (size_t i = 0; i < _rules.size(); i++) jaffarCommon::logger::log("%d", jaffarCommon::bitwise::getBitValue(_rulesStatus.data(), i) ? 1 : 0);
    jaffarCommon::logger::log("\n");

    // Printing game properties defined in the script file
    jaffarCommon::logger::log("[J+]  + Game Properties: \n");
    for (const auto& p : _propertyPrintVector)
    {
      // Getting property name
      const auto& name = p->getName();

      // Printing property name first
      jaffarCommon::logger::log("[J+]    + '%s':", name.c_str());

      // Calculating separation spaces for this property
      const auto propertySeparatorSize = separatorSize + maximumNameSize - name.size();

      // Printing separator spaces
      for (size_t i = 0; i < propertySeparatorSize; i++) jaffarCommon::logger::log(" ");

      // Then printing separator spaces
      if (p->getDatatype() == Property::datatype_t::dt_int8) jaffarCommon::logger::log("0x%02X  (%03d)\n", p->getValue<int8_t>(), p->getValue<int8_t>());
      if (p->getDatatype() == Property::datatype_t::dt_int16) jaffarCommon::logger::log("0x%04X  (%05d)\n", p->getValue<int16_t>(), p->getValue<int16_t>());
      if (p->getDatatype() == Property::datatype_t::dt_int32) jaffarCommon::logger::log("0x%08X  (%10d)\n", p->getValue<int32_t>(), p->getValue<int32_t>());
      if (p->getDatatype() == Property::datatype_t::dt_int64) jaffarCommon::logger::log("0x%16lX (%ld)\n", p->getValue<int64_t>(), p->getValue<int64_t>());
      if (p->getDatatype() == Property::datatype_t::dt_uint8) jaffarCommon::logger::log("0x%02X  (%03u)\n", p->getValue<uint8_t>(), p->getValue<uint8_t>());
      if (p->getDatatype() == Property::datatype_t::dt_uint16) jaffarCommon::logger::log("0x%04X  (%05u)\n", p->getValue<uint16_t>(), p->getValue<uint16_t>());
      if (p->getDatatype() == Property::datatype_t::dt_uint32) jaffarCommon::logger::log("0x%08X  (%10u)\n", p->getValue<uint32_t>(), p->getValue<uint32_t>());
      if (p->getDatatype() == Property::datatype_t::dt_uint64) jaffarCommon::logger::log("0x%16lX (%lu)\n", p->getValue<uint64_t>(), p->getValue<uint64_t>());
      if (p->getDatatype() == Property::datatype_t::dt_float32) jaffarCommon::logger::log("%f      (0x%X)\n", p->getValue<float>(), p->getValue<uint32_t>());
      if (p->getDatatype() == Property::datatype_t::dt_float64) jaffarCommon::logger::log("%f      (0x%lX)\n", p->getValue<double>(), p->getValue<uint64_t>());
      if (p->getDatatype() == Property::datatype_t::dt_bool) jaffarCommon::logger::log("%1u\n", p->getValue<bool>());
    }

    // Printing game-specific stuff now
    printInfoImpl();
  }

  // Evaluates the rule set on a given frame. Returns true if it is a fail.
  __INLINE__ void evaluateRules()
  {
    // Calling the pre-update hook
    ruleUpdatePreHook();

    // Second, check which unsatisfied rules have been satisfied now
    for (auto& rule : _rules)
    {
      // Getting rule index
      const auto ruleIdx = rule->getIndex();

      // Evaluate rule only if it's not yet satisfied
      if (jaffarCommon::bitwise::getBitValue(_rulesStatus.data(), ruleIdx) == false)
      {
        // Checking if conditions are met
        bool isSatisfied = rule->evaluate();

        // If it's achieved, update its status and run its actions
        if (isSatisfied) satisfyRule(*rule);
      }
    }

    // Running game-specific rule actions
    runGameSpecificRuleActions();

    // Calling the pre-update hook
    ruleUpdatePostHook();
  }

  __INLINE__ void runGameSpecificRuleActions()
  {
    // First, checking if the rules have been satisfied
    for (auto& rule : _rules)
    {
      // Getting rule index
      const auto ruleIdx = rule->getIndex();

      // Run ations only if rule is satisfied
      if (jaffarCommon::bitwise::getBitValue(_rulesStatus.data(), ruleIdx) == true)
        for (const auto& action : rule->getActions()) action();
    }
  }

  __INLINE__ void updateGameStateType()
  {
    // Clearing game state type before we evaluate satisfied rules
    _stateType = stateType_t::normal;

    // Clearing checkpoint level and tolerance
    _checkpointLevel = 0;

    // Second, we run the specified actions for the satisfied rules in label order
    for (auto& rule : _rules)
    {
      // Getting rule index
      const auto ruleIdx = rule->getIndex();

      // Run actions
      if (jaffarCommon::bitwise::getBitValue(_rulesStatus.data(), ruleIdx) == true)
      {
        // Modify game state, depending on rule type

        // Evaluate checkpoint rule and store tolerance if specified
        if (rule->isCheckpointRule())
        {
          _checkpointLevel++;
          _checkpointTolerance = rule->getCheckpointTolerance();
        }

        // Evaluate save state rule and path if specified -- only if the current rule label is greater than the last rule to activate this
        if (rule->isSaveSolutionRule() && (ssize_t)ruleIdx > _saveSolutionCurrentLastRuleIdx)   _saveSolutionCurrentLastRuleId = ruleIdx;
        
        // Winning in the same rule superseeds checkpoint, and failing superseed everything
        if (rule->isWinRule()) _stateType = stateType_t::win;
        if (rule->isFailRule()) _stateType = stateType_t::fail;
      }
    }
  }

  __INLINE__ void updateReward()
  {
    // First, we resetting reward to zero
    _reward = 0.0;

    // Second, we get the reward from every satisfied rule
    for (auto& rule : _rules)
    {
      // Getting rule index
      const auto ruleIdx = rule->getIndex();

      // Run actions
      if (jaffarCommon::bitwise::getBitValue(_rulesStatus.data(), ruleIdx) == true)
      {
        // Getting reward from satisfied rule
        const auto ruleReward = rule->getReward();

        // Adding it to the state reward
        _reward += ruleReward;
      }
    }

    // Adding any game-specific rewards
    _reward += calculateGameSpecificReward();
  }

  std::unique_ptr<Condition> parseCondition(const nlohmann::json& conditionJs)
  {
    // Parsing operator name
    const auto& opName = jaffarCommon::json::getString(conditionJs, "Op");

    // Getting operator type from its name
    const auto opType = Condition::getOperatorType(opName);

    // Parsing first operand (property name)
    const auto& property1Name = jaffarCommon::json::getString(conditionJs, "Property");

    // Getting property name hash, for indexing
    const auto property1NameHash = jaffarCommon::hash::hashString(property1Name);

    // Making sure the requested property exists in the property map
    if (_propertyMap.contains(property1NameHash) == false) JAFFAR_THROW_LOGIC("[ERROR] Property '%s' has not been declared.\n", property1Name.c_str());

    // Getting property object
    const auto property1 = _propertyMap[property1NameHash].get();

    // Getting property data type
    auto datatype1 = property1->getDatatype();

    // Parsing second operand (number)
    if (conditionJs.contains("Value") == false) JAFFAR_THROW_LOGIC("[ERROR] Rule condition missing 'Value' key.\n");
    if (conditionJs["Value"].is_number() == false && conditionJs["Value"].is_string() == false && conditionJs["Value"].is_boolean() == false)
      JAFFAR_THROW_LOGIC("[ERROR] Wrong format for 'Value' entry in rule condition. It must be a string or number");

    // If value is a number, take it as immediate
    if (conditionJs["Value"].is_number())
    {
      if (datatype1 == Property::datatype_t::dt_uint8) return std::make_unique<_vCondition<uint8_t>>(opType, property1, nullptr, 0, conditionJs["Value"].get<uint8_t>());
      if (datatype1 == Property::datatype_t::dt_uint16) return std::make_unique<_vCondition<uint16_t>>(opType, property1, nullptr, 0, conditionJs["Value"].get<uint16_t>());
      if (datatype1 == Property::datatype_t::dt_uint32) return std::make_unique<_vCondition<uint32_t>>(opType, property1, nullptr, 0, conditionJs["Value"].get<uint32_t>());
      if (datatype1 == Property::datatype_t::dt_uint64) return std::make_unique<_vCondition<uint64_t>>(opType, property1, nullptr, 0, conditionJs["Value"].get<uint64_t>());

      if (datatype1 == Property::datatype_t::dt_int8) return std::make_unique<_vCondition<int8_t>>(opType, property1, nullptr, 0, conditionJs["Value"].get<int8_t>());
      if (datatype1 == Property::datatype_t::dt_int16) return std::make_unique<_vCondition<int16_t>>(opType, property1, nullptr, 0, conditionJs["Value"].get<int16_t>());
      if (datatype1 == Property::datatype_t::dt_int32) return std::make_unique<_vCondition<int32_t>>(opType, property1, nullptr, 0, conditionJs["Value"].get<int32_t>());
      if (datatype1 == Property::datatype_t::dt_int64) return std::make_unique<_vCondition<int64_t>>(opType, property1, nullptr, 0, conditionJs["Value"].get<int64_t>());

      if (datatype1 == Property::datatype_t::dt_float32) return std::make_unique<_vCondition<float>>(opType, property1, nullptr, 0, conditionJs["Value"].get<float>());
      if (datatype1 == Property::datatype_t::dt_float64) return std::make_unique<_vCondition<double>>(opType, property1, nullptr, 0, conditionJs["Value"].get<double>());
    }

    // If value is a boolean, take it as immediate
    if (conditionJs["Value"].is_boolean())
    {
      if (datatype1 == Property::datatype_t::dt_bool) return std::make_unique<_vCondition<bool>>(opType, property1, nullptr, 0, conditionJs["Value"].get<bool>());
    }

    // If value is a string, take value as property number 2
    if (conditionJs["Value"].is_string())
    {
      // Parsing second operand (property name)
      const auto& property2Name = jaffarCommon::json::getString(conditionJs, "Value");

      // Getting property name hash, for indexing
      const auto property2NameHash = jaffarCommon::hash::hashString(property2Name);

      // Making sure the requested property exists in the property map
      if (_propertyMap.contains(property2NameHash) == false) JAFFAR_THROW_LOGIC("[ERROR] Property '%s' has not been declared.\n", property2Name.c_str());

      // Getting property object
      const auto property2 = _propertyMap[property2NameHash].get();

      if (datatype1 == Property::datatype_t::dt_uint8) return std::make_unique<_vCondition<uint8_t>>(opType, property1, property2, 0, 0);
      if (datatype1 == Property::datatype_t::dt_uint16) return std::make_unique<_vCondition<uint16_t>>(opType, property1, property2, 0, 0);
      if (datatype1 == Property::datatype_t::dt_uint32) return std::make_unique<_vCondition<uint32_t>>(opType, property1, property2, 0, 0);
      if (datatype1 == Property::datatype_t::dt_uint64) return std::make_unique<_vCondition<uint64_t>>(opType, property1, property2, 0, 0);

      if (datatype1 == Property::datatype_t::dt_int8) return std::make_unique<_vCondition<int8_t>>(opType, property1, property2, 0, 0);
      if (datatype1 == Property::datatype_t::dt_int16) return std::make_unique<_vCondition<int16_t>>(opType, property1, property2, 0, 0);
      if (datatype1 == Property::datatype_t::dt_int32) return std::make_unique<_vCondition<int32_t>>(opType, property1, property2, 0, 0);
      if (datatype1 == Property::datatype_t::dt_int64) return std::make_unique<_vCondition<int64_t>>(opType, property1, property2, 0, 0);

      if (datatype1 == Property::datatype_t::dt_float32) return std::make_unique<_vCondition<float>>(opType, property1, property2, 0, 0);
      if (datatype1 == Property::datatype_t::dt_float64) return std::make_unique<_vCondition<double>>(opType, property1, property2, 0, 0);
      if (datatype1 == Property::datatype_t::dt_bool) return std::make_unique<_vCondition<bool>>(opType, property1, property2, 0, 0);
    }

    JAFFAR_THROW_LOGIC("[ERROR] Rule contains an invalid 'Value' key.\n", conditionJs["Value"].dump().c_str());
  }

  // Returns pointer to the internal emulator
  __INLINE__ Emulator* getEmulator() const { return _emulator.get(); }

  // Function to obtain emulator based on name
  static std::unique_ptr<Game> getGame(const nlohmann::json& emulatorConfig, const nlohmann::json& gameConfig);

  // Function to get the frame rate
  __INLINE__ float getFrameRate() const { return _frameRate; }

  // Function to get the reward
  __INLINE__ float getReward() const { return _reward; }

  // Function to get the state type
  __INLINE__ stateType_t getStateType() const { return _stateType; }

  // Function to get the state's checkpoint level
  __INLINE__ size_t getCheckpointLevel() const { return _checkpointLevel; }

  // Function to get the state's checkpoint level
  __INLINE__ size_t getCheckpointTolerance() const { return _checkpointTolerance; }

  // Function to get the state's path to save state, if set
  __INLINE__ bool isSaveSolution() const { return _saveSolutionCurrentLastRuleId > _saveSolutionCurrentLastRuleIdx; }

  // Function to get the previous last rule idx to set a save solution
  __INLINE__ ssize_t getSaveSolutionPrevLastRuleIdx() const { return _saveSolutionCurrentLastRuleIdx; }

  // Function to get the current last rule idx to set a save solution
  __INLINE__ ssize_t getSaveSolutionCurrentLastRuleIdx() const { return _saveSolutionCurrentLastRuleId; }

    // Function to get the state's path to save state, if set
  __INLINE__ const std::string getSaveSolutionPath() const
  {
     return isSaveSolution() ? _rules[_saveSolutionCurrentLastRuleId]->getSaveSolutionPath() : "";
  }

  // Function to get game name in runtime
  __INLINE__ std::string getName() const { return _gameName; }

  // Returns whether the game was initialized
  __INLINE__ bool isInitialized() const { return _isInitialized; }

  // Function for new input discovery
  virtual jaffarCommon::hash::hash_t getStateInputHash() { return jaffarCommon::hash::hash_t(); };

  // Function to enable a game code to provide additional allowed inputs based on complex decisions
  virtual __INLINE__ void getAdditionalAllowedInputs(std::vector<InputSet::inputIndex_t>& allowedInputSet) {}

  // Function to report what all the possible input that the game might require
  virtual __INLINE__ std::set<std::string> getAllPossibleInputs() { return {}; }

  // Player-specific commands
  virtual void playerPrintCommands() const {}           // If the game has any specific player commands, print them now
  virtual bool playerParseCommand(const int command) { return false; } // If the game has any specific player commands, print them now

  // Function to get direct state hash without passing an hashing engine
  virtual jaffarCommon::hash::hash_t getDirectStateHash() const { return jaffarCommon::hash::hash_t(); }

protected:

  void* registerGameProperty(const std::string& name, void* const pointer, const Property::datatype_t dataType, const Property::endianness_t endianness)
  {
    // Creating property
    auto property = std::make_unique<Property>(name, pointer, dataType, endianness);

    // Getting property name hash as key
    const auto propertyNameHash = property->getNameHash();

    // Adding property to the map for later reference
    _propertyMap[propertyNameHash] = std::move(property);

    // Return the pointer proper (this is just sugar to make the use of this function more compact)
    return pointer;
  }

  // Parsing game rules
  void parseRules(const nlohmann::json& rulesJson)
  {
    // Reset the rules container
    _rules.clear();
    _rulesStatus.clear();

    // Evaluate each rule
    for (size_t idx = 0; idx < rulesJson.size(); idx++)
    {
      // Getting specific rule json object
      const auto& ruleJs = rulesJson[idx];

      // Check if rule is a key/value object
      if (ruleJs.is_object() == false) JAFFAR_THROW_LOGIC("Passed rule is not a JSON object. Dump: \n %s", ruleJs.dump(2).c_str());

      // Getting rule label
      auto label = jaffarCommon::json::getNumber<Rule::label_t>(ruleJs, "Label");

      // Creating new rule with the given label
      auto rule = std::make_unique<Rule>(idx, label);

      // Parsing json into a rule class
      parseRule(*rule, ruleJs);

      // Adding new rule to the collection
      _rules.push_back(std::move(rule));
    }

    // Checking all cross references are correct
    for (const auto& rule : _rules)
      for (const auto& label : rule->getSatisfyRuleLabels())
      {
        bool subRuleFound = false;
        for (const auto& subRule : _rules)
          if (subRule->getLabel() == label)
          {
            rule->addSatisfyRule(subRule.get());
            subRuleFound = true;
          }
        if (subRuleFound == false) JAFFAR_THROW_LOGIC("Rule label %u referenced by rule %u in the 'Satisfies' array does not exist.\n", label, rule->getIndex());
      }

    // Create rule status vector
    _rulesStatus.resize(jaffarCommon::bitwise::getByteStorageForBitCount(_rules.size()));

    // Clearing the status vector evaluation
    for (size_t i = 0; i < _rules.size(); i++) jaffarCommon::bitwise::setBitValue(_rulesStatus.data(), i, false);
  }

  // Individual rule parser
  void parseRule(Rule& rule, const nlohmann::json& ruleJs)
  {
    // Getting rule condition array
    const auto& conditions = jaffarCommon::json::getArray<nlohmann::json>(ruleJs, "Conditions");

    // Getting rule action array
    const auto& actions = jaffarCommon::json::getArray<nlohmann::json>(ruleJs, "Actions");

    // Parsing satisfies vector
    const auto& satisfiesVectorJs = jaffarCommon::json::getArray<nlohmann::json>(ruleJs, "Satisfies");

    // Parsing rule conditions
    for (const auto& condition : conditions) rule.addCondition(parseCondition(condition));

    // Parsing rule actions
    for (const auto& action : actions) parseRuleAction(rule, action);

    // Parsing satisfies vector
    for (const auto& s : satisfiesVectorJs)
    {
      // Check for correct format
      if (s.is_number() == false) JAFFAR_THROW_LOGIC("Wrong format provided in 'Satisfies' array in rule '%s'\n", ruleJs.dump(2).c_str());

      // Adding the satisfies label
      rule.addSatisfyRuleLabel(s.get<Rule::label_t>());
    }
  }

  void parseRuleAction(Rule& rule, const nlohmann::json& actionJs)
  {
    // Getting action type
    std::string actionType = jaffarCommon::json::getString(actionJs, "Type");

    // Running the action, depending on the type
    bool recognizedActionType = false;

    if (actionType == "Add Reward")
    {
      rule.setReward(jaffarCommon::json::getNumber<float>(actionJs, "Value"));
      recognizedActionType = true;
    }

    // Storing fail state
    if (actionType == "Trigger Fail")
    {
      rule.setFailRule(true);
      recognizedActionType = true;
    }

    // Storing win state
    if (actionType == "Trigger Win")
    {
      rule.setWinRule(true);
      recognizedActionType = true;
    }

    // Storing checkpoint flags
    if (actionType == "Trigger Checkpoint")
    {
      rule.setCheckpointRule(true);
      rule.setCheckpointTolerance(jaffarCommon::json::getNumber<size_t>(actionJs, "Tolerance"));
      recognizedActionType = true;
    }

    // Storing save state flags
    if (actionType == "Trigger Save Solution")
    {
      rule.setSaveSolutionRule(true);
      rule.setSaveSolutionPath(jaffarCommon::json::getString(actionJs, "Path"));
      recognizedActionType = true;
    }

    // If not recognized yet, it must be a game specific action
    if (recognizedActionType == false) recognizedActionType = parseRuleActionImpl(rule, actionType, actionJs);

    // If not recognized at all, then fail
    if (recognizedActionType == false) JAFFAR_THROW_LOGIC("[ERROR] Unrecognized action '%s' in rule %lu\n", actionType.c_str(), rule.getLabel());
  }

  // Marks the given rule as satisfied, executes its actions, and recursively runs on its sub-satisfied rules
  __INLINE__ void satisfyRule(Rule& rule)
  {
    // Recursively run actions for the yet unsatisfied rules that are satisfied by this one and mark them as satisfied
    for (const auto subRule : rule.getSatisfyRules())
    {
      // Getting index from the subrule
      auto subRuleIdx = subRule->getIndex();

      // Only activate it if it hasn't been activated before
      if (jaffarCommon::bitwise::getBitValue(_rulesStatus.data(), subRuleIdx) == false) satisfyRule(*subRule);
    }

    // Getting rule index
    const auto ruleIdx = rule.getIndex();

    // Setting status to satisfied
    jaffarCommon::bitwise::setBitValue(_rulesStatus.data(), ruleIdx, true);
  }

  virtual void  initializeImpl() {};
  virtual void  registerGameProperties()                                                                       = 0;
  virtual void  serializeStateImpl(jaffarCommon::serializer::Base& serializer) const                           = 0;
  virtual void  deserializeStateImpl(jaffarCommon::deserializer::Base& deserializer)                           = 0;
  virtual float calculateGameSpecificReward() const                                                            = 0;
  virtual void  computeAdditionalHashing(MetroHash128& hashEngine) const                                       = 0;
  virtual void  printInfoImpl() const                                                                          = 0;
  virtual void  advanceStateImpl(const InputSet::inputIndex_t input)                                           = 0;
  virtual bool  parseRuleActionImpl(Rule& rule, const std::string& actionType, const nlohmann::json& actionJs) = 0;

  // Optional hooks
  virtual __INLINE__ void stateUpdatePreHook() {};
  virtual __INLINE__ void stateUpdatePostHook() {};
  virtual __INLINE__ void ruleUpdatePreHook() {};
  virtual __INLINE__ void ruleUpdatePostHook() {};

  // Current game state type
  stateType_t _stateType;

  // Current game state reward
  float _reward = 0.0;

  // Represents the current state's checkpoint level
  size_t _checkpointLevel = 0;

  // For game states that represent checkpoints, store their tolerance here
  size_t _checkpointTolerance = 0;

  // Save state will only activate if the rule id is bigger than the last rule id that activated it (Previous to preserve the state where it changes)
  ssize_t _saveSolutionCurrentLastRuleIdx = -1;

  // Save state will only activate if the rule id is bigger than the last rule id that activated it
  ssize_t _saveSolutionCurrentLastRuleId = -1;

  // Underlying emulator instance
  const std::unique_ptr<Emulator> _emulator;

  // Game script rules. Using vector to preserve their ordering
  std::vector<std::unique_ptr<Rule>> _rules;

  // Storage for status vector indicating whether the rules have been satisfied
  std::vector<uint8_t> _rulesStatus;

  // Storage for the parse property names that are meant to be printed
  std::vector<std::string> _printablePropertyNames;

  // Storage for the parse property names that are meant to be hased
  std::vector<std::string> _hashablePropertyNames;

  // Property hash set to quickly distinguish states from each other. Its a vector to keep the ordering
  std::vector<const Property*> _propertyHashVector;

  // Property print vector for printing game information to screen. Its a vector to keep the ordering
  std::vector<const Property*> _propertyPrintVector;

  // Property map to store all registered properties for future reference, indexed by their name hash
  std::map<jaffarCommon::hash::hash_t, std::unique_ptr<Property>> _propertyMap;

  // Inverse frame rate to play the game with, required for correct playback
  float _frameRate;

  // Flag to bypass emulator saving to allow the game take care of it entirely
  bool _bypassEmulatorState;

  // Game name (for runtime use)
  std::string _gameName;

  // Temporal storage of the rules json for delayed parsing
  nlohmann::json _rulesJs;

  // Stores whether the game has been initialized
  bool _isInitialized = false;
};

} // namespace jaffarPlus