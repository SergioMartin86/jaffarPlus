#pragma once

#include <set>
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

  // Base constructor
  Game(std::unique_ptr<Emulator>& emulator, const nlohmann::json& config) : _emulator(std::move(emulator))
  {
    // Parse anything that might be generally relevant here
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


  // Function to advance state. Returns a vector with the performed inputs (including skip frames)
  virtual std::vector<std::string> advanceState(const std::string& input)
  {
    // Performing the requested input
    const auto performedInputs = advanceStateImpl(input);

    // Updating derived values
    updateGameSpecificValues();

    // Then re-evaluate rules
    // evaluateRules();

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

     // Updating the game's derived values
     updateGameSpecificValues();

     // Then re-evaluate rules
     // evaluateRules();
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
  virtual hash_t computeHash() const = 0;

  // This function updates derivate values (those who require calculation from raw values) after a state is re-loaded
  virtual void updateGameSpecificValues() = 0;

  // Function to print
  void printStateInfo() const
  {
   const auto hash = hashToString(computeHash());
   LOG("[J+]  + Hash:                               %s\n", hash.c_str());

   printStateInfoImpl();
  }

  // Parsing game rules
  void parseRules(const nlohmann::json& rulesJson)
  {
    // Parse each individual rule
    if (rulesJson.is_array() == false) EXIT_WITH_ERROR("Passed rules JSON object is not an array.");
 
    // Evaluate each rule
    for (auto& ruleJson : rulesJson)
    {
      // Check if rule is a key/value object
      if (rulesJson.is_object() == false) EXIT_WITH_ERROR("Passed rule is not a JSON object.");
      
      // Parsing json into a rule class
      auto rule = std::move(parseRule(ruleJson));
    }
  
    // Then re-evaluate them
    //evaluateRules();
  }
 
  // Individual rule parser 
  std::unique_ptr<Rule> parseRule(const nlohmann::json& ruleJs) 
  {
    // Getting rule label
    auto label = JSON_GET_NUMBER(Rule::ruleLabel_t, ruleJs, "Label");

    // Getting rule condition array
    const auto& conditions = JSON_GET_ARRAY(ruleJs, "Conditions");

    // Getting rule action array
    const auto& actions = JSON_GET_ARRAY(ruleJs, "Actions");

    // Creating new rule with the given label
    auto rule = std::make_unique<Rule>(label);
 
    // Parsing rule conditions
    for (const auto& condition : conditions) parseRuleCondition(*rule, condition);  

    // Parsing rule actions
    for (const auto& action : actions) parseRuleAction(*rule, action);  

    return rule;
  }

  void parseRuleCondition(Rule& rule, const nlohmann::json& conditionJs)
  {
    // Parsing operator type
    const auto& op = JSON_GET_ARRAY(conditionJs, "Op");

    // Parsing first operand (property name)
    const auto& op = JSON_GET_STRING(conditionJs, "Property");
    datatype_t dtype = getPropertyType(conditionJs);
    auto property = getPropertyPointer(conditionJs, (GameInstance*)gameInstance);

    // Parsing second operand (number)
    if (isDefined(conditionJs, "Value") == false) EXIT_WITH_ERROR("[ERROR] Rule %lu condition missing 'Value' key.\n", _label);

    bool valueFound = false;
    if (conditionJs["Value"].is_number())
    {
     // Creating new condition object
     Condition *condition = NULL;
     if (dtype == dt_uint8) condition = new _vCondition<uint8_t>(operation, property, NULL, conditionJs["Value"].get<uint8_t>());
     if (dtype == dt_uint16) condition = new _vCondition<uint16_t>(operation, property, NULL, conditionJs["Value"].get<uint16_t>());
     if (dtype == dt_uint32) condition = new _vCondition<uint32_t>(operation, property, NULL, conditionJs["Value"].get<uint32_t>());
     if (dtype == dt_int8) condition = new _vCondition<int8_t>(operation, property, NULL, conditionJs["Value"].get<int8_t>());
     if (dtype == dt_int16) condition = new _vCondition<int16_t>(operation, property, NULL, conditionJs["Value"].get<int16_t>());
     if (dtype == dt_int32) condition = new _vCondition<int32_t>(operation, property, NULL, conditionJs["Value"].get<int32_t>());
     if (dtype == dt_double) condition = new _vCondition<double>(operation, property, NULL, conditionJs["Value"].get<double>());
     if (dtype == dt_float) condition = new _vCondition<float>(operation, property, NULL, conditionJs["Value"].get<float>());

     // Adding condition to the list
     _conditions.push_back(condition);

     valueFound = true;
    }

    if (conditionJs["Value"].is_string())
    {
     // Hack: fooling property parser with value
     nlohmann::json newCondition;
     newCondition["Property"] = conditionJs["Value"];

     // Creating new property
     datatype_t valueType = getPropertyType(newCondition);
     if (valueType != dtype) EXIT_WITH_ERROR("[ERROR] Rule %lu, property (%s) and value (%s) types must coincide.\n", _label, conditionJs["Property"].get<std::string>(), conditionJs["Value"].get<std::string>());

     // Getting value pointer
     auto valuePtr = getPropertyPointer(newCondition, (GameInstance*)gameInstance);

     // Adding condition to the list
     Condition *condition = NULL;
     if (dtype == dt_uint8) condition = new _vCondition<uint8_t>(operation, property, valuePtr, 0);
     if (dtype == dt_uint16) condition = new _vCondition<uint16_t>(operation, property, valuePtr, 0);
     if (dtype == dt_uint32) condition = new _vCondition<uint32_t>(operation, property, valuePtr, 0);
     if (dtype == dt_int8) condition = new _vCondition<int8_t>(operation, property, valuePtr, 0);
     if (dtype == dt_int16) condition = new _vCondition<int16_t>(operation, property, valuePtr, 0);
     if (dtype == dt_int32) condition = new _vCondition<int32_t>(operation, property, valuePtr, 0);
     if (dtype == dt_double) condition = new _vCondition<double>(operation, property, valuePtr, 0);
     if (dtype == dt_float) condition = new _vCondition<float>(operation, property, valuePtr, 0);
     _conditions.push_back(condition);

     valueFound = true;
    }

    if (valueFound == false) EXIT_WITH_ERROR("[ERROR] Rule %lu contains an invalid 'Value' key.\n", _label, conditionJs["Value"].dump().c_str());
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
    if (recognizedActionType == false) EXIT_WITH_ERROR("[ERROR] Unrecognized action '%s' in rule %lu\n", rule.getLabel(), actionType.c_str());
  }

  // Function to get emulator name
  static std::string getName();

  // Returns pointer to the internal emulator
  Emulator* getEmulator() { return _emulator.get(); }

  // Function to obtain emulator based on name
  static std::unique_ptr<Game> getGame(const std::string& gameName, std::unique_ptr<Emulator>& emulator, const nlohmann::json& config);

  protected:

  virtual void initializeImpl() = 0;
  virtual size_t getGameSpecificStorageSize() const = 0;
  virtual void printStateInfoImpl() const = 0;
  virtual std::vector<std::string> advanceStateImpl(const std::string& input) = 0;
  virtual bool parseRuleActionImpl(Rule& rule, const std::string& actionType, const nlohmann::json& actionJs) = 0;

  // Underlying emulator instance
  const std::unique_ptr<Emulator> _emulator;

  // Game-Specific storage size and pointer
  size_t _gameSpecificStorageSize;
  uint8_t* _gameSpecificStorage;

  // Game script rules
  std::vector<std::unique_ptr<Rule>> _rules;
};

} // namespace jaffarPlus