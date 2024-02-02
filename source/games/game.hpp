#pragma once

#include <unordered_set>
#include <vector>
#include <map>
#include <memory>
#include <common/hash.hpp>
#include <common/json.hpp>
#include <emulators/emulator.hpp>
#include <games/rule.hpp>

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

  // Base constructor
  Game(std::unique_ptr<Emulator>& emulator, const nlohmann::json& config) : _emulator(std::move(emulator))
  {
    // Parsing game properties
    const auto& propertiesJs = JSON_GET_ARRAY(config, "Properties");
    for (const auto& propertyJs : propertiesJs)
    {
      // Getting property settings
      const auto name = JSON_GET_STRING(propertyJs, "Name");
      const auto dataTypeString = JSON_GET_STRING(propertyJs, "Data Type");
      const auto endiannessString = JSON_GET_STRING(propertyJs, "Endianness");
      const auto memoryBlock = JSON_GET_STRING(propertyJs, "Memory Block");
      const auto offsetString = JSON_GET_STRING(propertyJs, "Offset");
      const auto hashable = JSON_GET_BOOLEAN(propertyJs, "Hashable");
      const auto printable = JSON_GET_BOOLEAN(propertyJs, "Printable");

      // Parsing datatype
      Property::datatype_t dataType;
      bool datatypeRecognized = false;
      if (dataTypeString == "UINT8")   { dataType = Property::datatype_t::dt_uint8;    datatypeRecognized = true; }
      if (dataTypeString == "UINT16")  { dataType = Property::datatype_t::dt_uint16;   datatypeRecognized = true; }
      if (dataTypeString == "UINT32")  { dataType = Property::datatype_t::dt_uint32;   datatypeRecognized = true; }
      if (dataTypeString == "UINT64")  { dataType = Property::datatype_t::dt_uint64;   datatypeRecognized = true; }
      if (dataTypeString == "INT8")    { dataType = Property::datatype_t::dt_int8;     datatypeRecognized = true; }
      if (dataTypeString == "INT16")   { dataType = Property::datatype_t::dt_int16;    datatypeRecognized = true; }
      if (dataTypeString == "INT32")   { dataType = Property::datatype_t::dt_int32;    datatypeRecognized = true; }
      if (dataTypeString == "INT64")   { dataType = Property::datatype_t::dt_int64;    datatypeRecognized = true; }
      if (dataTypeString == "BOOL")    { dataType = Property::datatype_t::dt_bool;     datatypeRecognized = true; }
      if (dataTypeString == "FLOAT32") { dataType = Property::datatype_t::dt_float32;  datatypeRecognized = true; }
      if (dataTypeString == "FLOAT64") { dataType = Property::datatype_t::dt_float64;  datatypeRecognized = true; }
      if (datatypeRecognized == false) EXIT_WITH_ERROR("Data type '%s' not recognized.", dataTypeString.c_str());

      // Parsing endianness
      Property::endianness_t endianness;
      bool endiannessRecognized = false;
      if (endiannessString == "Little") { endianness = Property::endianness_t::little; endiannessRecognized = true; }
      if (endiannessString == "Big") { endianness = Property::endianness_t::big; endiannessRecognized = true; }
      if (endiannessRecognized == false) EXIT_WITH_ERROR("Endianness '%s' not recognized.", endiannessString.c_str());

      // Getting offset
      const auto offset = std::strtoull(offsetString.c_str(), nullptr, 0);

      // Calculating property pointer
      const auto pointer = _emulator->getProperty(memoryBlock).pointer + offset;

      // Creating property
      auto property = std::make_unique<Property>(name, pointer, dataType, endianness);

      // If this is a hashable property, add it to the hash set
      if (hashable) _propertyHashSet.insert(property.get());

      // If this is a printable property, add it to the set
      if (printable) _propertyPrintSet.push_back(property.get());

      // Getting property name hash as key 
      const auto propertyNameHash = property->getNameHash();

      // Adding property to the map for later reference
      _propertyMap[propertyNameHash] = std::move(property);
    } 
  };

  Game() = delete;
  virtual ~Game() = default;

  // Function to initialize game instance
  void initialize()
  {
    // Parsing and allocating game-specific storage
    _gameSpecificStorageSize = getGameSpecificStorageSize();
    _gameSpecificStorage = (uint8_t*)calloc(_gameSpecificStorageSize, sizeof(uint8_t));

    // Initializing the particular game selected
    initializeImpl();

    // Updating game specific values
    updateGameSpecificValues();
  }
 
  // Function to update game state internal information
  void updateGameState()
  {
    // Updating derived values
    updateGameSpecificValues();

    // Then re-evaluate rules
    evaluateRules();

    // Determining new game state type
    updateGameStateType();

    // Running game-specific rule actions
    runGameSpecificRuleActions();
    
    // Updating game reward
    updateReward();
  }
  
  // Function to advance state. Returns a vector with the performed inputs (including skip frames)
  virtual std::vector<std::string> advanceState(const std::string& input)
  {
    // Performing the requested input
    const auto performedInputs = advanceStateImpl(input);

    return performedInputs;
  }

  // Serialization routine -- simply a call to the underlying emulator
  void serializeState(uint8_t* outputStateData) const
  { 
     size_t pos = 0;

     // Serializing game-specific data
     memcpy(&outputStateData[pos], _gameSpecificStorage, _gameSpecificStorageSize);
     pos += _gameSpecificStorageSize;

     // Serializing internal emulator state
     _emulator->serializeState(&outputStateData[pos]);
     pos += _emulator->getStateSize();
  }

  // Deserialization routine
  void deserializeState(const uint8_t* inputStateData)
  {
     size_t pos = 0;

     // Deserializing game-specific data
     memcpy(_gameSpecificStorage, &inputStateData[pos], _gameSpecificStorageSize);
     pos += _gameSpecificStorageSize;

     // Deserializing state data into the emulator
     _emulator->deserializeState(&inputStateData[pos]); 
     pos += _emulator->getStateSize();
  }

  // This function returns the size of the game state
  size_t getStateSize() const
  {
    size_t stateSize = 0;

    // Adding the size of game specific storage
    stateSize += _gameSpecificStorageSize;

    // Adding the size of the emulator state
    stateSize += _emulator->getStateSize();

    return stateSize;
  }

  // This function computes the hash for the current state
  virtual hash_t computeHash() const 
  {
    // Storage for hash calculation
    MetroHash128 hashEngine;

    // Processing hashable game properties
    for (const auto& p : _propertyHashSet) hashEngine.Update(p->getPointer(), p->getSize());

    // Processing any additional game-specific hash 
    computeAdditionalHashing(hashEngine);
 
    hash_t result;
    hashEngine.Finalize(reinterpret_cast<uint8_t *>(&result));
    return result;
  }

  // Function to print
  void printStateInfo() const
  {
   // Getting maximum printable property name, for formatting purposes
   const size_t separatorSize = 4;
   size_t maximumNameSize = 0;
   for (const auto& p : _propertyPrintSet) maximumNameSize = std::max(maximumNameSize, p->getName().size());

   // Printing game state
   LOG("[J+]  + Game State Type: ");
   if (_stateType == stateType_t::normal) LOG("Normal");
   if (_stateType == stateType_t::win) LOG("Win");
   if (_stateType == stateType_t::fail) LOG("Fail");
   if (_stateType == stateType_t::checkpoint) LOG("Checkpoint");
   LOG("\n");

   // Printing game state
   LOG("[J+]  + Game State Reward: %f\n", _reward);

   // Printing rule status
   LOG("[J+]  + Rule Status: ");
   for (size_t i = 0; i < _rules.size(); i++) LOG("%d", _rulesStatus[i] ? 1 : 0);
   LOG("\n");

   // Getting state hash 
   const auto hash = hashToString(computeHash());

   // Printing state hash
   LOG("[J+]  + Hash: %s\n", hash.c_str());

   // Printing game properties defined in the script file
   LOG("[J+]  + Game Properties: \n");
   for (const auto& p : _propertyPrintSet)
   {
     // Getting property name
     const auto& name = p->getName();

     // Printing property name first
     LOG("[J+]    + '%s':", name.c_str());

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
     if (p->getDatatype() == Property::datatype_t::dt_float32)  LOG("0x%f    (0x%X)\n",  p->getValue<float>(),    p->getValue<uint32_t>());
     if (p->getDatatype() == Property::datatype_t::dt_float64)  LOG("0x%f    (0x%lX)\n", p->getValue<double>(),   p->getValue<uint64_t>());
     if (p->getDatatype() == Property::datatype_t::dt_bool)     LOG("%1u\n",             p->getValue<bool>() );
   }

   // Printing game-specific stuff now
   printStateInfoImpl();
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
    _rulesStatus.resize(_rules.size());

    // Clearing the status vector evaluation
    for (size_t i = 0; i < _rulesStatus.size(); i++) _rulesStatus[i] = false;
  }
 
  // Individual rule parser 
  void parseRule(Rule& rule, const nlohmann::json& ruleJs) 
  {
    // Getting rule condition array
    const auto& conditions = JSON_GET_ARRAY(ruleJs, "Conditions");

    // Getting rule action array
    const auto& actions = JSON_GET_ARRAY(ruleJs, "Actions");

    // Parsing rule conditions
    for (const auto& condition : conditions) rule.addCondition(parseRuleCondition(condition));  

    // Parsing rule actions
    for (const auto& action : actions) parseRuleAction(rule, action);  
  }

  std::unique_ptr<Condition> parseRuleCondition(const nlohmann::json& conditionJs)
  {
    // Parsing operator name
    const auto& opName = JSON_GET_STRING(conditionJs, "Op");
   
    // Getting operator type from its name
    const auto opType =  Condition::getOperatorType(opName);

    // Parsing first operand (property name)
    const auto& property1Name = JSON_GET_STRING(conditionJs, "Property");

    // Getting property name hash, for indexing
    const auto property1NameHash = hashString(property1Name);

    // Making sure the requested property exists in the property map
    if (_propertyMap.contains(property1NameHash) == false) EXIT_WITH_ERROR("[ERROR] Property '%s' has not been declared.\n", property1Name.c_str());

    // Getting property object
    const auto property1 = _propertyMap[property1NameHash].get();

    // Getting property data type
    auto datatype1 = property1->getDatatype();

    // Parsing second operand (number)
    if (conditionJs.contains("Value") == false) EXIT_WITH_ERROR("[ERROR] Rule condition missing 'Value' key.\n");

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
      const auto property2NameHash = hashString(property2Name);

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

  // Evaluates the rule set on a given frame. Returns true if it is a fail.
  void evaluateRules() 
  {
    // First, checking if the rules have been satisfied
    for (auto& entry : _rules)
    {
      // Getting rule
      auto& rule = *entry.second;

      // Getting rule index
      const auto ruleIdx = rule.getIndex(); 

      // Evaluate rule only if it's not yet satisfied
      if (_rulesStatus[ruleIdx] == false)
      {
        // Checking if conditions are met
        bool isSatisfied = rule.evaluate();

        // If it's achieved, update its status and run its actions
        if (isSatisfied) satisfyRule(rule);
      }
    }
  }

  void runGameSpecificRuleActions()
  {
     // First, checking if the rules have been satisfied
    for (auto& entry : _rules)
    {
      // Getting rule
      auto& rule = *entry.second;

      // Getting rule index
      const auto ruleIdx = rule.getIndex(); 

      // Run ations only if rule is satisfied
      if (_rulesStatus[ruleIdx] == true) for (const auto& action : rule.getActions()) action();
    }  
  }


  void updateGameStateType()
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
      if (_rulesStatus[ruleIdx] == true)
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

  void updateReward()
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
      if (_rulesStatus[ruleIdx] == true)
      {
       // Getting reward from satisfied rule
       const auto ruleReward = rule.getReward();

       // Adding it to the state reward
       _reward += ruleReward;
      }
    }

    // Adding any game-specific rewards
    _reward += calculateGameSpecificReward();
  }

  // Marks the given rule as satisfied, executes its actions, and recursively runs on its sub-satisfied rules
  void satisfyRule(Rule& rule) 
  {
    // Recursively run actions for the yet unsatisfied rules that are satisfied by this one and mark them as satisfied
    for (const auto& satisfyRuleLabel : rule.getSatisfyRuleLabels())
    {
      // Making sure referenced label exists
      if (_rules.contains(satisfyRuleLabel) == false) EXIT_WITH_ERROR("[ERROR] Unrecognized rule label %lu in satisfy array\n", satisfyRuleLabel);

      // Obtaining subrule to satisfy
      auto& subRule = _rules.at(satisfyRuleLabel);

      // Getting index from the subrule
      auto subRuleIdx = subRule->getIndex();

      // Only activate it if it hasn't been activated before
      if (_rulesStatus[subRuleIdx] == false) satisfyRule(*subRule);
    }

    // Getting rule index
    const auto ruleIdx = rule.getIndex(); 

    // Setting status to satisfied
    _rulesStatus[ruleIdx] = true;
  }

  // Function to get emulator name
  static std::string getName();

  // Returns pointer to the internal emulator
  Emulator* getEmulator() { return _emulator.get(); }

  // Function to obtain emulator based on name
  static std::unique_ptr<Game> getGame(const std::string& gameName, std::unique_ptr<Emulator>& emulator, const nlohmann::json& config);

  protected:

  // This function updates derivate values (those who require calculation from raw values) after a state is re-loaded
  virtual float calculateGameSpecificReward() const = 0;
  virtual void updateGameSpecificValues() = 0;
  virtual void computeAdditionalHashing(MetroHash128& hashEngine) const = 0;
  virtual void initializeImpl() = 0;
  virtual size_t getGameSpecificStorageSize() const = 0;
  virtual void printStateInfoImpl() const = 0;
  virtual std::vector<std::string> advanceStateImpl(const std::string& input) = 0;
  virtual bool parseRuleActionImpl(Rule& rule, const std::string& actionType, const nlohmann::json& actionJs) = 0;

  // Current game state type
  stateType_t _stateType;

  // Current game state reward
  float _reward = 0.0;

  // For game states that represent checkpoints, store their tolerance here
  size_t _checkpointTolerance = 0;

  // Underlying emulator instance
  const std::unique_ptr<Emulator> _emulator;

  // Game-Specific storage size and pointer
  size_t _gameSpecificStorageSize;
  uint8_t* _gameSpecificStorage;

  // Game script rules. Using vector to preserve their ordering
  std::map<Rule::label_t, std::unique_ptr<Rule>> _rules;

  // Storage for status vector indicating whether the rules have been satisfied
  std::vector<bool> _rulesStatus;

  // Property hash set to quickly distinguish states from each other 
  std::unordered_set<const Property*> _propertyHashSet;

  // Property print vector for printing game information to screen. Its a vector to keep the order
  std::vector<const Property*> _propertyPrintSet;

  // Property map to store all registered properties for future reference, indexed by their name hash
  std::map<hash_t, std::unique_ptr<Property>> _propertyMap;
};

} // namespace jaffarPlus