#include <gtest/gtest.h>
#include "BLI_stack.h"

TEST(StackTest, PushPop) {
    BLI_Stack *stack = BLI_stack_new(sizeof(int), "test stack");
    int value = 42;
    BLI_stack_push(stack, &value);

    int result;
    BLI_stack_pop(stack, &result);

    ASSERT_EQ(result, value) << "Popped value does not match pushed value";

    BLI_stack_free(stack);
}

TEST(StackTest, IsEmpty) {
    BLI_Stack *stack = BLI_stack_new(sizeof(int), "test stack");

    ASSERT_TRUE(BLI_stack_is_empty(stack)) << "Stack should be empty initially";

    int value = 42;
    BLI_stack_push(stack, &value);

    ASSERT_FALSE(BLI_stack_is_empty(stack)) << "Stack should not be empty after push";

    BLI_stack_free(stack);
}

TEST(StackTest, PushPopMultiple) {
    BLI_Stack *stack = BLI_stack_new(sizeof(int), "test stack");

    int value1 = 1;
    BLI_stack_push(stack, &value1);

    int value2 = 2;
    BLI_stack_push(stack, &value2);

    int value3 = 3;
    BLI_stack_push(stack, &value3);

    int result;
    BLI_stack_pop(stack, &result);
    ASSERT_EQ(result, value3) << "Popped value does not match the last pushed value";

    BLI_stack_pop(stack, &result);
    ASSERT_EQ(result, value2) << "Popped value does not match the second last pushed value";

    BLI_stack_pop(stack, &result);
    ASSERT_EQ(result, value1) << "Popped value does not match the first pushed value";

    ASSERT_TRUE(BLI_stack_is_empty(stack)) << "Stack should be empty after popping all values";

    BLI_stack_free(stack);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
