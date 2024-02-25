#pragma once

#include <unordered_set>
#include <vector>
#include <map>
#include <memory>
#include <utility>
#include <jaffarCommon/include/serializers/base.hpp>
#include <jaffarCommon/include/deserializers/base.hpp>
#include <jaffarCommon/include/bitwise.hpp>
#include <jaffarCommon/include/hash.hpp>
#include <jaffarCommon/include/json.hpp>
#include "emulator.hpp"
#include "rule.hpp"

namespace jaffarPlus
{

class Game
{
 public:

  enum stateType_t
  {
    normal = 0,
    win = 1,
    fail = 2,
    checkpoint = 3
  };

  // Constructor that takes an already created emulator
  Game(std::unique_ptr<Emulator> emulator, const nlohmann::json& config) : _emulator(std::move(emulator))
  {
    // Getting emulator name (for runtime use)
    _gameName = JSON_GET_STRING(config, "Game Name");

    // Parsing printable game properties
    _frameRate = JSON_GET_NUMBER(float, config, "Frame Rate");

    // Marking printable properties
    const auto& printPropertiesJs = JSON_GET_ARRAY(config, "Print Properties");
    for (const auto& propertyJs : printPropertiesJs)
    {
      if (propertyJs.is_string() == false) EXIT_WITH_ERROR("Printable property entry is not a string");
      const auto propertyName = propertyJs.get<std::string>();
      _printablePropertyNames.push_back(propertyName);
    }

     // Parsing hashable game properties
    const auto& hashPropertiesJs = JSON_GET_ARRAY(config, "Hash Properties");
    for (const auto& propertyJs : hashPropertiesJs)
    {
      if (propertyJs.is_string() == false) EXIT_WITH_ERROR("Hashable property entry is not a string");
      const auto propertyName = propertyJs.get<std::string>();
      _hashablePropertyNames.push_back(propertyName);
    }

    // Storing rules JSON for later parsing
    _rulesJs = JSON_GET_ARRAY(config, "Rules");  
  };

  void initialize()
  {
    // Registering printable properties
    for (const auto& property : _printablePropertyNames)
    {
      // Getting property name hash
      const auto propertyHash = jaffarCommon::hashString(property);

      // Checking the property is registered 
      if (_propertyMap.contains(propertyHash) == false) EXIT_WITH_ERROR("Property '%s' is not registered in this game", property.c_str());

      // If so, add its pointer to the print property vector
      _propertyPrintVector.push_back(_propertyMap.at(propertyHash).get());
    }

    // Registering hashable properties
    for (const auto& property : _hashablePropertyNames)
    {
      // Getting property name hash
      const auto propertyHash = jaffarCommon::hashString(property);

      // Checking the property is registered 
      if (_propertyMap.contains(propertyHash) == false) EXIT_WITH_ERROR("Property '%s' is not registered in this game", property.c_str());

      // If so, add its pointer to the print property vector
      _propertyHashVector.push_back(_propertyMap.at(propertyHash).get());
    }

    // Calling the post-update hook
    stateUpdatePostHook();

    // Now parsing rules
    parseRules(_rulesJs);
  }

  Game() = delete;
  virtual ~Game() = default;

  // Function to advance state. 
  inline void advanceState(const std::string& input)
  {
    // Calling the pre-update hook
    stateUpdatePreHook();

    // Performing the requested input
    advanceStateImpl(input);

    // Calling the post-update hook
    stateUpdatePostHook();
  }

  // Differential serialization routine
  inline void serializeState(jaffarCommon::serializer::Base& serializer) const
  {
    // Serializing internal emulator state
    _emulator->serializeState(serializer);

    // Storage for game-specific data
    serializeStateImpl(serializer);

    // Serializing reward
    serializer.pushContiguous(&_reward, sizeof(_reward));

    // Serializing state type
    serializer.pushContiguous(&_stateType, sizeof(_stateType));

    // Serializing rule states
    serializer.pushContiguous(_rulesStatus.data(), _rulesStatus.size());
  }

