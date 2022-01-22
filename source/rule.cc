#include "rule.h"

Rule::Rule(nlohmann::json ruleJs, quickNESInstance *nes)
{
  // Adding identifying label for the rule
  if (isDefined(ruleJs, "Label") == false) EXIT_WITH_ERROR("[ERROR] Rule missing 'Label' key.\n");
  _label = ruleJs["Label"].get<size_t>();

  // Defining default values
  _reward = 0.0f;
  _marioMagnetIntensityX = 0.0f;
  _marioMagnetIntensityY = 0.0f;
  _isWinRule = false;
  _isFailRule = false;

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
    datatype_t dtype = getPropertyType(conditionJs["Property"].get<std::string>());
    auto property = getPropertyPointer(conditionJs["Property"].get<std::string>(), nes);

    // Parsing second operand (number)
    if (isDefined(conditionJs, "Value") == false) EXIT_WITH_ERROR("[ERROR] Rule %lu condition missing 'Value' key.\n", _label);

    bool valueFound = false;
    if (conditionJs["Value"].is_number())
    {
     // Creating new condition object
     Condition *condition;
     if (dtype == dt_uint8) condition = new _vCondition<uint8_t>(operation, property, NULL, conditionJs["Value"].get<uint8_t>());
     if (dtype == dt_uint16) condition = new _vCondition<uint16_t>(operation, property, NULL, conditionJs["Value"].get<uint16_t>());
     if (dtype == dt_uint32) condition = new _vCondition<uint32_t>(operation, property, NULL, conditionJs["Value"].get<uint32_t>());
     if (dtype == dt_int8) condition = new _vCondition<int8_t>(operation, property, NULL, conditionJs["Value"].get<int8_t>());
     if (dtype == dt_int16) condition = new _vCondition<int16_t>(operation, property, NULL, conditionJs["Value"].get<int16_t>());
     if (dtype == dt_int32) condition = new _vCondition<int32_t>(operation, property, NULL, conditionJs["Value"].get<int32_t>());

     // Adding condition to the list
     _conditions.push_back(condition);

     valueFound = true;
    }

    if (conditionJs["Value"].is_string())
    {
     // Creating new property
     datatype_t valueType = getPropertyType(conditionJs["Value"].get<std::string>());
     if (valueType != dtype) EXIT_WITH_ERROR("[ERROR] Rule %lu, property (%s) and value (%s) types must coincide.\n", _label, conditionJs["Property"].get<std::string>(), conditionJs["Value"].get<std::string>());

     // Getting value pointer
     auto valuePtr = getPropertyPointer(conditionJs["Value"].get<std::string>(), nes);

     // Adding condition to the list
     Condition *condition;
     if (dtype == dt_uint8) condition = new _vCondition<uint8_t>(operation, property, valuePtr, 0);
     if (dtype == dt_uint16) condition = new _vCondition<uint16_t>(operation, property, valuePtr, 0);
     if (dtype == dt_uint32) condition = new _vCondition<uint32_t>(operation, property, valuePtr, 0);
     if (dtype == dt_int8) condition = new _vCondition<int8_t>(operation, property, valuePtr, 0);
     if (dtype == dt_int16) condition = new _vCondition<int16_t>(operation, property, valuePtr, 0);
     if (dtype == dt_int32) condition = new _vCondition<int32_t>(operation, property, valuePtr, 0);
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

   if (actionType == "Set Mario Screen Offset Magnet Intensity")
   {
     if (isDefined(actionJs, "Value") == false) EXIT_WITH_ERROR("[ERROR] Rule %lu Action %lu missing 'Value' key.\n", _label, actionId);
     _marioScreenOffsetMagnetIntensityX = actionJs["Value"].get<float>();
     recognizedActionType = true;
   }

   if (actionType == "Set Screen Horizontal Magnet Intensity")
   {
     if (isDefined(actionJs, "Value") == false) EXIT_WITH_ERROR("[ERROR] Rule %lu Action %lu missing 'Value' key.\n", _label, actionId);
     _screenMagnetIntensityX = actionJs["Value"].get<float>();
     recognizedActionType = true;
   }

   if (actionType == "Set Mario Horizontal Magnet Intensity")
   {
     if (isDefined(actionJs, "Value") == false) EXIT_WITH_ERROR("[ERROR] Rule %lu Action %lu missing 'Value' key.\n", _label, actionId);
     _marioMagnetIntensityX = actionJs["Value"].get<float>();
     recognizedActionType = true;
   }

   if (actionType == "Set Mario Vertical Magnet Intensity")
   {
     if (isDefined(actionJs, "Value") == false) EXIT_WITH_ERROR("[ERROR] Rule %lu Action %lu missing 'Value' key.\n", _label, actionId);
     _marioMagnetIntensityY = actionJs["Value"].get<float>();
     recognizedActionType = true;
   }

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

  EXIT_WITH_ERROR("[Error] Rule %lu, unrecognized operator: %s\n", _label, operation.c_str());

  return op_equal;
}

datatype_t Rule::getPropertyType(const std::string &property)
{
  if (property == "Mario State") return dt_uint8;
  if (property == "Mario Animation") return dt_uint8;
  if (property == "Mario Walking Frame") return dt_uint8;
  if (property == "Mario Walking Mode") return dt_uint8;

  if (property == "Screen Position X") return dt_uint16;

  if (property == "Mario Position Y") return dt_uint8;
  if (property == "Mario Velocity X") return dt_int8;

  if (property == "Mario Base Position X") return dt_uint8;
  if (property == "Mario Relative Position X") return dt_uint8;
  if (property == "Mario Position X") return dt_uint16;

  if (property == "Current World") return dt_uint8;
  if (property == "Current Stage") return dt_uint8;
  if (property == "Current Screen") return dt_uint8;
  if (property == "Level Entry Flag") return dt_uint8;
  if (property == "Warp Area Offset") return dt_uint16;
  if (property == "Enemy 1 Type") return dt_uint8;
  if (property == "Enemy 2 Type") return dt_uint8;
  if (property == "Enemy 3 Type") return dt_uint8;
  if (property == "Enemy 4 Type") return dt_uint8;
  if (property == "Enemy 5 Type") return dt_uint8;
  if (property == "Mario Screen Offset") return dt_int16;

  EXIT_WITH_ERROR("[Error] Rule %lu, unrecognized property: %s\n", _label, property.c_str());

  return dt_uint8;
}

void *Rule::getPropertyPointer(const std::string &property, quickNESInstance *nes)
{
  if (property == "Mario State") return nes->_marioState;
  if (property == "Mario Animation") return nes->_marioAnimation;
  if (property == "Mario Walking Frame") return nes->_marioWalkingFrame;
  if (property == "Mario Walking Mode") return nes->_marioWalkingMode;

  if (property == "Screen Position X") return &nes->_screenPosX; // Derivative value

  if (property == "Mario Base Position X") return nes->_marioBasePosX;
  if (property == "Mario Relative Position X") return nes->_marioRelPosX;
  if (property == "Mario Position X") return &nes->_marioPosX; // Derivative value

  if (property == "Mario Position Y") return nes->_marioPosY;
  if (property == "Mario Velocity X") return nes->_marioVelX;
  if (property == "Current World") return &nes->_currentWorld; // Derivative value
  if (property == "Current Stage") return &nes->_currentStage; // Derivative value
  if (property == "Current Screen") return nes->_screenBasePosX;
  if (property == "Level Entry Flag") return nes->_levelEntryFlag;
  if (property == "Warp Area Offset") return nes->_warpAreaOffset;
  if (property == "Enemy 1 Type") return nes->_enemy1Type;
  if (property == "Enemy 2 Type") return nes->_enemy2Type;
  if (property == "Enemy 3 Type") return nes->_enemy3Type;
  if (property == "Enemy 4 Type") return nes->_enemy4Type;
  if (property == "Enemy 5 Type") return nes->_enemy5Type;
  if (property == "Mario Screen Offset") return &nes->_marioScreenOffset; // Derivative value

  EXIT_WITH_ERROR("[Error] Rule %lu, unrecognized property: %s\n", _label, property.c_str());

  return NULL;
}
