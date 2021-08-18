#include "rule.h"

extern size_t _currentStep;

Rule::Rule(nlohmann::json ruleJs, blastemInstance *blastem)
{
  // Adding identifying label for the rule
  if (isDefined(ruleJs, "Label") == false) EXIT_WITH_ERROR("[ERROR] Rule missing 'Label' key.\n");
  _label = ruleJs["Label"].get<size_t>();

  // Defining default values
  _reward = 0.0;
  _isWinRule = false;
  _isFailRule = false;
  _isRestartRule = false; // Force Ctrl+A
  _isRemoveGuard = false;
  _isFlushRule = false;

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
    size_t dtype = getPropertyType(conditionJs["Property"].get<std::string>());
    auto property = getPropertyPointer(conditionJs["Property"].get<std::string>(), blastem);

    // Parsing second operand (number)
    if (isDefined(conditionJs, "Value") == false) EXIT_WITH_ERROR("[ERROR] Rule %lu condition missing 'Value' key.\n", _label);

    bool valueFound = false;
    if (conditionJs["Value"].is_number())
    {
     // Creating new condition object
     Condition *condition;
     if (dtype == 8) condition = new _vCondition<uint8_t>(operation, property, NULL, conditionJs["Value"].get<uint8_t>());
     if (dtype == 16) condition = new _vCondition<uint16_t>(operation, property, NULL, conditionJs["Value"].get<uint16_t>());
     if (dtype == 32) condition = new _vCondition<uint32_t>(operation, property, NULL, conditionJs["Value"].get<uint32_t>());

     // Adding condition to the list
     _conditions.push_back(condition);

     valueFound = true;
    }

    if (conditionJs["Value"].is_string())
    {
     // Creating new property
     size_t valueType = getPropertyType(conditionJs["Value"].get<std::string>());
     if (valueType != dtype) EXIT_WITH_ERROR("[ERROR] Rule %lu, property (%s) and value (%s) types must coincide.\n", _label, conditionJs["Property"].get<std::string>(), conditionJs["Value"].get<std::string>());

     // Getting value pointer
     auto valuePtr = getPropertyPointer(conditionJs["Value"].get<std::string>(), blastem);

     // Adding condition to the list
     Condition *condition;
     if (dtype == 8) condition = new _vCondition<uint8_t>(operation, property, valuePtr, 0);
     if (dtype == 16) condition = new _vCondition<uint16_t>(operation, property, valuePtr, 0);
     if (dtype == 32) condition = new _vCondition<uint32_t>(operation, property, valuePtr, 0);
     _conditions.push_back(condition);

     valueFound = true;
    }

    if (valueFound == false) EXIT_WITH_ERROR("[ERROR] Rule %lu contains an invalid 'Value' key.\n", _label, conditionJs["Value"].dump().c_str());
  }

  // Adding Dependencies. All of them must be achieved for the rule to count
  if (isDefined(ruleJs, "Dependencies") == false) EXIT_WITH_ERROR("[ERROR] Rule missing 'Dependencies' key.\n");
  _dependenciesLabels = ruleJs["Dependencies"].get<std::vector<size_t>>();
  _dependenciesIndexes.resize(_dependenciesLabels.size());

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

   // Storing restart state
   if (actionType == "Restart Level")
   {
     _isRestartRule = true;
     recognizedActionType = true;
   }

   // Storing flush state
   if (actionType == "Flush Database")
   {
     _isFlushRule = true;
     recognizedActionType = true;
   }


   // Removing guard
   if (actionType == "Remove Guard")
   {
     _isRemoveGuard = true;
     recognizedActionType = true;
   }

   if (actionType == "Set Kid Horizontal Magnet Intensity")
   {
     magnet_t newMagnet;
     if (isDefined(actionJs, "Room") == false) EXIT_WITH_ERROR("[ERROR] Rule %lu Action %lu missing 'Room' key.\n", _label, actionId);
     if (isDefined(actionJs, "Value") == false) EXIT_WITH_ERROR("[ERROR] Rule %lu Action %lu missing 'Value' key.\n", _label, actionId);
     newMagnet.value = actionJs["Value"].get<float>();
     newMagnet.room = actionJs["Room"].get<uint8_t>();
     _kidMagnetIntensityX.push_back(newMagnet);
     recognizedActionType = true;
   }

   if (actionType == "Set Kid Horizontal Magnet Position")
   {
     magnet_t newMagnet;
     if (isDefined(actionJs, "Room") == false) EXIT_WITH_ERROR("[ERROR] Rule %lu Action %lu missing 'Room' key.\n", _label, actionId);
     if (isDefined(actionJs, "Value") == false) EXIT_WITH_ERROR("[ERROR] Rule %lu Action %lu missing 'Value' key.\n", _label, actionId);
     newMagnet.value = actionJs["Value"].get<float>();
     newMagnet.room = actionJs["Room"].get<uint8_t>();
     _kidMagnetPositionX.push_back(newMagnet);
     recognizedActionType = true;
   }

   if (actionType == "Set Kid Vertical Magnet Intensity")
   {
     magnet_t newMagnet;
     if (isDefined(actionJs, "Room") == false) EXIT_WITH_ERROR("[ERROR] Rule %lu Action %lu missing 'Room' key.\n", _label, actionId);
     if (isDefined(actionJs, "Value") == false) EXIT_WITH_ERROR("[ERROR] Rule %lu Action %lu missing 'Value' key.\n", _label, actionId);
     newMagnet.value = actionJs["Value"].get<float>();
     newMagnet.room = actionJs["Room"].get<uint8_t>();
     _kidMagnetIntensityY.push_back(newMagnet);
     recognizedActionType = true;
   }

   if (actionType == "Set Guard Horizontal Magnet Intensity")
   {
     magnet_t newMagnet;
     if (isDefined(actionJs, "Room") == false) EXIT_WITH_ERROR("[ERROR] Rule %lu Action %lu missing 'Room' key.\n", _label, actionId);
     if (isDefined(actionJs, "Value") == false) EXIT_WITH_ERROR("[ERROR] Rule %lu Action %lu missing 'Value' key.\n", _label, actionId);
     newMagnet.value = actionJs["Value"].get<float>();
     newMagnet.room = actionJs["Room"].get<uint8_t>();
     _guardMagnetIntensityX.push_back(newMagnet);
     recognizedActionType = true;
   }

   if (actionType == "Set Guard Horizontal Magnet Position")
   {
     magnet_t newMagnet;
     if (isDefined(actionJs, "Room") == false) EXIT_WITH_ERROR("[ERROR] Rule %lu Action %lu missing 'Room' key.\n", _label, actionId);
     if (isDefined(actionJs, "Value") == false) EXIT_WITH_ERROR("[ERROR] Rule %lu Action %lu missing 'Value' key.\n", _label, actionId);
     newMagnet.value = actionJs["Value"].get<float>();
     newMagnet.room = actionJs["Room"].get<uint8_t>();
     _guardMagnetPositionX.push_back(newMagnet);
     recognizedActionType = true;
   }

   if (actionType == "Set Guard Vertical Magnet Intensity")
   {
     magnet_t newMagnet;
     if (isDefined(actionJs, "Room") == false) EXIT_WITH_ERROR("[ERROR] Rule %lu Action %lu missing 'Room' key.\n", _label, actionId);
     if (isDefined(actionJs, "Value") == false) EXIT_WITH_ERROR("[ERROR] Rule %lu Action %lu missing 'Value' key.\n", _label, actionId);
     newMagnet.value = actionJs["Value"].get<float>();
     newMagnet.room = actionJs["Room"].get<uint8_t>();
     _guardMagnetIntensityY.push_back(newMagnet);
     recognizedActionType = true;
   }

   if (recognizedActionType == false) EXIT_WITH_ERROR("[ERROR] Unrecognized rule %lu, action %lu type: %s\n", _label, actionId, actionType.c_str());
  }
}