  // Differential deserialization routine
  inline void deserializeState(jaffarCommon::deserializer::Base& deserializer)
  {
    // Calling the pre-update hook
    stateUpdatePreHook();

    // Storage for the internal emulator state
    _emulator->deserializeState(deserializer);

    // Storage for game-specific data
    deserializeStateImpl(deserializer);

    // Calling the post-update hook
    stateUpdatePostHook();

    // Deserializing reward
    deserializer.popContiguous(&_reward, sizeof(_reward));

    // Serializing state type
    deserializer.popContiguous(&_stateType, sizeof(_stateType));

    // Calling the pre-rule update hook
    ruleUpdatePreHook();    

    // Deserializing rules status
    deserializer.popContiguous(_rulesStatus.data(), _rulesStatus.size());

    // Running game specific rule actions
    runGameSpecificRuleActions();

    // Calling the post-rule update hook
    ruleUpdatePostHook();    
  }

  inline size_t getDifferentialStateSize(const size_t maxDifferences) const
  {
    size_t stateSize = 0;

    // Adding maximum differences (in place of emulator storage)
    stateSize += maxDifferences;

    // Adding the size of current reward
    stateSize += sizeof(_reward);

    // Adding the size of rules status
    stateSize += _rulesStatus.size();

    return stateSize;
  }

  // This function computes the hash for the current state
  inline void computeHash(MetroHash128& hashEngine) const 
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
   const size_t separatorSize = 4;
   size_t maximumNameSize = 0;
   for (const auto& p : _propertyPrintVector) maximumNameSize = std::max(maximumNameSize, p->getName().size());

   // Printing game state
   LOG("[J++]  + Game State Type: ");
   if (_stateType == stateType_t::normal) LOG("Normal");
   if (_stateType == stateType_t::win) LOG("Win");
   if (_stateType == stateType_t::fail) LOG("Fail");
   if (_stateType == stateType_t::checkpoint) LOG("Checkpoint");
   LOG("\n");

   // Printing game state
   LOG("[J++]  + Game State Reward: %f\n", _reward);

   // Printing rule status
   LOG("[J++]  + Rule Status: ");
   for (size_t i = 0; i < _rules.size(); i++) LOG("%d", jaffarCommon::getBitValue(_rulesStatus.data(), i) ? 1 : 0);
   LOG("\n");

   // Printing game properties defined in the script file
   LOG("[J++]  + Game Properties: \n");
   for (const auto& p : _propertyPrintVector)
   {
     // Getting property name
     const auto& name = p->getName();

     // Printing property name first
     LOG("[J++]    + '%s':", name.c_str());

     // Calculating separation spaces for this property
     const auto propertySeparatorSize = separatorSize + maximumNameSize - name.size();

     // Printing separator spaces
     for (size_t i = 0; i < propertySeparatorSize; i++) LOG(" ");

     // Then printing separator spaces
     if (p->getDatatype() == Property::datatype_t::dt_int8)     LOG("0x%02X  (%03d)\n",  p->getValue<int8_t>()  , p->getValue<int8_t>()  );
     if (p->getDatatype() == Property::datatype_t::dt_int16)    LOG("0x%04X  (%05d)\n",  p->getValue<int16_t>() , p->getValue<int16_t>() );
     if (p->getDatatype() == Property::datatype_t::dt_int32)    LOG("0x%08X  (%10d)\n",  p->getValue<int32_t>() , p->getValue<int32_t>() );
     if (p->getDatatype() == Property::datatype_t::dt_int64)    LOG("0x%16lX (%ld)\n",   p->getValue<int64_t>() , p->getValue<int64_t>() );
     if (p->getDatatype() == Property::datatype_t::dt_uint8)    LOG("0x%02X  (%03u)\n",  p->getValue<uint8_t>() , p->getValue<uint8_t>() );
     if (p->getDatatype() == Property::datatype_t::dt_uint16)   LOG("0x%04X  (%05u)\n",  p->getValue<uint16_t>(), p->getValue<uint16_t>());
     if (p->getDatatype() == Property::datatype_t::dt_uint32)   LOG("0x%08X  (%10u)\n",  p->getValue<uint32_t>(), p->getValue<uint32_t>());
     if (p->getDatatype() == Property::datatype_t::dt_uint64)   LOG("0x%16lX (%lu)\n",   p->getValue<uint64_t>(), p->getValue<uint64_t>());
     if (p->getDatatype() == Property::datatype_t::dt_float32)  LOG("%f      (0x%X)\n",  p->getValue<float>(),    p->getValue<uint32_t>());
     if (p->getDatatype() == Property::datatype_t::dt_float64)  LOG("%f      (0x%lX)\n", p->getValue<double>(),   p->getValue<uint64_t>());
     if (p->getDatatype() == Property::datatype_t::dt_bool)     LOG("%1u\n",             p->getValue<bool>() );
   }

