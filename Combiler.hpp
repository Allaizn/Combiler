/*
* The following operators are expected to exist for things relating to decider combinators:
*    - For each type T:
*         (1)  T == T -> bool
*         (2)  T != T -> bool
*    - For each comparison "oo" in { ==, !=, >=, <=, >, < } and each "right" in { signal, int, deciCom::Input::Right }
*         (3)  signal               oo right -> deciCom::Input::SignalType (*)
*         (4)  Any                  oo right -> deciCom::Input::AnyType
*         (5)  All                  oo right -> deciCom::Input::AllType
*         (6)  Each                 oo right -> deciCom::Input::EachType
*         (7)  wildCard             oo right -> deciCom::Input
*         (8)  deciCom::Input::Left oo right -> deciCom::Input (*)
*    - For the output constants "value" in { 1, input } (where the latter is macro defined)
*         (9)  signal                 += value -> deciCom::Output
*        (10)  All                    += value -> deciCom::Output::AllType
*        (11)  Each                   += value -> deciCom::Output::EachType
*        (12)  wildCard               += value -> deciCom::Output
*        (13)  deciCom::Input::Output += value -> deciCom::Output
*    - For the macro defined operator "then":
*        (14)  deciCom::Input::SignalType then deciCom::Output           -> deciCom
*        (15)  deciCom::Input::SignalType then deciCom::Output::AllType  -> deciCom
*        (16)  deciCom::Input::AnyType    then deciCom::Output           -> deciCom
*        (17)  deciCom::Input::AnyType    then deciCom::Output::AllType  -> deciCom
*        (18)  deciCom::Input::AllType    then deciCom::Output           -> deciCom
*        (19)  deciCom::Input::AllType    then deciCom::Output::AllType  -> deciCom
*        (20)  deciCom::Input::EachType   then deciCom::Output           -> deciCom
*        (21)  deciCom::Input::EachType   then deciCom::Output::EachType -> deciCom
*        (22)  deciCom::Input             then deciCom::Output           -> deciCom
*        (23)  deciCom::Input             then deciCom::Output::AllType  -> deciCom
*        (24)  deciCom::Input             then deciCom::Output::EachType -> deciCom
*    - The following equality comparisons with oo in { ==, != }:
*        (25)  wildCard oo Any                  -> bool
*        (26)  wildCard oo All                  -> bool
*        (27)  wildCard oo Each                 -> bool
*        (28)  wildCard oo deciCom::Input::Left -> bool
*        (29)  Any      oo deciCom::Input::Left -> bool
*        (30)  All      oo deciCom::Input::Left -> bool
*        (31)  Each     oo deciCom::Input::Left -> bool
*        (32)  signal   oo deciCom::Input::Left -> bool
*        (33)  Any                  oo wildCard -> bool
*        (34)  All                  oo wildCard -> bool
*        (35)  Each                 oo wildCard -> bool
*        (36)  deciCom::Input::Left oo wildCard -> bool
*        (37)  deciCom::Input::Left oo Any      -> bool
*        (38)  deciCom::Input::Left oo All      -> bool
*        (39)  deciCom::Input::Left oo Each     -> bool
*        (40)  deciCom::Input::Left oo signal   -> bool  (*)
*        (41)  signal oo deciCom::Input::Right -> bool   (*)
*        (42)  int    oo deciCom::Input::Right -> bool
*        (43)  deciCom::Input::Right oo signal -> bool
*        (44)  deciCom::Input::Right oo int    -> bool
*        (45)  deciCom::Input::SignalType oo deciCom::Input -> bool
*        (46)  deciCom::Input::AnyType    oo deciCom::Input -> bool
*        (47)  deciCom::Input::AllType    oo deciCom::Input -> bool
*        (48)  deciCom::Input::EachType   oo deciCom::Input -> bool
*        (49)  deciCom::Input oo deciCom::Input::SignalType -> bool
*        (50)  deciCom::Input oo deciCom::Input::AnyType    -> bool
*        (51)  deciCom::Input oo deciCom::Input::AllType    -> bool
*        (52)  deciCom::Input oo deciCom::Input::EachType   -> bool
*        (53)  int oo deciCom::Output::Value -> bool
*        (54)  deciCom::Output::Value oo int -> bool
*        (55)  deciCom::Output::AllType  oo deciCom::Output -> bool
*        (56)  deciCom::Output::EachType oo deciCom::Output -> bool
*        (57)  deciCom::Output oo deciCom::Output::AllType  -> bool
*        (58)  deciCom::Output oo deciCom::Output::EachType -> bool
*
*
* Rules (1-2) and (3) conflict each other in the case of right = "signal"/ T = "signal".
* The solution for this is to modify rule (3) in the following way:
*       (3.1)  signal == signal                -> deciCom::Input::SignalType::Boolable
*       (3.2)  signal != signal                -> deciCom::Input::SignalType::Boolable
*       (3.3)  signal >= right -> deciCom::Input::SignalType
*       (3.4)  signal <= right -> deciCom::Input::SignalType
*       (3.5)  signal  > right -> deciCom::Input::SignalType
*       (3.6)  signal  < right -> deciCom::Input::SignalType
*       (3.7)  deciCom::Input::SignalType::Boolable -> bool   (implicit cast)
* This ensures that only the equality operations are implicitly castable to bool,
* and it doesn't disallow an implicit conversion from SignalType::nonOrdered to SignalType.
*
* Rules (3) also conflicts with rule (41) in the case of right = "deciCom::Input::Right".
* The solution to this is to specialize the "signal =="/ "signal !=" cases completely:
*       (3.8)  signal == deciCom::Input::Right -> deciCom::Input::SignalType::Boolable
*       (3.9)  signal != deciCom::Input::Right -> deciCom::Input::SignalType::Boolable
*      (3.10)  signal == int   -> deciCom::Input::SignalType
*      (3.11)  signal != int   -> deciCom::Input::SignalType
*
* Note that the input type signal of rule (3) is not implicitly convertible to deciCom::Input::Left,
* since such a conversion would make (coal > 0 then each = 1) syntactically legal.
* Note that the input types of rules (4-5) are not implicitly convertible to deciCom::Input::Left
* or wildCard, since such a conversion would make (any|all > 0 then each = 1) syntactically legal.
* Note that the input type Each of rule (6) is not implicitly convertible to deciCom::Input::Left
* or wildCard, since such a conversion would make (each > 0 then all = 1) syntactically legal.
*
* Note that the output type deciCom::Input::SignalType of rule (3) is not implicitly convertible to
* deciCom::Input, since such a conversion would make (coal > 0 then each = 1) syntactically legal.
* Note that the output types of rules (4-5) are not implicitly convertible to deciCom::Input,
* since such a conversion would make (any|all > 0 then each = 1) syntactically legal.
* Note that the input type deciCom::Input::EachType of rule (6) is not implicitly convertible to
* deciCom::Input, since such a conversion would make (each > 0 then all = 1) syntactically legal.
*
* Rules (8) and (40) conflict each other in the case of right = "signal", which we solve similar
* to the (3)-(41) conflict:
*       (8.1)  deciCom::Input::Left == signal                -> deciCom::Input::Boolable
*       (8.2)  deciCom::Input::Left != signal                -> deciCom::Input::Boolable
*       (8.3)  deciCom::Input::Left >= right                 -> deciCom::Input
*       (8.4)  deciCom::Input::Left <= right                 -> deciCom::Input
*       (8.5)  deciCom::Input::Left  > right                 -> deciCom::Input
*       (8.6)  deciCom::Input::Left  < right                 -> deciCom::Input
*       (8.7)  deciCom::Input::Boolable -> bool   (implicit cast)
*       (8.8)  deciCom::Input::Left == deciCom::Input::Right -> deciCom::Input
*       (8.9)  deciCom::Input::Left != deciCom::Input::Right -> deciCom::Input
*      (8.10)  deciCom::Input::Left == int                   -> deciCom::Input
*      (8.11)  deciCom::Input::Left != int                   -> deciCom::Input
*
* Rule (12-13) are not syntactically checkable for validity (since Any is not an allowed decider
* output), but they need to perform a compile time check to enforce the game rules.
*
* Note that the output type deciCom::Output::AllType of rule (10) is not implicitly convertible to
* deciCom::Output, since such a conversion would make (each > 0 then all = 1) syntactically legal.
* Note that the output type deciCom::Output::EachType of rule (11) is not implicitly convertible to
* deciCom::Output, since such a conversion would make (coal > 0 then each = 1) syntactically legal.
*
* Rules (14, 16, 18, 20, 22, 23, 24) cannot guarantee a correct setting syntactically (e.g. when
* created by rule 12 or 13), and thus need compile time checks.
*/