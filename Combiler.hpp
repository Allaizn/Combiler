#pragma once

#include <cassert>
#include <variant>
#include <optional>
#include <vector>
#include <type_traits>

template <class ...Fs>
struct overload : Fs... {
  overload(Fs const&... fs) : Fs{ fs }...
  {}

  using Fs::operator()...;
};


struct Any
{
  bool operator==(Any const&) const { return true; }
  bool operator!=(Any const&) const { return false; }
};
struct All
{
  bool operator==(All const&) const { return true; }
  bool operator!=(All const&) const { return false; }
};
struct Each
{
  bool operator==(Each const&) const { return true; }
  bool operator!=(Each const&) const { return false; }
};

struct signal
{
  struct Description;
  Description const* const description;

  explicit signal(Description const* const& desc) : description(desc) {}
};
struct signal::Description
{
  std::string codeSyntax;
  std::string gameSyntax;
  std::string type;
  int index;

  Description(std::string const& codeSyntax, std::string const& gameSyntax, std::string const& type, int index)
    : codeSyntax(codeSyntax), gameSyntax(gameSyntax), type(type), index(index) {}
};

constexpr Any any;
constexpr All all;
constexpr Each each;

struct deciCom
{
  struct Mode
  {
    struct Description;
    // helper class to enable decider syntax with variable operator
    struct Helper;
    enum class Enum;

    static std::vector<Mode> const modes;
    static std::vector<Mode> createModes();
    Description const * const description;

    Mode(Description const * const& desc) : description(desc) { }
  };
  struct Input
  {
    using Left = std::variant<Any, All, Each, signal>;
    using Right = std::variant<int, signal>;

    Left left;
    Right right;
    Mode mode;

    Input(Left const& left, Right const& right, Mode const& mode) : left(left), right(right), mode(mode) {}

    struct SignalType
    {
      signal left;
      Right right;
      Mode mode;

      SignalType(signal const& signal, Right const& right, Mode const& mode) : left(left), right(right), mode(mode) {}
      explicit operator Input() const;

      struct Boolable;
    };
    struct AnyType;
    struct AllType;
    struct EachType;
    struct Boolable;
  };
  struct Output
  {
    using Type = std::variant<All, Each, signal>;
    using Value = std::optional<int>;

    Type output;
    Value value;

    Output(Type const& output, Value const& value) : output(output), value(value) {}

    struct AllType;
    struct EachType;
  };
  Input::Left left;
  Input::Right right;
  Mode mode;
  Output::Type output;
  Output::Value value;

  deciCom(Input::Left const& left, Input::Right const& right, Mode const& mode, Output::Type output, Output::Value value)
    : left(left), right(right), mode(mode), output(output), value(value) 
  {
    assert(!std::holds_alternative<Each>(output) || std::holds_alternative<Each>(left));
    assert(!std::holds_alternative<All>(output) || !std::holds_alternative<Each>(left));
  }
};

struct deciCom::Mode::Description
{
  std::string name;
  std::string codeSyntax;
  std::string gameSyntax;
  int index;

  Description(std::string const& name, std::string const& codeSyntax, std::string const& gameSyntax, int index)
    : name(name), codeSyntax(codeSyntax), gameSyntax(gameSyntax) {}
};
struct deciCom::Input::Boolable : deciCom::Input
{
  Boolable(Left const& left, Right const& right, Mode const& mode) : Input(left, right, mode) {}
  operator bool() const 
  { 
    return std::holds_alternative<signal>(left) 
        && std::holds_alternative<signal>(right) 
        && std::get<signal>(left).description == std::get<signal>(right).description; 
  }
};
deciCom::Input::SignalType::operator deciCom::Input() const { return Input(left, right, mode); }
struct deciCom::Input::SignalType::Boolable : deciCom::Input::SignalType
{
  Boolable(signal const& left, Right const& right, Mode const& mode) : SignalType(left, right, mode) {}
  operator bool() const 
  { 
    return std::holds_alternative<signal>(right) 
        && left.description == std::get<signal>(right).description; 
  }
};
struct deciCom::Input::AnyType
{
  Right right;
  Mode mode;

  AnyType(Right const& right, Mode const& mode) : right(right), mode(mode) {}
  explicit operator Input() const { return Input(any, right, mode); }
};
struct deciCom::Input::AllType
{
  Right right;
  Mode mode;

  AllType(Right const& right, Mode const& mode) : right(right), mode(mode) {}
  explicit operator Input() const { return Input(all, right, mode); }
};
struct deciCom::Input::EachType
{
  Right right;
  Mode mode;

  EachType(Right const& right, Mode const& mode) : right(right), mode(mode) {}
  explicit operator Input() const { return Input(each, right, mode); }
};
struct deciCom::Output::AllType
{
  Value value;

  AllType(Value const& value) : value(value) {}
  explicit operator Output() const { return Output(all, value); }
};
struct deciCom::Output::EachType
{
  Value value;

  EachType(Value const& value) : value(value) {}
  explicit operator Output() const { return Output(each, value); }
};

struct ariCom
{
  struct Mode
  {
    struct Description;
    struct Helper;
    enum class Enum;
    static std::vector<Mode> const modes;
    static std::vector<Mode> createModes();
    Description const * const description;

    Mode(Description const * const& desc) : description(desc) { }
  };
  struct Input
  {
    using Left = std::variant<int, Each, signal>;
    using Right = std::variant<int, signal>;
    
    Left left;
    Right right;
    Mode mode;

    Input(Left const& left, Right const& right, Mode const& mode) : left(left), right(right), mode(mode) {}

    struct IntSignalType;
  };
  struct Output
  {
    std::variant<Each, signal> value;
    Output(signal const& value) : value(value) {}
    explicit Output(Each const&) : value(each) {}
  };

  Input::Left left;
  Input::Right right;
  Mode mode;
  Output output;

  ariCom(Input::Left const& left, Input::Right const& right, Mode const& mode, Output output)
    : left(left), right(right), mode(mode), output(output) 
  {
    assert(!std::holds_alternative<Each>(output.value) || std::holds_alternative<Each>(left));
  }
};
struct ariCom::Mode::Description
{
  std::string name;
  std::string codeSyntax;
  std::string gameSyntax;
  int index;

  Description(std::string const& name, std::string const& codeSyntax, std::string const& gameSyntax, int index)
    : name(name), codeSyntax(codeSyntax), gameSyntax(gameSyntax) {}
};
struct ariCom::Input::IntSignalType
{
  std::variant<int, signal> left;
  Right right;
  Mode mode;

  IntSignalType(std::variant<int, signal> const& left, Right const& right, Mode const& mode) : left(left), right(right), mode(mode) {}
  explicit operator Input() const 
  { 
    if(std::holds_alternative<int>(left))
      return Input(std::get<int>(left), right, mode); 
    else 
      return Input(std::get<signal>(left), right, mode);
  }
};


struct wildCard
{
  std::variant<Any, All, Each> value;
  wildCard(std::variant<Any, All, Each> const& value) : value(value) {}
  explicit wildCard(Any const&) : value(any) {}
  explicit wildCard(All const&) : value(all) {}
  explicit wildCard(Each const&) : value(each) {}