   // Printing game-specific stuff now
   printInfoImpl();
  }

  // Evaluates the rule set on a given frame. Returns true if it is a fail.
  inline void evaluateRules() 
  {
    // Calling the pre-update hook
    ruleUpdatePreHook();

    // Second, check which unsatisfied rules have been satisfied now
    for (auto& entry : _rules)
    {
      // Getting rule
      auto& rule = *entry.second;

      // Getting rule index
      const auto ruleIdx = rule.getIndex(); 

      // Evaluate rule only if it's not yet satisfied
      if (jaffarCommon::getBitValue(_rulesStatus.data(), ruleIdx) == false)
      {
        // Checking if conditions are met
        bool isSatisfied = rule.evaluate();

        // If it's achieved, update its status and run its actions
        if (isSatisfied) satisfyRule(rule);
      }
    }

    // Running game-specific rule actions
    runGameSpecificRuleActions();

    // Calling the pre-update hook
    ruleUpdatePostHook();
  }

  inline void runGameSpecificRuleActions()
  {
     // First, checking if the rules have been satisfied
    for (auto& entry : _rules)
    {
      // Getting rule
      auto& rule = *entry.second;

      // Getting rule index
      const auto ruleIdx = rule.getIndex(); 

      // Run ations only if rule is satisfied
      if (jaffarCommon::getBitValue(_rulesStatus.data(), ruleIdx) == true) for (const auto& action : rule.getActions()) action();
    }  
  }

  inline void updateGameStateType()
  {
    // Clearing game state type before we evaluate satisfied rules
    _stateType = stateType_t::normal;

    // Second, we run the specified actions for the satisfied rules in label order
    for (auto& entry : _rules)
    {
      // Getting rule
      auto& rule = *entry.second;

      // Getting rule index
      const auto ruleIdx = rule.getIndex(); 

      // Run actions
      if (jaffarCommon::getBitValue(_rulesStatus.data(), ruleIdx) == true)
      {
       // Modify game state, depending on rule type

       // Evaluate checkpoint rule and store tolerance if specified
       if (rule.isCheckpointRule())
       {
        _stateType = stateType_t::checkpoint;
        _checkpointTolerance = rule.getCheckpointTolerance();
       } 

       // Winning in the same rule superseeds checkpoint, and failing superseed everything
       if (rule.isWinRule()) _stateType = stateType_t::win;
       if (rule.isFailRule()) _stateType = stateType_t::fail;
      }
    }
  }

  inline void updateReward()
  {
    // First, we resetting reward to zero
    _reward = 0.0;

    // Second, we get the reward from every satisfied rule
    for (auto& entry : _rules)
    {
      // Getting rule
      auto& rule = *entry.second;

      // Getting rule index
      const auto ruleIdx = rule.getIndex(); 

      // Run actions
      if (jaffarCommon::getBitValue(_rulesStatus.data(), ruleIdx) == true)
      {
       // Getting reward from satisfied rule
       const auto ruleReward = rule.getReward();

       // Adding it to the state reward
       _reward += ruleReward;
      }
    }

    // Adding any game-specific rewards
    _reward += calculateGameSpecificReward();

    // Sanity check
    if (std::isfinite(_reward) == false)
    {
      LOG("[J+] Error: Non finite reward detected: %f\n", _reward);
      printInfo();
      EXIT_WITH_ERROR("Invalid Reward");
    } 
  }

