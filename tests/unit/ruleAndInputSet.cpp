#include "condition.hpp"
#include "inputSet.hpp"
#include "property.hpp"
#include "rule.hpp"
#include <gtest/gtest.h>
#include <jaffarCommon/exceptions.hpp>
#include <memory>

using namespace jaffarPlus;

// Builds a condition that evaluates to the requested boolean, using an equality
// test on two immediates (1 == 1 -> true, 1 == 0 -> false).
static std::unique_ptr<Condition> makeBoolCondition(bool value)
{
  return std::make_unique<_vCondition<uint32_t>>(Condition::op_equal, nullptr, nullptr, static_cast<uint32_t>(1), static_cast<uint32_t>(value ? 1 : 0));
}

// ------------------------------------------------------------------
// Rule
// ------------------------------------------------------------------

TEST(Rule, IndexAndLabel)
{
  Rule r(3, 42);
  EXPECT_EQ(r.getIndex(), 3u);
  EXPECT_EQ(r.getLabel(), 42u);
}

TEST(Rule, Defaults)
{
  Rule r(0, 0);
  EXPECT_FLOAT_EQ(r.getReward(), 0.0f);
  EXPECT_FALSE(r.isWinRule());
  EXPECT_FALSE(r.isFailRule());
  EXPECT_FALSE(r.isCheckpointRule());
  EXPECT_EQ(r.getCheckpointTolerance(), 0u);
  EXPECT_FALSE(r.isSaveSolutionRule());
  EXPECT_TRUE(r.getSaveSolutionPath().empty());
  EXPECT_TRUE(r.getActions().empty());
  EXPECT_TRUE(r.getSatisfyRuleLabels().empty());
  EXPECT_TRUE(r.getSatisfyRules().empty());
}

TEST(Rule, SettersAndGetters)
{
  Rule r(1, 7);

  r.setReward(2.5f);
  EXPECT_FLOAT_EQ(r.getReward(), 2.5f);

  r.setWinRule(true);
  EXPECT_TRUE(r.isWinRule());

  r.setFailRule(true);
  EXPECT_TRUE(r.isFailRule());

  r.setCheckpointRule(true);
  EXPECT_TRUE(r.isCheckpointRule());

  r.setCheckpointTolerance(11);
  EXPECT_EQ(r.getCheckpointTolerance(), 11u);

  r.setSaveSolutionRule(true);
  EXPECT_TRUE(r.isSaveSolutionRule());

  r.setSaveSolutionPath("/tmp/solution.sol");
  EXPECT_EQ(r.getSaveSolutionPath(), "/tmp/solution.sol");
}

TEST(Rule, ActionsAndSatisfyRules)
{
  Rule r(0, 0);

  int counter = 0;
  r.addAction([&counter]() { counter++; });
  ASSERT_EQ(r.getActions().size(), 1u);
  r.getActions()[0]();
  EXPECT_EQ(counter, 1);

  r.addSatisfyRuleLabel(100);
  r.addSatisfyRuleLabel(200);
  EXPECT_EQ(r.getSatisfyRuleLabels().size(), 2u);
  EXPECT_EQ(r.getSatisfyRuleLabels().count(100), 1u);

  Rule sub(2, 200);
  r.addSatisfyRule(&sub);
  ASSERT_EQ(r.getSatisfyRules().size(), 1u);
  EXPECT_EQ(r.getSatisfyRules()[0]->getLabel(), 200u);
}

TEST(Rule, EvaluateNoConditionsIsTrue)
{
  Rule r(0, 0);
  EXPECT_TRUE(r.evaluate());
}

TEST(Rule, EvaluateAllConditionsTrue)
{
  Rule r(0, 0);
  r.addCondition(makeBoolCondition(true));
  r.addCondition(makeBoolCondition(true));
  EXPECT_TRUE(r.evaluate());
}

TEST(Rule, EvaluateWithOneFalseConditionIsFalse)
{
  Rule r(0, 0);
  r.addCondition(makeBoolCondition(true));
  r.addCondition(makeBoolCondition(false));
  EXPECT_FALSE(r.evaluate());
}

// ------------------------------------------------------------------
// InputSet
// ------------------------------------------------------------------

TEST(InputSet, Defaults)
{
  InputSet s;
  EXPECT_TRUE(s.getInputIndexes().empty());
  EXPECT_FALSE(s.getStopInputEvaluationFlag());
}

TEST(InputSet, AddInputsAndStopFlag)
{
  InputSet s;
  s.addInput(5);
  s.addInput(9);
  s.addInput(5); // duplicate: set semantics
  EXPECT_EQ(s.getInputIndexes().size(), 2u);
  EXPECT_EQ(s.getInputIndexes().count(9), 1u);

  s.setStopInputEvaluationFlag(true);
  EXPECT_TRUE(s.getStopInputEvaluationFlag());
}

TEST(InputSet, EvaluateNoConditionsIsTrue)
{
  InputSet s;
  EXPECT_TRUE(s.evaluate());
}

TEST(InputSet, EvaluateAllConditionsTrue)
{
  InputSet s;
  s.addCondition(makeBoolCondition(true));
  s.addCondition(makeBoolCondition(true));
  EXPECT_TRUE(s.evaluate());
}

TEST(InputSet, EvaluateWithOneFalseConditionIsFalse)
{
  InputSet s;
  s.addCondition(makeBoolCondition(true));
  s.addCondition(makeBoolCondition(false));
  EXPECT_FALSE(s.evaluate());
}