  operator deciCom::Input::Left() const
  {
    return std::visit(overload(
      [](Any const&) { return deciCom::Input::Left(any); },
      [](All const&) { return deciCom::Input::Left(all); },
      [](Each const&) { return deciCom::Input::Left(each); }
    ), this->value);
  }
  operator deciCom::Output::Type() const
  {
    return std::visit(overload(
      [](Any const&) { assert(false); return deciCom::Output::Type(all); }, // deciCom output is never allowed to be any
      [](All const&) { return deciCom::Output::Type(all); },
      [](Each const&) { return deciCom::Output::Type(each); }
    ), this->value);
  }
  operator ariCom::Input::Left() const
  {
    std::visit(overload(
      [](Any const&) { assert(false); }, // ariCom input is never allowed to be any
      [](All const&) { assert(false); }, // ariCom input is never allowed to be all
      [](Each const&) {  }
    ), this->value);
    return ariCom::Input::Left(each);
  }
  explicit operator ariCom::Output() const
  {
    std::visit(overload(
      [](Any const&) { assert(false); }, // ariCom output is never allowed to be any
      [](All const&) { assert(false); }, // ariCom output is never allowed to be all
      [](Each const&) {  }
    ), this->value);
    return ariCom::Output(each);
  }
};

#define operations(equalArg, compArg)  \
  op(smaller,      <,  "<", compArg)   \
  op(greater,      >,  ">", compArg)   \
  op(equal,        ==, "=", equalArg)  \
  op(greaterEqual, >=, u8"≥", compArg) \
  op(smallerEqual, <=, u8"≤", compArg) \
  op(notEqual,     !=, u8"≠", equalArg)


enum class deciCom::Mode::Enum {
#define op(opName, codeSyntax, gameSyntax, arg) \
  opName,
  operations(,) };
#undef op