  std::unique_ptr<Condition> parseCondition(const nlohmann::json& conditionJs)
  {
    // Parsing operator name
    const auto& opName = JSON_GET_STRING(conditionJs, "Op");
   
    // Getting operator type from its name
    const auto opType =  Condition::getOperatorType(opName);

    // Parsing first operand (property name)
    const auto& property1Name = JSON_GET_STRING(conditionJs, "Property");

    // Getting property name hash, for indexing
    const auto property1NameHash = jaffarCommon::hashString(property1Name);

    // Making sure the requested property exists in the property map
    if (_propertyMap.contains(property1NameHash) == false) EXIT_WITH_ERROR("[ERROR] Property '%s' has not been declared.\n", property1Name.c_str());

    // Getting property object
    const auto property1 = _propertyMap[property1NameHash].get();

    // Getting property data type
    auto datatype1 = property1->getDatatype();

    // Parsing second operand (number)
    if (conditionJs.contains("Value") == false) EXIT_WITH_ERROR("[ERROR] Rule condition missing 'Value' key.\n");
    if (conditionJs["Value"].is_number() == false && conditionJs["Value"].is_string() == false) EXIT_WITH_ERROR("[ERROR] Wrong format for 'Value' entry in rule condition. It must be a string or number");

    // If value is a number, take it as immediate
    if (conditionJs["Value"].is_number())
    {
     if (datatype1 == Property::datatype_t::dt_uint8)   return std::make_unique<_vCondition<uint8_t> >(opType, property1, nullptr, 0, conditionJs["Value"].get<uint8_t>());
     if (datatype1 == Property::datatype_t::dt_uint16)  return std::make_unique<_vCondition<uint16_t>>(opType, property1, nullptr, 0, conditionJs["Value"].get<uint16_t>());
     if (datatype1 == Property::datatype_t::dt_uint32)  return std::make_unique<_vCondition<uint32_t>>(opType, property1, nullptr, 0, conditionJs["Value"].get<uint32_t>());
     if (datatype1 == Property::datatype_t::dt_uint64)  return std::make_unique<_vCondition<uint64_t>>(opType, property1, nullptr, 0, conditionJs["Value"].get<uint64_t>());

     if (datatype1 == Property::datatype_t::dt_int8)    return std::make_unique<_vCondition<int8_t>  >(opType, property1, nullptr, 0, conditionJs["Value"].get<int8_t>());
     if (datatype1 == Property::datatype_t::dt_int16)   return std::make_unique<_vCondition<int16_t> >(opType, property1, nullptr, 0, conditionJs["Value"].get<int16_t>());
     if (datatype1 == Property::datatype_t::dt_int32)   return std::make_unique<_vCondition<int32_t> >(opType, property1, nullptr, 0, conditionJs["Value"].get<int32_t>());
     if (datatype1 == Property::datatype_t::dt_int64)   return std::make_unique<_vCondition<int64_t> >(opType, property1, nullptr, 0, conditionJs["Value"].get<int64_t>());

     if (datatype1 == Property::datatype_t::dt_float32) return std::make_unique<_vCondition<float>   >(opType, property1, nullptr, 0, conditionJs["Value"].get<float>());
     if (datatype1 == Property::datatype_t::dt_float64) return std::make_unique<_vCondition<double>  >(opType, property1, nullptr, 0, conditionJs["Value"].get<double>());
     if (datatype1 == Property::datatype_t::dt_bool)    return std::make_unique<_vCondition<bool>    >(opType, property1, nullptr, 0, conditionJs["Value"].get<bool>());
    }

    // If value is a string, take value as property number 2
    if (conditionJs["Value"].is_string())
    {
      // Parsing second operand (property name)
      const auto& property2Name = JSON_GET_STRING(conditionJs, "Value");

      // Getting property name hash, for indexing
      const auto property2NameHash = jaffarCommon::hashString(property2Name);

      // Making sure the requested property exists in the property map
      if (_propertyMap.contains(property2NameHash) == false) EXIT_WITH_ERROR("[ERROR] Property '%s' has not been declared.\n", property2Name.c_str());

      // Getting property object
      const auto property2 = _propertyMap[property2NameHash].get();

      if (datatype1 == Property::datatype_t::dt_uint8)   return std::make_unique<_vCondition<uint8_t> >(opType, property1, property2, 0, 0);
      if (datatype1 == Property::datatype_t::dt_uint16)  return std::make_unique<_vCondition<uint16_t>>(opType, property1, property2, 0, 0);
      if (datatype1 == Property::datatype_t::dt_uint32)  return std::make_unique<_vCondition<uint32_t>>(opType, property1, property2, 0, 0);
      if (datatype1 == Property::datatype_t::dt_uint64)  return std::make_unique<_vCondition<uint64_t>>(opType, property1, property2, 0, 0);

      if (datatype1 == Property::datatype_t::dt_int8)    return std::make_unique<_vCondition<int8_t>  >(opType, property1, property2, 0, 0);
      if (datatype1 == Property::datatype_t::dt_int16)   return std::make_unique<_vCondition<int16_t> >(opType, property1, property2, 0, 0);
      if (datatype1 == Property::datatype_t::dt_int32)   return std::make_unique<_vCondition<int32_t> >(opType, property1, property2, 0, 0);
      if (datatype1 == Property::datatype_t::dt_int64)   return std::make_unique<_vCondition<int64_t> >(opType, property1, property2, 0, 0);

      if (datatype1 == Property::datatype_t::dt_float32) return std::make_unique<_vCondition<float>   >(opType, property1, property2, 0, 0);
      if (datatype1 == Property::datatype_t::dt_float64) return std::make_unique<_vCondition<double>  >(opType, property1, property2, 0, 0);
      if (datatype1 == Property::datatype_t::dt_bool)    return std::make_unique<_vCondition<bool>    >(opType, property1, property2, 0, 0);
    }

    EXIT_WITH_ERROR("[ERROR] Rule contains an invalid 'Value' key.\n", conditionJs["Value"].dump().c_str());
  }

