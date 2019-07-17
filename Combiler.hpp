#pragma once

#include <cassert>
#include <variant>
#include <optional>
#include <vector>
#include <array>
#include <sstream>
#include "zlib.h"

template <class ...Fs>
struct overload : Fs... {
  overload(Fs const&... fs) : Fs{ fs }...
  {}

  using Fs::operator()...;
};

enum class color
{
  // red
  r,
  // green
  g,
  // red & green
  rg
};

template <color c>
struct connector
{
  size_t source;
  explicit connector(size_t n) : source(n) {}

  bool operator==(connector const& o) const;
  bool operator!=(connector const& o) const;
};
template<>
struct connector<color::rg>
{
  connector<color::r> r;
  connector<color::g> g;
  connector(connector<color::r> const& r, connector<color::g> const& g) : r(r), g(g) {}

  bool operator==(connector const& o) const { return this->r == o.r && this->g == o.g; }
  bool operator!=(connector const& o) const { return this->r != o.r || this->g != o.g; }
};


template <color c>
struct wire
{
  connector<c> source;
  wire(connector<c> const& s);
  static wire loop();

  bool operator==(wire const& o) const { return this->source == o.source; }
  bool operator!=(wire const& o) const { return this->source != o.source; }
};
template<>
struct wire<color::rg>
{
  wire<color::r> r;
  wire<color::g> g;
  wire(wire<color::r> const& r, wire<color::g> const& g) : r(r), g(g) {}
  wire(connector<color::rg> const& p) : r(p.r), g(p.g) {}
  static wire loop() { return wire(wire<color::r>::loop(), wire<color::g>::loop()); }

  bool operator==(wire const& o) const { return this->r == o.r && this->g == o.g; }
  bool operator!=(wire const& o) const { return this->r != o.r || this->g != o.g; }
};
wire<color::rg> operator+(wire<color::r> const& r, wire<color::g> const& g) { return wire<color::rg>(r, g); }
wire<color::rg> operator+(wire<color::g> const& g, wire<color::r> const& r) { return wire<color::rg>(r, g); }

struct Any
{
  bool operator==(Any const&) const { return true; }
  bool operator!=(Any const&) const { return false; }
  void toString(std::ostringstream& output) const
  {
    output << "\"type\":\"virtual\",\"name\":\"signal-anything\"";
  }
};
struct All
{
  bool operator==(All const&) const { return true; }
  bool operator!=(All const&) const { return false; }
  void toString(std::ostringstream& output) const
  {
    output << "\"type\":\"virtual\",\"name\":\"signal-everything\"";
  }
};
struct Each
{
  bool operator==(Each const&) const { return true; }
  bool operator!=(Each const&) const { return false; }
  void toString(std::ostringstream& output) const
  {
    output << "\"type\":\"virtual\",\"name\":\"signal-each\"";
  }
};
struct signal
{
  struct WithValue;
  struct Description;
  Description const * description;

  explicit signal(Description const * desc) : description(desc) {}
  WithValue operator=(int32_t const& value) const;

  void toString(std::ostringstream& output) const;
};
struct signal::WithValue
{
  signal sig;
  int32_t value;
  void toString(std::ostringstream& output) const
  {
    output << "\"signal\":{";
    sig.toString(output);
    output << "},\"count\":" << value;
  }
};
signal::WithValue signal::operator=(int32_t const& value) const { return signal::WithValue{ *this, value }; }
struct signal::Description
{
  std::string codeSyntax;
  std::string gameSyntax;
  std::string type;
  size_t index;

  Description(std::string const& codeSyntax, std::string const& gameSyntax, std::string const& type, size_t index)
    : codeSyntax(codeSyntax), gameSyntax(gameSyntax), type(type), index(index) {}
};

void signal::toString(std::ostringstream& output) const
{
  output << "\"type\":\"" << description->type << "\",\"name\":\"" << description->gameSyntax << "\"";
}

#define any Any()
#define all All()
#define each Each()

struct deciComData
{
  struct Mode
  {
    struct Description;
    // helper class to enable decider syntax with variable operator
    struct Helper;
    enum class Enum;

    static std::vector<Mode> const modes;
    //static std::vector<Mode> createModes();
    Description const * description;

    Mode(Description const * desc) : description(desc) { }
  };
  struct Input
  {
    using Left = std::variant<Any, All, Each, signal>;
    using Right = std::variant<int32_t, signal>;

    Left left;
    Right right;
    Mode mode;

    Input(Left const& left, Right const& right, Mode const& mode) : left(left), right(right), mode(mode) {}

    struct SignalType
    {
      signal left;
      Right right;
      Mode mode;

      SignalType(signal const& signal, Right const& right, Mode const& mode) : left(signal), right(right), mode(mode) {}
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
    using Value = std::optional<int32_t>;

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

  deciComData(Input::Left const& left, Input::Right const& right, Mode const& mode, Output::Type const& output, Output::Value const& value)
    : left(left), right(right), mode(mode), output(output), value(value) 
  {
    assert(!std::holds_alternative<Each>(output) || std::holds_alternative<Each>(left));
    assert(!std::holds_alternative<All>(output) || !std::holds_alternative<Each>(left));
  }
};

struct deciComData::Mode::Description
{
  std::string name;
  std::string codeSyntax;
  std::string gameSyntax;
  size_t index;

  Description(std::string const& name, std::string const& codeSyntax, std::string const& gameSyntax, size_t index)
    : name(name), codeSyntax(codeSyntax), gameSyntax(gameSyntax), index(index) {}
};
struct deciComData::Input::Boolable : deciComData::Input
{
  Boolable(Left const& left, Right const& right, Mode const& mode) : Input(left, right, mode) {}
  operator bool() const 
  { 
    return std::holds_alternative<signal>(left) 
        && std::holds_alternative<signal>(right) 
        && std::get<signal>(left).description == std::get<signal>(right).description; 
  }
};
deciComData::Input::SignalType::operator deciComData::Input() const { return Input(left, right, mode); }
struct deciComData::Input::SignalType::Boolable : deciComData::Input::SignalType
{
  Boolable(signal const& left, Right const& right, Mode const& mode) : SignalType(left, right, mode) {}
  operator bool() const 
  { 
    return std::holds_alternative<signal>(right) 
        && left.description == std::get<signal>(right).description; 
  }
};
struct deciComData::Input::AnyType
{
  Right right;
  Mode mode;

  AnyType(Right const& right, Mode const& mode) : right(right), mode(mode) {}
  explicit operator Input() const { return Input(any, right, mode); }
};
struct deciComData::Input::AllType
{
  Right right;
  Mode mode;

  AllType(Right const& right, Mode const& mode) : right(right), mode(mode) {}
  explicit operator Input() const { return Input(all, right, mode); }
};
struct deciComData::Input::EachType
{
  Right right;
  Mode mode;

  EachType(Right const& right, Mode const& mode) : right(right), mode(mode) {}
  explicit operator Input() const { return Input(each, right, mode); }
};
struct deciComData::Output::AllType
{
  Value value;

  AllType(Value const& value) : value(value) {}
  explicit operator Output() const { return Output(all, value); }
};
struct deciComData::Output::EachType
{
  Value value;

  EachType(Value const& value) : value(value) {}
  explicit operator Output() const { return Output(each, value); }
};

struct ariComData
{
  struct Mode
  {
    struct Description;
    struct Helper;
    enum class Enum;
    static std::vector<Mode> const modes;
    static std::vector<Mode> createModes();
    Description const * description;

    Mode(Description const * desc) : description(desc) { }
  };
  struct Input
  {
    using Left = std::variant<int32_t, Each, signal>;
    using Right = std::variant<int32_t, signal>;
    
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

  ariComData(Input::Left const& left, Input::Right const& right, Mode const& mode, Output output)
    : left(left), right(right), mode(mode), output(output) 
  {
    assert(!std::holds_alternative<Each>(output.value) || std::holds_alternative<Each>(left));
  }
};
struct ariComData::Mode::Description
{
  std::string name;
  std::string codeSyntax;
  std::string gameSyntax;
  size_t index;

  Description(std::string const& name, std::string const& codeSyntax, std::string const& gameSyntax, size_t index)
    : name(name), codeSyntax(codeSyntax), gameSyntax(gameSyntax), index(index) {}
};
struct ariComData::Input::IntSignalType
{
  std::variant<int32_t, signal> left;
  Right right;
  Mode mode;