bool Rule::evaluate()
{
  bool isAchieved = true;

  // The rule is achieved only if all conditions are met
  for (size_t i = 0; i < _conditions.size(); i++)
    isAchieved = isAchieved && _conditions[i]->evaluate();

  return isAchieved;
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

size_t Rule::getPropertyType(const std::string &property)
{
 if (property == "Kid Frame") return 8;
 if (property == "Kid Current HP") return 8;
 if (property == "Kid Max HP") return 8;
 if (property == "Kid Position X") return 16;
 if (property == "Kid Position Y") return 16;
 if (property == "Kid Falling Speed") return 8;
 if (property == "Kid Direction") return 8;
 if (property == "Kid Room") return 8;
 if (property == "Kid Has Sword") return 8;

 if (property == "Guard Frame") return 8;
 if (property == "Guard Current HP") return 8;
 if (property == "Guard 2 Current HP") return 8;
 if (property == "Guard Max HP") return 8;
 if (property == "Guard Position X") return 16;
 if (property == "Guard Position Y") return 16;
 if (property == "Guard Direction") return 8;
 if (property == "Guard Room") return 8;

 if (property == "Video Frame") return 16;
 if (property == "Frames Per Step") return 8;
 if (property == "Current Level") return 8;
 if (property == "Drawn Room") return 8;
 if (property == "Checkpoint Pointer") return 32;
 if (property == "Is Feather Fall") return 8;

 if (property == "Level 3 Exit Door State") return 8;
 if (property == "Level 3 Fast Exit Door State") return 8;
 if (property == "Level 3 Fast Route Door State") return 8;
 if (property == "Level 4 Exit Door State") return 8;
 if (property == "Level 5 Carpet Hatch State") return 8;
 if (property == "Level 6 Right Exit Door State") return 8;
 if (property == "Level 7 Post Potion Door State") return 8;
 if (property == "Level 7 Exit Door State") return 8;
 if (property == "Level 8 Exit Door State") return 8;
 if (property == "Level 11 Exit Door State") return 8;
 if (property == "Level 11 Post Potion Door State") return 8;
 if (property == "Level 11 Exit Room Left Door State") return 8;
 if (property == "Level 12 First Door Door State") return 8;
 if (property == "Level 12 Exit Door State") return 8;
 if (property == "Level 12 Weird Room Door State") return 8;
 if (property == "Level 13 First Room Door State") return 8;
 if (property == "Level 13 Exit Door State") return 8;
 if (property == "Level 14 Fake Jaffars Left") return 8;

 EXIT_WITH_ERROR("[Error] Rule %lu, unrecognized property: %s\n", _label, property.c_str());

 return 1;
}

void *Rule::getPropertyPointer(const std::string &property, blastemInstance *blastem)
{
  if (property == "Kid Frame") return &blastem->_state.kidFrame;
  if (property == "Kid Current HP") return &blastem->_state.kidCurrentHP;
  if (property == "Kid Max HP") return &blastem->_state.kidMaxHP;
  if (property == "Kid Position X") return &blastem->_state.kidPositionX;
  if (property == "Kid Position Y") return &blastem->_state.kidPositionY;
  if (property == "Kid Falling Speed") return &blastem->_state.kidFallingSpeed;
  if (property == "Kid Direction") return &blastem->_state.kidDirection;
  if (property == "Kid Room") return &blastem->_state.kidRoom;
  if (property == "Kid Has Sword") return &blastem->_state.kidHasSword;

  if (property == "Guard Frame") return &blastem->_state.guardFrame;
  if (property == "Guard Current HP") return &blastem->_state.guardCurrentHP;
  if (property == "Guard 2 Current HP") return &blastem->_state.guard2CurrentHP;
  if (property == "Guard Max HP") return &blastem->_state.guardMaxHP;
  if (property == "Guard Position X") return &blastem->_state.guardPositionX;
  if (property == "Guard Position Y") return &blastem->_state.guardPositionY;
  if (property == "Guard Direction") return &blastem->_state.guardDirection;
  if (property == "Guard Room") return &blastem->_state.guardRoom;

  if (property == "Video Frame") return &blastem->_state.videoFrame;
  if (property == "Frames Per Step") return &blastem->_state.framesPerStep;
  if (property == "Current Level") return &blastem->_state.currentLevel;
  if (property == "Drawn Room") return &blastem->_state.drawnRoom;
  if (property == "Checkpoint Pointer") return &blastem->_state.checkpointPointer;
  if (property == "Is Feather Fall") return &blastem->_state.slowfallFramesLeft;

  if (property == "Level 3 Exit Door State") return &blastem->_state.lvl3ExitDoor;
  if (property == "Level 3 Fast Exit Door State") return &blastem->_state.lvl3FastExitDoor;
  if (property == "Level 3 Fast Route Door State") return &blastem->_state.lvl3FastRouteDoor;
  if (property == "Level 4 Exit Door State") return &blastem->_state.lvl4ExitDoor;
  if (property == "Level 5 Carpet Hatch State") return &blastem->_state.lvl5CarpetHatch;
  if (property == "Level 6 Right Exit Door State") return &blastem->_state.lvl6RightDoor;
  if (property == "Level 7 Post Potion Door State") return &blastem->_state.lvl7PostPotionDoor;
  if (property == "Level 7 Exit Door State") return &blastem->_state.lvl7ExitDoor;
  if (property == "Level 8 Exit Door State") return &blastem->_state.lvl8ExitDoor;
  if (property == "Level 11 Exit Door State") return &blastem->_state.lvl11ExitDoor;
  if (property == "Level 11 Post Potion Door State") return &blastem->_state.lvl11PostPotionDoor;
  if (property == "Level 11 Exit Room Left Door State") return &blastem->_state.lvl11ExitRoomLeftDoor;
  if (property == "Level 12 First Door Door State") return &blastem->_state.lvl12FirstDoor;
  if (property == "Level 12 Exit Door State") return &blastem->_state.lvl12ExitDoor;
  if (property == "Level 12 Weird Room Door State") return &blastem->_state.lvl12WeirdRoomDoor;
  if (property == "Level 13 First Room Door State") return &blastem->_state.lvl13FirstRoomDoor;
  if (property == "Level 13 Exit Door State") return &blastem->_state.lvl13ExitDoor;
  if (property == "Level 14 Fake Jaffars Left") return &blastem->_state.lvl14FakeJaffarsLeft;

  EXIT_WITH_ERROR("[Error] Rule %lu, unrecognized property: %s\n", _label, property.c_str());

  return NULL;
}