  // Returns pointer to the internal emulator
  inline Emulator* getEmulator() const { return _emulator.get(); }

  // Function to obtain emulator based on name
  static std::unique_ptr<Game> getGame(const nlohmann::json& emulatorConfig, const nlohmann::json& gameConfig);
 
  // Function to get the frame rate
  inline float getFrameRate() const { return _frameRate; }

  // Function to get the reward
  inline float getReward() const { return _reward; }
  
  // Function to get the state type
  inline stateType_t getStateType() const { return _stateType; }

  // Function to get game name in runtime
  inline std::string getName() const { return _gameName; }

  protected:

  void registerGameProperty(const std::string& name, void* const pointer, const Property::datatype_t dataType, const Property::endianness_t endianness)
  {
    // Creating property
    auto property = std::make_unique<Property>(name, pointer, dataType, endianness);

    // Getting property name hash as key 
    const auto propertyNameHash = property->getNameHash();

    // Adding property to the map for later reference
    _propertyMap[propertyNameHash] = std::move(property);
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
      if (ruleJs.is_object() == false) EXIT_WITH_ERROR("Passed rule is not a JSON object. Dump: \n %s", ruleJs.dump(2).c_str());
      
      // Getting rule label
      auto label = JSON_GET_NUMBER(Rule::label_t, ruleJs, "Label");

      // Creating new rule with the given label
      auto rule = std::make_unique<Rule>(idx, label);
 
      // Parsing json into a rule class
      parseRule(*rule, ruleJs);

      // Adding new rule to the collection
      _rules[rule->getLabel()] = std::move(rule);
    }

    // Create rule status vector
    _rulesStatus.resize(jaffarCommon::getByteStorageForBitCount(_rules.size()));

