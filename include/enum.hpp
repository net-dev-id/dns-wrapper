#include <cstring>

#define HANDLE_CASE_1(val)                                                     \
  case val:                                                                    \
    return #val;
#define HANDLE_CASE_2(val, ...) HANDLE_CASE_1(val) HANDLE_CASE_1(__VA_ARGS__)
#define HANDLE_CASE_3(val, ...) HANDLE_CASE_1(val) HANDLE_CASE_2(__VA_ARGS__)
#define HANDLE_CASE_4(val, ...) HANDLE_CASE_1(val) HANDLE_CASE_3(__VA_ARGS__)
#define HANDLE_CASE_5(val, ...) HANDLE_CASE_1(val) HANDLE_CASE_4(__VA_ARGS__)

#define HANDLE_RCASE_1(type, val)                                              \
  if (0 == strcmp(s, #val))                                                    \
    return type::val;
#define HANDLE_RCASE_2(type, val, ...)                                         \
  HANDLE_RCASE_1(type, val) HANDLE_RCASE_1(type, __VA_ARGS__)
#define HANDLE_RCASE_3(type, val, ...)                                         \
  HANDLE_RCASE_1(type, val) HANDLE_RCASE_2(type, __VA_ARGS__)
#define HANDLE_RCASE_4(type, val, ...)                                         \
  HANDLE_RCASE_1(type, val) HANDLE_RCASE_3(type, __VA_ARGS__)
#define HANDLE_RCASE_5(type, val, ...)                                         \
  HANDLE_RCASE_1(type, val) HANDLE_RCASE_4(type, __VA_ARGS__)

#define MAKE_ENUM_OLD(type, N, ...)                                            \
  class type {                                                                 \
  public:                                                                      \
    enum Value { __Invalid = -1, __VA_ARGS__, __MAX };                         \
    type() = default;                                                          \
    constexpr type(Value v) : value(v) {}                                      \
    constexpr const char *toCstring() { return type::type##ToCstring(*this); } \
    static constexpr const char *type##ToCstring(type e) {                     \
      switch (e.value) { HANDLE_CASE_##N(__VA_ARGS__) }                        \
      return "invalid";                                                        \
    }                                                                          \
    static constexpr type cstringTo##type(const char *s) {                     \
      HANDLE_RCASE_##N(type, __VA_ARGS__) return type::__Invalid;              \
    }                                                                          \
                                                                               \
  private:                                                                     \
    Value value;                                                               \
  };

/* compile with:
   c++ -std=c++20 -Wall -Werror make_enum.cc -o make_enum
 */

#define PARENS ()

// Rescan macro tokens 256 times
#define EXPAND(arg) EXPAND1(EXPAND1(EXPAND1(EXPAND1(arg))))
#define EXPAND1(arg) EXPAND2(EXPAND2(EXPAND2(EXPAND2(arg))))
#define EXPAND2(arg) EXPAND3(EXPAND3(EXPAND3(EXPAND3(arg))))
#define EXPAND3(arg) EXPAND4(EXPAND4(EXPAND4(EXPAND4(arg))))
#define EXPAND4(arg) arg

#define FOR_EACH(macro, ...)                                                   \
  __VA_OPT__(EXPAND(FOR_EACH_HELPER(macro, __VA_ARGS__)))
#define FOR_EACH_HELPER(macro, a1, ...)                                        \
  macro(a1) __VA_OPT__(FOR_EACH_AGAIN PARENS(macro, __VA_ARGS__))
#define FOR_EACH_AGAIN() FOR_EACH_HELPER

#define ENUM_CASE(name)                                                        \
  case name:                                                                   \
    return #name;

#define IF_VALUE(name)                                                         \
  if (0 == strcmp(#name, toCstring(name)))                                     \
    return name;

#define MAKE_ENUM_COPIED(type, ...)                                            \
  enum type { __VA_ARGS__ };                                                   \
  constexpr const char *toCstring(type _e) {                                   \
    using enum type;                                                           \
    switch (_e) {                                                              \
      FOR_EACH(ENUM_CASE, __VA_ARGS__)                                         \
    default:                                                                   \
      return "unknown";                                                        \
    }                                                                          \
  }                                                                            \
  constexpr type fromCstring(const char *e) { FOR_EACH(IF_VALUE, __VA_ARGS__) }

#define MAKE_ENUM(type, ...)                                                   \
  class type {                                                                 \
  public:                                                                      \
    enum Value { __VA_ARGS__ };                                                \
    type() = default;                                                          \
    constexpr type(Value v) : value(v) {}                                      \
    constexpr const char *toCstring() { return type::type##ToCstring(*this); } \
    static constexpr const char *type##ToCstring(type e) {                     \
      switch (e.value) { HANDLE_CASE_##N(__VA_ARGS__) }                        \
      return "invalid";                                                        \
    }                                                                          \
    static constexpr type cstringTo##type(const char *s) {                     \
      HANDLE_RCASE_##N(type, __VA_ARGS__) return type::__Invalid;              \
    }                                                                          \
                                                                               \
  private:                                                                     \
    Value value;                                                               \
  };
