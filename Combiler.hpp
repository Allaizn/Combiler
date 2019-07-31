#define COMBILER_IMPLEMENTATION

#ifndef COMBILER_HEADER
#define COMBILER_HEADER
#include <variant>
#include <optional>
#include <vector>
#include <array>


struct All;
struct Each;
struct signal;
struct wildCard;
struct network;
template<class T>
struct pointer
{
  size_t index;

  pointer(std::nullptr_t = nullptr) : index(-1) {};
  pointer(size_t const& i) : index(i) {};
  T* operator->() const;
  operator T* () const { return this->operator->(); }
};

#define EqualityOperatorsDecl(className)   \
bool operator==(className const& o) const; \
bool operator!=(className const& o) const

enum class color
{
  r, // red
  g, // green
  rg // red & green
};
template <color = color::rg> struct wire;
template <color = color::rg> union deciCom;
template <color = color::rg> union ariCom;

template <color c = color::rg>
struct connector
{
  pointer<network> source;

  EqualityOperatorsDecl(connector);
private:
  connector(pointer<network> const& source) : source(source) {}
  network* operator->() const { return this->source; }
  network& network() const { return *this->source; }
  friend ::network;
  friend wire<c>;
  friend connector<c> operator+=(connector<c> const&, connector<c> const&);
};
template<>
struct connector<color::rg>
{
  using R = connector<color::r>;
  using G = connector<color::g>;
  using RG = connector<color::rg>;
  connector<color::r> r;
  connector<color::g> g;

  EqualityOperatorsDecl(connector);
};
connector(...)->connector<color::rg>;

//template<color c1, color c2> connector<(c1 == c2 ? c1 : color::rg)> operator+=(connector<c1> const&, connector<c2> const&); // can't use this since conCom won't convert to connector due to ADL
connector<color::rg> operator+=(connector<color::rg> const&, connector<color::r>  const&);
connector<color::rg> operator+=(connector<color::rg> const&, connector<color::g>  const&);
connector<color::rg> operator+=(connector<color::rg> const&, connector<color::rg> const&);
connector<color::r>  operator+=(connector<color::r>  const&, connector<color::r>  const&);
connector<color::rg> operator+=(connector<color::r>  const&, connector<color::g>  const&);
connector<color::rg> operator+=(connector<color::r>  const&, connector<color::rg> const&);
connector<color::rg> operator+=(connector<color::g>  const&, connector<color::r>  const&);
connector<color::g>  operator+=(connector<color::g>  const&, connector<color::g>  const&);
connector<color::rg> operator+=(connector<color::g>  const&, connector<color::rg> const&);

template<color c> connector<c> operator> (connector<color::rg> const&, deciCom<c> const&); // only valid if red and green parts behave identically
template<color c> connector<c> operator> (connector<color::rg> const&, ariCom<c> const&); // only valid if red and green parts behave identically

template <color c>
struct wire
{
  connector<c> source;

  wire(connector<c> const& source);
  EqualityOperatorsDecl(wire);
  static wire loop();
  wire<c> operator<<=(connector<c> const&) const;
  void markAsOutput() const;
private:
  network* operator->() const { return source.operator->(); }
  network& network() const { return *this->operator->(); }
  friend wire<color::rg>;
};
template<>
struct wire<color::rg>
{
  using R = wire<color::r>;
  using G = wire<color::g>;
  using RG = wire<color::rg>;
  wire<color::r> r;
  wire<color::g> g;

  wire(wire<color::r> const& r, wire<color::g> const& g) : r(r), g(g) {}
  wire(connector<color::rg> const& rg) : r(rg.r), g(rg.g) {}
  EqualityOperatorsDecl(wire);
  static wire loop() { return wire{ R::loop(), G::loop() }; }
  wire<color::rg> operator<<=(connector<color::rg> const&) const;
  void markAsOutput() const { this->r.markAsOutput(); this->g.markAsOutput(); }
};
wire(...)->wire<color::rg>;

template<color c> connector<c> operator>(wire<color::r> const&, deciCom<c> const&);
template<color c> connector<c> operator>(wire<color::g> const&, deciCom<c> const&);
template<color c> connector<c> operator>(wire<color::r> const&, ariCom<c> const&);
template<color c> connector<c> operator>(wire<color::g> const&, ariCom<c> const&);
template<color c> connector<c> operator>>(wire<color::rg> const&, deciCom<c> const&);
template<color c> connector<c> operator>>(wire<color::rg> const&, ariCom<c> const&);
wire<color::rg> operator+(wire<color::r> const& r, wire<color::g> const& g) { return wire<color::rg>(r, g); }
wire<color::rg> operator+(wire<color::g> const& g, wire<color::r> const& r) { return wire<color::rg>(r, g); }



#define deciComDataInputLeft   std::variant<Any, All, Each, signal>
#define deciComDataInputRight  std::variant<int32_t, signal>
#define deciComDataOutputType  std::variant<All, Each, signal>
#define deciComDataOutputValue std::optional<int32_t>

#define ariComDataInputLeft    std::variant<int32_t, Each, signal>
#define ariComDataInputRight   std::variant<signal, int32_t>
#define ariComDataOutput       std::variant<Each, signal>

struct Any
{
  constexpr bool operator==(Any const&) const { return true; }
  constexpr bool operator!=(Any const&) const { return false; }
  constexpr bool operator==(All const&) const { return false; }
  constexpr bool operator!=(All const&) const { return true; }
  constexpr bool operator==(Each const&) const { return false; }
  constexpr bool operator!=(Each const&) const { return true; }
  EqualityOperatorsDecl(wildCard);
  EqualityOperatorsDecl(deciComDataInputLeft);
} constexpr any;
struct All
{
  constexpr bool operator==(Any const&) const { return false; }
  constexpr bool operator!=(Any const&) const { return true; }
  constexpr bool operator==(All const&) const { return true; }
  constexpr bool operator!=(All const&) const { return false; }
  constexpr bool operator==(Each const&) const { return false; }
  constexpr bool operator!=(Each const&) const { return true; }
  EqualityOperatorsDecl(wildCard);
  EqualityOperatorsDecl(deciComDataInputLeft);
  EqualityOperatorsDecl(deciComDataOutputType);
} constexpr all;
struct Each
{
  constexpr bool operator==(Any const&) const { return false; }
  constexpr bool operator!=(Any const&) const { return true; }
  constexpr bool operator==(All const&) const { return false; }
  constexpr bool operator!=(All const&) const { return true; }
  constexpr bool operator==(Each const&) const { return true; }
  constexpr bool operator!=(Each const&) const { return false; }
  EqualityOperatorsDecl(wildCard);
  EqualityOperatorsDecl(deciComDataInputLeft);
  EqualityOperatorsDecl(deciComDataOutputType);
  EqualityOperatorsDecl(ariComDataInputLeft);
  EqualityOperatorsDecl(ariComDataOutput);
} constexpr each;
struct wildCard
{
  std::variant<Any, All, Each> value;
  wildCard(std::variant<Any, All, Each> const& value) : value(value) {}
  explicit wildCard(Any const&) : value(any) {}
  explicit wildCard(All const&) : value(all) {}
  explicit wildCard(Each const&) : value(each) {}

  operator deciComDataInputLeft() const;
  operator deciComDataOutputType() const;
  operator ariComDataInputLeft() const;
  operator ariComDataOutput() const;
  EqualityOperatorsDecl(Any);
  EqualityOperatorsDecl(All);
  EqualityOperatorsDecl(Each);
  EqualityOperatorsDecl(wildCard);
  EqualityOperatorsDecl(deciComDataInputLeft);
  EqualityOperatorsDecl(deciComDataOutputType);
  EqualityOperatorsDecl(ariComDataInputLeft);
  EqualityOperatorsDecl(ariComDataOutput);
};

struct signal
{
  struct WithValue;
  struct Description
  {
    std::string codeSyntax;
    std::string gameSyntax;
    std::string type;
    size_t index;
    EqualityOperatorsDecl(Description);
  } const* description;

  signal(Description* const& desc) : description(desc) {}
  WithValue operator=(int32_t const& value) const;
  //EqualityOperatorsDecl(signal);                -- provided using Boolable types
  EqualityOperatorsDecl(deciComDataInputLeft);
  //EqualityOperatorsDecl(deciComDataInputRight); -- provided using Boolable types
  EqualityOperatorsDecl(deciComDataOutputType);
  EqualityOperatorsDecl(ariComDataInputLeft);
  EqualityOperatorsDecl(ariComDataInputRight);
  EqualityOperatorsDecl(ariComDataOutput);
};
struct signal::WithValue
{
  int32_t value;
  signal sig;
  EqualityOperatorsDecl(WithValue);
};



struct deciComData
{
  struct Mode
  {
    struct Helper;
    enum class Enum;
    struct Description
    {
      std::string name;
      std::string codeSyntax;
      std::string gameSyntax;
      size_t index;
      EqualityOperatorsDecl(Description);
    } const* description;

    EqualityOperatorsDecl(Mode);
    static std::vector<Mode> const modes;
  };
  struct Input
  {
    using Left = deciComDataInputLeft;
    using Right = deciComDataInputRight;
    struct Boolable;
#define op(Name, mod, type, inst)          \
    struct Name                            \
    {                                      \
      mod type left inst;                  \
      Right right;                         \
      Mode mode;                           \
      explicit operator Input() const;     \
      EqualityOperatorsDecl(Name);         \
      EqualityOperatorsDecl(Input);        \
    }
    op(Any,  static constexpr, ::Any, = any);
    op(All,  static constexpr, ::All, = all);
    op(Each, static constexpr, ::Each, = each);
    op(Signal, struct Boolable;, signal, );
#undef op
    struct Signal::Boolable : Signal
    {
      operator bool() const;
    };

    Left left;
    Right right;
    Mode mode;
    EqualityOperatorsDecl(Input);
    EqualityOperatorsDecl(Input::Any);
    EqualityOperatorsDecl(Input::All);
    EqualityOperatorsDecl(Input::Each);
    EqualityOperatorsDecl(Input::Signal);
  };
  struct Input::Boolable : Input
  {
    operator bool() const;
  };
  struct Output
  {
    using Type = deciComDataOutputType;
    using Value = deciComDataOutputValue;
#define op(Name, mod, type, inst)            \
    struct Name                              \
    {                                        \
      mod type output inst; \
      Value value;                           \
      explicit operator Output() const;      \
      EqualityOperatorsDecl(Name);           \
      EqualityOperatorsDecl(Output);         \
    }
    op(All,  static constexpr, ::All,  = all);
    op(Each, static constexpr, ::Each, = each);
    op(Signal, , signal, );
#undef op

