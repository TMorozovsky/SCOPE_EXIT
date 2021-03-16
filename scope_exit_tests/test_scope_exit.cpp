#include <gtest/gtest.h>
#include <stdexcept>

#include "scope_exit/scope_exit.hpp"

namespace
{
  struct test_exception final : public std::runtime_error
  {
    explicit test_exception(int test_value = 0)
      : std::runtime_error{"test exception"}
      , test_value{test_value}
    {
    }

    int test_value = 0;
  };

  struct unwinding_exception final : public std::exception
  {
    [[nodiscard]] const char* what() const noexcept override
    {
      return "unwinding exception";
    }
  };

  template<typename Callable>
  struct exec_in_destructor final
  {
    static_assert(std::is_same_v<Callable, std::decay_t<Callable>>);

    explicit exec_in_destructor(Callable&& callable)
      : callable{std::move(callable)}
    {
    }

    exec_in_destructor(const exec_in_destructor&) = delete;
    exec_in_destructor& operator=(const exec_in_destructor&) = delete;

    ~exec_in_destructor()
    {
      std::move(this->callable)();
    }

    Callable callable;
  };
}

TEST(test_scope_exit, scope_exit_executes_its_function_when_no_exceptions_are_thrown)
{
  int counter = 0;

  if (true)
  {
    SCOPE_EXIT
    {
      ++counter;
    };
  }

  EXPECT_EQ(counter, 1);
}

TEST(test_scope_exit, scope_exit_executes_its_function_when_an_exception_is_thrown)
{
  int counter = 0;

  try
  {
    SCOPE_EXIT
    {
      ++counter;
    };

    throw test_exception{};
  }
  catch (const test_exception&)
  {
  }

  EXPECT_EQ(counter, 1);
}

TEST(test_scope_exit, scope_success_executes_its_function_when_no_exceptions_are_thrown)
{
  int counter = 0;

  if (true)
  {
    SCOPE_SUCCESS
    {
      ++counter;
    };
  }

  EXPECT_EQ(counter, 1);
}

TEST(test_scope_exit, scope_success_does_not_execute_its_function_when_an_exception_is_thrown)
{
  int counter = 0;

  try
  {
    SCOPE_SUCCESS
    {
      ++counter;
    };

    throw test_exception{};
  }
  catch (const test_exception&)
  {
  }

  EXPECT_EQ(counter, 0);
}

TEST(test_scope_exit, scope_fail_does_not_execute_its_function_when_no_exceptions_are_thrown)
{
  int counter = 0;

  if (true)
  {
    SCOPE_FAIL
    {
      ++counter;
    };
  }

  EXPECT_EQ(counter, 0);
}

TEST(test_scope_exit, scope_fail_executes_its_function_when_an_exception_is_thrown)
{
  int counter = 0;

  try
  {
    SCOPE_FAIL
    {
      ++counter;
    };

    throw test_exception{};
  }
  catch (const test_exception&)
  {
  }

  EXPECT_EQ(counter, 1);
}

TEST(test_scope_exit, scope_success_allows_to_throw_from_its_function)
{
  int caught_value = -1;

  try
  {
    SCOPE_SUCCESS
    {
      throw test_exception{42};
    };
  }
  catch (const test_exception& e)
  {
    caught_value = e.test_value;
  }

  EXPECT_EQ(caught_value, 42);
}

TEST(test_scope_exit, scope_success_can_throw_during_stack_unwinding)
{
  bool caught_test_exception = false;
  bool caught_unwinding_exception = false;
  int number_of_uncaught_exceptions_when_entered_destructor_of_exec_object = -1;

  try
  {
    exec_in_destructor exec_object{[&]()
    {
      number_of_uncaught_exceptions_when_entered_destructor_of_exec_object = std::uncaught_exceptions();
      try
      {
        SCOPE_SUCCESS
        {
          throw test_exception{};
        };
      }
      catch (const test_exception&)
      {
        caught_test_exception = true;
      }
    }};

    throw unwinding_exception{};
  }
  catch (const unwinding_exception&)
  {
    caught_unwinding_exception = true;
  }

  EXPECT_EQ(number_of_uncaught_exceptions_when_entered_destructor_of_exec_object, 1);
  EXPECT_TRUE(caught_test_exception);
  EXPECT_TRUE(caught_unwinding_exception);
}

namespace
{
  constexpr int compile_time_testing_result = []()
  {
    int result = 0;
    if (true)
    {
      SCOPE_EXIT
      {
        result = 42;
      };
    }
    return result;
  }();

  static_assert(compile_time_testing_result == 42);
}