  IntSignalType(std::variant<int32_t, signal> const& left, Right const& right, Mode const& mode) : left(left), right(right), mode(mode) {}
  explicit operator Input() const 
  { 
    if(std::holds_alternative<int32_t>(left))
      return Input(std::get<int32_t>(left), right, mode);
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

  operator deciComData::Input::Left() const
  {
    return std::visit(overload(
      [](Any const&) { return deciComData::Input::Left(any); },
      [](All const&) { return deciComData::Input::Left(all); },
      [](Each const&) { return deciComData::Input::Left(each); }
    ), this->value);
  }
  operator deciComData::Output::Type() const
  {
    return std::visit(overload(
      [](Any const&) { assert(false); return deciComData::Output::Type(all); }, // deciCom output is never allowed to be any
      [](All const&) { return deciComData::Output::Type(all); },
      [](Each const&) { return deciComData::Output::Type(each); }
    ), this->value);
  }
  operator ariComData::Input::Left() const
  {
    assert(std::holds_alternative<Each>(this->value)); // ariCom input is never allowed to be any or all
    return ariComData::Input::Left(each);
  }
  explicit operator ariComData::Output() const
  {
    assert(std::holds_alternative<Each>(this->value)); // ariCom output is never allowed to be any or all
    return ariComData::Output(each);
  }
};

#define operations(equalArg, compArg)               \
  op(smaller,      <,  "<",               compArg)  \
  op(greater,      >,  ">",               compArg)  \
  op(equal,        ==, "=",               equalArg) \
  op(greaterEqual, >=, u8"≥"/*"\u2265"*/, compArg)  \
  op(smallerEqual, <=, u8"≤"/*"\u2264"*/, compArg)  \
  op(notEqual,     !=, u8"≠"/*"\u2260"*/, equalArg)


enum class deciComData::Mode::Enum {
#define op(opName, codeSyntax, gameSyntax, arg) \
  opName,
  operations(,) };
#undef op

std::vector<deciComData::Mode> const deciComData::Mode::modes = []()
{
  std::vector<Mode> result;
  size_t i = 0;
#define op(name, codeSyntax, gameSyntax, arg) \
  result.push_back(Mode(new deciComData::Mode::Description(#name, #codeSyntax, gameSyntax, i++)));
  operations(, )
#undef op
    return result;
}();
#define input deciComData::Output::Value()

namespace deciComMode {
#define op(opName, codeSyntax, gameSyntax, arg) \
  deciComData::Mode const opName = deciComData::Mode::modes[size_t(deciComData::Mode::Enum::##opName)];
  operations(,)
#undef op
}

#define allRightOps(codeSyntax, leftType, outputType, generalOutType, signalOutType, ...)                                      \
auto operator codeSyntax(leftType, deciComData::Input::Right const& right) { return outputType##generalOutType(__VA_ARGS__); } \
auto operator codeSyntax(leftType, signal const& right)                    { return outputType##signalOutType(__VA_ARGS__); }  \
auto operator codeSyntax(leftType, int32_t const& right)                   { return outputType(__VA_ARGS__); }

#define op(opName, codeSyntax, gameSyntax, arg)                                                                                           \
allRightOps(codeSyntax, deciComData::Input::Left const& left, deciComData::Input,             arg, arg, left, right, deciComMode::opName) \
allRightOps(codeSyntax, Any const&,                           deciComData::Input::AnyType,       ,    , right, deciComMode::opName)       \
allRightOps(codeSyntax, All const&,                           deciComData::Input::AllType,       ,    , right, deciComMode::opName)       \
allRightOps(codeSyntax, Each const&,                          deciComData::Input::EachType,      ,    , right, deciComMode::opName)       \
allRightOps(codeSyntax, signal const& left,                   deciComData::Input::SignalType, arg, arg, left, right, deciComMode::opName) \
allRightOps(codeSyntax, wildCard const& left,                 deciComData::Input,                ,    , left, right, deciComMode::opName)
operations(::Boolable,)
#undef op
#undef allRightOps
#undef operations

auto operator+=(deciComData::Output::Type const& left, deciComData::Output::Value const& right) { return deciComData::Output(left, right); }
auto operator+=(signal const& left,                    deciComData::Output::Value const& right) { return deciComData::Output(left, right); }
auto operator+=(All const&,                            deciComData::Output::Value const& right) { return deciComData::Output::AllType(right); }
auto operator+=(Each const&,                           deciComData::Output::Value const& right) { return deciComData::Output::EachType(right); }

template<color c>
struct deciCom
{
  std::shared_ptr<deciComData> data;
  deciCom(std::shared_ptr<deciComData> const& other) : data(other) {}
};
template<>
struct deciCom<color::rg>
{
  std::shared_ptr<deciComData> data;
  deciCom<color::r> r;
  deciCom<color::g> g;

  deciCom(deciComData::Input::Left const& left
    , deciComData::Input::Right const& right
    , deciComData::Mode const& mode
    , deciComData::Output::Type const& output
    , deciComData::Output::Value const& value)
    : data(std::make_shared<deciComData>(left, right, mode, output, value)), r(this->data), g(this->data) {}

  deciComData* operator->() { return data.operator->(); }
};
deciCom(deciComData::Input::Left, deciComData::Input::Right, deciComData::Mode, deciComData::Output::Type, deciComData::Output::Value)->deciCom<color::rg>;

#define then >>=
auto operator then(deciComData::Input const& left,             deciComData::Output const& right)           { return deciCom<color::rg>(left.left, left.right, left.mode, right.output, right.value); }
auto operator then(deciComData::Input const& left,             deciComData::Output::AllType const& right)  { return deciCom<color::rg>(left.left, left.right, left.mode, all, right.value); }
auto operator then(deciComData::Input const& left,             deciComData::Output::EachType const& right) { return deciCom<color::rg>(left.left, left.right, left.mode, each, right.value); }
auto operator then(deciComData::Input::SignalType const& left, deciComData::Output const& right)           { return deciCom<color::rg>(left.left, left.right, left.mode, right.output, right.value); }
auto operator then(deciComData::Input::SignalType const& left, deciComData::Output::AllType const& right)  { return deciCom<color::rg>(left.left, left.right, left.mode, all, right.value); }
auto operator then(deciComData::Input::AllType const& left,    deciComData::Output const& right)           { return deciCom<color::rg>(all, left.right, left.mode, right.output, right.value); }
auto operator then(deciComData::Input::AllType const& left,    deciComData::Output::AllType const& right)  { return deciCom<color::rg>(all, left.right, left.mode, all, right.value); }
auto operator then(deciComData::Input::AnyType const& left,    deciComData::Output const& right)           { return deciCom<color::rg>(any, left.right, left.mode, right.output, right.value); }
auto operator then(deciComData::Input::AnyType const& left,    deciComData::Output::AllType const& right)  { return deciCom<color::rg>(any, left.right, left.mode, all, right.value); }
auto operator then(deciComData::Input::EachType const& left,   deciComData::Output const& right)           { return deciCom<color::rg>(each, left.right, left.mode, right.output, right.value); }
auto operator then(deciComData::Input::EachType const& left,   deciComData::Output::EachType const& right) { return deciCom<color::rg>(each, left.right, left.mode, each, right.value); }


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

enum class ariComData::Mode::Enum {
#define op(opName, codeSyntax, gameSyntax) \
  opName,
  operations };
#undef op


std::vector<ariComData::Mode> ariComData::Mode::createModes()
{
  std::vector<ariComData::Mode> result;
  size_t i = 0;
#define op(name, codeSyntax, gameSyntax) \
    result.push_back(ariComData::Mode(new ariComData::Mode::Description(#name, #codeSyntax, gameSyntax, i++)));  
  operations;
#undef op
  return result;
}
std::vector<ariComData::Mode> const ariComData::Mode::modes = ariComData::Mode::createModes();

namespace ariComMode
{
#define op(opName, codeSyntax, gameSyntax) \
  ariComData::Mode const opName = ariComData::Mode::modes[size_t(ariComData::Mode::Enum::##opName)];
  operations
#undef op
}

#define AND &
#define OR |
#define XOR ||

#define op(opName, codeSyntax, gameSyntax)                                                                                                                                         \
auto operator codeSyntax(ariComData::Input::Left const& left, ariComData::Input::Right const& right) { return ariComData::Input(left, right, ariComMode::opName); }                \
auto operator codeSyntax(int32_t const& left,                 ariComData::Input::Right const& right) { return ariComData::Input::IntSignalType(left, right, ariComMode::opName); } \
auto operator codeSyntax(Each const&,                         ariComData::Input::Right const& right) { return ariComData::Input(each, right, ariComMode::opName); }                \
auto operator codeSyntax(signal const& left,                  ariComData::Input::Right const& right) { return ariComData::Input::IntSignalType(left, right, ariComMode::opName); }
operations
#undef op
#undef operations

template<color c>
struct ariCom
{
  std::shared_ptr<ariComData> data;
  ariCom(std::shared_ptr<ariComData> const& other) : data(other) {}
};
template<>
struct ariCom<color::rg>
{
  std::shared_ptr<ariComData> data;
  ariCom<color::r> r;
  ariCom<color::g> g;

  ariCom(ariComData::Input::Left const& left
    , ariComData::Input::Right const& right
    , ariComData::Mode const& mode
    , ariComData::Output const& output)
    : data(std::make_shared<ariComData>(left, right, mode, output)), r(this->data), g(this->data) {}

  ariComData* operator->() { return data.operator->(); }
};
ariCom(ariComData::Input::Left, ariComData::Input::Right, ariComData::Mode, ariComData::Output)->ariCom<color::rg>;

#define on >>=
auto operator on(ariComData::Input const& left,                ariComData::Output const& right) { return ariCom<color::rg>(left.left, left.right, left.mode, right); }
auto operator on(ariComData::Input const& left,                Each const&)                     { return left on ariComData::Output(each); }
auto operator on(ariComData::Input const& left,                wildCard const& right)           { return left on ariComData::Output(right); }
auto operator on(ariComData::Input::IntSignalType const& left, ariComData::Output const& right) { return ariComData::Input(left) on right; }


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

autoOperators(deciComData::Mode, description)
autoOperators(deciComData, left, right, mode, output, value)
autoOperators(deciComData::Mode::Description, index)
autoOperators(deciComData::Input, left, right, mode)
autoOperators(deciComData::Input::AllType, right, mode)
autoOperators(deciComData::Input::AnyType, right, mode)
autoOperators(deciComData::Input::EachType, right, mode)
autoOperators(deciComData::Input::SignalType, left, right, mode)
autoOperators(deciComData::Output, output, value)
autoOperators(deciComData::Output::AllType, value)
autoOperators(deciComData::Output::EachType, value)

autoOperators(ariComData::Mode, description)
autoOperators(ariComData::Output, value)
autoOperators(ariComData, left, right, mode, output)
autoOperators(ariComData::Mode::Description, index)
autoOperators(ariComData::Input, left, right, mode)
autoOperators(ariComData::Input::IntSignalType, left, right, mode)

#undef autoOperator
#define autoOperator(in, out, ...) std::operator==(left, right)
autoOperators(deciComData::Input::Left)
autoOperators(deciComData::Input::Right)
autoOperators(deciComData::Output::Type)
autoOperators(deciComData::Output::Value)
autoOperators(ariComData::Input::Left)
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

bool operator==(deciComData::Input const& left, deciComData::Input::AnyType const& right) { return left == deciComData::Input(right); }
bool operator==(deciComData::Input const& left, deciComData::Input::AllType const& right) { return left == deciComData::Input(right); }
bool operator==(deciComData::Input const& left, deciComData::Input::EachType const& right) { return left == deciComData::Input(right); }
bool operator==(deciComData::Input const& left, deciComData::Input::SignalType const& right) { return left == deciComData::Input(right); }
bool operator!=(deciComData::Input const& left, deciComData::Input::AnyType const& right) { return left != deciComData::Input(right); }
bool operator!=(deciComData::Input const& left, deciComData::Input::AllType const& right) { return left != deciComData::Input(right); }
bool operator!=(deciComData::Input const& left, deciComData::Input::EachType const& right) { return left != deciComData::Input(right); }
bool operator!=(deciComData::Input const& left, deciComData::Input::SignalType const& right) { return left != deciComData::Input(right); }

bool operator==(deciComData::Input::AnyType const& left, deciComData::Input const& right) { return deciComData::Input(left) == right; }
bool operator==(deciComData::Input::AllType const& left, deciComData::Input const& right) { return deciComData::Input(left) == right; }
bool operator==(deciComData::Input::EachType const& left, deciComData::Input const& right) { return deciComData::Input(left) == right; }
bool operator==(deciComData::Input::SignalType const& left, deciComData::Input const& right) { return deciComData::Input(left) == right; }
bool operator!=(deciComData::Input::AnyType const& left, deciComData::Input const& right) { return deciComData::Input(left) != right; }
bool operator!=(deciComData::Input::AllType const& left, deciComData::Input const& right) { return deciComData::Input(left) != right; }
bool operator!=(deciComData::Input::EachType const& left, deciComData::Input const& right) { return deciComData::Input(left) != right; }
bool operator!=(deciComData::Input::SignalType const& left, deciComData::Input const& right) { return deciComData::Input(left) != right; }

bool operator==(deciComData::Output const& left, deciComData::Output::AllType const& right) { return left == deciComData::Output(right); }
bool operator==(deciComData::Output const& left, deciComData::Output::EachType const& right) { return left == deciComData::Output(right); }
bool operator!=(deciComData::Output const& left, deciComData::Output::AllType const& right) { return left != deciComData::Output(right); }
bool operator!=(deciComData::Output const& left, deciComData::Output::EachType const& right) { return left != deciComData::Output(right); }

bool operator==(deciComData::Output::AllType const& left, deciComData::Output const& right) { return deciComData::Output(left) == right; }
bool operator==(deciComData::Output::EachType const& left, deciComData::Output const& right) { return deciComData::Output(left) == right; }
bool operator!=(deciComData::Output::AllType const& left, deciComData::Output const& right) { return deciComData::Output(left) != right; }
bool operator!=(deciComData::Output::EachType const& left, deciComData::Output const& right) { return deciComData::Output(left) != right; }

bool operator==(ariComData::Input const& left, ariComData::Input::IntSignalType const& right) { return left == ariComData::Input(right); }
bool operator!=(ariComData::Input const& left, ariComData::Input::IntSignalType const& right) { return left != ariComData::Input(right); }
bool operator==(ariComData::Input::IntSignalType const& left, ariComData::Input const& right) { return ariComData::Input(left) == right; }
bool operator!=(ariComData::Input::IntSignalType const& left, ariComData::Input const& right) { return ariComData::Input(left) != right; }

bool operator==(ariComData::Output const& out, Each const&) { return std::holds_alternative<Each>(out.value); }
bool operator!=(ariComData::Output const& out, Each const&) { return !std::holds_alternative<Each>(out.value); }
bool operator==(Each const&, ariComData::Output const& out) { return std::holds_alternative<Each>(out.value); }
bool operator!=(Each const&, ariComData::Output const& out) { return !std::holds_alternative<Each>(out.value); }

bool operator==(ariComData::Output const& out, wildCard const& card) { return std::holds_alternative<Each>(out.value) && std::holds_alternative<Each>(card.value); }
bool operator!=(ariComData::Output const& out, wildCard const& card) { return !std::holds_alternative<Each>(out.value) || !std::holds_alternative<Each>(card.value); }
bool operator==(wildCard const& card, ariComData::Output const& out) { return std::holds_alternative<Each>(out.value) && std::holds_alternative<Each>(card.value); }
bool operator!=(wildCard const& card, ariComData::Output const& out) { return !std::holds_alternative<Each>(out.value) || !std::holds_alternative<Each>(card.value); }


struct deciComData::Mode::Helper
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

auto operator<(deciComData::Input::Left const& left, deciComData::Mode const& right) { return deciComData::Mode::Helper(left, right); }
auto operator<(Any const&,                           deciComData::Mode const& right) { return deciComData::Mode::Helper::AnyType(right); }
auto operator<(All const&,                           deciComData::Mode const& right) { return deciComData::Mode::Helper::AllType(right); }
auto operator<(Each const&,                          deciComData::Mode const& right) { return deciComData::Mode::Helper::EachType(right); }
auto operator<(signal const& left,                   deciComData::Mode const& right) { return deciComData::Mode::Helper::SignalType(left, right); }
auto operator<(wildCard const& left,                 deciComData::Mode const& right) { return deciComData::Mode::Helper(left, right); }

auto operator>(deciComData::Mode::Helper const& left, deciComData::Input::Right const& right)             { return deciComData::Input(left.left, right, left.mode); }
auto operator>(deciComData::Mode::Helper const& left, signal const& right)                                { return deciComData::Input(left.left, right, left.mode); }
auto operator>(deciComData::Mode::Helper const& left, int32_t const& right)                               { return deciComData::Input(left.left, right, left.mode); }
auto operator>(deciComData::Mode::Helper::AnyType const& left, deciComData::Input::Right const& right)    { return deciComData::Input::AnyType(right, left.mode); }
auto operator>(deciComData::Mode::Helper::AnyType const& left, signal const& right)                       { return deciComData::Input::AnyType(right, left.mode); }
auto operator>(deciComData::Mode::Helper::AnyType const& left, int32_t const& right)                      { return deciComData::Input::AnyType(right, left.mode); }
auto operator>(deciComData::Mode::Helper::AllType const& left, deciComData::Input::Right const& right)    { return deciComData::Input::AllType(right, left.mode); }
auto operator>(deciComData::Mode::Helper::AllType const& left, signal const& right)                       { return deciComData::Input::AllType(right, left.mode); }
auto operator>(deciComData::Mode::Helper::AllType const& left, int32_t const& right)                      { return deciComData::Input::AllType(right, left.mode); }
auto operator>(deciComData::Mode::Helper::EachType const& left, deciComData::Input::Right const& right)   { return deciComData::Input::EachType(right, left.mode); }
auto operator>(deciComData::Mode::Helper::EachType const& left, signal const& right)                      { return deciComData::Input::EachType(right, left.mode); }
auto operator>(deciComData::Mode::Helper::EachType const& left, int32_t const& right)                     { return deciComData::Input::EachType(right, left.mode); }
auto operator>(deciComData::Mode::Helper::SignalType const& left, deciComData::Input::Right const& right) { return deciComData::Input::SignalType(left.left, right, left.mode); }
auto operator>(deciComData::Mode::Helper::SignalType const& left, signal const& right)                    { return deciComData::Input::SignalType(left.left, right, left.mode); }
auto operator>(deciComData::Mode::Helper::SignalType const& left, int32_t const& right)                   { return deciComData::Input::SignalType(left.left, right, left.mode); }

struct ariComData::Mode::Helper
{
  Input::Left left;
  Mode mode;
  Helper(Input::Left const& left, Mode const& mode) : left(left), mode(mode) {}
  struct IntSignalType
  {
    std::variant<int32_t, signal> left;
    Mode mode;
    IntSignalType(std::variant<int32_t, signal> const& left, Mode const& mode) : left(left), mode(mode) {}
  };
};

auto operator<(ariComData::Input::Left const& left, ariComData::Mode const& right) { return ariComData::Mode::Helper(left, right); }
auto operator<(int32_t const& left,                 ariComData::Mode const& right) { return ariComData::Mode::Helper::IntSignalType(left, right); }
auto operator<(Each const&,                         ariComData::Mode const& right) { return ariComData::Mode::Helper(each, right); }
auto operator<(signal const& left,                  ariComData::Mode const& right) { return ariComData::Mode::Helper::IntSignalType(left, right); }

auto operator>(ariComData::Mode::Helper const& left, ariComData::Input::Right const& right) { return ariComData::Input(left.left, right, left.mode); }
auto operator>(ariComData::Mode::Helper::IntSignalType const& left, ariComData::Input::Right const& right) { return ariComData::Input::IntSignalType(left.left, right, left.mode); }

using conComData = std::array<std::optional<signal::WithValue>, 18>;

std::variant<conComData, deciComData, ariComData> to(std::variant<deciComData, ariComData> const& from)
{
  return std::visit(overload(
    [](deciComData const& from) -> std::variant<conComData, deciComData, ariComData> { return from; },
    [](ariComData const& from)  -> std::variant<conComData, deciComData, ariComData> { return from; }
  ), from);
}

struct networkSource
{
  std::optional<std::variant<conComData, deciComData, ariComData>> combinator;
  std::optional<std::variant<wire<color::r>, wire<color::g>, wire<color::rg>>> combinatorInput;

  networkSource(conComData const& comb) : combinator(comb), combinatorInput() {}
  networkSource(std::variant<deciComData, ariComData> const& combinator
              , std::variant<wire<color::r>, wire<color::g>, wire<color::rg>> const& source)
    : combinator(to(combinator)), combinatorInput(source) {}
  networkSource() : combinator(), combinatorInput() {}
};
struct network
{
  std::vector<size_t> sources;
  std::vector<size_t> referencesInLookup; // contains i if and only if this == networks[networkLookup[i]]
  color c;
  bool canStillMerge = true;
  
  network(size_t const& index
    , color c)
    : sources()
    , referencesInLookup()
  {
    this->c = c;
    sources.push_back(index);
  }
};

std::vector<networkSource> sources;
std::vector<network> networks;
std::vector<size_t> networkLookup;

template<color c>
wire<c>::wire(connector<c> const& s) : source(s) { networks[networkLookup[this->source.source]].canStillMerge = false; }
template<color c>
bool connector<c>::operator==(connector<c> const& o) const { return networkLookup[this->source] == networkLookup[o.source]; }
// ^^ these won't specialize for wire|connector<color::rg>, since the complete struct was specialized

void merge(size_t left, size_t right)
{
  size_t lLeft = networkLookup[left];
  size_t lRight = networkLookup[right];
  network& ln = networks[lLeft];
  network& rn = networks[lRight];
  assert(ln.c == rn.c /* cannot merge connectors with different colors! */);
  assert(&ln != &rn /* cannot merge a network with itself! */);
  if (ln.canStillMerge)
  {
    // merge connectors ln and rn
    assert(ln.canStillMerge && rn.canStillMerge /* cannot merge connectors after converting them to wires! */);
    size_t lnEnd = ln.sources.size();
    ln.sources.insert(ln.sources.end(), std::make_move_iterator(rn.sources.begin()), std::make_move_iterator(rn.sources.end()));
    for (auto ref : rn.referencesInLookup)
      networkLookup[ref] = lLeft;
    ln.referencesInLookup.insert(ln.referencesInLookup.end(), std::make_move_iterator(rn.referencesInLookup.begin()), std::make_move_iterator(rn.referencesInLookup.end()));
    if (lRight != networks.size() - 1)
    {
      for (auto ref : networks.back().referencesInLookup)
        networkLookup[ref] = lRight;
      std::swap(networks.back(), rn);
    }
    networks.pop_back();
  }
  else
  {
    // merge connector rn into loop wire ln
    assert(ln.referencesInLookup.size() == 1 && ln.sources.size() == 1); // are you maybe trying to merge a second connector into the looped wire?
    assert(!sources[ln.sources[0]].combinator.has_value() && !sources[ln.sources[0]].combinatorInput.has_value());
    networkLookup[ln.referencesInLookup[0]] = lRight;
    rn.referencesInLookup.push_back(ln.referencesInLookup[0]);
    rn.canStillMerge = false;
    if (lLeft != networks.size() - 1)
    {
      for (auto ref : networks.back().referencesInLookup)
        networkLookup[ref] = lLeft;
      std::swap(networks.back(), ln);
    }
    networks.pop_back();
  }
}


template<color c>
connector<c> getConnector(networkSource&& source)
{
  sources.emplace_back(source);
  networks.emplace_back(network(sources.size() - 1, c));
  networkLookup.emplace_back(networks.size() - 1);
  networks.back().referencesInLookup.emplace_back(networkLookup.size() - 1);
  return connector<c>(networkLookup.size() - 1);
}
template<>
connector<color::rg> getConnector(networkSource&& source)
{
  sources.emplace_back(source);
  networks.emplace_back(network(sources.size() - 1, color::r));
  networkLookup.emplace_back(networks.size() - 1);
  networks.back().referencesInLookup.emplace_back(networkLookup.size() - 1);
  connector<color::r> r(networkLookup.size() - 1);

  networks.emplace_back(network(sources.size() - 1, color::g));
  networkLookup.emplace_back(networks.size() - 1);
  networks.back().referencesInLookup.emplace_back(networkLookup.size() - 1);
  connector<color::g> g(networkLookup.size() - 1);

  return connector<color::rg>(r, g);
}

template<color r>
wire<r> wire<r>::loop()
{
  return getConnector<r>(networkSource());
}

void operator <<=(wire<color::r> const& left, connector<color::r> const& right) { merge(left.source.source, right.source); }
void operator <<=(wire<color::g> const& left, connector<color::g> const& right) { merge(left.source.source, right.source); }
void operator <<=(wire<color::rg> const& left, connector<color::rg> const& right) { merge(left.r.source.source, right.r.source); merge(left.g.source.source, right.g.source); }

template<color c>
struct conCom
{
  std::shared_ptr<conComData> data;
  explicit conCom(std::shared_ptr<conComData> const& other) : data(other) {}

  operator connector<c>() { return getConnector<c>(*this->data); }
  operator wire<c>() { return this->operator connector<c>(); }
};
template<>
struct conCom<color::rg>
{
  std::shared_ptr<conComData> data;
  conCom<color::r> r;
  conCom<color::g> g;

  explicit conCom(conComData const& data)
    : data(std::make_shared<conComData>(data)), r(this->data), g(this->data) {}
  conCom(std::optional<signal::WithValue> const& val0 = {},  std::optional<signal::WithValue> const& val1 = {},  std::optional<signal::WithValue> const& val2 = {}
       , std::optional<signal::WithValue> const& val3 = {},  std::optional<signal::WithValue> const& val4 = {},  std::optional<signal::WithValue> const& val5 = {}
       , std::optional<signal::WithValue> const& val6 = {},  std::optional<signal::WithValue> const& val7 = {},  std::optional<signal::WithValue> const& val8 = {}
       , std::optional<signal::WithValue> const& val9 = {},  std::optional<signal::WithValue> const& val10 = {}, std::optional<signal::WithValue> const& val11 = {}
       , std::optional<signal::WithValue> const& val12 = {}, std::optional<signal::WithValue> const& val13 = {}, std::optional<signal::WithValue> const& val14 = {}
       , std::optional<signal::WithValue> const& val15 = {}, std::optional<signal::WithValue> const& val16 = {}, std::optional<signal::WithValue> const& val17 = {})
    : data(std::make_shared<conComData>(conComData{ val0, val1, val2, val3, val4, val5, val6, val7, val8, val9, val10, val11, val12, val13, val14, val15, val16, val17 })), r(this->data), g(this->data) {}

  operator connector<color::rg>() { return getConnector<color::rg>(*this->data); }
  operator wire<color::rg>() { return this->operator connector<color::rg>(); }
  conComData* operator->() { return data.operator->(); }
};
conCom(conComData)->conCom<color::rg>;
conCom(std::initializer_list<signal::WithValue>)->conCom<color::rg>;
conCom(...)->conCom<color::rg>;

template <color c>
wire(conCom<c>)->wire<c>;

template<color c> connector<c> operator>>(wire<color::rg> const& p,      deciCom<c> const& d) { return getConnector<c>(networkSource(*d.data, p)); }
template<color c> connector<c> operator> (wire<color::r> const& p,       deciCom<c> const& d) { return getConnector<c>(networkSource(*d.data, p)); }
template<color c> connector<c> operator> (wire<color::g> const& p,       deciCom<c> const& d) { return getConnector<c>(networkSource(*d.data, p)); }
template<color c> connector<c> operator> (connector<color::rg> const& p, deciCom<c> const& d)
{
  assert(networks[networkLookup[p.r.source]].sources == networks[networkLookup[p.g.source]].sources /* cannot pick one color arbitrarily if connectors aren't holding equal values! */);
  return getConnector<c>(networkSource(*d.data, p.r));
}

template<color c> connector<c> operator>>(wire<color::rg> const& p,      ariCom<c> const& a) { return getConnector<c>(networkSource(*a.data, p)); }
template<color c> connector<c> operator> (wire<color::r> const& r,       ariCom<c> const& a) { return getConnector<c>(networkSource(*a.data, r)); }
template<color c> connector<c> operator> (wire<color::g> const& g,       ariCom<c> const& a) { return getConnector<c>(networkSource(*a.data, g)); }
template<color c> connector<c> operator> (connector<color::rg> const& p, ariCom<c> const& a)
{
  assert(networks[networkLookup[p.r.source]].sources == networks[networkLookup[p.g.source]].sources /* cannot pick one color arbitrarily if connectors aren't holding equal values! */);
  return getConnector<c>(networkSource(*a.data, p.r));
}

connector<color::r>  operator+=(connector<color::r> const& left,  connector<color::r> const& right)  { merge(left.source, right.source); return left; }
connector<color::g>  operator+=(connector<color::g> const& left,  connector<color::g> const& right)  { merge(left.source, right.source); return left; }
connector<color::rg> operator+=(connector<color::rg> const& left, connector<color::rg> const& right) { merge(left.r.source, right.r.source); merge(left.g.source, right.g.source); return left; }

connector<color::rg> operator+=(connector<color::rg> const& left, connector<color::r> const& right)  { merge(left.r.source, right.source); return left; }
connector<color::rg> operator+=(connector<color::r> const& left,  connector<color::rg> const& right) { merge(left.source, right.r.source); return right; }
connector<color::rg> operator+=(connector<color::rg> const& left, connector<color::g> const& right)  { merge(left.g.source, right.source); return left; }
connector<color::rg> operator+=(connector<color::g> const& left,  connector<color::rg> const& right) { merge(left.source, right.g.source); return right; }

std::string toGameCode(const char* s)
{
  std::string result = s;
  std::replace(result.begin(), result.end(), '_', '-');
  return result;
}


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
    result.push_back(signal(new signal::Description(#name, toGameCode(#name), "item", i++)));
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
  signal const opName = itemSignals[size_t(itemSignalsEnum::##opName)];
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
    result.push_back(signal(new signal::Description(#name, toGameCode(#name), "fluid", i++)));
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
  signal const opName = fluidSignals[size_t(fluidSignalsEnum::##opName)];
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
    result.push_back(signal(new signal::Description(#score#name, "signal-"#name, "virtual", i++)));
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
  signal const score##opName  = virtualSignals[size_t(virtualSignalsEnum::##score##opName)];
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

#define assertInvalid(var, exp)  static_assert(!is_valid<decltype(var)>([](auto temp) constexpr -> decltype(void(exp)){}))
#define assertInvalid11(var, expr1, expr2, expr3, expr4, expr5, expr6, expr7, expr8, expr9, expr10, expr11)                                 \
    assertInvalid(var, expr1); assertInvalid(var, expr2); assertInvalid(var, expr3); assertInvalid(var, expr4); assertInvalid(var, expr5);  \
    assertInvalid(var, expr6); assertInvalid(var, expr7); assertInvalid(var, expr8); assertInvalid(var, expr9); assertInvalid(var, expr10); \
    assertInvalid(var, expr11)

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
                    deciComData deci,
                    deciComData::Mode deciMode,
                    deciComData::Mode::Description deciModeDesc,
                    deciComData::Input deciIn,
                    deciComData::Input::Left deciInLeft,
                    deciComData::Input::Right deciInRight,
                    deciComData::Input::AllType deciInAll,
                    deciComData::Input::AnyType deciInAny,
                    deciComData::Input::EachType deciInEach,
                    deciComData::Input::SignalType deciInSig,
                    deciComData::Input::SignalType::Boolable deciInSigBool,
                    deciComData::Input::Boolable deciInBool,
                    deciComData::Output deciOut,
                    deciComData::Output::Type deciOutType,
                    deciComData::Output::Value deciOutVal,
                    deciComData::Output::AllType deciOutAll,
                    deciComData::Output::EachType deciOutEach,

                    ariComData ari,
                    ariComData::Mode ariMode,
                    ariComData::Mode::Description ariModeDesc,
                    ariComData::Input ariIn,
                    ariComData::Input::Left ariInLeft,
                    ariComData::Input::Right ariInRight,
                    ariComData::Input::IntSignalType ariInNonEach,
                    ariComData::Output ariOut)
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

    assertType6(deciComData::Input::AnyType,  any  == 0, any  != 0, any  < 0, any  <= 0, any  > 0, any  >= 0);
    assertType6(deciComData::Input::AllType,  all  == 0, all  != 0, all  < 0, all  <= 0, all  > 0, all  >= 0);
    assertType6(deciComData::Input::EachType, each == 0, each != 0, each < 0, each <= 0, each > 0, each >= 0);

    assertType2(deciComData::Output::AllType,  all  += 1, all  += input);
    assertType2(deciComData::Output::EachType, each += 1, each += input);

    assertInvalid11(any, temp + 0, temp - 0, temp * 0, temp / 0, temp % 0, temp ^ 0, temp >> 0, temp << 0, temp AND 0, temp OR 0, temp XOR 0);
    assertInvalid11(all, temp + 0, temp - 0, temp * 0, temp / 0, temp % 0, temp ^ 0, temp >> 0, temp << 0, temp AND 0, temp OR 0, temp XOR 0);
    assertType11(ariComData::Input, each + 0, each - 0, each * 0, each / 0, each % 0, each ^ 0, each >> 0, each << 0, each AND 0, each OR 0, each XOR 0);

    // wildCard
    assertType2(bool, card == card, card != card);
    assertType4(bool, card == any,  card != any,  any == card,  any != card);
    assertType4(bool, card == all,  card != all,  all == card,  all != card);
    assertType4(bool, card == each, card != each, each == card, each != card);

    assertType11(ariComData::Input, card + 0, card - 0, card * 0, card / 0, card % 0, card ^ 0, card >> 0, card << 0, card AND 0, card OR 0, card XOR 0);

    // signal
    assert2Types2(bool, deciComData::Input::SignalType, coal == coal, coal != coal);
    assertType4(deciComData::Input::SignalType,                       coal < coal, coal <= coal, coal > coal, coal >= coal);
    assertType6(deciComData::Input::SignalType, coal == 0, coal != 0, coal < 0,    coal <= 0,    coal > 0,    coal >= 0);

    assertType2(deciComData::Output, coal += 0, coal += input);

    assertInvalid11(any, temp + coal, temp - coal, temp * coal, temp / coal, temp % coal, temp ^ coal, temp >> coal, temp << coal, temp AND coal, temp OR coal, temp XOR coal);
    assertInvalid11(all, temp + coal, temp - coal, temp * coal, temp / coal, temp % coal, temp ^ coal, temp >> coal, temp << coal, temp AND coal, temp OR coal, temp XOR coal);
    assertType11(ariComData::Input, each + coal, each - coal, each * coal, each / coal, each % coal, each ^ coal, each >> coal, each << coal, each AND coal, each OR coal, each XOR coal);
    assertType11(ariComData::Input, card + coal, card - coal, card * coal, card / coal, card % coal, card ^ coal, card >> coal, card << coal, card AND coal, card OR coal, card XOR coal);

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
    assert2Types2(bool, deciComData::Input, deciInLeft == coal, deciInLeft != coal);
    assertType4(deciComData::Input,                                   deciInLeft < coal, deciInLeft <= coal, deciInLeft > coal, deciInLeft >= coal);
    assertType6(deciComData::Input, deciInLeft == 0, deciInLeft != 0, deciInLeft < 0,    deciInLeft <= 0,    deciInLeft > 0,    deciInLeft >= 0);
    assertType4(bool, deciInLeft == any,  deciInLeft != any,  any  == deciInLeft, any  != deciInLeft);
    assertType4(bool, deciInLeft == all,  deciInLeft != all,  all  == deciInLeft, all  != deciInLeft);
    assertType4(bool, deciInLeft == each, deciInLeft != each, each == deciInLeft, each != deciInLeft);
    assertType4(bool, deciInLeft == card, deciInLeft != card, card == deciInLeft, card != deciInLeft);

    //deciCom::Input::Right;
    assertType2(bool, deciInRight == deciInRight, deciInRight != deciInRight);
    assert2Types2(bool, deciComData::Input::SignalType, coal == deciInRight, coal != deciInRight);
    assertType4(deciComData::Input::SignalType,                                         coal < deciInRight, coal <= deciInRight, coal > deciInRight, coal >= deciInRight);
    assertType6(deciComData::Input::AnyType,  any  == deciInRight, any  != deciInRight, any  < deciInRight, any  <= deciInRight, any  > deciInRight, any >= deciInRight);
    assertType6(deciComData::Input::AllType,  all  == deciInRight, all  != deciInRight, all  < deciInRight, all  <= deciInRight, all  > deciInRight, all >= deciInRight);
    assertType6(deciComData::Input::EachType, each == deciInRight, each != deciInRight, each < deciInRight, each <= deciInRight, each > deciInRight, each >= deciInRight);
    assert2Types2(bool, deciComData::Input, deciInLeft == deciInRight, deciInLeft != deciInRight);
    assertType4(deciComData::Input, deciInLeft < deciInRight, deciInLeft <= deciInRight, deciInLeft > deciInRight, deciInLeft >= deciInRight);

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
    assertType(deciCom<color::rg>, deciIn then deciOut);
    assertType6(deciCom<color::rg>, deciInAll then deciOut, deciInAny then deciOut, deciInEach then deciOut,
                                    deciInSig then deciOut, deciInSigBool then deciOut, deciInBool then deciOut);

    //deciCom::Output::Value;
    assertType2(bool, deciOutVal == deciOutVal, deciOutVal != deciOutVal);
    assertType2(bool, deciOutVal == 0, 0 != deciOutVal);
    assertType2(bool, deciOutVal == input, input != deciOutVal);
    assertType(deciComData::Output::EachType, each += deciOutVal);
    assertType(deciComData::Output::AllType, all += deciOutVal);
    assertType(deciComData::Output, coal += deciOutVal);

    //deciCom::Output::Type;
    assertType2(bool, deciOutType == deciOutType, deciOutType != deciOutType);
    assertType2(deciComData::Output, deciOutType += 0, deciOutType += input);
    assertType(deciComData::Output, deciOutType += deciOutVal);

    //deciCom::Output::AllType;
    assertType2(bool, deciOutAll == deciOutAll, deciOutAll != deciOutAll);
    assertType2(bool, deciOutAll == deciOut,    deciOut != deciOutAll);
    assertType(deciCom<color::rg>, deciIn then deciOutAll);
    assertType5(deciCom<color::rg>, deciInAll then deciOutAll, deciInAny then deciOutAll, /*deciInEach then deciOutAll,*/
                                    deciInSig then deciOutAll, deciInSigBool then deciOutAll, deciInBool then deciOutAll);
    assertInvalid(deciInEach, temp then deciOutAll);

    //deciCom::Output::EachType;
    assertType2(bool, deciOutEach == deciOutEach, deciOutEach != deciOutEach);
    assertType2(bool, deciOutEach == deciOut,    deciOutEach != deciOutEach);
    assertType(deciCom<color::rg>, deciIn then deciOutEach);
    assertType2(deciCom<color::rg>, /*deciInAll then deciOutEach, deciInAny then deciOutEach,*/ deciInEach then deciOutEach,
                                    /*deciInSig then deciOutEach, deciInSigBool then deciOutEach,*/ deciInBool then deciOutEach);
    assertInvalid(deciInAll, temp then deciOutEach);
    assertInvalid(deciInAny, temp then deciOutEach);
    assertInvalid(deciInSig, temp then deciOutEach);
    assertInvalid(deciInSigBool, temp then deciOutEach);

    //ariCom;
    assertType2(bool, ari == ari, ari != ari);
    // ariCom::Mode
    assertType2(bool, ariMode == ariMode, ariMode != ariMode); // TODO(Allaizn): add syntax for variable decider operator
    //ariCom::Mode::Description;
    assertType2(bool, ariModeDesc == ariModeDesc, ariModeDesc != ariModeDesc);

    //ariCom::Input;
    assertType2(bool, ariIn == ariIn, ariIn != ariIn);
    assertType2(ariCom<color::rg>, ariIn on each, ariIn on coal);
    //ariCom::Input::Left;
    assertType2(bool, ariInLeft == ariInLeft, ariInLeft != ariInLeft);
    assertType4(bool, ariInLeft == 0,    ariInLeft != 0,    0    == ariInLeft, 0    != ariInLeft);
    assertType4(bool, ariInLeft == coal, ariInLeft != coal, coal == ariInLeft, coal != ariInLeft);
    assertType4(bool, ariInLeft == each, ariInLeft != each, each == ariInLeft, each != ariInLeft);
    assertType4(bool, ariInLeft == card, ariInLeft != card, card == ariInLeft, card != ariInLeft);

    assertType11(ariComData::Input, ariInLeft + 0,     ariInLeft - 0,     ariInLeft * 0,      ariInLeft / 0,     ariInLeft % 0,    ariInLeft ^ 0,
                                    ariInLeft >> 0,    ariInLeft << 0,    ariInLeft AND 0,    ariInLeft OR 0,    ariInLeft XOR 0);
    assertType11(ariComData::Input, ariInLeft + coal,  ariInLeft - coal,  ariInLeft * coal,   ariInLeft / coal,  ariInLeft % coal, ariInLeft ^ coal,
                                    ariInLeft >> coal, ariInLeft << coal, ariInLeft AND coal, ariInLeft OR coal, ariInLeft XOR coal);

    //ariCom::Input::Right;
    assertType2(bool, ariInRight == ariInRight, ariInRight != ariInRight);
    assertType4(bool, ariInRight == 0,    ariInRight != 0,    0    == ariInRight, 0    != ariInRight);
    assertType2(bool, ariInRight == coal, ariInRight != coal);
    static_assert(std::is_convertible_v<decltype(coal == ariInRight), bool>);
    static_assert(std::is_convertible_v<decltype(coal != ariInRight), bool>);
    assertInvalid11(any, temp +  ariInRight, temp -  ariInRight, temp  *  ariInRight, temp /  ariInRight, temp  %  ariInRight, temp ^ ariInRight,
                         temp >> ariInRight, temp << ariInRight, temp AND ariInRight, temp OR ariInRight, temp XOR ariInRight);
    assertInvalid11(all, temp +  ariInRight, temp -  ariInRight, temp  *  ariInRight, temp /  ariInRight, temp  %  ariInRight, temp ^ ariInRight, 
                         temp >> ariInRight, temp << ariInRight, temp AND ariInRight, temp OR ariInRight, temp XOR ariInRight);
    assertType11(ariComData::Input, each +  ariInRight, each -  ariInRight, each  *  ariInRight, each /  ariInRight, each  %  ariInRight, each ^ ariInRight, 
                                    each >> ariInRight, each << ariInRight, each AND ariInRight, each OR ariInRight, each XOR ariInRight);
    assertType11(ariComData::Input, card +  ariInRight, card -  ariInRight, card  *  ariInRight, card /  ariInRight, card  %  ariInRight, card ^ ariInRight, 
                                    card >> ariInRight, card << ariInRight, card AND ariInRight, card OR ariInRight, card XOR ariInRight);
    assertType11(ariComData::Input, ariInLeft +  ariInRight, ariInLeft -  ariInRight, ariInLeft  *  ariInRight, ariInLeft /  ariInRight, ariInLeft  %  ariInRight, ariInLeft ^ ariInRight,
                                    ariInLeft >> ariInRight, ariInLeft << ariInRight, ariInLeft AND ariInRight, ariInLeft OR ariInRight, ariInLeft XOR ariInRight);

    //ariCom::Input::IntSignalType;
    assertType2(bool, ariInNonEach == ariInNonEach, ariInNonEach != ariInNonEach);
    assertType4(bool, ariInNonEach == ariIn, ariInNonEach != ariIn, ariIn == ariInNonEach, ariIn != ariInNonEach);

    assertType(ariCom<color::rg>, ariInNonEach on coal);
    //assertInvalid(ariInNonEach on each);

    //ariCom::Output;
    assertType2(bool, ariOut == ariOut, ariOut != ariOut);
    assertType4(bool, ariOut == coal, ariOut != coal, coal == ariOut, coal != ariOut);
    assertType4(bool, ariOut == each, ariOut != each, each == ariOut, each != ariOut);
    assertType4(bool, ariOut == card, ariOut != card, card == ariOut, card != ariOut);

    assertType2(ariCom<color::rg>, ariIn on ariOut, ariIn on ariOut);
    assertType2(ariCom<color::rg>, ariInNonEach on ariOut, ariInNonEach on ariOut);

    // left = deciInLeft, any, all, each, card, coal
    // right = deciInRight, coal, 0
    assertType3(deciComData::Input,             deciInLeft <deciMode> deciInRight, deciInLeft <deciMode> coal, deciInLeft <deciMode> 0);
    assertType3(deciComData::Input::AnyType,    any        <deciMode> deciInRight, any        <deciMode> coal, any        <deciMode> 0);
    assertType3(deciComData::Input::AllType,    all        <deciMode> deciInRight, all        <deciMode> coal, all        <deciMode> 0);
    assertType3(deciComData::Input::EachType,   each       <deciMode> deciInRight, each       <deciMode> coal, each       <deciMode> 0);
    assertType3(deciComData::Input,             card       <deciMode> deciInRight, card       <deciMode> coal, card       <deciMode> 0);
    assertType3(deciComData::Input::SignalType, coal       <deciMode> deciInRight, coal       <deciMode> coal, coal       <deciMode> 0);


    // left = ariInLeft, 0, each, card, coal
    // right = ariInRight, coal, 0
    assertType3(ariComData::Input,                ariInLeft <ariMode> ariInRight, ariInLeft <ariMode> coal, ariInLeft <ariMode> 0);
    assertType3(ariComData::Input::IntSignalType, 0         <ariMode> ariInRight, 0         <ariMode> coal, 0         <ariMode> 0);
    assertType3(ariComData::Input,                each      <ariMode> ariInRight, each      <ariMode> coal, each      <ariMode> 0);
    assertType3(ariComData::Input,                card      <ariMode> ariInRight, card      <ariMode> coal, card      <ariMode> 0);
    assertType3(ariComData::Input::IntSignalType, coal      <ariMode> ariInRight, coal      <ariMode> coal, coal      <ariMode> 0);
  }
}

#endif

struct Entity {
  size_t networkSourceIndex = -1;
  size_t entity_number = -1;
  //std::string name; - decide based on networkSourceIndex
  std::tuple<float, float> position;
  uint32_t direction = 0; // not written if 0
  std::vector<std::tuple<size_t, int32_t>> rConnections; // second is only written for ari/deci, first is index into entites vector
  std::vector<std::tuple<size_t, int32_t>> gConnections; // second is only written for ari/deci, first is index into entites vector
  std::optional<std::vector<std::tuple<size_t, int32_t>>> r2Connections;
  std::optional<std::vector<std::tuple<size_t, int32_t>>> g2Connections;

  std::vector<std::tuple<size_t, int32_t>>& operator()(color c, int i = 1)
  {
    if (c == color::r)
      return i == 1 ? rConnections : r2Connections.value();
    else
      return i == 1 ? gConnections : g2Connections.value();
  }
};

std::string encode64(const std::string& data)
{
  uint32_t bits = 0;
  uint32_t countOfBitsUsed = 0;

  std::string result = "0";
  const std::array<const char, 64> base64Chars =
  {
    'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
    'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f',
    'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
    'w', 'x', 'y', 'z', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '+', '/'
  };
  for (uint8_t c : data)
  {
    bits = (bits << 8) | c;
    countOfBitsUsed += 8;

    if (countOfBitsUsed >= 12)
    {
      result.append(1, base64Chars[bits >> (countOfBitsUsed -= 6)]);
      bits &= (1 << countOfBitsUsed) - 1;
      result.append(1, base64Chars[bits >> (countOfBitsUsed -= 6)]);
      bits &= (1 << countOfBitsUsed) - 1;
    }
    else if (countOfBitsUsed >= 6)
    {
      result.append(1, base64Chars[bits >> (countOfBitsUsed -= 6)]);
      bits &= (1 << countOfBitsUsed) - 1;
    }
  }
  if (countOfBitsUsed != 0)
  {
    result.append(1, base64Chars[(bits << 6) >> countOfBitsUsed]);
    result.append(3 - countOfBitsUsed / 2, '=');
  }
  return result;
}

std::string compress(const std::string& data)
{
  auto requiredSize = compressBound(static_cast<uLong>(data.size()));
  std::string result;
  result.resize(requiredSize);

  auto code = compress2(reinterpret_cast<Bytef*>(&result[0]), &requiredSize, reinterpret_cast<const Bytef*>(data.data()), static_cast<uLong>(data.size()), 9);
  switch (code)
  {
  case Z_OK: break;
  case Z_MEM_ERROR: throw std::runtime_error("Compression failed: not enough memory.");
  case Z_BUF_ERROR: throw std::runtime_error("Compression failed: not enough room in the output buffer.");
  case Z_STREAM_ERROR: throw std::runtime_error("Invalid compression level: 9.");
  default:
    throw std::runtime_error("Compression failed: unknown error.");
  }

  result.resize(requiredSize);
  return result;
}

std::string stringify(std::vector<Entity>& ents)
{
  std::ostringstream output;
  output << "{\"blueprint\":{\"icons\":[{\"signal\":{\"type\":\"item\",\"name\":\"decider-combinator\"},\"index\":1}],\"entities\":[";
  for (size_t i = 0; i < ents.size(); i++)
  {
    Entity& e = ents[i];
    output << "{\"entity_number\":" << (i + 1) << ",\"position\":{\"x\":" << std::get<0>(e.position) << ",\"y\":" << -std::get<1>(e.position) << "},";
    if (e.direction != 0)
      output << "\"direction\":" << e.direction << ",";
    output << "\"name\":\"";
    if (e.networkSourceIndex == -1)
      output << "medium-electric-pole\",";
    else
      std::visit(overload(
        [&output](conComData const& c) -> void {
          output << "constant-combinator\",\"control_behavior\":{\"filters\":[";
          bool first = true;
          for (size_t j = 0; j < c.size(); j++)
            if (c[j].has_value())
            {
              if (!first) output << ","; else first = false;
              output << "{";
              c[j].value().toString(output);
              output << ",\"index\":" << (j + 1) << "}";
            }
          output << "]},";
        },
        [&output](deciComData const& d) -> void {
          output << "decider-combinator\",\"control_behavior\":{\"decider_conditions\":{\"first_signal\":{";
          std::visit(overload(
            [&output](auto const& s) { s.toString(output); } // templates to all, each, any & signal
          ), d.left);
          output << "},";
          std::visit(overload(
            [&output](int32_t const& i) { output << "\"constant\":" << i; },
            [&output](signal const& s) { output << "\"second_signal\":{"; s.toString(output); output << "}"; }
          ), d.right);
          output << ",\"comparator\":\"" << d.mode.description->gameSyntax << "\",\"output_signal\":{";
          std::visit(overload(
            [&output](auto const& s) { s.toString(output); } // templates to all, each & signal
          ), d.output);
          output << "},\"copy_count_from_input\":";
          output << (d.value.has_value() ? "false}}," : "true}},");
        },
          [&output](ariComData const& a) -> void {
          output << "arithmetic-combinator\",\"control_behavior\":{\"arithmetic_conditions\":{";
          std::visit(overload(
            [&output](int32_t const& i) { output << "\"first_constant\":" << i << ","; },
            [&output](auto const& s) { output << "\"first_signal\":{"; s.toString(output); output << "},"; } // templates to each & signal
          ), a.left);
          std::visit(overload(
            [&output](int32_t const& i) { output << "\"second_constant\":" << i << ","; },
            [&output](signal const& s) { output << "\"second_signal\":{"; s.toString(output); output << "},"; }
          ), a.right);
          output << "\"operation\":\"" << a.mode.description->gameSyntax << "\",\"output_signal\":{";
          std::visit(overload(
            [&output](auto const& s) { s.toString(output); output << "}}},"; } // templates to each & signal
          ), a.output.value);
        }
        ), sources[e.networkSourceIndex].combinator.value());
    output << "\"connections\":{\"1\":{";
    if (e.rConnections.size() != 0)
    {
      output << "\"red\":[";
      for (size_t j = 0; j < e.rConnections.size(); j++)
      {
        const auto [first, second] = e.rConnections[j];
        output << "{\"entity_id\":" << first;
        if (second != 0)
          output << ",\"circuit_id\":" << second;
        output << "}";
        if (j != e.rConnections.size() - 1)
          output << ",";
      }
      output << "]";
    }
    if (e.gConnections.size() != 0)
    {
      if (e.rConnections.size() != 0)
        output << ",";
      output << "\"green\":[";
      for (size_t j = 0; j < e.gConnections.size(); j++)
      {
        const auto [first, second] = e.gConnections[j];
        output << "{\"entity_id\":" << first;
        if (second != 0)
          output << ",\"circuit_id\":" << second;
        output << "}";
        if (j != e.gConnections.size() - 1)
          output << ",";
      }
      output << "]";
    }
    if (!e.r2Connections.has_value())
      output << "}";
    else
    {
      output << "},\"2\": {";
      if (e.r2Connections.value().size() != 0)
      {
        output << "\"red\":[";
        for (size_t j = 0; j < e.r2Connections.value().size(); j++)
        {
          const auto [first, second] = e.r2Connections.value()[j];
          output << "{\"entity_id\":" << first;
          if (second != 0)
            output << ",\"circuit_id\":" << second;
          output << "}";
          if (j != e.r2Connections.value().size() - 1)
            output << ",";
        }
        output << "]";
      }
      if (e.g2Connections.value().size() != 0)
      {
        if (e.g2Connections.value().size() != 0)
          output << ",";
        output << "\"green\":[";
        for (size_t j = 0; j < e.g2Connections.value().size(); j++)
        {
          const auto [first, second] = e.g2Connections.value()[j];
          output << "{\"entity_id\":" << first;
          if (second != 0)
            output << ",\"circuit_id\":" << second;
          output << "}";
          if (j != e.g2Connections.value().size() - 1)
            output << ",";
        }
        output << "]";
      }
      output << "}";
    }
    if (i != ents.size() - 1)
      output << "}},";
    else
      output << "}}";
  }
  output << "],\"item\":\"blueprint\",\"version\":73018245120}}";
  return encode64(compress(output.str()));
}
size_t poleAt(uint64_t const& x, uint64_t const& y, std::vector<Entity>& entities, std::vector<std::vector<size_t>>& xyToPole)
{
  size_t result;
  if (xyToPole.size() > x && xyToPole[x].size() > y && (result = xyToPole[x][y]) != -1)
    return result;
  else
  {
    entities.emplace_back(Entity());
    Entity& pole = entities.back();
    pole.position = std::make_tuple(x, y);
    pole.entity_number = entities.size() - 1;
    uint64_t maxX = std::max<uint64_t>(xyToPole.size(), x);
    uint64_t maxY = xyToPole.size() == 0 ? y : std::max<uint64_t>(xyToPole[0].size(), y);
    for (size_t px = 0; px <= maxX; px++)
    {
      if (xyToPole.size() <= maxX)
        xyToPole.emplace_back(std::vector<size_t>());
      for (size_t py = xyToPole[px].size(); py <= maxY; py++)
        xyToPole[px].emplace_back(-1);
    }
    return xyToPole[x][y] = pole.entity_number;
  }
}
void connectToPoleAt(uint64_t const& x, uint64_t const& y, color const& c, int const& connection, size_t const& e, std::vector<Entity>& entities, std::vector<std::vector<size_t>>& xyToPole)
{
  uint64_t y0 = std::min<int64_t>(y, connection == 1 ? 8 : 9);
  size_t pole = poleAt(x, y0, entities, xyToPole);
  entities[e](c, connection).push_back(std::make_tuple(pole + 1, 0));
  entities[pole](c).push_back(std::make_tuple(e + 1, connection));
  size_t lastPole = pole;
  if (y0 < y)
  {
    for (y0 = y0 + 9; y0 < y; y0 += 9)
    {
      pole = poleAt(x, y0, entities, xyToPole);
      entities[lastPole](c).push_back(std::make_tuple(pole + 1, 0));
      entities[pole](c).push_back(std::make_tuple(lastPole + 1, 0));
      lastPole = pole;
    }
    pole = poleAt(x, y, entities, xyToPole);
    entities[lastPole](c).push_back(std::make_tuple(pole + 1, 0));
    entities[pole](c).push_back(std::make_tuple(lastPole + 1, 0));
  }
}

std::string compile()
{
  std::vector<Entity> entities;
  std::vector<size_t> sourceToEntityMapping;
  for (size_t i = 0; i < sources.size(); i++)
    if (sources[i].combinator.has_value())
    {
      Entity next;
      next.networkSourceIndex = i;
      next.entity_number = entities.size();
      sourceToEntityMapping.push_back(next.entity_number);
      std::visit(overload(
        [&next](conComData const&) -> void { next.position = std::make_tuple(next.entity_number, 1); },
        [&next](auto const&) -> void
        {
          next.position = std::make_tuple(next.entity_number, 0.5f);
          next.r2Connections = std::vector<std::tuple<size_t, int32_t>>();
          next.g2Connections = std::vector<std::tuple<size_t, int32_t>>();
        }
      ), sources[i].combinator.value());
      next.direction = 4; // this makes it less likely for wires to visually cross
      entities.emplace_back(next);
    }
    else
      sourceToEntityMapping.push_back(-1);
  std::vector<uint64_t> networkToY;
  int redNetworks = 0;
  int greenNetworks = 0;
  for (size_t i = 0; i < networks.size(); i++)
  {
    int raw = (networks[i].c == color::r ? redNetworks : greenNetworks)++;
    networkToY.push_back(((int64_t)raw - 1) / 7 * 9 + ((int64_t)raw - 1) % 7 + 3);
  }
  std::vector<std::vector<size_t>> xyToPole;
  for (size_t i = 0; i < networks.size(); i++)
  {
    for (size_t j = 0; j < networks[i].sources.size(); j++)
    {
      size_t e = sourceToEntityMapping[networks[i].sources[j]];
      bool constComb = !sources[entities[e].networkSourceIndex].combinatorInput.has_value();
      uint64_t x = sourceToEntityMapping[networks[i].sources[j]];
      uint64_t y = networkToY[i];
      connectToPoleAt(x, y, networks[i].c, constComb ? 1 : 2, e, entities, xyToPole);
      if (!constComb)
      {
        size_t sr, sg;
        std::tie(sr, sg) = std::visit(overload(
          [](wire<color::r> const& r) { return std::make_tuple(r.source.source, SIZE_MAX); },
          [](wire<color::g> const& g) { return std::make_tuple(SIZE_MAX, g.source.source); },
          [](wire<color::rg> const& rg) { return std::make_tuple(rg.r.source.source, rg.g.source.source); }),
          sources[networks[i].sources[j]].combinatorInput.value());
        if (sr != -1)
        {
          assert(networks[networkLookup[sr]].c == color::r);
          y = networkToY[networkLookup[sr]];
          connectToPoleAt(x, y, color::r, 1, e, entities, xyToPole);
        }
        if (sg != -1)
        {
          assert(networks[networkLookup[sg]].c == color::g);
          y = networkToY[networkLookup[sg]];
          connectToPoleAt(x, y, color::g, 1, e, entities, xyToPole);
        }
      }
    }
  }
  for (size_t i = 0; i < networks.size(); i++)
  {
    uint64_t y = networkToY[i];
    size_t current = -1;
    color c = networks[i].c;
    for (size_t x = 0; x < xyToPole.size(); x++)
      if (xyToPole[x][y] != -1 && entities[xyToPole[x][y]](c).size() != 0)
      {
        current = x;
        break;
      }
    assert(current != -1);
    for (size_t x = current + 1; x < xyToPole.size(); x++)
      if (xyToPole[x][y] != -1 && entities[xyToPole[x][y]](c).size() != 0)
      {
        for (size_t tx = current + 9; tx < x; tx += 9)
        {
          size_t pole = poleAt(tx, y, entities, xyToPole);
          entities[xyToPole[current][y]](networks[i].c).push_back(std::make_tuple(xyToPole[tx][y] + 1, 0));
          entities[xyToPole[tx][y]](networks[i].c).push_back(std::make_tuple(xyToPole[current][y] + 1, 0));
          current = tx;
        }
        entities[xyToPole[current][y]](networks[i].c).push_back(std::make_tuple(xyToPole[x][y] + 1, 0));
        entities[xyToPole[x][y]](networks[i].c).push_back(std::make_tuple(xyToPole[current][y] + 1, 0));
        current = x;
      }
  }

  return stringify(entities);
}