    Type output;
    Value value;
    EqualityOperatorsDecl(Output);
    EqualityOperatorsDecl(Output::All);
    EqualityOperatorsDecl(Output::Each);
    EqualityOperatorsDecl(Output::Signal);
  };
  
  Input::Left left;
  Input::Right right;
  Mode mode;
  Output::Type output;
  Output::Value value;
  EqualityOperatorsDecl(deciComData);
};
struct deciComData::Mode::Helper
{
  Input::Left left;
  Mode mode;
  struct Signal
  {
    signal left;
    Mode mode;
  };
  struct Any
  {
    static constexpr ::Any left = any;
    Mode mode;
  };
  struct All
  {
    static constexpr ::All left = all;
    Mode mode;
  };
  struct Each
  {
    static constexpr ::Each left = each;
    Mode mode;
  };
};
struct ariComData
{
  struct Mode
  {
    struct Helper;
    enum class Enum;
    struct Description
    {
      std::string name;
      std::string codeSyntax;
      std::string gameSyntax;
      size_t index;
      EqualityOperatorsDecl(Description);
    } const* description;

    EqualityOperatorsDecl(Mode);
    static std::vector<Mode> const modes;
  };
  struct Input
  {
    using Left = ariComDataInputLeft;
    using Right = ariComDataInputRight;
#define op(Name, Type, modifier, defaultVal)\
    struct Name                        \
    {                                  \
      modifier Type left defaultVal;   \
      Right right;                     \
      Mode mode;                       \
                                       \
      explicit operator Input() const; \
      EqualityOperatorsDecl(Name);     \
      EqualityOperatorsDecl(Input);    \
    }
    op(Int, int32_t, , );
    op(Signal, signal, , );
    op(Each, ::Each, static constexpr, = each);
#undef op

    Left left;
    Right right;
    Mode mode;
    EqualityOperatorsDecl(Input);
    EqualityOperatorsDecl(Input::Int);
    EqualityOperatorsDecl(Input::Signal);
    EqualityOperatorsDecl(Input::Each);
  };
  using Output = std::variant<Each, signal>;

  Input::Left left;
  Input::Right right;
  Mode mode;
  Output output;
  EqualityOperatorsDecl(ariComData);
};
struct ariComData::Mode::Helper
{
  Input::Left left;
  Mode mode;
  struct Signal
  {
    signal left;
    Mode mode;
  };
  struct Int
  {
    int32_t left;
    Mode mode;
  };
  struct Each
  {
    static constexpr ::Each left = each;
    Mode mode;
  };
};
using conComData = std::array<std::optional<signal::WithValue>, 18>;

#undef deciComDataInputLeft
#undef deciComDataInputRight
#undef deciComDataOutputType
#undef deciComDataOutputValue

#undef ariComDataInputLeft
#undef ariComDataInputRight
#undef ariComDataOutput



template<color c>
union deciCom
{
  deciComData data;

  EqualityOperatorsDecl(deciCom);
};
template<>
union deciCom<color::rg>
{
  using R = deciCom<color::r>;
  using G = deciCom<color::g>;
  using RG = deciCom<color::rg>;
  deciComData data;
  deciCom<color::r> r;
  deciCom<color::g> g;
  EqualityOperatorsDecl(deciCom);
};
deciCom(...)->deciCom<color::rg>;

template<color c>
union ariCom
{
  ariComData data;
  EqualityOperatorsDecl(ariCom);
};
template<>
union ariCom<color::rg>
{
  using R = ariCom<color::r>;
  using G = ariCom<color::g>;
  using RG = ariCom<color::rg>;
  ariComData data;
  ariCom<color::r> r;
  ariCom<color::g> g;
  EqualityOperatorsDecl(ariCom);
};
ariCom(...)->ariCom<color::rg>;

template<color c = color::rg>
union conCom
{
  conComData data;

  operator connector<c>() const;
  operator wire<c>() const { return this->operator connector<c>(); }
};
template<>
union conCom<color::rg>
{
  using R = conCom<color::r>;
  using G = conCom<color::g>;
  using RG = conCom<color::rg>;
  conComData data;
  conCom<color::r> r;
  conCom<color::g> g;

  explicit conCom(conComData const& data) : data(data) {}
  conCom(std::optional<signal::WithValue> const& val0 = {}, std::optional<signal::WithValue> const& val1 = {}, std::optional<signal::WithValue> const& val2 = {}
    , std::optional<signal::WithValue> const& val3 = {}, std::optional<signal::WithValue> const& val4 = {}, std::optional<signal::WithValue> const& val5 = {}
    , std::optional<signal::WithValue> const& val6 = {}, std::optional<signal::WithValue> const& val7 = {}, std::optional<signal::WithValue> const& val8 = {}
    , std::optional<signal::WithValue> const& val9 = {}, std::optional<signal::WithValue> const& val10 = {}, std::optional<signal::WithValue> const& val11 = {}
    , std::optional<signal::WithValue> const& val12 = {}, std::optional<signal::WithValue> const& val13 = {}, std::optional<signal::WithValue> const& val14 = {}
    , std::optional<signal::WithValue> const& val15 = {}, std::optional<signal::WithValue> const& val16 = {}, std::optional<signal::WithValue> const& val17 = {})
    : data(conComData{ val0, val1, val2, val3, val4, val5, val6, val7, val8, val9, val10, val11, val12, val13, val14, val15, val16, val17 }) {}

  operator connector<color::rg>() const;
  operator wire<color::rg>() const { return this->operator connector<color::rg>(); }
};
conCom(...)->conCom<color::rg>;
template <color c> wire(conCom<c>)->wire<c>;
template <color c> connector(conCom<c>)->connector<c>;
#undef EqualityOperatorsDecl

#define deciOperations(equalArg, compArg)           \
  op(smaller,      <,  "<",               compArg)  \
  op(greater,      >,  ">",               compArg)  \
  op(equal,        ==, "=",               equalArg) \
  op(greaterEqual, >=, u8"≥"/*"\u2265"*/, compArg)  \
  op(smallerEqual, <=, u8"≤"/*"\u2264"*/, compArg)  \
  op(notEqual,     !=, u8"≠"/*"\u2260"*/, equalArg)

#define ariOperations           \
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
#define AND &
#define OR |
#define XOR ||

enum class ariComData::Mode::Enum {
#define op(opName, codeSyntax, gameSyntax) \
  opName,
  ariOperations
#undef op
};
enum class deciComData::Mode::Enum {
#define op(opName, codeSyntax, gameSyntax, arg) \
  opName,
  deciOperations(, )
#undef op
};


namespace deciComModes {
#define op(opName, codeSyntax, gameSyntax, arg) \
  extern deciComData::Mode const opName;
  deciOperations(, )
#undef op
}
namespace ariComModes
{
#define op(opName, codeSyntax, gameSyntax) \
  extern ariComData::Mode const opName;
  ariOperations
#undef op
}




#define allRightOps(codeSyntax, leftType, outputType, extension)                              \
outputType##extension operator codeSyntax(leftType const&, deciComData::Input::Right const&); \
outputType##extension operator codeSyntax(leftType const&, signal const&);                    \
outputType            operator codeSyntax(leftType const&, int32_t const&);

#define op(opName, codeSyntax, gameSyntax, extension)                                    \
allRightOps(codeSyntax, deciComData::Input::Left, deciComData::Input,         extension) \
allRightOps(codeSyntax, wildCard,                 deciComData::Input,                  ) \
allRightOps(codeSyntax, Any,                      deciComData::Input::Any,             ) \
allRightOps(codeSyntax, All,                      deciComData::Input::All,             ) \
allRightOps(codeSyntax, Each,                     deciComData::Input::Each,            ) \
allRightOps(codeSyntax, signal,                   deciComData::Input::Signal, extension)
deciOperations(::Boolable, )
#undef op
#undef allRightOps

deciComData::Output::Value constexpr input = std::nullopt;
deciComData::Output         operator+=(deciComData::Output::Type const&, deciComData::Output::Value const&);
deciComData::Output::Signal operator+=(signal const&,                    deciComData::Output::Value const&);
deciComData::Output::All    operator+=(All const&,                       deciComData::Output::Value const&);
deciComData::Output::Each   operator+=(Each const&,                      deciComData::Output::Value const&);