std::vector<deciCom::Mode> deciCom::Mode::createModes()
{
  std::vector<Mode> result;
  size_t i = 0;
#define op(name, codeSyntax, gameSyntax, arg) \
  result.push_back(Mode(new deciCom::Mode::Description(#name, #codeSyntax, gameSyntax, i++)));
  operations(,)
#undef op
  return result;
}
deciCom::Output::Value const input = deciCom::Output::Value();
std::vector<deciCom::Mode> const deciCom::Mode::modes = deciCom::Mode::createModes();

namespace deciComMode {
#define op(opName, codeSyntax, gameSyntax, arg) \
  deciCom::Mode const opName = deciCom::Mode::modes[(int)deciCom::Mode::Enum::##opName];
  operations(,)
#undef op
}

#define allRightOps(codeSyntax, leftType, outputType, generalOutType, signalOutType, ...)                                  \
auto operator codeSyntax(leftType, deciCom::Input::Right const& right) { return outputType##generalOutType(__VA_ARGS__); } \
auto operator codeSyntax(leftType, signal const& right)                { return outputType##signalOutType(__VA_ARGS__); }  \
auto operator codeSyntax(leftType, int const& right)                   { return outputType(__VA_ARGS__); }

#define op(opName, codeSyntax, gameSyntax, arg)                                                                                   \
allRightOps(codeSyntax, deciCom::Input::Left const& left, deciCom::Input,             arg, arg, left, right, deciComMode::opName) \
allRightOps(codeSyntax, Any const&,                       deciCom::Input::AnyType,       ,    , right, deciComMode::opName)       \
allRightOps(codeSyntax, All const&,                       deciCom::Input::AllType,       ,    , right, deciComMode::opName)       \
allRightOps(codeSyntax, Each const&,                      deciCom::Input::EachType,      ,    , right, deciComMode::opName)       \
allRightOps(codeSyntax, signal const& left,               deciCom::Input::SignalType, arg, arg, left, right, deciComMode::opName) \
allRightOps(codeSyntax, wildCard const& left,             deciCom::Input,                ,    , left, right, deciComMode::opName)
operations(::Boolable,)
#undef op
#undef allRightOps
#undef operations

auto operator+=(deciCom::Output::Type const& left, deciCom::Output::Value const& right) { return deciCom::Output(left, right); }
auto operator+=(signal const& left,                deciCom::Output::Value const& right) { return deciCom::Output(left, right); }
auto operator+=(All const&,                        deciCom::Output::Value const& right) { return deciCom::Output::AllType(right); }
auto operator+=(Each const&,                       deciCom::Output::Value const& right) { return deciCom::Output::EachType(right); }

#define then >>=
auto operator then(deciCom::Input const& left,             deciCom::Output const& right)           { return deciCom(left.left, left.right, left.mode, right.output, right.value); }
auto operator then(deciCom::Input const& left,             deciCom::Output::AllType const& right)  { return deciCom(left.left, left.right, left.mode, all, right.value); }
auto operator then(deciCom::Input const& left,             deciCom::Output::EachType const& right) { return deciCom(left.left, left.right, left.mode, each, right.value); }
auto operator then(deciCom::Input::SignalType const& left, deciCom::Output const& right)           { return deciCom(left.left, left.right, left.mode, right.output, right.value); }
auto operator then(deciCom::Input::SignalType const& left, deciCom::Output::AllType const& right)  { return deciCom(left.left, left.right, left.mode, all, right.value); }
auto operator then(deciCom::Input::AllType const& left,    deciCom::Output const& right)           { return deciCom(all, left.right, left.mode, right.output, right.value); }
auto operator then(deciCom::Input::AllType const& left,    deciCom::Output::AllType const& right)  { return deciCom(all, left.right, left.mode, all, right.value); }
auto operator then(deciCom::Input::AnyType const& left,    deciCom::Output const& right)           { return deciCom(any, left.right, left.mode, right.output, right.value); }
auto operator then(deciCom::Input::AnyType const& left,    deciCom::Output::AllType const& right)  { return deciCom(any, left.right, left.mode, all, right.value); }
auto operator then(deciCom::Input::EachType const& left,   deciCom::Output const& right)           { return deciCom(each, left.right, left.mode, right.output, right.value); }
auto operator then(deciCom::Input::EachType const& left,   deciCom::Output::EachType const& right) { return deciCom(each, left.right, left.mode, each, right.value); }


#define operations              \
  op(multiplicaton, *,   "*")   \
  op(division,      /,   "/")   \
  op(addition,      +,   "+")   \
  op(subtraction,   -,   "-")   \
  op(modulo,        %,   "%")   \
  op(power,         ^,   "^")   \
  op(shiftLeft,     <<,  "<<")  \
  op(shiftRight,    >>,  ">>")  \
  op(bitAnd,        AND, "AND") \
  op(bitOr,         OR,  "OR")  \
  op(bitXor,        XOR, "XOR")

enum class ariCom::Mode::Enum {
#define op(opName, codeSyntax, gameSyntax) \
  opName,
  operations };
#undef op


std::vector<ariCom::Mode> ariCom::Mode::createModes()
{
  std::vector<ariCom::Mode> result;
  size_t i = 0;
#define op(name, codeSyntax, gameSyntax) \
    result.push_back(ariCom::Mode(new ariCom::Mode::Description(#name, #codeSyntax, gameSyntax, i++)));  
  operations;
#undef op
  return result;
}
std::vector<ariCom::Mode> const ariCom::Mode::modes = ariCom::Mode::createModes();

namespace ariComMode
{
#define op(opName, codeSyntax, gameSyntax) \
  ariCom::Mode const opName = ariCom::Mode::modes[(int)ariCom::Mode::Enum::##opName];
  operations
#undef op
}

#define AND &
#define OR |
#define XOR ||

#define op(opName, codeSyntax, gameSyntax)                                                                                                                             \
auto operator codeSyntax(ariCom::Input::Left const& left, ariCom::Input::Right const& right) { return ariCom::Input(left, right, ariComMode::opName); }                \
auto operator codeSyntax(int const& left,                 ariCom::Input::Right const& right) { return ariCom::Input::IntSignalType(left, right, ariComMode::opName); } \
auto operator codeSyntax(Each const&,                     ariCom::Input::Right const& right) { return ariCom::Input(each, right, ariComMode::opName); }                \
auto operator codeSyntax(signal const& left,              ariCom::Input::Right const& right) { return ariCom::Input::IntSignalType(left, right, ariComMode::opName); }
operations
#undef op
#undef operations

#define on >>=
auto operator on(ariCom::Input const& left,                ariCom::Output const& right) { return ariCom(left.left, left.right, left.mode, right); }
auto operator on(ariCom::Input const& left,                Each const&)                 { return left on ariCom::Output(each); }
auto operator on(ariCom::Input const& left,                wildCard const& right)       { return left on ariCom::Output(right); }
auto operator on(ariCom::Input::IntSignalType const& left, ariCom::Output const& right) { return ariCom::Input(left) on right; }


#define EXPAND(x) x
#define autoOperator1(in, out, f1)                 left.f1 in right.f1
#define autoOperator2(in, out, f1, f2)             left.f1 in right.f1 out autoOperator1(in, out, f2)
#define autoOperator3(in, out, f1, f2, f3)         left.f1 in right.f1 out autoOperator2(in, out, f2, f3)
#define autoOperator4(in, out, f1, f2, f3, f4)     left.f1 in right.f1 out autoOperator3(in, out, f2, f3, f4)
#define autoOperator5(in, out, f1, f2, f3, f4, f5) left.f1 in right.f1 out autoOperator4(in, out, f2, f3, f4, f5)
#define autoOperatorName(_1, _2, _3, _4, _5, Name, ...) Name
#define autoOperator(in, out, ...) \
EXPAND(autoOperatorName(__VA_ARGS__,autoOperator5,autoOperator4,autoOperator3,autoOperator2,autoOperator1)(in, out, __VA_ARGS__))

#define autoOperators(type, ...)                                                                   \
bool operator==(type const& left, type const& right) { return autoOperator(==, &&, __VA_ARGS__); } \
bool operator!=(type const& left, type const& right) { return autoOperator(!=, ||, __VA_ARGS__); }

autoOperators(wildCard, value.index())
//autoOperators(signal, description)
autoOperators(signal::Description, index)

autoOperators(deciCom::Mode, description)
autoOperators(deciCom, left, right, mode, output, value)
autoOperators(deciCom::Mode::Description, index)
autoOperators(deciCom::Input, left, right, mode)
autoOperators(deciCom::Input::AllType, right, mode)
autoOperators(deciCom::Input::AnyType, right, mode)
autoOperators(deciCom::Input::EachType, right, mode)
autoOperators(deciCom::Input::SignalType, left, right, mode)
autoOperators(deciCom::Output, output, value)
autoOperators(deciCom::Output::AllType, value)
autoOperators(deciCom::Output::EachType, value)

autoOperators(ariCom::Mode, description)
autoOperators(ariCom::Output, value)
autoOperators(ariCom, left, right, mode, output)
autoOperators(ariCom::Mode::Description, index)
autoOperators(ariCom::Input, left, right, mode)
autoOperators(ariCom::Input::IntSignalType, left, right, mode)

#undef autoOperator
#define autoOperator(in, out, ...) std::operator==(left, right)
autoOperators(deciCom::Input::Left)
autoOperators(deciCom::Input::Right)
autoOperators(deciCom::Output::Type)
autoOperators(deciCom::Output::Value)
autoOperators(ariCom::Input::Left)
//autoOperators(ariCom::Input::Right) same as deciCom::Input::Right

// the cross wild card ops are necessary, since it's possible to do them anyway e.g. due to implicit conversions to deciCom::Input::Left
// the problem is that it also implicitly converts to deciCom::Output::Type, and thus creates ambiguity :/

bool operator==(Any  const&, All  const&) { return false; }
bool operator==(Any  const&, Each const&) { return false; }
bool operator==(All  const&, Any  const&) { return false; }
bool operator==(All  const&, Each const&) { return false; }
bool operator==(Each const&, Any  const&) { return false; }
bool operator==(Each const&, All  const&) { return false; }
bool operator!=(Any  const&, All  const&) { return true; }
bool operator!=(Any  const&, Each const&) { return true; }
bool operator!=(All  const&, Any  const&) { return true; }
bool operator!=(All  const&, Each const&) { return true; }
bool operator!=(Each const&, Any  const&) { return true; }
bool operator!=(Each const&, All  const&) { return true; }

bool operator==(Any  const&, wildCard const& card) { return std::holds_alternative<Any>(card.value); }
bool operator==(All  const&, wildCard const& card) { return std::holds_alternative<All>(card.value); }
bool operator==(Each const&, wildCard const& card) { return std::holds_alternative<Each>(card.value); }
bool operator!=(Any  const&, wildCard const& card) { return !std::holds_alternative<Any>(card.value); }
bool operator!=(All  const&, wildCard const& card) { return !std::holds_alternative<All>(card.value); }
bool operator!=(Each const&, wildCard const& card) { return !std::holds_alternative<Each>(card.value); }

bool operator==(wildCard const& card, Any  const&) { return std::holds_alternative<Any>(card.value); }
bool operator==(wildCard const& card, All  const&) { return std::holds_alternative<All>(card.value); }
bool operator==(wildCard const& card, Each const&) { return std::holds_alternative<Each>(card.value); }
bool operator!=(wildCard const& card, Any  const&) { return !std::holds_alternative<Any>(card.value); }
bool operator!=(wildCard const& card, All  const&) { return !std::holds_alternative<All>(card.value); }
bool operator!=(wildCard const& card, Each const&) { return !std::holds_alternative<Each>(card.value); }

bool operator==(deciCom::Input const& left, deciCom::Input::AnyType const& right) { return left == deciCom::Input(right); }
bool operator==(deciCom::Input const& left, deciCom::Input::AllType const& right) { return left == deciCom::Input(right); }
bool operator==(deciCom::Input const& left, deciCom::Input::EachType const& right) { return left == deciCom::Input(right); }
bool operator==(deciCom::Input const& left, deciCom::Input::SignalType const& right) { return left == deciCom::Input(right); }
bool operator!=(deciCom::Input const& left, deciCom::Input::AnyType const& right) { return left != deciCom::Input(right); }
bool operator!=(deciCom::Input const& left, deciCom::Input::AllType const& right) { return left != deciCom::Input(right); }
bool operator!=(deciCom::Input const& left, deciCom::Input::EachType const& right) { return left != deciCom::Input(right); }
bool operator!=(deciCom::Input const& left, deciCom::Input::SignalType const& right) { return left != deciCom::Input(right); }

bool operator==(deciCom::Input::AnyType const& left, deciCom::Input const& right) { return deciCom::Input(left) == right; }
bool operator==(deciCom::Input::AllType const& left, deciCom::Input const& right) { return deciCom::Input(left) == right; }
bool operator==(deciCom::Input::EachType const& left, deciCom::Input const& right) { return deciCom::Input(left) == right; }
bool operator==(deciCom::Input::SignalType const& left, deciCom::Input const& right) { return deciCom::Input(left) == right; }
bool operator!=(deciCom::Input::AnyType const& left, deciCom::Input const& right) { return deciCom::Input(left) != right; }
bool operator!=(deciCom::Input::AllType const& left, deciCom::Input const& right) { return deciCom::Input(left) != right; }
bool operator!=(deciCom::Input::EachType const& left, deciCom::Input const& right) { return deciCom::Input(left) != right; }
bool operator!=(deciCom::Input::SignalType const& left, deciCom::Input const& right) { return deciCom::Input(left) != right; }

bool operator==(deciCom::Output const& left, deciCom::Output::AllType const& right) { return left == deciCom::Output(right); }
bool operator==(deciCom::Output const& left, deciCom::Output::EachType const& right) { return left == deciCom::Output(right); }
bool operator!=(deciCom::Output const& left, deciCom::Output::AllType const& right) { return left != deciCom::Output(right); }
bool operator!=(deciCom::Output const& left, deciCom::Output::EachType const& right) { return left != deciCom::Output(right); }

bool operator==(deciCom::Output::AllType const& left, deciCom::Output const& right) { return deciCom::Output(left) == right; }
bool operator==(deciCom::Output::EachType const& left, deciCom::Output const& right) { return deciCom::Output(left) == right; }
bool operator!=(deciCom::Output::AllType const& left, deciCom::Output const& right) { return deciCom::Output(left) != right; }
bool operator!=(deciCom::Output::EachType const& left, deciCom::Output const& right) { return deciCom::Output(left) != right; }

bool operator==(ariCom::Input const& left, ariCom::Input::IntSignalType const& right) { return left == ariCom::Input(right); }
bool operator!=(ariCom::Input const& left, ariCom::Input::IntSignalType const& right) { return left != ariCom::Input(right); }
bool operator==(ariCom::Input::IntSignalType const& left, ariCom::Input const& right) { return ariCom::Input(left) == right; }
bool operator!=(ariCom::Input::IntSignalType const& left, ariCom::Input const& right) { return ariCom::Input(left) != right; }

bool operator==(ariCom::Output const& out, Each const&) { return std::holds_alternative<Each>(out.value); }
bool operator!=(ariCom::Output const& out, Each const&) { return !std::holds_alternative<Each>(out.value); }
bool operator==(Each const&, ariCom::Output const& out) { return std::holds_alternative<Each>(out.value); }
bool operator!=(Each const&, ariCom::Output const& out) { return !std::holds_alternative<Each>(out.value); }

bool operator==(ariCom::Output const& out, wildCard const& card) { return std::holds_alternative<Each>(out.value) && std::holds_alternative<Each>(card.value); }
bool operator!=(ariCom::Output const& out, wildCard const& card) { return !std::holds_alternative<Each>(out.value) || !std::holds_alternative<Each>(card.value); }
bool operator==(wildCard const& card, ariCom::Output const& out) { return std::holds_alternative<Each>(out.value) && std::holds_alternative<Each>(card.value); }
bool operator!=(wildCard const& card, ariCom::Output const& out) { return !std::holds_alternative<Each>(out.value) || !std::holds_alternative<Each>(card.value); }


struct deciCom::Mode::Helper
{
  Input::Left left;
  Mode mode;
  Helper(Input::Left const& left, Mode const& mode) : left(left), mode(mode) {}
  struct SignalType
  {
    signal left;
    Mode mode;
    SignalType(signal const& left, Mode const& mode) : left(left), mode(mode) {}
  };
  struct AnyType
  {
    Mode mode;
    AnyType(Mode const& mode) : mode(mode) {}
  };
  struct AllType
  {
    Mode mode;
    AllType(Mode const& mode) : mode(mode) {}
  };
  struct EachType
  {
    Mode mode;
    EachType(Mode const& mode) : mode(mode) {}
  };
};

auto operator<(deciCom::Input::Left const& left, deciCom::Mode const& right) { return deciCom::Mode::Helper(left, right); }
auto operator<(Any const&,                       deciCom::Mode const& right) { return deciCom::Mode::Helper::AnyType(right); }
auto operator<(All const&,                       deciCom::Mode const& right) { return deciCom::Mode::Helper::AllType(right); }
auto operator<(Each const&,                      deciCom::Mode const& right) { return deciCom::Mode::Helper::EachType(right); }
auto operator<(signal const& left,               deciCom::Mode const& right) { return deciCom::Mode::Helper::SignalType(left, right); }
auto operator<(wildCard const& left,             deciCom::Mode const& right) { return deciCom::Mode::Helper(left, right); }

auto operator>(deciCom::Mode::Helper const& left, deciCom::Input::Right const& right) { return deciCom::Input(left.left, right, left.mode); }
auto operator>(deciCom::Mode::Helper const& left, signal const& right)                { return deciCom::Input(left.left, right, left.mode); }
auto operator>(deciCom::Mode::Helper const& left, int const& right)                   { return deciCom::Input(left.left, right, left.mode); }
auto operator>(deciCom::Mode::Helper::AnyType const& left, deciCom::Input::Right const& right) { return deciCom::Input::AnyType(right, left.mode); }
auto operator>(deciCom::Mode::Helper::AnyType const& left, signal const& right)                { return deciCom::Input::AnyType(right, left.mode); }
auto operator>(deciCom::Mode::Helper::AnyType const& left, int const& right)                   { return deciCom::Input::AnyType(right, left.mode); }
auto operator>(deciCom::Mode::Helper::AllType const& left, deciCom::Input::Right const& right) { return deciCom::Input::AllType(right, left.mode); }
auto operator>(deciCom::Mode::Helper::AllType const& left, signal const& right)                { return deciCom::Input::AllType(right, left.mode); }
auto operator>(deciCom::Mode::Helper::AllType const& left, int const& right)                   { return deciCom::Input::AllType(right, left.mode); }
auto operator>(deciCom::Mode::Helper::EachType const& left, deciCom::Input::Right const& right) { return deciCom::Input::EachType(right, left.mode); }
auto operator>(deciCom::Mode::Helper::EachType const& left, signal const& right)                { return deciCom::Input::EachType(right, left.mode); }
auto operator>(deciCom::Mode::Helper::EachType const& left, int const& right)                   { return deciCom::Input::EachType(right, left.mode); }
auto operator>(deciCom::Mode::Helper::SignalType const& left, deciCom::Input::Right const& right) { return deciCom::Input::SignalType(left.left, right, left.mode); }
auto operator>(deciCom::Mode::Helper::SignalType const& left, signal const& right)                { return deciCom::Input::SignalType(left.left, right, left.mode); }
auto operator>(deciCom::Mode::Helper::SignalType const& left, int const& right)                   { return deciCom::Input::SignalType(left.left, right, left.mode); }

struct ariCom::Mode::Helper
{
  Input::Left left;
  Mode mode;
  Helper(Input::Left const& left, Mode const& mode) : left(left), mode(mode) {}
  struct IntSignalType
  {
    std::variant<int, signal> left;
    Mode mode;
    IntSignalType(std::variant<int, signal> const& left, Mode const& mode) : left(left), mode(mode) {}
  };
};

auto operator<(ariCom::Input::Left const& left, ariCom::Mode const& right) { return ariCom::Mode::Helper(left, right); }
auto operator<(int const& left,                 ariCom::Mode const& right) { return ariCom::Mode::Helper::IntSignalType(left, right); }
auto operator<(Each const&,                     ariCom::Mode const& right) { return ariCom::Mode::Helper(each, right); }
auto operator<(signal const& left,              ariCom::Mode const& right) { return ariCom::Mode::Helper::IntSignalType(left, right); }

auto operator>(ariCom::Mode::Helper const& left, ariCom::Input::Right const& right) { return ariCom::Input(left.left, right, left.mode); }
auto operator>(ariCom::Mode::Helper::IntSignalType const& left, ariCom::Input::Right const& right) { return ariCom::Input::IntSignalType(left.left, right, left.mode); }


namespace itemSignal
{
#define operations                                                                                                                                                                                                                                                                                                               \
  op(wooden_chest)        op(iron_chest)           op(steel_chest)                    op(storage_tank)                                                                                                                                                                                                                           \
  op(transport_belt)      op(fast_transport_belt)  op(express_transport_belt)         op(underground_belt)                op(fast_underground_belt)   op(express_underground_belt) op(splitter)                 op(fast_splitter)        op(express_splitter)                                                                    \
  op(burner_inserter)     op(inserter)             op(long_handed_inserter)           op(fast_inserter)                   op(filter_inserter)         op(stack_inserter)           op(stack_filter_inserter)                                                                                                                     \
  op(small_electric_pole) op(medium_electric_pole) op(big_electric_pole)              op(substation)                      op(pipe)                    op(pipe_to_ground)           op(pump)                                                                                                                                      \
  op(rail)                op(train_stop)           op(rail_signal)                    op(rail_chain_signal)               op(locomotive)              op(cargo_wagon)              op(fluid_wagon)              op(artillery_wagon)      op(car)              op(tank)                                                           \
  op(logistic_robot)      op(construction_robot)   op(logistic_chest_active_provider) op(logistic_chest_passive_provider) op(logistic_chest_storage)  op(logistic_chest_buffer)    op(logistic_chest_requester) op(roboport)                                                                                                     \
  op(small_lamp)          op(red_wire)             op(green_wire)                     op(arithmetic_combinator)           op(decider_combinator)      op(constant_combinator)      op(power_switch)             op(programmable_speaker)                                                                                         \
  op(stone_brick)         op(concrete)             op(hazard_concrete)                op(refined_concrete)                op(refined_hazard_concrete) op(landfill)                 op(cliff_explosives)                                                                                                                          \
                                                                                                                                                                                                                                                                                                                                 \
  op(repair_pack)          op(blueprint)             op(deconstruction_planner) op(upgrade_planner) op(blueprint_book)                                                                                                                                                                                                           \
  op(boiler)               op(steam_engine)          op(steam_turbine)          op(solar_panel)     op(accumulator)        op(nuclear_reactor)      op(heat_exchanger)       op(heat_pipe)                                                                                                                                       \
  op(burner_mining_drill)  op(electric_mining_drill) op(offshore_pump)          op(pumpjack)                                                                                                                                                                                                                                     \
  op(stone_furnace)        op(steel_furnace)         op(electric_furnace)                                                                                                                                                                                                                                                        \
  op(assembling_machine_1) op(assembling_machine_2)  op(assembling_machine_3)   op(oil_refinery)    op(chemical_plant)     op(centrifuge)           op(lab)                                                                                                                                                                      \
  op(beacon)               op(speed_module)          op(speed_module_2)         op(speed_module_3)  op(effectivity_module) op(effectivity_module_2) op(effectivity_module_3) op(productivity_module) op(productivity_module_2) op(productivity_module_3)                                                                         \
                                                                                                                                                                                                                                                                                                                                 \
  op(wood)                    op(coal)                  op(stone)                 op(iron_ore)              op(copper_ore)              op(uranium_ore)          op(raw_fish)                                                                                                                                                    \
  op(iron_plate)              op(copper_plate)          op(solid_fuel)            op(steel_plate)           op(plastic_bar)             op(sulfur)               op(battery)            op(explosives)                                                                                                                           \
  op(crude_oil_barrel)        op(heavy_oil_barrel)      op(light_oil_barrel)      op(lubricant_barrel)      op(petroleum_gas_barrel)    op(sulfuric_acid_barrel) op(water_barrel)                                                                                                                                                \
  op(copper_cable)            op(iron_stick)            op(iron_gear_wheel)       op(empty_barrel)          op(electronic_circuit)      op(advanced_circuit)     op(processing_unit)    op(engine_unit)       op(electric_engine_unit)      op(flying_robot_frame)                                                               \
  op(satellite)               op(rocket_control_unit)   op(low_density_structure) op(rocket_fuel)           op(nuclear_fuel)            op(uranium_235)          op(uranium_238)        op(uranium_fuel_cell) op(used_up_uranium_fuel_cell)                                                                                      \
  op(automation_science_pack) op(logistic_science_pack) op(military_science_pack) op(chemical_science_pack) op(production_science_pack) op(utility_science_pack) op(space_science_pack)                                                                                                                                          \
                                                                                                                                                                                                                                                                                                                                 \
  op(pistol)                      op(submachine_gun)                  op(shotgun)                 op(combat_shotgun)              op(rocket_launcher)        op(flamethrower)          op(land_mine)                                                                                                                             \
  op(firearm_magazine)            op(piercing_rounds_magazine)        op(uranium_rounds_magazine) op(shotgun_shell)               op(piercing_shotgun_shell) op(cannon_shell)          op(explosive_cannon_shell)           op(uranium_cannon_shell)        op(explosive_uranium_cannon_shell) op(artillery_shell)               \
  op(rocket)                      op(explosive_rocket)                op(atomic_bomb)             op(flamethrower_ammo)                                                                                                                                                                                                          \
  op(grenade)                     op(cluster_grenade)                 op(poison_capsule)          op(slowdown_capsule)            op(defender_capsule)       op(distractor_capsule)    op(destroyer_capsule)                op(discharge_defense_remote)    op(artillery_targeting_remote)                                       \
  op(light_armor)                 op(heavy_armor)                     op(modular_armor)           op(power_armor)                 op(power_armor_mk2)                                                                                                                                                                            \
  op(solar_panel_equipment)       op(fusion_reactor_equipment)        op(energy_shield_equipment) op(energy_shield_mk2_equipment) op(battery_equipment)      op(battery_mk2_equipment) op(personal_laser_defense_equipment) op(discharge_defense_equipment) op(belt_immunity_equipment)        op(exoskeleton_equipment)         \
  op(personal_roboport_equipment) op(personal_roboport_mk2_equipment) op(night_vision_equipment)                                                                                                                                                                                                                                 \
  op(stone_wall)                  op(gate)                            op(gun_turret)              op(laser_turret)                op(flamethrower_turret)    op(artillery_turret)      op(radar) op(rocket_silo)


  std::vector<signal> const itemSignals = ([]() {
    std::vector<signal> result;
    size_t i = 0;
#define op(name) \
    result.push_back(signal(new signal::Description(#name, #name, "item", i++)));
    operations
#undef op
      return result; 
  })();
  enum class itemSignalsEnum
  {
#define op(name) name,
    operations
#undef op
  };

#define op(opName) \
  signal const opName = itemSignals[int(itemSignalsEnum::##opName)];
  operations
#undef op
#undef operations
}
namespace fluidSignal
{
#define operations \
  op(water)  op(crude_oil)  op(heavy_oil)  op(light_oil)  op(petroleum_gas)  op(sulfuric_acid)  op(lubricant)

  std::vector<signal> const fluidSignals = ([]() {
    std::vector<signal> result;
    size_t i = 0;
#define op(name) \
    result.push_back(signal(new signal::Description(#name, #name, "fluid", i++)));
    operations
#undef op
      return result;
  })();

  enum class fluidSignalsEnum
  {
#define op(name) name,
    operations
#undef op
  };
#define op(opName) \
  signal const opName = fluidSignals[int(fluidSignalsEnum::##opName)];
  operations
#undef op
#undef operations
}
namespace virtualSignal
{
#define operations \
  op(0, _)  op(1, _)  op(2, _)  op(3, _)  op(4, _)  op(5, _)  op(6, _)  op(7, _)  op(8, _)  op(9, _)  \
  op(A,) op(B,) op(C,) op(D,) op(E,) op(F,) op(G,) op(H,) op(I,) op(J,) op(K,) op(L,) op(M,)          \
  op(N,) op(O,) op(P,) op(Q,) op(R,) op(S,) op(T,) op(U,) op(V,) op(W,) op(X,) op(Y,) op(Z,)          \
  op(red,) op(green,) op(blue,) op(yellow,) op(pink,) op(cyan,) op(white,) op(grey,) op(black,)       \
  op(check,) op(dot,) op(info,)

  std::vector<signal> const virtualSignals = ([]() {
    std::vector<signal> result;
    size_t i = 0;
#define op(name, score) \
    result.push_back(signal(new signal::Description(#score#name, "signal"#score#name, "virtual", i++)));
    operations
#undef op
    return result;
  })();

  enum class virtualSignalsEnum
  {
#define op(name,score) score##name,
    operations
#undef op
  };
#define op(opName, score) \
  signal const score##opName  = virtualSignals[int(virtualSignalsEnum::##score##opName)];
  operations
#undef op
#undef operations
}


#ifdef ENABLE_TESTS

template <typename... Ts, typename T>
static constexpr auto is_valid(T)
{
  return std::is_invocable<T, Ts...>{};
}

#define assert1Invalid(var0, exp)       static_assert(!is_valid<decltype(var0)>                ([](auto var0)            constexpr -> decltype(void(exp)){}))
#define assert2Invalid(var0, var1, exp) static_assert(!is_valid<decltype(var0), decltype(var1)>([](auto var0, auto var1) constexpr -> decltype(void(exp)){}))
#define assert1Invalid11(var, expr1, expr2, expr3, expr4, expr5, expr6, expr7, expr8, expr9, expr10, expr11)        \
    assert1Invalid(var, expr1); assert1Invalid(var, expr2); assert1Invalid(var, expr3); assert1Invalid(var, expr4); \
    assert1Invalid(var, expr5); assert1Invalid(var, expr6); assert1Invalid(var, expr7); assert1Invalid(var, expr8); \
    assert1Invalid(var, expr9); assert1Invalid(var, expr10); assert1Invalid(var, expr11)

#define assertType(type, expr) static_assert(std::is_same<type, decltype(expr)>::value)
#define assertType2(type, expr1, expr2) assertType(type, expr1); assertType(type, expr2)
#define assertType3(type, expr1, expr2, expr3) assertType2(type, expr1, expr2); assertType(type, expr3)
#define assertType4(type, expr1, expr2, expr3, expr4) assertType2(type, expr1, expr2); assertType2(type, expr3, expr4)
#define assertType5(type, expr1, expr2, expr3, expr4, expr5) assertType4(type, expr1, expr2, expr3, expr4); assertType(type, expr5)
#define assertType6(type, expr1, expr2, expr3, expr4, expr5, expr6) assertType4(type, expr1, expr2, expr3, expr4); assertType2(type, expr5, expr6)
#define assertType11(type, expr1, expr2, expr3, expr4, expr5, expr6, expr7, expr8, expr9, expr10, expr11) \
assertType6(type, expr1, expr2, expr3, expr4, expr5, expr6); assertType5(type, expr7, expr8, expr9, expr10, expr11)

#define assert2Types(type0, type1, expr) static_assert(std::is_convertible_v<decltype(expr), type0> && std::is_convertible_v<decltype(expr), type1>)
#define assert2Types2(type0, type1, expr1, expr2) assert2Types(type0, type1, expr1); assert2Types(type0, type1, expr2)

namespace unit_tests
{
  void testFunction(wildCard card,
                    signal coal,
                    signal::Description sigDesc,
                    deciCom deci,
                    deciCom::Mode deciMode,
                    deciCom::Mode::Description deciModeDesc,
                    deciCom::Input deciIn,
                    deciCom::Input::Left deciInLeft,
                    deciCom::Input::Right deciInRight,
                    deciCom::Input::AllType deciInAll,
                    deciCom::Input::AnyType deciInAny,
                    deciCom::Input::EachType deciInEach,
                    deciCom::Input::SignalType deciInSig,
                    deciCom::Input::SignalType::Boolable deciInSigBool,
                    deciCom::Input::Boolable deciInBool,
                    deciCom::Output deciOut,
                    deciCom::Output::Type deciOutType,
                    deciCom::Output::Value deciOutVal,
                    deciCom::Output::AllType deciOutAll,
                    deciCom::Output::EachType deciOutEach,

                    ariCom ari,
                    ariCom::Mode ariMode,
                    ariCom::Mode::Description ariModeDesc,
                    ariCom::Input ariIn,
                    ariCom::Input::Left ariInLeft,
                    ariCom::Input::Right ariInRight,
                    ariCom::Input::IntSignalType ariInNonEach,
                    ariCom::Output ariOut)
  {
    // deciCom::Output::Type
    assertType2(bool, input == input, input != input);

    // Any, All, Each
    assertType2(bool, any  == any,  any  != any);
    assertType2(bool, all  == all,  all  != all);
    assertType2(bool, each == each, each != each);

    assertType2(bool, any == all,  any  != all);
    assertType2(bool, any == each, any  != each);
    assertType2(bool, all == any,  all  != any);
    assertType2(bool, all == each, all  != each);
    assertType2(bool, each == any, each != any);
    assertType2(bool, each == all, each != all);

    assertType6(deciCom::Input::AnyType,  any  == 0, any  != 0, any  < 0, any  <= 0, any  > 0, any  >= 0);
    assertType6(deciCom::Input::AllType,  all  == 0, all  != 0, all  < 0, all  <= 0, all  > 0, all  >= 0);
    assertType6(deciCom::Input::EachType, each == 0, each != 0, each < 0, each <= 0, each > 0, each >= 0);

    assertType2(deciCom::Output::AllType,  all  += 1, all  += input);
    assertType2(deciCom::Output::EachType, each += 1, each += input);

    assert1Invalid11(any,       any  + 0, any  - 0, any  * 0, any  / 0, any  % 0, any  ^ 0, any  >> 0, any  << 0, any  AND 0, any  OR 0, any  XOR 0);
    assert1Invalid11(all,       all  + 0, all  - 0, all  * 0, all  / 0, all  % 0, all  ^ 0, all  >> 0, all  << 0, all  AND 0, all  OR 0, all  XOR 0);
    assertType11(ariCom::Input, each + 0, each - 0, each * 0, each / 0, each % 0, each ^ 0, each >> 0, each << 0, each AND 0, each OR 0, each XOR 0);

    // wildCard
    assertType2(bool, card == card, card != card);
    assertType4(bool, card == any,  card != any,  any == card,  any != card);
    assertType4(bool, card == all,  card != all,  all == card,  all != card);
    assertType4(bool, card == each, card != each, each == card, each != card);

    assertType11(ariCom::Input, card + 0, card - 0, card * 0, card / 0, card % 0, card ^ 0, card >> 0, card << 0, card AND 0, card OR 0, card XOR 0);

    // signal
    assert2Types2(bool, deciCom::Input::SignalType, coal == coal, coal != coal);
    assertType4(deciCom::Input::SignalType,                       coal < coal, coal <= coal, coal > coal, coal >= coal);
    assertType6(deciCom::Input::SignalType, coal == 0, coal != 0, coal < 0,    coal <= 0,    coal > 0,    coal >= 0);

    assertType2(deciCom::Output, coal += 0, coal += input);
    
    assert1Invalid11(any,       any  + coal, any  - coal, any  * coal, any  / coal, any  % coal, any  ^ coal, any  >> coal, any  << coal, any  AND coal, any  OR coal, any  XOR coal);
    assert1Invalid11(all,       all  + coal, all  - coal, all  * coal, all  / coal, all  % coal, all  ^ coal, all  >> coal, all  << coal, all  AND coal, all  OR coal, all  XOR coal);
    assertType11(ariCom::Input, each + coal, each - coal, each * coal, each / coal, each % coal, each ^ coal, each >> coal, each << coal, each AND coal, each OR coal, each XOR coal);
    assertType11(ariCom::Input, card + coal, card - coal, card * coal, card / coal, card % coal, card ^ coal, card >> coal, card << coal, card AND coal, card OR coal, card XOR coal);

    // signalDescription
    assertType2(bool, sigDesc == sigDesc, sigDesc != sigDesc);

    // deciCom
    assertType2(bool, deci == deci, deci != deci);
    // deciCom::Mode
    assertType2(bool, deciMode == deciMode, deciMode != deciMode); // TODO(Allaizn): add syntax for variable decider operator
    //deciCom::Mode::Description;
    assertType2(bool, deciModeDesc == deciModeDesc, deciModeDesc != deciModeDesc);

    //deciCom::Input;
    assertType2(bool, deciIn == deciIn, deciIn != deciIn);

    //deciCom::Input::Left;
    assertType2(bool, deciInLeft == deciInLeft, deciInLeft != deciInLeft);
    assert2Types2(bool, deciCom::Input, deciInLeft == coal, deciInLeft != coal);
    assertType4(deciCom::Input,                                   deciInLeft < coal, deciInLeft <= coal, deciInLeft > coal, deciInLeft >= coal);
    assertType6(deciCom::Input, deciInLeft == 0, deciInLeft != 0, deciInLeft < 0,    deciInLeft <= 0,    deciInLeft > 0,    deciInLeft >= 0);
    assertType4(bool, deciInLeft == any,  deciInLeft != any,  any  == deciInLeft, any  != deciInLeft);
    assertType4(bool, deciInLeft == all,  deciInLeft != all,  all  == deciInLeft, all  != deciInLeft);
    assertType4(bool, deciInLeft == each, deciInLeft != each, each == deciInLeft, each != deciInLeft);
    assertType4(bool, deciInLeft == card, deciInLeft != card, card == deciInLeft, card != deciInLeft);

    //deciCom::Input::Right;
    assertType2(bool, deciInRight == deciInRight, deciInRight != deciInRight);
    assert2Types2(bool, deciCom::Input::SignalType, coal == deciInRight, coal != deciInRight);
    assertType4(deciCom::Input::SignalType,                                         coal < deciInRight, coal <= deciInRight, coal > deciInRight, coal >= deciInRight);
    assertType6(deciCom::Input::AnyType,  any  == deciInRight, any  != deciInRight, any  < deciInRight, any  <= deciInRight, any  > deciInRight, any >= deciInRight);
    assertType6(deciCom::Input::AllType,  all  == deciInRight, all  != deciInRight, all  < deciInRight, all  <= deciInRight, all  > deciInRight, all >= deciInRight);
    assertType6(deciCom::Input::EachType, each == deciInRight, each != deciInRight, each < deciInRight, each <= deciInRight, each > deciInRight, each >= deciInRight);
    assert2Types2(bool, deciCom::Input, deciInLeft == deciInRight, deciInLeft != deciInRight);
    assertType4(deciCom::Input, deciInLeft < deciInRight, deciInLeft <= deciInRight, deciInLeft > deciInRight, deciInLeft >= deciInRight);

    //deciCom::Input::AnyType;
    assertType2(bool, deciInAny == deciInAny, deciInAny != deciInAny);
    //deciCom::Input::AllType;
    assertType2(bool, deciInAll == deciInAll, deciInAll != deciInAll);
    assertType2(bool, deciInAll == deciIn,    deciIn    != deciInAll);
    //deciCom::Input::EachType;
    assertType2(bool, deciInEach == deciInEach, deciInEach != deciInEach);
    assertType2(bool, deciInEach == deciIn,     deciIn     != deciInEach);
    //deciCom::Input::SignalType;
    assertType2(bool, deciInSig == deciInSig, deciInSig != deciInSig);
    assertType2(bool, deciInSig == deciIn,    deciIn    != deciInSig);
    //deciCom::Input::SignalType::Boolable;
    assertType2(bool, deciInSigBool == deciInSigBool, deciInSigBool != deciInSigBool);
    assertType2(bool, deciInSigBool == deciIn,        deciIn        != deciInSigBool);
    //deciCom::Input::Boolable;
    assertType2(bool, deciInBool == deciInBool, deciInBool != deciInBool);
    assertType2(bool, deciInBool == deciIn,     deciIn     != deciInBool);

    //deciCom::Output;
    assertType2(bool, deciOut == deciOut, deciOut != deciOut);
    assertType(deciCom, deciIn then deciOut);
    assertType6(deciCom, deciInAll then deciOut, deciInAny then deciOut, deciInEach then deciOut, 
                         deciInSig then deciOut, deciInSigBool then deciOut, deciInBool then deciOut);

    //deciCom::Output::Value;
    assertType2(bool, deciOutVal == deciOutVal, deciOutVal != deciOutVal);
    assertType2(bool, deciOutVal == 0, 0 != deciOutVal);
    assertType2(bool, deciOutVal == input, input != deciOutVal);
    assertType(deciCom::Output::EachType, each += deciOutVal);
    assertType(deciCom::Output::AllType, all += deciOutVal);
    assertType(deciCom::Output, coal += deciOutVal);

    //deciCom::Output::Type;
    assertType2(bool, deciOutType == deciOutType, deciOutType != deciOutType);
    assertType2(deciCom::Output, deciOutType += 0, deciOutType += input);
    assertType(deciCom::Output, deciOutType += deciOutVal);

    //deciCom::Output::AllType;
    assertType2(bool, deciOutAll == deciOutAll, deciOutAll != deciOutAll);
    assertType2(bool, deciOutAll == deciOut,    deciOut != deciOutAll);
    assertType(deciCom, deciIn then deciOutAll);
    assertType5(deciCom, deciInAll then deciOutAll, deciInAny then deciOutAll, /*deciInEach then deciOutAll,*/
                         deciInSig then deciOutAll, deciInSigBool then deciOutAll, deciInBool then deciOutAll);
    assert2Invalid(deciInEach, deciOutAll, deciInEach then deciOutAll);

    //deciCom::Output::EachType;
    assertType2(bool, deciOutEach == deciOutEach, deciOutEach != deciOutEach);
    assertType2(bool, deciOutEach == deciOut,    deciOutEach != deciOutEach);
    assertType(deciCom, deciIn then deciOutEach);
    assertType2(deciCom, /*deciInAll then deciOutEach, deciInAny then deciOutEach,*/ deciInEach then deciOutEach,
                         /*deciInSig then deciOutEach, deciInSigBool then deciOutEach,*/ deciInBool then deciOutEach);
    assert2Invalid(deciInAll, deciOutEach, deciInAll then deciOutEach);
    assert2Invalid(deciInAny, deciOutEach, deciInAny then deciOutEach);
    assert2Invalid(deciInSig, deciOutEach, deciInSig then deciOutEach);
    assert2Invalid(deciInSigBool, deciOutEach, deciInSigBool then deciOutEach);

    //ariCom;
    assertType2(bool, ari == ari, ari != ari);
    // ariCom::Mode
    assertType2(bool, ariMode == ariMode, ariMode != ariMode); // TODO(Allaizn): add syntax for variable decider operator
    //ariCom::Mode::Description;
    assertType2(bool, ariModeDesc == ariModeDesc, ariModeDesc != ariModeDesc);

    //ariCom::Input;
    assertType2(bool, ariIn == ariIn, ariIn != ariIn);
    assertType2(ariCom, ariIn on each, ariIn on coal);

    //ariCom::Input::Left;
    assertType2(bool, ariInLeft == ariInLeft, ariInLeft != ariInLeft);
    assertType4(bool, ariInLeft == 0,    ariInLeft != 0,    0    == ariInLeft, 0    != ariInLeft);
    assertType4(bool, ariInLeft == coal, ariInLeft != coal, coal == ariInLeft, coal != ariInLeft);
    assertType4(bool, ariInLeft == each, ariInLeft != each, each == ariInLeft, each != ariInLeft);
    assertType4(bool, ariInLeft == card, ariInLeft != card, card == ariInLeft, card != ariInLeft);

    assertType11(ariCom::Input, ariInLeft + 0,     ariInLeft - 0,     ariInLeft * 0,      ariInLeft / 0,     ariInLeft % 0,    ariInLeft ^ 0,
                                ariInLeft >> 0,    ariInLeft << 0,    ariInLeft AND 0,    ariInLeft OR 0,    ariInLeft XOR 0);
    assertType11(ariCom::Input, ariInLeft + coal,  ariInLeft - coal,  ariInLeft * coal,   ariInLeft / coal,  ariInLeft % coal, ariInLeft ^ coal,
                                ariInLeft >> coal, ariInLeft << coal, ariInLeft AND coal, ariInLeft OR coal, ariInLeft XOR coal);

    //ariCom::Input::Right;
    assertType2(bool, ariInRight == ariInRight, ariInRight != ariInRight);
    assertType4(bool, ariInRight == 0,    ariInRight != 0,    0    == ariInRight, 0    != ariInRight);
    assertType2(bool, ariInRight == coal, ariInRight != coal);
    static_assert(std::is_convertible_v<decltype(coal == ariInRight), bool>);
    static_assert(std::is_convertible_v<decltype(coal != ariInRight), bool>);
    assert1Invalid11(any,       any  +  ariInRight, any  -  ariInRight, any   *  ariInRight, any  /  ariInRight, any   %  ariInRight, any  ^ ariInRight,
                                any  >> ariInRight, any  << ariInRight, any  AND ariInRight, any  OR ariInRight, any  XOR ariInRight);
    assert1Invalid11(all,       all  +  ariInRight, all  -  ariInRight, all   *  ariInRight, all  /  ariInRight, all   %  ariInRight, all  ^ ariInRight, 
                                all  >> ariInRight, all  << ariInRight, all  AND ariInRight, all  OR ariInRight, all  XOR ariInRight);
    assertType11(ariCom::Input, each +  ariInRight, each -  ariInRight, each  *  ariInRight, each /  ariInRight, each  %  ariInRight, each ^ ariInRight, 
                                each >> ariInRight, each << ariInRight, each AND ariInRight, each OR ariInRight, each XOR ariInRight);
    assertType11(ariCom::Input, card +  ariInRight, card -  ariInRight, card  *  ariInRight, card /  ariInRight, card  %  ariInRight, card ^ ariInRight, 
                                card >> ariInRight, card << ariInRight, card AND ariInRight, card OR ariInRight, card XOR ariInRight);
    assertType11(ariCom::Input, ariInLeft +  ariInRight, ariInLeft -  ariInRight, ariInLeft  *  ariInRight, ariInLeft /  ariInRight, ariInLeft  %  ariInRight, ariInLeft ^ ariInRight,
                                ariInLeft >> ariInRight, ariInLeft << ariInRight, ariInLeft AND ariInRight, ariInLeft OR ariInRight, ariInLeft XOR ariInRight);

    //ariCom::Input::IntSignalType;
    assertType2(bool, ariInNonEach == ariInNonEach, ariInNonEach != ariInNonEach);
    assertType4(bool, ariInNonEach == ariIn, ariInNonEach != ariIn, ariIn == ariInNonEach, ariIn != ariInNonEach);

    assertType(ariCom, ariInNonEach on coal);
    assert2Invalid(ariInNonEach, each, ariInNonEach on each);

    //ariCom::Output;
    assertType2(bool, ariOut == ariOut, ariOut != ariOut);
    assertType4(bool, ariOut == coal, ariOut != coal, coal == ariOut, coal != ariOut);
    assertType4(bool, ariOut == each, ariOut != each, each == ariOut, each != ariOut);
    assertType4(bool, ariOut == card, ariOut != card, card == ariOut, card != ariOut);

    assertType2(ariCom, ariIn on ariOut, ariIn on ariOut);
    assertType2(ariCom, ariInNonEach on ariOut, ariInNonEach on ariOut);

    // left = deciInLeft, any, all, each, card, coal
    // right = deciInRight, coal, 0
    assertType3(deciCom::Input,             deciInLeft <deciMode> deciInRight, deciInLeft <deciMode> coal, deciInLeft <deciMode> 0);
    assertType3(deciCom::Input::AnyType,    any        <deciMode> deciInRight, any        <deciMode> coal, any        <deciMode> 0);
    assertType3(deciCom::Input::AllType,    all        <deciMode> deciInRight, all        <deciMode> coal, all        <deciMode> 0);
    assertType3(deciCom::Input::EachType,   each       <deciMode> deciInRight, each       <deciMode> coal, each       <deciMode> 0);
    assertType3(deciCom::Input,             card       <deciMode> deciInRight, card       <deciMode> coal, card       <deciMode> 0);
    assertType3(deciCom::Input::SignalType, coal       <deciMode> deciInRight, coal       <deciMode> coal, coal       <deciMode> 0);


    // left = ariInLeft, 0, each, card, coal
    // right = ariInRight, coal, 0
    assertType3(ariCom::Input,                ariInLeft <ariMode> ariInRight, ariInLeft <ariMode> coal, ariInLeft <ariMode> 0);
    assertType3(ariCom::Input::IntSignalType, 0         <ariMode> ariInRight, 0         <ariMode> coal, 0         <ariMode> 0);
    assertType3(ariCom::Input,                each      <ariMode> ariInRight, each      <ariMode> coal, each      <ariMode> 0);
    assertType3(ariCom::Input,                card      <ariMode> ariInRight, card      <ariMode> coal, card      <ariMode> 0);
    assertType3(ariCom::Input::IntSignalType, coal      <ariMode> ariInRight, coal      <ariMode> coal, coal      <ariMode> 0);
  }
}

#endif