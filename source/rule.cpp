#include "rule.hpp"
#include "gameRule.hpp"
#include "gameInstance.hpp"

Rule::Rule()
{
  // Defining default values
  _reward = 0.0f;

  // Setting default win/fail values
  _isWinRule = false;
  _isFailRule = false;
}

void Rule::initialize(nlohmann::json ruleJs, void* gameInstance)
{
  // Adding identifying label for the rule
  if (isDefined(ruleJs, "Label") == false) EXIT_WITH_ERROR("[ERROR] Rule missing 'Label' key.\n");
  _label = ruleJs["Label"].get<size_t>();

  // Adding conditions. All of them must be satisfied for the rule to count
  if (isDefined(ruleJs, "Conditions") == false) EXIT_WITH_ERROR("[ERROR] Rule missing 'Conditions' key.\n");
  for (size_t i = 0; i < ruleJs["Conditions"].size(); i++)
  {
    auto conditionJs = ruleJs["Conditions"][i];

    // Parsing operation type
    if (isDefined(conditionJs, "Op") == false) EXIT_WITH_ERROR("[ERROR] Rule %lu condition missing 'Op' key.\n", _label);
    operator_t operation = getOperationType(conditionJs["Op"].get<std::string>());

    // Parsing first operand (property name)
    if (isDefined(conditionJs, "Property") == false) EXIT_WITH_ERROR("[ERROR] Rule %lu condition missing 'Property' key.\n", _label);
    if (conditionJs["Property"].is_string() == false) EXIT_WITH_ERROR("[ERROR] Rule %lu condition operand 1 must be a string with the name of a property.\n", _label);
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

  // Storing condition count
  _conditionCount = _conditions.size();

  // Adding Rules that are satisfied by this rule activation
  if (isDefined(ruleJs, "Satisfies") == false) EXIT_WITH_ERROR("[ERROR] Rule missing 'Satisfies' key.\n");
  _satisfiesLabels = ruleJs["Satisfies"].get<std::vector<size_t>>();
  _satisfiesIndexes.resize(_satisfiesLabels.size());

  // Adding actions
  if (isDefined(ruleJs, "Actions") == false) EXIT_WITH_ERROR("[ERROR] Rule missing 'Actions' key.\n");
  parseActions(ruleJs["Actions"]);
}

void Rule::parseActions(nlohmann::json actionsJs)
{
 for (size_t actionId = 0; actionId < actionsJs.size(); actionId++)
 {
   const auto actionJs = actionsJs[actionId];

   // Getting action type
   if (isDefined(actionJs, "Type") == false) EXIT_WITH_ERROR("[ERROR] Rule %lu Action %lu missing 'Type' key.\n", _label, actionId);
   std::string actionType = actionJs["Type"].get<std::string>();

   // Running the action, depending on the type
   bool recognizedActionType = false;

   if (actionType == "Add Reward")
   {
     if (isDefined(actionJs, "Value") == false) EXIT_WITH_ERROR("[ERROR] Rule %lu Action %lu missing 'Value' key.\n", _label, actionId);
     _reward = actionJs["Value"].get<float>();
     recognizedActionType = true;
   }

   // Storing fail state
   if (actionType == "Trigger Fail")
   {
     _isFailRule = true;
     recognizedActionType = true;
   }

   // Storing win state
   if (actionType == "Trigger Win")
   {
     _isWinRule = true;
     recognizedActionType = true;
   }

   // If not recognized yet, it must be a game specific action
   if (recognizedActionType == false) recognizedActionType = parseGameAction(actionJs, actionId);

   // If not recognized at all, then fail
   if (recognizedActionType == false) EXIT_WITH_ERROR("[ERROR] Unrecognized rule %lu, action %lu type: %s\n", _label, actionId, actionType.c_str());
  }
}

operator_t Rule::getOperationType(const std::string &operation)
{
  if (operation == "==") return op_equal;
  if (operation == "!=") return op_not_equal;
  if (operation == ">") return op_greater;
  if (operation == ">=") return op_greater_or_equal;
  if (operation == "<") return op_less;
  if (operation == "<=") return op_less_or_equal;
  if (operation == "BitTrue") return op_bit_true;
  if (operation == "BitFalse") return op_bit_false;

  EXIT_WITH_ERROR("[Error] Rule %lu, unrecognized operator: %s\n", _label, operation.c_str());

  return op_equal;
}