#define then >>=
#define op(leftT, rightT) \
deciCom<color::rg> operator then(deciComData::Input##leftT const&, deciComData::Output##rightT const&);
op(        ,)   op(        ,::All)   op(        ,::Each)   op(        ,::Signal)
op(::Signal,)   op(::Signal,::All) /*op(::Signal,::Each)*/ op(::Signal,::Signal)
op(::All   ,)   op(::All   ,::All) /*op(::All   ,::Each)*/ op(::All   ,::Signal)
op(::Any   ,)   op(::Any   ,::All) /*op(::Any   ,::Each)*/ op(::Any   ,::Signal)
op(::Each  ,) /*op(::Each  ,::All)*/ op(::Each  ,::Each)   op(::Each  ,::Signal)
#undef op




#define allRightOps(mode, ext, left, secOp)                                         \
ariComData::Input##ext operator mode(left const&, ariComData::Input::Right const&); \
ariComData::Input##ext operator mode(left const&, signal const&);                   \
secOp(mode, ext, left)
#define opEmpty(mode, ext, left)
#define opInt(mode, ext, left)                                                      \
ariComData::Input##ext operator mode(left const&, int32_t const&);

#define op(opName, codeSyntax, gameSyntax)                          \
allRightOps(codeSyntax, ,         ariComData::Input::Left, opInt)   \
allRightOps(codeSyntax, ::Int,    int32_t,                 opEmpty) \
allRightOps(codeSyntax, ::Signal, signal,                  opInt)   \
allRightOps(codeSyntax, ::Each,   Each,                    opInt)
ariOperations
#undef op
#undef opInt
#undef opEmpty
#undef allRightOps

#define on >>=
#define op(leftT, rightT) \
ariCom<color::rg> operator on(ariComData::Input##leftT const& left, rightT const& right);
op(        , ariComData::Output)   op(        , Each)   op(        , wildCard)   op(        , signal)
op(::Int   , ariComData::Output) /*op(::Int   , Each)   op(::Int   , wildCard)*/ op(::Int   , signal)
op(::Signal, ariComData::Output) /*op(::Signal, Each)   op(::Signal, wildCard)*/ op(::Signal, signal)
op(::Each  , ariComData::Output)   op(::Each  , Each)   op(::Each  , wildCard)   op(::Each  , signal)
#undef op


deciComData::Mode::Helper         operator<(deciComData::Input::Left const& left, deciComData::Mode const& right) { return { left, right }; }
deciComData::Mode::Helper::Any    operator<(Any const&,                           deciComData::Mode const& right) { return {       right }; }
deciComData::Mode::Helper::All    operator<(All const&,                           deciComData::Mode const& right) { return {       right }; }
deciComData::Mode::Helper::Each   operator<(Each const&,                          deciComData::Mode const& right) { return {       right }; }
deciComData::Mode::Helper::Signal operator<(signal const& left,                   deciComData::Mode const& right) { return { left, right }; }
deciComData::Mode::Helper         operator<(wildCard const& left,                 deciComData::Mode const& right) { return { left, right }; }

#undef autoOperators
#define autoOperators(rightType)                                                                                                                        \
deciComData::Input         operator>(deciComData::Mode::Helper const& left,         rightType const& right) { return { left.left, right, left.mode }; } \
deciComData::Input::Signal operator>(deciComData::Mode::Helper::Signal const& left, rightType const& right) { return { left.left, right, left.mode }; } \
deciComData::Input::Any    operator>(deciComData::Mode::Helper::Any const& left,    rightType const& right) { return {            right, left.mode }; } \
deciComData::Input::All    operator>(deciComData::Mode::Helper::All const& left,    rightType const& right) { return {            right, left.mode }; } \
deciComData::Input::Each   operator>(deciComData::Mode::Helper::Each const& left,   rightType const& right) { return {            right, left.mode }; }

autoOperators(deciComData::Input::Right)
autoOperators(signal)
autoOperators(int32_t)
#undef autoOperators


ariComData::Mode::Helper         operator<(ariComData::Input::Left const& left, ariComData::Mode const& right) { return { left, right }; }
ariComData::Mode::Helper::Int    operator<(int32_t const& left,                 ariComData::Mode const& right) { return { left, right }; }
ariComData::Mode::Helper::Each   operator<(Each const&,                         ariComData::Mode const& right) { return {       right }; }
ariComData::Mode::Helper::Signal operator<(signal const& left,                  ariComData::Mode const& right) { return { left, right }; }
ariComData::Mode::Helper         operator<(wildCard const& left,                ariComData::Mode const& right) { return { left, right }; }

#undef autoOperators
#define autoOperators(rightType)                                                                                                                      \
ariComData::Input         operator>(ariComData::Mode::Helper const& left,         rightType const& right) { return { left.left, right, left.mode }; } \
ariComData::Input::Signal operator>(ariComData::Mode::Helper::Signal const& left, rightType const& right) { return { left.left, right, left.mode }; } \
ariComData::Input::Int    operator>(ariComData::Mode::Helper::Int const& left,    rightType const& right) { return { left.left, right, left.mode }; } \
ariComData::Input::Each   operator>(ariComData::Mode::Helper::Each const& left,   rightType const& right) { return {            right, left.mode }; }

autoOperators(ariComData::Input::Right)
autoOperators(signal)
autoOperators(int32_t)
#undef autoOperators







#define EqualityOperatorsDecl1(leftT, rightT) \
bool operator==(leftT const&, rightT const&); \
bool operator!=(leftT const&, rightT const&)
#define EqualityOperatorsDecl(leftT, rightT) \
EqualityOperatorsDecl1(leftT, rightT);       \
EqualityOperatorsDecl1(rightT, leftT)


EqualityOperatorsDecl1(deciComData::Input::Left, Any);
EqualityOperatorsDecl1(deciComData::Input::Left, All);
EqualityOperatorsDecl1(deciComData::Input::Left, Each);
EqualityOperatorsDecl1(deciComData::Output::Type, All);
EqualityOperatorsDecl1(deciComData::Output::Type, Each);
EqualityOperatorsDecl1(ariComData::Input::Left, Each);
EqualityOperatorsDecl1(ariComData::Output, Each);

EqualityOperatorsDecl(deciComData::Input::Right, int32_t);
EqualityOperatorsDecl(ariComData::Input::Left, int32_t);
EqualityOperatorsDecl(ariComData::Input::Right, int32_t);

EqualityOperatorsDecl1(deciComData::Input::Right, signal);
EqualityOperatorsDecl1(deciComData::Output::Type, signal);
EqualityOperatorsDecl1(ariComData::Input::Left, signal);
EqualityOperatorsDecl1(ariComData::Input::Right, signal);
EqualityOperatorsDecl1(ariComData::Output, signal);

EqualityOperatorsDecl1(deciComData::Input::Left, wildCard);
EqualityOperatorsDecl1(deciComData::Output::Type, wildCard);
EqualityOperatorsDecl1(ariComData::Input::Left, wildCard);
EqualityOperatorsDecl1(ariComData::Output, wildCard);

EqualityOperatorsDecl1(deciComData::Input::Right, deciComData::Input::Left); // - deciCom input, other case handled by Input::Boolable
EqualityOperatorsDecl(deciComData::Input::Left, deciComData::Output::Type);
EqualityOperatorsDecl(deciComData::Input::Left, ariComData::Input::Left);
EqualityOperatorsDecl(deciComData::Input::Left, ariComData::Input::Right);
EqualityOperatorsDecl(deciComData::Input::Left, ariComData::Output);
EqualityOperatorsDecl(deciComData::Input::Right, deciComData::Output::Type);
EqualityOperatorsDecl(deciComData::Input::Right, ariComData::Input::Left);
EqualityOperatorsDecl(deciComData::Input::Right, ariComData::Input::Right);
EqualityOperatorsDecl(deciComData::Input::Right, ariComData::Output);
EqualityOperatorsDecl(deciComData::Output::Type, ariComData::Input::Left);
EqualityOperatorsDecl(deciComData::Output::Type, ariComData::Input::Right);
EqualityOperatorsDecl(deciComData::Output::Type, ariComData::Output);
EqualityOperatorsDecl(ariComData::Input::Left, ariComData::Input::Right);
EqualityOperatorsDecl(ariComData::Input::Left, ariComData::Output);
EqualityOperatorsDecl(ariComData::Input::Right, ariComData::Output);
#undef EqualityOperatorsDecl
#undef EqualityOperatorsDecl1


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
  op(repair_pack)          op(blueprint)             op(deconstruction_planner) op(upgrade_planner) op(blueprint_book)     op(copy_paste_tool)      op(cut_paste_tool)                                                                                                                                                           \
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
    size_t i = 0;
    auto toGameCode = [](std::string s){ std::replace(s.begin(), s.end(), '_', '-'); return s; };
    return std::vector<signal>{
#define op(name) \
    signal{new signal::Description{#name, toGameCode(#name), "item", i++}},
    operations
#undef op
  }; })();

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
    size_t i = 0;
    auto toGameCode = [](std::string s) { std::replace(s.begin(), s.end(), '_', '-'); return s; };
    return std::vector<signal>{
#define op(name) \
    signal{new signal::Description{#name, toGameCode(#name), "fluid", i++}},
      operations
#undef op
  }; })();

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
    size_t i = 0;
    auto toGameCode = [](std::string s) { std::replace(s.begin(), s.end(), '_', '-'); return s; };
    return std::vector<signal>{
#define op(name, score) \
    signal{new signal::Description{#score#name, "signal-"#name, "virtual", i++}},
      operations
#undef op
  }; })();

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

std::string compileFirstOrSimulate(uint16_t lengthOfValueHistory);
#ifndef COMBILER_IMPLEMENTATION
#undef ariOperations
#undef deciOperations
#endif
#endif
















#ifdef COMBILER_IMPLEMENTATION
#include <sstream>
#include <cassert>
#include "zlib.h"

template <class ...Fs>
struct overload : Fs... {
  overload(Fs const& ... fs) : Fs{ fs }...
  {}

  using Fs::operator()...;
};
signal::WithValue signal::operator=(int32_t const& value) const { return { value, *this }; }

wildCard::operator deciComData::Input::Left() const
{
  return std::visit(overload(
    [](Any  const&) -> deciComData::Input::Left { return any; },
    [](All  const&) -> deciComData::Input::Left { return all; },
    [](Each const&) -> deciComData::Input::Left { return each; }
  ), this->value);
}
wildCard::operator deciComData::Output::Type() const
{
  return std::visit(overload(
    [](Any  const&) -> deciComData::Output::Type { assert(false && "Decider output signal cannot be Any (which is currently stored in the wildCard)!"); return {}; },
    [](All  const&) -> deciComData::Output::Type { return all; },
    [](Each const&) -> deciComData::Output::Type { return each; }
  ), this->value);
}
wildCard::operator ariComData::Input::Left() const
{
  return std::visit(overload(
    [](Any  const&) -> ariComData::Input::Left { assert(false && "Arithmetic left input signal cannot be Any (which is currently stored in the wildCard)!"); return {}; },
    [](All  const&) -> ariComData::Input::Left { assert(false && "Arithmetic left input signal cannot be All (which is currently stored in the wildCard)!"); return {}; },
    [](Each const&) -> ariComData::Input::Left { return each; }
  ), this->value);
}
wildCard::operator ariComData::Output() const
{
  return std::visit(overload(
    [](Any  const&) -> ariComData::Output { assert(false && "Arithmetic output signal cannot be Any (which is currently stored in the wildCard)!"); return {}; },
    [](All  const&) -> ariComData::Output { assert(false && "Arithmetic output signal cannot be All (which is currently stored in the wildCard)!"); return {}; },
    [](Each const&) -> ariComData::Output { return each; }
  ), this->value);
}

deciComData::Input::Any   ::operator deciComData::Input() const { return { this->left, this->right, this->mode }; }
deciComData::Input::All   ::operator deciComData::Input() const { return { this->left, this->right, this->mode }; }
deciComData::Input::Each  ::operator deciComData::Input() const { return { this->left, this->right, this->mode }; }
deciComData::Input::Signal::operator deciComData::Input() const { return { this->left, this->right, this->mode }; }

deciComData::Output::All   ::operator deciComData::Output() const { return { this->output, this->value }; }
deciComData::Output::Each  ::operator deciComData::Output() const { return { this->output, this->value }; }
deciComData::Output::Signal::operator deciComData::Output() const { return { this->output, this->value }; }

ariComData::Input::Int   ::operator ariComData::Input() const { return { this->left, this->right, this->mode }; }
ariComData::Input::Each  ::operator ariComData::Input() const { return { this->left, this->right, this->mode }; }
ariComData::Input::Signal::operator ariComData::Input() const { return { this->left, this->right, this->mode }; }

std::vector<deciComData::Mode> const deciComData::Mode::modes = []()
{
  std::vector<Mode> result;
  size_t i = 0;
#define op(name, codeSyntax, gameSyntax, arg) \
  result.push_back(Mode{new deciComData::Mode::Description{#name, #codeSyntax, gameSyntax, i++}});
  deciOperations(, )
#undef op
    return result;
}();

std::vector<ariComData::Mode> const ariComData::Mode::modes = []()
{
  size_t i = 0;
  std::vector<ariComData::Mode> result = {
#define op(name, codeSyntax, gameSyntax) \
    ariComData::Mode{new ariComData::Mode::Description{#name, #codeSyntax, gameSyntax, i++}},  
  ariOperations
#undef op
  };
  return result;
}();

namespace deciComModes {
#define op(opName, codeSyntax, gameSyntax, arg) \
  deciComData::Mode const opName = deciComData::Mode::modes[static_cast<size_t>(deciComData::Mode::Enum::##opName)];
  deciOperations(, )
#undef op
}
namespace ariComModes
{
#define op(opName, codeSyntax, gameSyntax) \
  ariComData::Mode const opName = ariComData::Mode::modes[size_t(ariComData::Mode::Enum::##opName)];
  ariOperations
#undef op
}


#define allRightOps(codeSyntax, leftType, outputType, extension, ...)                                                 \
outputType##extension operator codeSyntax(leftType, deciComData::Input::Right const& right) { return {__VA_ARGS__}; } \
outputType##extension operator codeSyntax(leftType, signal const& right)                    { return {__VA_ARGS__}; } \
outputType            operator codeSyntax(leftType, int32_t const& right)                   { return {__VA_ARGS__}; }

#define op(opName, codeSyntax, gameSyntax, arg)                                                                                   \
allRightOps(codeSyntax, deciComData::Input::Left const& left, deciComData::Input,         arg, left, right, deciComModes::opName) \
allRightOps(codeSyntax, Any const&,                           deciComData::Input::Any,       ,       right, deciComModes::opName) \
allRightOps(codeSyntax, All const&,                           deciComData::Input::All,       ,       right, deciComModes::opName) \
allRightOps(codeSyntax, Each const&,                          deciComData::Input::Each,      ,       right, deciComModes::opName) \
allRightOps(codeSyntax, signal const& left,                   deciComData::Input::Signal, arg, left, right, deciComModes::opName) \
allRightOps(codeSyntax, wildCard const& left,                 deciComData::Input,            , left, right, deciComModes::opName)
deciOperations(::Boolable, )
#undef op
#undef allRightOps

deciComData::Output         operator+=(deciComData::Output::Type const& left, deciComData::Output::Value const& right) { return { left, right }; }
deciComData::Output::Signal operator+=(signal const& left,                    deciComData::Output::Value const& right) { return { left, right }; }
deciComData::Output::All    operator+=(All const&,                            deciComData::Output::Value const& right) { return { right }; }
deciComData::Output::Each   operator+=(Each const&,                           deciComData::Output::Value const& right) { return { right }; }

#define op(leftT, rightT) \
deciCom<color::rg> operator then(deciComData::Input##leftT const& left, deciComData::Output##rightT const& right) { return { left.left, left.right, left.mode, right.output, right.value }; }
op(        ,)   op(        ,::All)   op(        ,::Each)   op(        ,::Signal)
op(::Signal,)   op(::Signal,::All) /*op(::Signal,::Each)*/ op(::Signal,::Signal)
op(::All   ,)   op(::All   ,::All) /*op(::All   ,::Each)*/ op(::All   ,::Signal)
op(::Any   ,)   op(::Any   ,::All) /*op(::Any   ,::Each)*/ op(::Any   ,::Signal)
op(::Each  ,) /*op(::Each  ,::All)*/ op(::Each  ,::Each)   op(::Each  ,::Signal)
#undef op


#define allRightOps(mode, ext, leftT, secOp, ...)                                                                          \
ariComData::Input##ext operator mode(leftT const& left, ariComData::Input::Right const& right) { return { __VA_ARGS__ }; } \
ariComData::Input##ext operator mode(leftT const& left, signal const& right)                   { return { __VA_ARGS__ }; } \
secOp(mode, ext, leftT, __VA_ARGS__)
#define opEmpty(mode, ext, leftT, ...)
#define opInt(mode, ext, leftT, ...)                                                                                       \
ariComData::Input##ext operator mode(leftT const& left, int32_t const& right)                  { return { __VA_ARGS__}; }

#define op(opName, codeSyntax, gameSyntax)                                                            \
allRightOps(codeSyntax, ,         ariComData::Input::Left, opInt,   left, right, ariComModes::opName) \
allRightOps(codeSyntax, ::Int,    int32_t,                 opEmpty, left, right, ariComModes::opName) \
allRightOps(codeSyntax, ::Signal, signal,                  opInt,   left, right, ariComModes::opName) \
allRightOps(codeSyntax, ::Each,   Each,                    opInt,         right, ariComModes::opName)
ariOperations
#undef op
#undef opInt
#undef opEmpty
#undef allRightOps

#define op(leftT, rightT) \
ariCom<color::rg> operator on(ariComData::Input##leftT const& left, rightT const& right) { return { left.left, left.right, left.mode, right }; }
op(        , ariComData::Output)   op(        , Each)   op(        , wildCard)   op(        , signal)
op(::Int   , ariComData::Output) /*op(::Int   , Each)   op(::Int   , wildCard)*/ op(::Int   , signal)
op(::Signal, ariComData::Output) /*op(::Signal, Each)   op(::Signal, wildCard)*/ op(::Signal, signal)
op(::Each  , ariComData::Output)   op(::Each  , Each)   op(::Each  , wildCard)   op(::Each  , signal)
#undef op



deciComData::Input::Signal::Boolable::operator bool() const 
{
  return std::holds_alternative<signal>(this->right) && this->left.description == std::get<signal>(this->right).description; 
}
deciComData::Input::Boolable::operator bool() const 
{
  return std::holds_alternative<signal>(this->left) && std::holds_alternative<signal>(this->right) && std::get<signal>(this->left).description == std::get<signal>(this->right).description;
}

#undef ariOperations
#undef deciOperations

#define EXPAND(x) x
#define autoOperator1(in, out, f1)                 this->f1 in o.f1
#define autoOperator2(in, out, f1, f2)             this->f1 in o.f1 out autoOperator1(in, out, f2)
#define autoOperator3(in, out, f1, f2, f3)         this->f1 in o.f1 out autoOperator2(in, out, f2, f3)
#define autoOperator4(in, out, f1, f2, f3, f4)     this->f1 in o.f1 out autoOperator3(in, out, f2, f3, f4)
#define autoOperator5(in, out, f1, f2, f3, f4, f5) this->f1 in o.f1 out autoOperator4(in, out, f2, f3, f4, f5)
#define autoOperatorName(_1, _2, _3, _4, _5, Name, ...) Name
#define autoOperator(in, out, ...) \
EXPAND(autoOperatorName(__VA_ARGS__,autoOperator5,autoOperator4,autoOperator3,autoOperator2,autoOperator1)(in, out, __VA_ARGS__))


#define autoOperators(type, ...)                                                         \
bool type::operator==(type const& o) const { return autoOperator(==, &&, __VA_ARGS__); } \
bool type::operator!=(type const& o) const { return autoOperator(!=, ||, __VA_ARGS__); }

autoOperators(wildCard, value)
//autoOperators(signal, description)  -- provided using Boolable types
autoOperators(signal::Description, index)

autoOperators(deciComData::Mode, description)
autoOperators(deciComData::Mode::Description, index)
autoOperators(deciComData::Input, left, right, mode)
autoOperators(deciComData::Input::All, right, mode)
autoOperators(deciComData::Input::Any, right, mode)
autoOperators(deciComData::Input::Each, right, mode)
autoOperators(deciComData::Input::Signal, left, right, mode)
autoOperators(deciComData::Output, output, value)
autoOperators(deciComData::Output::All, value)
autoOperators(deciComData::Output::Each, value)
autoOperators(deciComData::Output::Signal, value)
autoOperators(deciComData, left, right, mode, output, value)

autoOperators(ariComData::Mode, description)
autoOperators(ariComData::Mode::Description, index)
autoOperators(ariComData::Input, left, right, mode)
autoOperators(ariComData::Input::Int, left, right, mode)
autoOperators(ariComData::Input::Signal, left, right, mode)
autoOperators(ariComData::Input::Each, right, mode)
autoOperators(ariComData, left, right, mode, output)




#undef autoOperators
#define autoOperators(leftType, rightType)                                                                     \
bool operator==(leftType const& left, rightType const& right) { return left == static_cast<leftType>(right); } \
bool operator!=(leftType const& left, rightType const& right) { return left != static_cast<leftType>(right); } \
bool operator==(rightType const& left, leftType const& right) { return static_cast<leftType>(left) == right; } \
bool operator!=(rightType const& left, leftType const& right) { return static_cast<leftType>(left) != right; }

autoOperators(deciComData::Input, deciComData::Input::Any)
autoOperators(deciComData::Input, deciComData::Input::All)
autoOperators(deciComData::Input, deciComData::Input::Each)
autoOperators(deciComData::Input, deciComData::Input::Signal)

autoOperators(deciComData::Output, deciComData::Output::All)
autoOperators(deciComData::Output, deciComData::Output::Each)
autoOperators(deciComData::Output, deciComData::Output::Signal)

autoOperators(ariComData::Input, ariComData::Input::Int)
autoOperators(ariComData::Input, ariComData::Input::Signal)
autoOperators(ariComData::Input, ariComData::Input::Each)
#undef autoOperators


#define EqualityOperator(leftT, card)                                                           \
bool operator==(leftT const& left, card const&) { return  std::holds_alternative<card>(left); } \
bool operator!=(leftT const& left, card const&) { return !std::holds_alternative<card>(left); } \
bool card::operator==(leftT const& left) const  { return  std::holds_alternative<card>(left); } \
bool card::operator!=(leftT const& left) const  { return !std::holds_alternative<card>(left); } 
EqualityOperator(deciComData::Input::Left, Any)
EqualityOperator(deciComData::Input::Left, All)
EqualityOperator(deciComData::Input::Left, Each)
EqualityOperator(deciComData::Output::Type, All)
EqualityOperator(deciComData::Output::Type, Each)
EqualityOperator(ariComData::Input::Left, Each)
EqualityOperator(ariComData::Output, Each)
#undef EqualityOperator

#define EqualityOperator(leftT, rightT)                                                                                                      \
bool operator==(leftT const& left, signal const& right) { return  std::holds_alternative<signal>(left) && std::get<signal>(left) == right; } \
bool operator!=(leftT const& left, signal const& right) { return !std::holds_alternative<signal>(left) || std::get<signal>(left) != right; } \
bool signal::operator==(rightT const& left) const { return  std::holds_alternative<signal>(left) && std::get<signal>(left) == *this; }       \
bool signal::operator!=(rightT const& left) const { return !std::holds_alternative<signal>(left) || std::get<signal>(left) != *this; }

// we need to special case deciCom input left/right since (left == signal) and (signal == right) get interpreted as deciCom inputs
EqualityOperator(deciComData::Input::Right, deciComData::Input::Left)
EqualityOperator(deciComData::Output::Type, deciComData::Output::Type)
EqualityOperator(ariComData::Input::Left, ariComData::Input::Left)
EqualityOperator(ariComData::Input::Right, ariComData::Input::Right)
EqualityOperator(ariComData::Output, ariComData::Output)



#define autoOperators(type, card)                                                                 \
bool type::operator==(card const&) const { return  std::holds_alternative<card>(this->value); }   \
bool type::operator!=(card const&) const { return !std::holds_alternative<card>(this->value); }   \
bool card::operator==(type const& out) const { return  std::holds_alternative<card>(out.value); } \
bool card::operator!=(type const& out) const { return !std::holds_alternative<card>(out.value); }

autoOperators(wildCard, Any)
autoOperators(wildCard, All)
autoOperators(wildCard, Each)
#undef autoOperators

#define autoOperators(type)                                                        \
bool wildCard::operator==(type const& right) const                                 \
{                                                                                  \
  return std::visit(overload(                                                      \
    [&](Any const&) { return std::holds_alternative<Any>(this->value); },          \
    [&](All const&) { return std::holds_alternative<All>(this->value); },          \
    [&](Each const&){ return std::holds_alternative<Each>(this->value); },         \
    [](auto const&) { return false; }                                              \
      ), right);                                                                   \
}                                                                                  \
bool wildCard::operator!=(type const& right) const { return !(*this == right); }   \
bool operator==(type const& left, wildCard const& right) { return right == left; } \
bool operator!=(type const& left, wildCard const& right) { return right != left; }

autoOperators(deciComData::Input::Left)
autoOperators(deciComData::Output::Type)
autoOperators(ariComData::Input::Left)
autoOperators(ariComData::Output)
#undef autoOperators

#define autoOperators(type)                                                                                                                     \
bool operator==(type const& left, int32_t const& right) { return  std::holds_alternative<int32_t>(left) && std::get<int32_t>(left) == right; }  \
bool operator!=(type const& left, int32_t const& right) { return !std::holds_alternative<int32_t>(left) || std::get<int32_t>(left) != right; }  \
bool operator==(int32_t const& left, type const& right) { return  std::holds_alternative<int32_t>(right) && std::get<int32_t>(right) == left; } \
bool operator!=(int32_t const& left, type const& right) { return !std::holds_alternative<int32_t>(right) || std::get<int32_t>(right) != left; }

autoOperators(deciComData::Input::Right)
autoOperators(ariComData::Input::Left)
autoOperators(ariComData::Input::Right)
#undef autoOperators

bool operator==(deciComData::Input::Right const& left, deciComData::Input::Left const& right) { return right == left; }
bool operator!=(deciComData::Input::Right const& left, deciComData::Input::Left const& right) { return right != left; }

#define op [](int32_t const&) -> bool { return false; },
#define autoOperators(leftT, rightT, falseT)                                                             \
bool operator==(leftT const& left, rightT const& right)                                                  \
{                                                                                                        \
  return std::visit(overload(falseT [right](auto const& left) -> bool { return left == right; }), left); \
}                                                                                                        \
bool operator!=(leftT const& left, rightT const& right) { return !(left == right); }                     \
bool operator==(rightT const& left, leftT const& right) { return   right == left;  }                     \
bool operator!=(rightT const& left, leftT const& right) { return !(right == left); }

autoOperators(ariComData::Input::Right, ariComData::Input::Left, )
autoOperators(ariComData::Input::Right, deciComData::Input::Right, )
autoOperators(ariComData::Output, deciComData::Input::Left, )
autoOperators(ariComData::Output, deciComData::Output::Type, )
autoOperators(deciComData::Input::Right, ariComData::Input::Left, )
autoOperators(deciComData::Output::Type, deciComData::Input::Left, )
autoOperators(deciComData::Input::Right, deciComData::Output::Type, op)
autoOperators(deciComData::Input::Right, ariComData::Output, op)
autoOperators(ariComData::Input::Left, deciComData::Input::Left, op)
autoOperators(ariComData::Input::Left, ariComData::Output, op)
autoOperators(ariComData::Input::Left, deciComData::Output::Type, op)
autoOperators(ariComData::Input::Right, deciComData::Output::Type, op)
autoOperators(ariComData::Input::Right, deciComData::Input::Left, op)
autoOperators(ariComData::Input::Right, ariComData::Output, op)
#undef op
#undef autoOperators

#define setFlag(var, flag) var = static_cast<decltype(var)>(var | flag)

struct entity;
struct network
{
  struct source
  {
    static std::vector<source> list;
    enum {
      none             = 0b0000,
      isConCom         = 0b0001,
      isDeciCom        = 0b0010,
      isAriCom         = 0b0100,
      isDeciOrAri      = 0b0110,
      isOutputRelevant = 0b1000
    } flags;
    pointer<entity> entity;
    union
    {
      conComData cCombinator = {};
      struct
      {
        union {
          deciComData dCombinator;
          ariComData aCombinator;
        };
        pointer<network> redInput;
        pointer<network> greenInput;
      };
    };

    source(conComData const& comb) : cCombinator(comb), flags(isConCom) {}
    source(std::variant<deciComData, ariComData> const& combinator
         , std::variant<wire<color::r>, wire<color::g>, wire<color::rg>> const& source)
    {
      std::visit(overload(
        [&](deciComData const& d) { this->dCombinator = d; this->flags = isDeciCom; },
        [&](ariComData const& a) { this->aCombinator = a; this->flags = isAriCom; }
      ), combinator);
      std::tie(this->redInput, this->greenInput) = std::visit(overload(
        [](wire<color::r> const& r) { return std::make_tuple(r.source.source, pointer<network>(nullptr)); },
        [](wire<color::g> const& g) { return std::make_tuple(pointer<network>(nullptr), g.source.source); },
        [](wire<color::rg> const& rg) { return std::make_tuple(rg.r.source.source, rg.g.source.source); }
      ), source);
    }

    template <color c>
    connector<c> getConnector() const;
  };
  static std::vector<network> list;
  static std::vector<size_t> lookup;

  std::vector<pointer<source>> sources;
  std::vector<size_t> inverseLookup; // contains i if and only if this == network::list[network::lookup[i]]
  std::vector<pointer<source>> targets;
  color c;
  enum : uint8_t {
    none             = 0b0000,
    isCompleted      = 0b0001,
    isLoop           = 0b0010,
    isOutputRelevant = 0b0100,
    isMainOutput     = 0b1000
  } flags;

  static size_t simIndex, lookupIndex;
  std::vector<std::vector<signal::WithValue>> values = { {} };
  std::vector<signal::WithValue>* lastValues = nullptr;
  std::vector<signal::WithValue>* nextValues = &values[0];

  void markAsOutput() { setFlag(this->flags, isMainOutput); }
  network& operator+=(network& other)
  {
    if (network::simIndex != 0)
      return other;
    assert(this->c == other.c && "cannot merge networks with different colors!");
    assert(this != &other && "cannot merge a network with itself!");
    assert(this->inverseLookup.size() >= 1 && "internal error. Network somehow doesn't have an index.");
    assert(other.inverseLookup.size() >= 1 && "internal error. Network somehow doesn't have an index.");
    if (this->flags & network::isLoop)
    {
      // merge connector this into loop wire other
      assert((this->flags & network::isCompleted) && "internal error. Loop wire that isn't completed.");
      assert(!(other.flags & network::isCompleted) && "cannot merge a second network into a competed loop wire!");
      assert(this->sources.size() == 0 && "cannot merge a connector into a completed wire! Are you maybe trying to merge a second connector into a looped wire?");
      assert(other.targets.size() == 0 && "internal error. Connector somehow contains target information.");

      other.targets = std::move(this->targets);
      setFlag(this->flags, network::isCompleted);
    }
    else
    {
      // merge connector this into connector other
      assert(!(this->flags & network::isCompleted) && "cannot merge a completed wire into another network!");
      assert(!(other.flags & network::isCompleted) && "cannot merge a completed wire into another network!");
      assert(this->targets.size() == 0 && "internal error. Connector somehow contains target information.");
      assert(other.targets.size() == 0 && "internal error. Connector somehow contains target information.");

      other.sources.insert(other.sources.end(), std::make_move_iterator(this->sources.begin()), std::make_move_iterator(this->sources.end()));
    }
    size_t oldLookup = network::lookup[this->inverseLookup[0]];
    for (auto il : this->inverseLookup)
      network::lookup[il] = network::lookup[other.inverseLookup[0]];
    other.inverseLookup.insert(other.inverseLookup.end(), std::make_move_iterator(this->inverseLookup.begin()), std::make_move_iterator(this->inverseLookup.end()));

    setFlag(other.flags, this->flags);
    if (oldLookup != network::list.size() - 1)
    {
      for (auto ref : network::list.back().inverseLookup)
        network::lookup[ref] = oldLookup;
      std::swap(network::list.back(), *this);
    }
    network::list.pop_back();
    return other;
  }

  void simulate(signal::WithValue const&);
  void simulate(conComData const&);
  void simulate(ariComData const&, std::vector<signal::WithValue> const&);
  void simulate(deciComData const&, std::vector<signal::WithValue> const&);
};
size_t network::simIndex = 0;
size_t network::lookupIndex = 0;

template<color c> void wire<c>::markAsOutput() const { this->network().markAsOutput(); }
template<color c> wire<c>::wire(connector<c> const& source) : source(source) 
{
  setFlag(this->source->flags, network::isCompleted);
}

template<class T>
T* pointer<T>::operator->() const { return this->index == -1 ? nullptr : &T::list[this->index]; }
template<>
network* pointer<network>::operator->() const { return this->index == -1 ? nullptr : &network::list[network::lookup[this->index]]; }

std::vector<network> network::list;
std::vector<size_t> network::lookup;
std::vector<network::source> network::source::list;

std::vector<signal::WithValue> operator+(pointer<network> const& red, pointer<network> const& green)
{
  if (red == nullptr)
    return *green->lastValues;
  if (green == nullptr)
    return *red->lastValues;

  std::vector<signal::WithValue> sum = *red->lastValues;
  for (auto sv : *green->lastValues)
  {
    bool newSig = true;
    for (signal::WithValue& nsv : sum)
      if (sv.sig == nsv.sig)
      {
        nsv.value += sv.value;
        newSig = false;
        break;
      }
    if (newSig)
      sum.push_back(sv);
  }
  return sum;
}

template<color c>
connector<c> network::source::getConnector() const
{
  if (::network::simIndex == 0)
  {
    if (this->flags & network::source::isDeciOrAri)
    {
      if (this->redInput.index != -1)
        this->redInput->targets.emplace_back(network::source::list.size());
      if (this->greenInput.index != -1)
        this->greenInput->targets.emplace_back(network::source::list.size());
    }

    network::list.emplace_back(network{ { pointer<network::source>{ network::source::list.size() } }, { network::lookup.size() }, {}, c, network::none });
    network::lookup.emplace_back(network::list.size() - 1);

    network::source::list.emplace_back(*this);
    return connector<c>(pointer<network>{ network::lookup.size() - 1 });
  }
  else
  {
    switch (this->flags & (network::source::isDeciOrAri | network::source::isConCom))
    {
    case network::source::isConCom:  network::list[network::lookup[network::lookupIndex]].simulate(this->cCombinator); break;
    case network::source::isAriCom:  network::list[network::lookup[network::lookupIndex]].simulate(this->aCombinator, this->redInput + this->greenInput); break;
    case network::source::isDeciCom: network::list[network::lookup[network::lookupIndex]].simulate(this->dCombinator, this->redInput + this->greenInput); break;
    }
    return connector<c>(pointer<network>{ network::lookupIndex++ });
  }
}
template<>
connector<color::rg> network::source::getConnector() const
{
  if (::network::simIndex == 0)
  {
    if (this->flags & network::source::isDeciOrAri)
    {
      if (this->redInput.index != -1)
        this->redInput->targets.emplace_back(network::source::list.size());
      if (this->greenInput.index != -1)
        this->greenInput->targets.emplace_back(network::source::list.size());
    }

    network::list.emplace_back(network{ { pointer<network::source>{ network::source::list.size() } }, { network::lookup.size() }, {}, color::r, network::none });
    network::lookup.emplace_back(network::list.size() - 1);
    connector<color::r> r(pointer<network>{ network::lookup.size() - 1 });

    network::list.emplace_back(network{ { pointer<network::source>{ network::source::list.size() } }, { network::lookup.size() }, {}, color::g, network::none });
    network::lookup.emplace_back(network::list.size() - 1);
    connector<color::g> g(pointer<network>{ network::lookup.size() - 1 });

    network::source::list.emplace_back(*this);
    return connector<color::rg>{r, g};
  }
  else
  {
    std::vector<signal::WithValue> sum;
    switch (this->flags & (network::source::isDeciOrAri | network::source::isConCom))
    {
    case network::source::isConCom:  
      network::list[network::lookup[network::lookupIndex]].simulate(this->cCombinator);
      network::list[network::lookup[network::lookupIndex + 1]].simulate(this->cCombinator);
      break;
    case network::source::isAriCom:
      sum = this->redInput + this->greenInput;
      network::list[network::lookup[network::lookupIndex    ]].simulate(this->aCombinator, sum);
      network::list[network::lookup[network::lookupIndex + 1]].simulate(this->aCombinator, sum);
      break;
    case network::source::isDeciCom:
      sum = this->redInput + this->greenInput;
      network::list[network::lookup[network::lookupIndex    ]].simulate(this->dCombinator, sum);
      network::list[network::lookup[network::lookupIndex + 1]].simulate(this->dCombinator, sum);
      break;
    }
    return connector<color::rg>{ pointer<network>{ network::lookupIndex++ }, pointer<network>{ network::lookupIndex++ } };
  }
}


void network::simulate(signal::WithValue const& sv)
{
  if (sv.value == 0)
    return;
  for (signal::WithValue& nsv : *this->nextValues)
    if (sv.sig == nsv.sig)
    {
      nsv.value += sv.value;
      return;
    }
  this->nextValues->push_back(sv);
}
void network::simulate(conComData const& con)
{
  for (auto osv : con)
    if (osv.has_value())
      this->simulate(osv.value());
}

int32_t pow(int32_t left, int32_t right) {
  if (right == 0)
    return 1;
  if (left == 0)
    return 0;
  if (left == 1)
    return 1;
  if (left == -1)
    return ((right << 31) >> 30) + 1;
  if (right < 0)
    return 0;

  int32_t result = 1;
  for (int32_t y = left; right; y *= y, right >>= 1)
    if (right & 2)
      result *= y;

  return result;
}

int32_t calculate(ariComData::Mode const& mode, int32_t const& left, int32_t const& right)
{
  switch (static_cast<ariComData::Mode::Enum>(mode.description->index))
  {
  case ariComData::Mode::Enum::multiplicaton: return left * right;
  case ariComData::Mode::Enum::division:      return right == 0 || (right == -1 && left == INT32_MIN) ? 0 : left / right;
  case ariComData::Mode::Enum::addition:      return right + left;
  case ariComData::Mode::Enum::subtraction:   return left - right;
  case ariComData::Mode::Enum::modulo:        return right == 0 || (right == -1 && left == INT32_MIN) ? 0 : left % right;
  case ariComData::Mode::Enum::power:         return pow(left, right);
  case ariComData::Mode::Enum::shiftLeft:     return left << right;
  case ariComData::Mode::Enum::shiftRight:    return left >> right;
  case ariComData::Mode::Enum::bitAnd:        return left & right;
  case ariComData::Mode::Enum::bitOr:         return left | right;
  case ariComData::Mode::Enum::bitXor:        return left ^ right;
  }
  assert(false && "invalid arithmetic combinator mode!");
  throw;
}

bool decide(deciComData::Mode const& mode, int32_t const& left, int32_t const& right)
{
  switch (static_cast<deciComData::Mode::Enum>(mode.description->index))
  {
  case deciComData::Mode::Enum::smaller:      return left <  right;
  case deciComData::Mode::Enum::greater:      return left >  right;
  case deciComData::Mode::Enum::equal:        return left == right;
  case deciComData::Mode::Enum::greaterEqual: return left >= right;
  case deciComData::Mode::Enum::smallerEqual: return left <= right;
  case deciComData::Mode::Enum::notEqual:     return left != right;
  }
  assert(false && "invalid decider combinator mode!");
  throw;
}

void network::simulate(ariComData const& ari, std::vector<signal::WithValue> const& in)
{
  int32_t rightNum = std::visit(overload(
    [](int32_t const& i) { return i; },
    [&in](signal const& s) 
    {
      for (auto& sv : in)
        if (sv.sig == s)
          return sv.value;
      return 0; 
    }
  ), ari.right);

  std::visit(overload(
    [&, ari, in, rightNum](int32_t const& leftNum)
    {
      assert(std::holds_alternative<signal>(ari.output) && "arithmetic combinator can't have each output without each input!");
      this->simulate(signal::WithValue{ calculate(ari.mode, leftNum, rightNum), std::get<signal>(ari.output) });
    },
    [&, rightNum](Each const&) 
    {
      std::visit(overload(
        [this, &ari, &in, &rightNum](signal const& outSig)
        {
          for (auto& sv : in)
            this->simulate(signal::WithValue{ calculate(ari.mode, sv.value, rightNum), outSig });
        },
        [this, &ari, &in, &rightNum](Each const&)
        {
          for (auto& sv : in)
            this->simulate(signal::WithValue{ calculate(ari.mode, sv.value, rightNum), sv.sig });
        }
        ), ari.output);
    },
    [&](signal const& leftSig) 
    {
      int32_t leftNum = 0;
      for (auto& sv : in)
        if (sv.sig == leftSig)
        {
          leftNum = sv.value;
          break;
        }
      assert(std::holds_alternative<signal>(ari.output) && "arithmetic combinator can't have each output without each input!");
      this->simulate(signal::WithValue{ calculate(ari.mode, leftNum, rightNum), std::get<signal>(ari.output) });
    }
  ), ari.left);

}
void network::simulate(deciComData const& deci, std::vector<signal::WithValue> const& in)
{
  int32_t rightNum = std::visit(overload(
    [](int32_t const& i) { return i; },
    [&in](signal const& s)
    {
      for (auto& sv : in)
        if (sv.sig == s)
          return sv.value;
      return 0;
    }
  ), deci.right);

  bool result = std::visit(overload(
    [&, deci, in, rightNum](Any const&) 
    {
      for (auto& sv : in)
        if (decide(deci.mode, sv.value, rightNum))
          return true;
      return false;
    },
    [&, deci, in, rightNum](All const&)
    {
      for (auto& sv : in)
        if (!decide(deci.mode, sv.value, rightNum))
          return false;
      return true;
    },
    [&, deci, in, rightNum](signal const& s)
    {
      int32_t leftNum = 0;
      for (auto& sv : in)
        if (sv.sig == s)
        {
          leftNum = sv.value; 
          break;
        }
      return decide(deci.mode, leftNum, rightNum);
    },
    [&, deci, in, rightNum](Each const&)
    {
      assert(std::holds_alternative<Each>(deci.output) || std::holds_alternative<signal>(deci.output) && "decider combinator can only have signal or each output when input is each!");
      assert((!deci.value.has_value() || deci.value.value() == 1) && "decider combinator output value can only be 1!");
      if (std::holds_alternative<signal>(deci.output))
      {
        signal out = std::get<signal>(deci.output);
        int32_t sum = 0;
        if (deci.value.has_value())
        {
          for (auto& sv : in)
            if (decide(deci.mode, sv.value, rightNum))
              sum++;
          this->simulate(signal::WithValue{ sum * deci.value.value(), out });
        }
        else
        {
          for (auto& sv : in)
            if (decide(deci.mode, sv.value, rightNum))
              sum += sv.value;
          this->simulate(signal::WithValue{ sum, out });
        }
      }
      else
      {
        if (deci.value.has_value())
        {
          for (auto& sv : in)
            if (decide(deci.mode, sv.value, rightNum))
              this->simulate(signal::WithValue{ deci.value.value(), sv.sig });
        }
        else
        {
          for (auto& sv : in)
            if (decide(deci.mode, sv.value, rightNum))
              this->simulate(sv);
        }
      }
      return false;
    }
  ), deci.left);

  if (result)
  {
    assert(std::holds_alternative<All>(deci.output) || std::holds_alternative<signal>(deci.output) && "decider combinator can only have signal or all output when input is all!");
    assert((!deci.value.has_value() || deci.value.value() == 1) && "decider combinator output value can only be 1!");
    std::visit(overload(
      [](Each const&) { assert(false && "this code cannot be reached"); },
      [&, deci, in](All const&) 
      {
        if (deci.value.has_value())
        {
          int32_t val = deci.value.value();
          for (auto& sv : in)
              this->simulate(signal::WithValue{ val, sv.sig });
        }
        else
          for (auto& sv : in)
            this->simulate(sv);
      },
      [&, deci, in](signal const& s) 
      {
        if (deci.value.has_value())
          this->simulate(signal::WithValue{ deci.value.value(), s });
        else
        {
          for (auto& sv : in)
            if (sv.sig == s)
            {
              this->simulate(sv);
              break;
            }
        }
      }
    ), deci.output);
  }
}




template<color c>
conCom<c>::operator connector<c>() const { return network::source(this->data).getConnector<c>(); }
conCom<color::rg>::operator connector<color::rg>() const { return network::source(this->data).getConnector<color::rg>(); }

template<color c>
wire<c> wire<c>::loop()
{
  if (::network::simIndex == 0)
  {
    ::network::list.emplace_back(::network{ {}, { ::network::lookup.size() }, {}, c, network::isLoop });
    ::network::lookup.emplace_back(::network::list.size() - 1);
    return wire(pointer<::network>(network::lookup.size() - 1));
  }
  else
    return wire(pointer<::network>(::network::lookupIndex++));
}
wire<color::rg>   wire<color::rg>::operator<<=(connector<color::rg> const& other) const { this->r <<= other.r; this->g <<= other.g; return *this; }
template<color c> wire<c> wire<c>::operator<<=(connector<c> const& other) const { this->network() += other.network(); return *this; }

template<color c> connector<c> operator>>(wire<color::rg> const& w, deciCom<c> const& d) { return network::source(d.data, w).getConnector<c>(); }
template<color c> connector<c> operator>>(wire<color::rg> const& w,  ariCom<c> const& d) { return network::source(d.data, w).getConnector<c>(); }
template<color c> connector<c> operator> (wire<color::r>  const& w, deciCom<c> const& d) { return network::source(d.data, w).getConnector<c>(); }
template<color c> connector<c> operator> (wire<color::r>  const& w,  ariCom<c> const& d) { return network::source(d.data, w).getConnector<c>(); }
template<color c> connector<c> operator> (wire<color::g>  const& w, deciCom<c> const& d) { return network::source(d.data, w).getConnector<c>(); }
template<color c> connector<c> operator> (wire<color::g>  const& w,  ariCom<c> const& d) { return network::source(d.data, w).getConnector<c>(); }
template<color c> connector<c> operator> (connector<color::rg> const& w, deciCom<c> const& d) { return network::source(d.data, w.r).getConnector<c>(); }
template<color c> connector<c> operator> (connector<color::rg> const& w,  ariCom<c> const& d) { return network::source(d.data, w.r).getConnector<c>(); }


connector<color::r>  operator+=(connector<color::r> const& left, connector<color::r> const& right) { left.network() += right.network(); return left; }
connector<color::g>  operator+=(connector<color::g> const& left, connector<color::g> const& right) { left.network() += right.network(); return left; }
connector<color::rg> operator+=(connector<color::rg> const& left, connector<color::rg> const& right) { left.r  += right.r; left.g  += right.g; return left; }
connector<color::rg> operator+=(connector<color::rg> const& left, connector<color::r> const& right) { left.r += right; return left; }
connector<color::rg> operator+=(connector<color::rg> const& left, connector<color::g> const& right) { left.g += right; return left; }
connector<color::rg> operator+=(connector<color::r> const& left, connector<color::rg> const& right) { left += right.r; return { left, right.g }; }
connector<color::rg> operator+=(connector<color::g> const& left, connector<color::rg> const& right) { left += right.g; return { right.r, left }; }
connector<color::rg> operator+=(connector<color::r> const& left, connector<color::g> const& right) { return { left, right }; }
connector<color::rg> operator+=(connector<color::g> const& left, connector<color::r> const& right) { return { right, left }; }











enum class connectionType { standard, input, output,  };

struct entity
{
  static std::vector<entity> list;
  static std::vector<std::vector<pointer<entity>>> xyToPole;
  pointer<network::source> source;
  pointer<entity> entity_number;
  std::tuple<float, float> position;
  uint32_t direction = 0;

  std::vector<std::vector<std::tuple<pointer<entity>, connectionType>>> rConnection = { {} };
  std::vector<std::vector<std::tuple<pointer<entity>, connectionType>>> gConnection = { {} };

  std::vector<std::tuple<pointer<entity>, connectionType>>& operator()(color c, connectionType i = connectionType::standard)
  {
    if (c == color::r)
      return rConnection[i == connectionType::output];
    else
      return gConnection[i == connectionType::output];
  }
};
std::vector<entity> entity::list;
std::vector<std::vector<pointer<entity>>> entity::xyToPole;

pointer<entity> poleAt(uint64_t const& x, uint64_t const& y)
{
  pointer<entity> result;
  if (entity::xyToPole.size() > x && entity::xyToPole[x].size() > y && (result = entity::xyToPole[x][y]))
    return result;
  else
  {
    entity::list.emplace_back(entity());
    entity& pole = entity::list.back();
    pole.position = { static_cast<float>(x), static_cast<float>(y) };
    pole.entity_number = entity::list.size() - 1;
    pole.source = nullptr;
    uint64_t maxX = std::max<uint64_t>(entity::xyToPole.size(), x + 1);
    if (maxX > 0) maxX--;
    uint64_t maxY = entity::xyToPole.size() == 0 ? y + 1 : std::max<uint64_t>(entity::xyToPole[0].size(), y + 1);
    if (maxY > 0) maxY--;
    for (size_t px = 0; px <= maxX; px++)
    {
      if (entity::xyToPole.size() <= maxX)
        entity::xyToPole.emplace_back(std::vector<pointer<entity>>(maxY + 1, nullptr));
      else
        for (size_t py = entity::xyToPole[px].size(); py <= maxY; py++)
          entity::xyToPole[px].emplace_back(-1);
    }
    return entity::xyToPole[x][y] = pole.entity_number;
  }
}

void flagSourcesForOutput(network& net)
{
  if (!(net.flags & network::isOutputRelevant))
  {
    setFlag(net.flags, network::isOutputRelevant);
    assert(net.sources.size() > 0 && "looped wires need to be written to at some point using \"wire <<= connector\".");
    for (size_t j = 0; j < net.sources.size(); j++)
    {
      network::source& source = *net.sources[j];
      setFlag(source.flags, network::source::isOutputRelevant);
      if (source.flags & network::source::isDeciOrAri)
      {
        if (source.redInput.index != -1)
          flagSourcesForOutput(*source.redInput);
        if (source.greenInput.index != -1)
          flagSourcesForOutput(*source.greenInput);
      }
    }
  }
}

std::string encode64(const std::string& data)
{
  std::string result = "0";
  result.reserve((data.length() + 2) / 3 * 4 + 1);
  const std::array<const char, 64> base64Chars =
  {
    'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
    'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f',
    'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
    'w', 'x', 'y', 'z', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '+', '/'
  };
  size_t i;
  uint32_t bits;
  for (i = 2; i < data.length(); i += 3)
  {
    bits = (static_cast<uint8_t>(data[i - 2]) << 16) 
         | (static_cast<uint8_t>(data[i - 1]) <<  8) 
         |  static_cast<uint8_t>(data[i]);
    result.append(1, base64Chars[ bits >> 18            ]);
    result.append(1, base64Chars[(bits >> 12) & 0b111111]);
    result.append(1, base64Chars[(bits >>  6) & 0b111111]);
    result.append(1, base64Chars[(bits      ) & 0b111111]);
  }
  switch (i - data.length())
  {
  case 0:
    bits = (static_cast<uint8_t>(data[i - 2]) << 8) 
         |  static_cast<uint8_t>(data[i - 1]);
    result.append(1, base64Chars[ bits >> 10            ]);
    result.append(1, base64Chars[(bits >>  4) & 0b111111]);
    result.append(1, base64Chars[(bits <<  2) & 0b111111]);
    result.append(1, '=');
    break;
  case 1:
    bits = static_cast<uint8_t>(data[i - 2]);
    result.append(1, base64Chars[ bits >> 2            ]);
    result.append(1, base64Chars[(bits << 4) & 0b111111]);
    result.append(2, '=');
    break;
  case 2:
    break;
  }
  assert(result.length() == (data.length() + 2) / 3 * 4 + 1);
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

std::ostream& operator<<(std::ostream& out, Any const&) { return out << "{\"type\":\"virtual\",\"name\":\"signal-anything\"}"; }
std::ostream& operator<<(std::ostream& out, All const&) { return out << "{\"type\":\"virtual\",\"name\":\"signal-everything\"}"; }
std::ostream& operator<<(std::ostream& out, Each const&) { return out << "{\"type\":\"virtual\",\"name\":\"signal-each\"}"; }
std::ostream& operator<<(std::ostream& out, signal const& s) 
{
  return out << "{\"type\":\"" << s.description->type << "\",\"name\":\"" << s.description->gameSyntax << "\"}"; 
}
std::ostream& operator<<(std::ostream& out, signal::WithValue const& sv) 
{
  return out << "\"signal\":" << sv.sig << ",\"count\":" << sv.value;
}
std::ostream& operator<<(std::ostream& out, conComData const& c) 
{
  out << "constant-combinator\",\"control_behavior\":{\"filters\":[";
  for (size_t j = 0; j < c.size(); j++)
    if (c[j].has_value())
      out << "{" << c[j].value() << ",\"index\":" << (j + 1) << "},";
  out.seekp(-1, out.cur);
  return out << "]},";
}
std::ostream& operator<<(std::ostream& out, deciComData const& d) 
{
  out << "decider-combinator\",\"control_behavior\":{\"decider_conditions\":{\"first_signal\":";
  std::visit([&out](auto const& l) -> std::ostream& { return out << l; }, d.left) << ",";
  std::visit(overload(
    [&out](int32_t const& i) { out << "\"constant\":" << i; },
    [&out](signal const& s) { out << "\"second_signal\":" << s << ""; }
  ), d.right);
  out << ",\"comparator\":\"" << d.mode.description->gameSyntax << "\",\"output_signal\":";
  std::visit([&out](auto const& l) -> std::ostream& { return out << l; }, d.output);
  return out << ",\"copy_count_from_input\":" << (d.value.has_value() ? "false}}," : "true}},");
}
std::ostream& operator<<(std::ostream& out, ariComData const& a) 
{
  out << "arithmetic-combinator\",\"control_behavior\":{\"arithmetic_conditions\":{";
  std::visit(overload(
    [&out](int32_t const& i) { out << "\"first_constant\":" << i << ","; },
    [&out](auto const& s) { out << "\"first_signal\":" << s << ","; }
  ), a.left);
  std::visit(overload(
    [&out](int32_t const& i) { out << "\"second_constant\":" << i << ","; },
    [&out](signal const& s) { out << "\"second_signal\":" << s << ","; }
  ), a.right);
  out << "\"operation\":\"" << a.mode.description->gameSyntax << "\",\"output_signal\":";
  return std::visit([&out](auto const& l) -> std::ostream& { return out << l; }, a.output) << "}},";
}
std::ostream& operator<<(std::ostream& out, std::tuple<pointer<entity>, connectionType> const& connector) 
{
  const auto [first, second] = connector;
  out << "{\"entity_id\":" << first.index + 1;
  if (second != connectionType::standard)
    out << ",\"circuit_id\":" << static_cast<int>(second);
  return out << "}";
}
template<class T> std::ostream& operator<<(std::ostream& out, std::vector<T> const& vec)
{
  out << "[";
  for (auto& el : vec)
    out << el << ",";
  out.seekp(-1, out.cur);
  return out << "]";
}
std::ostream& operator<<(std::ostream& out, entity const& e)
{
  out << "{\"entity_number\":" << (&e - &entity::list[0] + 1) << ",\"position\":{\"x\":" << std::get<0>(e.position) << ",\"y\":" << -std::get<1>(e.position) << "},";
  if (e.direction != 0)
    out << "\"direction\":" << e.direction << ",";
  out << "\"name\":\"";
  if (e.source == nullptr)
    out << "medium-electric-pole\",";
  else
    switch (e.source->flags & (network::source::isDeciOrAri | network::source::isConCom))
    {
    case network::source::isConCom:  out << e.source->cCombinator; break;
    case network::source::isDeciCom: out << e.source->dCombinator; break;
    case network::source::isAriCom:  out << e.source->aCombinator; break;
    }
  out << "\"connections\":{";
  for (size_t i = 0; i < e.rConnection.size(); i++)
  {
    out << "\"" << (i + 1) << "\":{";
    if (e.rConnection[i].size() != 0)
    {
      out << "\"red\":" << e.rConnection[i];
      if (e.gConnection[i].size() != 0)
        out << ",\"green\":" << e.gConnection[i];
    }
    else
      out << "\"green\":" << e.gConnection[i];
    out << "},";
  }
  out.seekp(-1, out.cur);
  return out << "}}";
}
std::string stringify()
{
  std::ostringstream out;
  out << "{\"blueprint\":{\"icons\":[{\"signal\":" << itemSignal::decider_combinator << ",\"index\":1}],"
      << "\"entities\":" << entity::list << ",\"item\":\"blueprint\",\"version\":73018310664}}";
  return encode64(compress(out.str()));
}

std::string compile(uint16_t lengthOfValueHistory)
{
  network::simIndex = 1;
  network::lookupIndex = 0;
  for (network& net : network::list) 
  {
    net.flags = static_cast<decltype(net.flags)>(net.flags & !network::isOutputRelevant);
    net.values = std::vector(lengthOfValueHistory, std::vector<signal::WithValue>());
    net.lastValues = net.nextValues;
    net.nextValues = &net.values[network::simIndex - 1];
  }
  for (network& net : network::list)
    if (net.flags & network::isMainOutput)
      flagSourcesForOutput(net);
  for (network::source& source : network::source::list)
    if (source.flags & network::source::isOutputRelevant)
    {
      entity next;
      next.source = pointer<network::source>(&source - &network::source::list[0]);
      source.entity = next.entity_number = entity::list.size();
      next.position = { static_cast<float>(next.entity_number.index), source.flags & network::source::isDeciOrAri ? 0.5f : 1.0f };
      if (source.flags & network::source::isDeciOrAri)
      {
        next.rConnection.emplace_back(std::vector<std::tuple<pointer<entity>, connectionType>>());
        next.gConnection.emplace_back(std::vector<std::tuple<pointer<entity>, connectionType>>());
      }
      entity::list.emplace_back(next);
    }
  struct connection
  {
    pointer<entity> entity = -1;
    connectionType index = connectionType::standard;
    uint8_t wires = 0;
  };
  struct compiledNetwork
  {
    color c;
    enum : uint8_t {
      none               = 0b0000,
      isMainOutput       = 0b0001,
      needsExtenderPoles = 0b0010
    } flags;
    uint64_t y = 0;
    std::vector<connection> connections; // maps into entities via first, bool true = input, false = output
  };
  std::vector<compiledNetwork> cNetworks;
  for (network net : network::list)
    if (net.flags & network::isOutputRelevant)
    {
      compiledNetwork next;
      for (pointer<network::source>& source : net.sources)
        if (source->entity)
          next.connections.push_back({ source->entity, source->flags & network::source::isDeciOrAri ? connectionType::output : connectionType::standard });
      for (pointer<network::source>& target : net.targets)
        if (target->entity)
          next.connections.push_back({ target->entity, connectionType::input });
      next.c = net.c;
      next.flags = net.flags & network::isMainOutput ? compiledNetwork::isMainOutput : compiledNetwork::none;
      cNetworks.emplace_back(next);
    }
  for (compiledNetwork& n : cNetworks)
  {
    for (size_t j = 1; j < n.connections.size(); j++)
    {
      connection& l = n.connections[j - 1];
      connection& r = n.connections[j];
      if (std::abs(static_cast<int64_t>(l.entity - r.entity)) <= 10)
      {
        (*l.entity)(n.c, l.index).push_back({ r.entity, r.index });
        (*r.entity)(n.c, r.index).push_back({ l.entity, l.index });
      }
      else
      {
        setFlag(n.flags, compiledNetwork::needsExtenderPoles);
        l.wires++;
        r.wires++;
      }
    }
    if (n.flags & compiledNetwork::isMainOutput)
      n.connections.back().wires++;
  }
  int64_t redNetworks = 0;
  int64_t greenNetworks = 0;
  for (compiledNetwork& n : cNetworks)
    if (n.flags & (compiledNetwork::isMainOutput | compiledNetwork::needsExtenderPoles))
    {
      int64_t raw = (n.c == color::r ? redNetworks : greenNetworks)++ * 5;
      n.y = (raw - 1) / 7 * 9 + (raw - 1) % 7 + 3;
    }

  uint64_t outputPostX = entity::list.size() + 2;
  for (compiledNetwork& cnet : cNetworks)
  {
    size_t xLastPole = -1; pointer<entity> iLastPole = nullptr;
    for (connection con : cnet.connections)
      if (con.wires != 0)
      {
        uint64_t x = con.entity.index;
        uint64_t y0 = std::min<int64_t>(cnet.y, con.index == connectionType::input ? 8 : 9);
        pointer<entity> pole = poleAt(x, y0);
        (*con.entity)(cnet.c, con.index).push_back({ pole, connectionType::standard });
        (*pole)(cnet.c).push_back({ con.entity, con.index });
        pointer<entity> lastPole = pole;

        if (y0 < cnet.y)
          for (y0 = std::min(y0 + 9, cnet.y); y0 <= cnet.y; y0 += 9)
          {
            pole = poleAt(x, y0);
            (*lastPole)(cnet.c).push_back({ pole.index, connectionType::standard });
            (*pole)(cnet.c).push_back({ lastPole.index, connectionType::standard });
            lastPole = pole;
          }
        if (iLastPole)
        {
          while ((xLastPole = std::max(xLastPole + 1, std::min(xLastPole + 9, x))) <= x)
          {
            pointer<entity> pole = poleAt(xLastPole, cnet.y);
            (*iLastPole)(cnet.c).push_back({ pole.index, connectionType::standard });
            (*pole)(cnet.c).push_back({ iLastPole.index, connectionType::standard });
            iLastPole = pole;
          }
          if (con.wires == 1)
          {
            xLastPole = -1;
            iLastPole = nullptr;
          }
        }
        else
        {
          iLastPole = lastPole;
          xLastPole = x;
        }
      }
    if (cnet.flags & compiledNetwork::isMainOutput)
      while ((xLastPole = std::max(xLastPole + 1, std::min(xLastPole + 9, outputPostX))) <= outputPostX)
      {
        pointer<entity> pole = poleAt(xLastPole, cnet.y);
        (*iLastPole)(cnet.c).push_back({ pole.index, connectionType::standard });
        (*pole)(cnet.c).push_back({ iLastPole.index, connectionType::standard });
        iLastPole = pole;
      }
  }
  return stringify();
}


std::string compileFirstOrSimulate(uint16_t lengthOfValueHistory)
{
  if (network::simIndex == 0)
    return compile(lengthOfValueHistory);
  else
  {
    if (++network::simIndex > lengthOfValueHistory)
      network::simIndex -= lengthOfValueHistory;
    for(network& net : network::list)
      //if (net.flags & network::isOutputRelevant)
      {
        net.lastValues = net.nextValues;
        net.nextValues = &net.values[network::simIndex - 1];
        net.nextValues->clear();
      }
    network::lookupIndex = 0;
    return "";
  }
}

#endif
