#pragma once

#include <type_traits>
#include <exception>
#include <utility>

#if (__cplusplus >= 202002L) || (defined(_MSC_VER) && defined(_MSVC_LANG) && (_MSVC_LANG > 201703L))
#  define CONSTEXPR_DESTRUCTORS_SUPPORTED_BY_SCOPE_EXIT 1
#else
#  define CONSTEXPR_DESTRUCTORS_SUPPORTED_BY_SCOPE_EXIT 0
#endif

namespace detail::scope_exit_detail
{
  template<typename Function, typename Policy>
  class basic_scope_guard final : private Policy
  {
    static_assert(::std::is_class_v<Function> || ::std::is_pointer_v<Function>);

  public:
    constexpr explicit basic_scope_guard(Function&& function_on_exit) noexcept(::std::is_nothrow_move_constructible_v<Function>)
      : function_on_exit{::std::move(function_on_exit)}
    {
    }

    basic_scope_guard(const basic_scope_guard&) = delete;
    basic_scope_guard& operator=(const basic_scope_guard&) = delete;

#if CONSTEXPR_DESTRUCTORS_SUPPORTED_BY_SCOPE_EXIT
    constexpr
#else
    inline
#endif
      ~basic_scope_guard() noexcept(Policy::is_destructor_noexcept)
    {
      if (this->should_execute_function_in_destructor())
        ::std::move(this->function_on_exit)();
    }

  private:
    Function function_on_exit;
  };

  class policy_scope_exit
  {
  protected:
    constexpr static bool is_destructor_noexcept = true;

    constexpr bool should_execute_function_in_destructor() const noexcept
    {
      return true;
    }
  };

  class policy_with_initial_uncaught_exceptions
  {
  protected:
    inline policy_with_initial_uncaught_exceptions() noexcept
      : initial_uncaught_exceptions{::std::uncaught_exceptions()}
    {
    }

    const int initial_uncaught_exceptions;
  };

  class policy_scope_success : private policy_with_initial_uncaught_exceptions
  {
  protected:
    constexpr static bool is_destructor_noexcept = false;

    inline bool should_execute_function_in_destructor() const noexcept
    {
      return ::std::uncaught_exceptions() == this->initial_uncaught_exceptions;
    }
  };

  class policy_scope_fail : private policy_with_initial_uncaught_exceptions
  {
  protected:
    constexpr static bool is_destructor_noexcept = true;

    inline bool should_execute_function_in_destructor() const noexcept
    {
      return ::std::uncaught_exceptions() > this->initial_uncaught_exceptions;
    }
  };

  template<typename Function>
  using scope_exit_guard = basic_scope_guard<Function, policy_scope_exit>;

  template<typename Function>
  using scope_success_guard = basic_scope_guard<Function, policy_scope_success>;

  template<typename Function>
  using scope_fail_guard = basic_scope_guard<Function, policy_scope_fail>;

  struct scope_exit_guard_creator
  {
  };

  struct scope_success_guard_creator
  {
  };

  struct scope_fail_guard_creator
  {
  };

  template<typename Function>
  constexpr auto operator<<(scope_exit_guard_creator, Function&& function_on_exit)
  {
    return scope_exit_guard<::std::decay_t<Function>>{::std::forward<Function>(function_on_exit)};
  }

  template<typename Function>
  constexpr auto operator<<(scope_success_guard_creator, Function&& function_on_success)
  {
    return scope_success_guard<::std::decay_t<Function>>{::std::forward<Function>(function_on_success)};
  }

  template<typename Function>
  constexpr auto operator<<(scope_fail_guard_creator, Function&& function_on_fail)
  {
    return scope_fail_guard<::std::decay_t<Function>>{::std::forward<Function>(function_on_fail)};
  }
} // namespace detail::scope_exit_detail

#define IMPL_SCOPE_EXIT_CONCATENATE_IDENTIFIER_HELPER(identifier, line) identifier ## line
#define IMPL_SCOPE_EXIT_CONCATENATE_IDENTIFIER(identifier, line) IMPL_SCOPE_EXIT_CONCATENATE_IDENTIFIER_HELPER(identifier, line)
#define IMPL_SCOPE_EXIT_MAKE_UNIQUE_IDENTIFIER(identifier) IMPL_SCOPE_EXIT_CONCATENATE_IDENTIFIER(identifier, __LINE__)

#define SCOPE_EXIT \
  auto IMPL_SCOPE_EXIT_MAKE_UNIQUE_IDENTIFIER(scope_exit_guard_) = ::detail::scope_exit_detail::scope_exit_guard_creator{} << [&]() -> void

#define SCOPE_SUCCESS \
  auto IMPL_SCOPE_EXIT_MAKE_UNIQUE_IDENTIFIER(scope_success_guard_) = ::detail::scope_exit_detail::scope_success_guard_creator{} << [&]() -> void

#define SCOPE_FAIL \
  auto IMPL_SCOPE_EXIT_MAKE_UNIQUE_IDENTIFIER(scope_fail_guard_) = ::detail::scope_exit_detail::scope_fail_guard_creator{} << [&]() -> void