    // Clearing the status vector evaluation
    for (size_t i = 0; i < _rules.size(); i++) jaffarCommon::setBitValue(_rulesStatus.data(), i, false);
  }
 
  // Individual rule parser 
  void parseRule(Rule& rule, const nlohmann::json& ruleJs) 
  {
    // Getting rule condition array
    const auto& conditions = JSON_GET_ARRAY(ruleJs, "Conditions");

    // Getting rule action array
    const auto& actions = JSON_GET_ARRAY(ruleJs, "Actions");

    // Parsing rule conditions
    for (const auto& condition : conditions) rule.addCondition(parseCondition(condition));  

    // Parsing rule actions
    for (const auto& action : actions) parseRuleAction(rule, action);  
  }

  void parseRuleAction(Rule& rule, const nlohmann::json& actionJs) 
  {
    // Getting action type
    std::string actionType = JSON_GET_STRING(actionJs, "Type");

    // Running the action, depending on the type
    bool recognizedActionType = false;

    if (actionType == "Add Reward")
    {
      rule.setReward(JSON_GET_NUMBER(float, actionJs, "Value"));
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
      rule.setCheckpointTolerance(JSON_GET_NUMBER(size_t, actionJs, "Tolerance"));
      recognizedActionType = true;
    }

    // If not recognized yet, it must be a game specific action
    if (recognizedActionType == false) recognizedActionType = parseRuleActionImpl(rule, actionType, actionJs);

    // If not recognized at all, then fail
    if (recognizedActionType == false) EXIT_WITH_ERROR("[ERROR] Unrecognized action '%s' in rule %lu\n", actionType.c_str(), rule.getLabel());
  }

  // Marks the given rule as satisfied, executes its actions, and recursively runs on its sub-satisfied rules
  inline void satisfyRule(Rule& rule) 
  {
    // Recursively run actions for the yet unsatisfied rules that are satisfied by this one and mark them as satisfied
    for (const auto& satisfyRuleLabel : rule.getSatisfyRuleLabels())
    {
      // Making sure referenced label exists
      auto it = _rules.find(satisfyRuleLabel);
      if (it == _rules.end()) EXIT_WITH_ERROR("[ERROR] Unrecognized rule label %lu in satisfy array\n", satisfyRuleLabel);
      auto& subRule = it->second;

      // Getting index from the subrule
      auto subRuleIdx = subRule->getIndex();

      // Only activate it if it hasn't been activated before
      if (jaffarCommon::getBitValue(_rulesStatus.data(), subRuleIdx) == false) satisfyRule(*subRule);
    }

    // Getting rule index
    const auto ruleIdx = rule.getIndex(); 

    // Setting status to satisfied
    jaffarCommon::setBitValue(_rulesStatus.data(), ruleIdx, true);
  }

  virtual void serializeStateImpl(jaffarCommon::serializer::Base& serializer) const = 0;
  virtual void deserializeStateImpl(jaffarCommon::deserializer::Base& deserializer) = 0;
  virtual float calculateGameSpecificReward() const = 0;
  virtual void computeAdditionalHashing(MetroHash128& hashEngine) const = 0;
  virtual void printInfoImpl() const = 0;
  virtual void advanceStateImpl(const std::string& input) = 0;
  virtual bool parseRuleActionImpl(Rule& rule, const std::string& actionType, const nlohmann::json& actionJs) = 0;

  // Optional hooks
  virtual inline void stateUpdatePreHook() {};
  virtual inline void stateUpdatePostHook() {};
  virtual inline void ruleUpdatePreHook() {};
  virtual inline void ruleUpdatePostHook() {};

  // Current game state type
  stateType_t _stateType;

  // Current game state reward
  float _reward = 0.0;

  // For game states that represent checkpoints, store their tolerance here
  size_t _checkpointTolerance = 0;

  // Underlying emulator instance
  const std::unique_ptr<Emulator> _emulator;

  // Game script rules. Using vector to preserve their ordering
  std::map<Rule::label_t, std::unique_ptr<Rule>> _rules;

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
  std::map<jaffarCommon::hash_t, std::unique_ptr<Property>> _propertyMap;

  // Inverse frame rate to play the game with, required for correct playback
  float _frameRate;

  // Game name (for runtime use)
  std::string _gameName;

  // Temporal storage of the rules json for delayed parsing
  nlohmann::json _rulesJs;
};

} // namespace jaffarPlus