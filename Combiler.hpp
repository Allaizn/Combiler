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
*         (8)  deciCom::Input::Left oo right -> deciCom::Input
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
*        (32)  Any                  oo wildCard -> bool
*        (33)  All                  oo wildCard -> bool
*        (34)  Each                 oo wildCard -> bool
*        (35)  deciCom::Input::Left oo wildCard -> bool
*        (36)  deciCom::Input::Left oo Any      -> bool
*        (37)  deciCom::Input::Left oo All      -> bool
*        (38)  deciCom::Input::Left oo Each     -> bool
*        (39)  signal oo deciCom::Input::Right -> bool
*        (40)  int    oo deciCom::Input::Right -> bool
*        (41)  deciCom::Input::Right oo signal -> bool
*        (42)  deciCom::Input::Right oo int    -> bool
*        (43)  deciCom::Input::SignalType oo deciCom::Input -> bool
*        (44)  deciCom::Input::AnyType    oo deciCom::Input -> bool
*        (45)  deciCom::Input::AllType    oo deciCom::Input -> bool
*        (46)  deciCom::Input::EachType   oo deciCom::Input -> bool
*        (47)  deciCom::Input oo deciCom::Input::SignalType -> bool
*        (48)  deciCom::Input oo deciCom::Input::AnyType    -> bool
*        (49)  deciCom::Input oo deciCom::Input::AllType    -> bool
*        (50)  deciCom::Input oo deciCom::Input::EachType   -> bool
*        (51)  int oo deciCom::Output::Value -> bool
*        (52)  deciCom::Output::Value oo int -> bool
*        (53)  deciCom::Output::AllType  oo deciCom::Output -> bool
*        (54)  deciCom::Output::EachType oo deciCom::Output -> bool
*        (55)  deciCom::Output oo deciCom::Output::AllType  -> bool
*        (56)  deciCom::Output oo deciCom::Output::EachType -> bool
*
*
* Rules (1-2) and (3) conflict each other in the case of (signal == signal), and rules (1-2) also
* conflict with rule (37). The solution for this is to modify rule (3) in the following way:
*       (3.1) signal == signal                -> deciCom::Input::SignalType::nonOrdered
*       (3.2) signal != signal                -> deciCom::Input::SignalType::nonOrdered
*       (3.3) signal == deciCom::Input::Right -> deciCom::Input::SignalType::nonOrdered
*       (3.4) signal != deciCom::Input::Right -> deciCom::Input::SignalType::nonOrdered
*       (3.5) signal == int   -> deciCom::Input::SignalType
*       (3.6) signal != int   -> deciCom::Input::SignalType
*       (3.7) signal >= right -> deciCom::Input::SignalType
*       (3.8) signal <= right -> deciCom::Input::SignalType
*       (3.9) signal  > right -> deciCom::Input::SignalType
*      (3.10) signal  < right -> deciCom::Input::SignalType
*      (3.11) deciCom::Input::SignalType::nonOrdered -> bool   (implicit cast)
* This ensures that only the equality operations are implicitly castable to bool,
* and it doesn't disallow an implicit conversion from SignalType::nonOrdered to SignalType.
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