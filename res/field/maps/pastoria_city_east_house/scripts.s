#include "macros/scrcmd.inc"
#include "res/text/bank/pastoria_city_east_house.h"


    ScriptEntry PastoriaCityEastHouse_MoveManiac
    ScriptEntry PastoriaCityEastHouse_Youngster
    ScriptEntry _GuruScript
    ScriptEntryEnd

PastoriaCityEastHouse_MoveManiac:
    PlaySE SE_CONFIRM_sseq_3
    LockAll
    FacePlayer
    GoToIfUnset FLAG_TALKED_TO_PASTORIA_CITY_EAST_HOUSE_MOVE_MANIAC, PastoriaCityEastHouse_CheckHeartScale
    GoToIfNotEnoughMoney 5000, PastoriaCityEastHouse_ComeBackWithHeartScale
    GoTo PastoriaCityEastHouse_TryTeachMove

PastoriaCityEastHouse_CheckHeartScale:
    SetFlag FLAG_TALKED_TO_PASTORIA_CITY_EAST_HOUSE_MOVE_MANIAC
    Message PastoriaCityEastHouse_Text_TeachMoveForHeartScale
    GoToIfNotEnoughMoney 5000, PastoriaCityEastHouse_ComeBackWithHeartScale
    GoTo PastoriaCityEastHouse_TryTeachMove

PastoriaCityEastHouse_ComeBackWithHeartScale:
    Message PastoriaCityEastHouse_Text_ComeBackWithHeartScale
    WaitButton
    CloseMessage
    ReleaseAll
    End

PastoriaCityEastHouse_TryTeachMove:
    Message PastoriaCityEastHouse_Text_ThatsAHeartScale
    Message PastoriaCityEastHouse_Text_TutorWhichPokemon
    CloseMessage
    FadeScreenOut
    WaitFadeScreen
    SelectMoveTutorPokemon
    GetSelectedPartySlot VAR_0x8005
    ReturnToField
    FadeScreenIn
    WaitFadeScreen
    GoToIfEq VAR_0x8005, PARTY_SLOT_NONE, PastoriaCityEastHouse_ComeBackWithHeartScale
    GetPartyMonSpecies VAR_0x8005, VAR_RESULT
    GoToIfEq VAR_RESULT, 0, PastoriaCityEastHouse_EggsCantLearnMoves
    CheckHasLearnableReminderMoves VAR_RESULT, VAR_0x8005
    GoToIfEq VAR_RESULT, FALSE, PastoriaCityEastHouse_NoMovesToTeach
    Message PastoriaCityEastHouse_Text_TeachWhichMove
    CloseMessage
    FadeScreenOut
    WaitFadeScreen
    OpenMoveReminderMenu VAR_0x8005
    CheckLearnedReminderMove VAR_RESULT
    ReturnToField
    FadeScreenIn
    WaitFadeScreen
    GoToIfEq VAR_RESULT, 0xFF, PastoriaCityEastHouse_ComeBackWithHeartScale
    RemoveMoney 5000
    BufferPlayerName 3
    Message PastoriaCityEastHouse_Text_HandedOverHeartScale
    WaitButton
    CloseMessage
    ReleaseAll
    End

PastoriaCityEastHouse_NoMovesToTeach:
    Message PastoriaCityEastHouse_Text_NoMovesToTeach
    WaitButton
    CloseMessage
    ReleaseAll
    End

PastoriaCityEastHouse_EggsCantLearnMoves:
    Message PastoriaCityEastHouse_Text_EggsCantLearnMoves
    WaitButton
    CloseMessage
    ReleaseAll
    End

PastoriaCityEastHouse_Youngster:
    NPCMessage PastoriaCityEastHouse_Text_NewspaperGivesHeartScales
    End

_GuruScript:
    LockAll
    FacePlayer
_GuruMenu:
    Message PastoriaCityEastHouse_Text_GuruWelcome

    InitLocalTextListMenu 17, 1, 0, VAR_RESULT
    AddListMenuEntry PastoriaCityEastHouse_Text_GuruMenuChangeAbility, 0
    AddListMenuEntry PastoriaCityEastHouse_Text_GuruMenuChangeNature, 1
    AddListMenuEntry PastoriaCityEastHouse_Text_GuruMenuSetIVs, 2
    AddListMenuEntry PastoriaCityEastHouse_Text_GuruMenuSetEVs, 3
    AddListMenuEntry PastoriaCityEastHouse_Text_GuruMenuResetEVs, 4
    AddListMenuEntry PastoriaCityEastHouse_Text_GuruMenuCancel, 5
    ShowListMenu
    CloseMessage

    GoToIfEq VAR_RESULT, 0, _GuruAbility
    GoToIfEq VAR_RESULT, 1, _GuruNature
    GoToIfEq VAR_RESULT, 2, _GuruIVs
    GoToIfEq VAR_RESULT, 3, _GuruEVs
    GoToIfEq VAR_RESULT, 4, _GuruResetEVs
    GoTo _GuruCancel

@ ==========================================================
@ ======================== ABILITA' ========================
@ ==========================================================
_GuruAbility:
    GetGuruPrice GURU_SERVICE_ABILITY_IV, VAR_0x8008
    GoToIfEq VAR_0x8008, GURU_PRICE_LOCKED, _GuruNoBadges
    CheckMoney2 VAR_RESULT, VAR_0x8008
    GoToIfEq VAR_RESULT, FALSE, _GuruNoMoney

    Message PastoriaCityEastHouse_Text_GuruWhichPokemon
    CloseMessage
    FadeScreenOut
    WaitFadeScreen
    SelectMoveTutorPokemon
    GetSelectedPartySlot VAR_0x8005
    ReturnToField
    FadeScreenIn
    WaitFadeScreen

    GoToIfEq VAR_0x8005, PARTY_SLOT_NONE, _GuruMenu

    CheckAbilitySwap VAR_0x8005, VAR_0x8006, VAR_0x8007
    GoToIfEq VAR_0x8006, GURU_STATUS_EGG, _GuruEgg
    GoToIfEq VAR_0x8006, GURU_STATUS_ONE_ABILITY, _GuruOneAbility

    GoToIfEq VAR_0x8008, 0, _GuruAbilityFree
    BufferNumber 1, VAR_0x8008
    Message PastoriaCityEastHouse_Text_GuruAbilityCost
    ShowYesNoMenu VAR_RESULT
    CloseMessage
    GoToIfEq VAR_RESULT, MENU_YES, _GuruAbilityPay
    GoTo _GuruMenu

_GuruAbilityFree:
    Message PastoriaCityEastHouse_Text_GuruChampionFree
    ShowYesNoMenu VAR_RESULT
    CloseMessage
    GoToIfEq VAR_RESULT, MENU_YES, _GuruExecuteAbility
    GoTo _GuruMenu

_GuruAbilityPay:
    RemoveMoney2 VAR_0x8008
    Message PastoriaCityEastHouse_Text_GuruPaymentReceived
    WaitABPadPress
    CloseMessage
    GoTo _GuruExecuteAbility

_GuruExecuteAbility:
    ApplyAbilitySwap VAR_0x8005, VAR_0x8007
    PlaySE SE_CONFIRM_sseq_3
    Message PastoriaCityEastHouse_Text_GuruDone
    WaitABPadPress
    CloseMessage
    GoTo _GuruMenu

@ ==========================================================
@ ======================== NATURE ==========================
@ ==========================================================
_GuruNature:
    GetGuruPrice GURU_SERVICE_NATURE, VAR_0x8008
    GoToIfEq VAR_0x8008, GURU_PRICE_LOCKED, _GuruNoBadges
    CheckMoney2 VAR_RESULT, VAR_0x8008
    GoToIfEq VAR_RESULT, FALSE, _GuruNoMoney

    Message PastoriaCityEastHouse_Text_GuruWhichPokemon
    CloseMessage
    FadeScreenOut
    WaitFadeScreen
    SelectMoveTutorPokemon
    GetSelectedPartySlot VAR_0x8005
    ReturnToField
    FadeScreenIn
    WaitFadeScreen

    GoToIfEq VAR_0x8005, PARTY_SLOT_NONE, _GuruMenu

    CheckNatureSwap VAR_0x8005, VAR_0x8006
    GoToIfEq VAR_0x8006, GURU_STATUS_EGG, _GuruEgg

    Message PastoriaCityEastHouse_Text_GuruWhichNature
    InitLocalTextListMenu 17, 1, 0, VAR_0x8007
    AddListMenuEntry PastoriaCityEastHouse_Text_GuruNatureHardy, 0
    AddListMenuEntry PastoriaCityEastHouse_Text_GuruNatureLonely, 1
    AddListMenuEntry PastoriaCityEastHouse_Text_GuruNatureBrave, 2
    AddListMenuEntry PastoriaCityEastHouse_Text_GuruNatureAdamant, 3
    AddListMenuEntry PastoriaCityEastHouse_Text_GuruNatureNaughty, 4
    AddListMenuEntry PastoriaCityEastHouse_Text_GuruNatureBold, 5
    AddListMenuEntry PastoriaCityEastHouse_Text_GuruNatureDocile, 6
    AddListMenuEntry PastoriaCityEastHouse_Text_GuruNatureRelaxed, 7
    AddListMenuEntry PastoriaCityEastHouse_Text_GuruNatureImpish, 8
    AddListMenuEntry PastoriaCityEastHouse_Text_GuruNatureLax, 9
    AddListMenuEntry PastoriaCityEastHouse_Text_GuruNatureTimid, 10
    AddListMenuEntry PastoriaCityEastHouse_Text_GuruNatureHasty, 11
    AddListMenuEntry PastoriaCityEastHouse_Text_GuruNatureSerious, 12
    AddListMenuEntry PastoriaCityEastHouse_Text_GuruNatureJolly, 13
    AddListMenuEntry PastoriaCityEastHouse_Text_GuruNatureNaive, 14
    AddListMenuEntry PastoriaCityEastHouse_Text_GuruNatureModest, 15
    AddListMenuEntry PastoriaCityEastHouse_Text_GuruNatureMild, 16
    AddListMenuEntry PastoriaCityEastHouse_Text_GuruNatureQuiet, 17
    AddListMenuEntry PastoriaCityEastHouse_Text_GuruNatureBashful, 18
    AddListMenuEntry PastoriaCityEastHouse_Text_GuruNatureRash, 19
    AddListMenuEntry PastoriaCityEastHouse_Text_GuruNatureCalm, 20
    AddListMenuEntry PastoriaCityEastHouse_Text_GuruNatureGentle, 21
    AddListMenuEntry PastoriaCityEastHouse_Text_GuruNatureSassy, 22
    AddListMenuEntry PastoriaCityEastHouse_Text_GuruNatureCareful, 23
    AddListMenuEntry PastoriaCityEastHouse_Text_GuruNatureQuirky, 24
    ShowListMenu
    CloseMessage

    GoToIfEq VAR_0x8007, MENU_CANCEL, _GuruMenu

    GoToIfEq VAR_0x8008, 0, _GuruNatureFree

    BufferNumber 1, VAR_0x8008
    Message PastoriaCityEastHouse_Text_GuruNatureCost
    ShowYesNoMenu VAR_RESULT
    CloseMessage
    GoToIfEq VAR_RESULT, MENU_YES, _GuruNaturePay
    GoTo _GuruMenu

_GuruNatureFree:
    Message PastoriaCityEastHouse_Text_GuruChampionFree
    ShowYesNoMenu VAR_RESULT
    CloseMessage
    GoToIfEq VAR_RESULT, MENU_YES, _GuruExecuteNature
    GoTo _GuruMenu

_GuruNaturePay:
    RemoveMoney2 VAR_0x8008
    Message PastoriaCityEastHouse_Text_GuruPaymentReceived
    WaitABPadPress
    CloseMessage
    GoTo _GuruExecuteNature

_GuruExecuteNature:
    ApplyNatureSwap VAR_0x8005, VAR_0x8007
    PlaySE SE_CONFIRM_sseq_3
    Message PastoriaCityEastHouse_Text_GuruDone
    WaitABPadPress
    CloseMessage
    GoTo _GuruMenu

@ ==========================================================
@ ================= RESET EVs (GRATIS) =====================
@ ==========================================================
_GuruResetEVs:
    Message PastoriaCityEastHouse_Text_GuruWhichPokemon
    CloseMessage
    FadeScreenOut
    WaitFadeScreen
    SelectMoveTutorPokemon
    GetSelectedPartySlot VAR_0x8005
    ReturnToField
    FadeScreenIn
    WaitFadeScreen

    GoToIfEq VAR_0x8005, PARTY_SLOT_NONE, _GuruMenu

    CheckNatureSwap VAR_0x8005, VAR_0x8006
    GoToIfEq VAR_0x8006, GURU_STATUS_EGG, _GuruEgg

    ResetEVs VAR_0x8005
    PlaySE SE_CONFIRM_sseq_3
    Message PastoriaCityEastHouse_Text_GuruEVsReset
    WaitABPadPress
    CloseMessage
    GoTo _GuruMenu

@ ==========================================================
@ ======================== SET IVs =========================
@ ==========================================================
_GuruIVs:
    GetGuruPrice GURU_SERVICE_ABILITY_IV, VAR_0x8008
    GoToIfEq VAR_0x8008, GURU_PRICE_LOCKED, _GuruNoBadges
    CheckMoney2 VAR_RESULT, VAR_0x8008
    GoToIfEq VAR_RESULT, FALSE, _GuruNoMoney

    Message PastoriaCityEastHouse_Text_GuruWhichPokemon
    CloseMessage
    FadeScreenOut
    WaitFadeScreen
    SelectMoveTutorPokemon
    GetSelectedPartySlot VAR_0x8005
    ReturnToField
    FadeScreenIn
    WaitFadeScreen

    GoToIfEq VAR_0x8005, PARTY_SLOT_NONE, _GuruMenu

    CheckNatureSwap VAR_0x8005, VAR_0x8006
    GoToIfEq VAR_0x8006, GURU_STATUS_EGG, _GuruEgg

    GoToIfEq VAR_0x8008, 0, _GuruIVLoop

    BufferNumber 1, VAR_0x8008
    Message PastoriaCityEastHouse_Text_GuruIVCost
    ShowYesNoMenu VAR_RESULT
    CloseMessage
    GoToIfEq VAR_RESULT, MENU_YES, _GuruIVPay
    GoTo _GuruMenu

_GuruIVPay:
    RemoveMoney2 VAR_0x8008
    Message PastoriaCityEastHouse_Text_GuruPaymentReceived
    WaitABPadPress
    CloseMessage
    GoTo _GuruIVLoop

_GuruIVLoop:
    Message PastoriaCityEastHouse_Text_GuruWhichStat
    InitLocalTextListMenu 17, 1, 0, VAR_0x8009
    AddListMenuEntry PastoriaCityEastHouse_Text_GuruStatHP, STAT_HP
    AddListMenuEntry PastoriaCityEastHouse_Text_GuruStatAttack, STAT_ATTACK
    AddListMenuEntry PastoriaCityEastHouse_Text_GuruStatDefense, STAT_DEFENSE
    AddListMenuEntry PastoriaCityEastHouse_Text_GuruStatSpAtk, STAT_SPECIAL_ATTACK
    AddListMenuEntry PastoriaCityEastHouse_Text_GuruStatSpDef, STAT_SPECIAL_DEFENSE
    AddListMenuEntry PastoriaCityEastHouse_Text_GuruStatSpeed, STAT_SPEED
    AddListMenuEntry PastoriaCityEastHouse_Text_GuruBack, 6
    ShowListMenu
    CloseMessage

    @ Solo le statistiche vanno avanti, "Indietro" e B tornano al menu
    GoToIfLt VAR_0x8009, STAT_MAX, _GuruIVValueLoop

    @ PARACADUTE: Tasto B o "Back" (6) tornano al menù principale
    GoTo _GuruMenu

_GuruIVValueLoop:
    Message PastoriaCityEastHouse_Text_GuruWhichIV
    InitLocalTextListMenu 17, 1, 0, VAR_RESULT
    AddListMenuEntry PastoriaCityEastHouse_Text_GuruIV31, 0
    AddListMenuEntry PastoriaCityEastHouse_Text_GuruIV30, 1
    AddListMenuEntry PastoriaCityEastHouse_Text_GuruIV1, 2
    AddListMenuEntry PastoriaCityEastHouse_Text_GuruIV0, 3
    AddListMenuEntry PastoriaCityEastHouse_Text_GuruBack, 4
    ShowListMenu
    CloseMessage

    @ LISTA BIANCA
    GoToIfEq VAR_RESULT, 0, _IV31
    GoToIfEq VAR_RESULT, 1, _IV30
    GoToIfEq VAR_RESULT, 2, _IV1
    GoToIfEq VAR_RESULT, 3, _IV0

    @ PARACADUTE: Torna indietro
    GoTo _GuruIVLoop

_IV31:
    SetIV VAR_0x8005, VAR_0x8009, 31
    GoTo _IVSuccess
_IV30:
    SetIV VAR_0x8005, VAR_0x8009, 30
    GoTo _IVSuccess
_IV1:
    SetIV VAR_0x8005, VAR_0x8009, 1
    GoTo _IVSuccess
_IV0:
    SetIV VAR_0x8005, VAR_0x8009, 0
    GoTo _IVSuccess

_IVSuccess:
    PlaySE SE_CONFIRM_sseq_3
    Message PastoriaCityEastHouse_Text_GuruDone
    WaitABPadPress
    CloseMessage
    GoTo _GuruIVLoop

@ ==========================================================
@ ======================== SET EVs =========================
@ ==========================================================
_GuruEVs:
    GetGuruPrice GURU_SERVICE_EVS, VAR_0x8008
    GoToIfEq VAR_0x8008, GURU_PRICE_LOCKED, _GuruNoBadges
    CheckMoney2 VAR_RESULT, VAR_0x8008
    GoToIfEq VAR_RESULT, FALSE, _GuruNoMoney

    Message PastoriaCityEastHouse_Text_GuruWhichPokemon
    CloseMessage
    FadeScreenOut
    WaitFadeScreen
    SelectMoveTutorPokemon
    GetSelectedPartySlot VAR_0x8005
    ReturnToField
    FadeScreenIn
    WaitFadeScreen

    GoToIfEq VAR_0x8005, PARTY_SLOT_NONE, _GuruMenu

    CheckNatureSwap VAR_0x8005, VAR_0x8006
    GoToIfEq VAR_0x8006, GURU_STATUS_EGG, _GuruEgg

    GoToIfEq VAR_0x8008, 0, _GuruEVLoop

    BufferNumber 1, VAR_0x8008
    Message PastoriaCityEastHouse_Text_GuruEVCost
    ShowYesNoMenu VAR_RESULT
    CloseMessage
    GoToIfEq VAR_RESULT, MENU_YES, _GuruEVPay
    GoTo _GuruMenu

_GuruEVPay:
    RemoveMoney2 VAR_0x8008
    Message PastoriaCityEastHouse_Text_GuruPaymentReceived
    WaitABPadPress
    CloseMessage
    GoTo _GuruEVLoop

_GuruEVLoop:
    Message PastoriaCityEastHouse_Text_GuruWhichStat
    InitLocalTextListMenu 17, 1, 0, VAR_0x8009
    AddListMenuEntry PastoriaCityEastHouse_Text_GuruStatHP, STAT_HP
    AddListMenuEntry PastoriaCityEastHouse_Text_GuruStatAttack, STAT_ATTACK
    AddListMenuEntry PastoriaCityEastHouse_Text_GuruStatDefense, STAT_DEFENSE
    AddListMenuEntry PastoriaCityEastHouse_Text_GuruStatSpAtk, STAT_SPECIAL_ATTACK
    AddListMenuEntry PastoriaCityEastHouse_Text_GuruStatSpDef, STAT_SPECIAL_DEFENSE
    AddListMenuEntry PastoriaCityEastHouse_Text_GuruStatSpeed, STAT_SPEED
    AddListMenuEntry PastoriaCityEastHouse_Text_GuruBack, 6
    ShowListMenu
    CloseMessage

    @ LISTA BIANCA
    GoToIfLt VAR_0x8009, STAT_MAX, _GuruEVValueLoop

    @ PARACADUTE
    GoTo _GuruMenu

_GuruEVValueLoop:
    ModifyEV VAR_0x8005, VAR_0x8009, TRUE, 0, VAR_0x800A
    BufferNumber 0, VAR_0x800A

    Message PastoriaCityEastHouse_Text_GuruCurrentEVHowToTrain
    InitLocalTextListMenu 17, 1, 0, VAR_RESULT
    AddListMenuEntry PastoriaCityEastHouse_Text_GuruEVPlus252, 0
    AddListMenuEntry PastoriaCityEastHouse_Text_GuruEVPlus10, 1
    AddListMenuEntry PastoriaCityEastHouse_Text_GuruEVPlus1, 2
    AddListMenuEntry PastoriaCityEastHouse_Text_GuruEVMinus1, 3
    AddListMenuEntry PastoriaCityEastHouse_Text_GuruEVMinus10, 4
    AddListMenuEntry PastoriaCityEastHouse_Text_GuruEVMinus252, 5
    AddListMenuEntry PastoriaCityEastHouse_Text_GuruBack, 6
    ShowListMenu
    CloseMessage

    @ LISTA BIANCA
    GoToIfEq VAR_RESULT, 0, _EVAdd252
    GoToIfEq VAR_RESULT, 1, _EVAdd10
    GoToIfEq VAR_RESULT, 2, _EVAdd1
    GoToIfEq VAR_RESULT, 3, _EVSub1
    GoToIfEq VAR_RESULT, 4, _EVSub10
    GoToIfEq VAR_RESULT, 5, _EVSub252

    @ PARACADUTE
    GoTo _GuruEVLoop

_EVAdd252:
    ModifyEV VAR_0x8005, VAR_0x8009, TRUE, 252, VAR_0x8006
    GoTo _EVCheckLimit
_EVAdd10:
    ModifyEV VAR_0x8005, VAR_0x8009, TRUE, 10, VAR_0x8006
    GoTo _EVCheckLimit
_EVAdd1:
    ModifyEV VAR_0x8005, VAR_0x8009, TRUE, 1, VAR_0x8006
    GoTo _EVCheckLimit
_EVSub1:
    ModifyEV VAR_0x8005, VAR_0x8009, FALSE, 1, VAR_0x8006
    GoTo _EVCheckLimit
_EVSub10:
    ModifyEV VAR_0x8005, VAR_0x8009, FALSE, 10, VAR_0x8006
    GoTo _EVCheckLimit
_EVSub252:
    ModifyEV VAR_0x8005, VAR_0x8009, FALSE, 252, VAR_0x8006
    GoTo _EVCheckLimit

_EVCheckLimit:
    GoToIfEq VAR_0x8006, TRUE, _GuruEVLimitReached
    PlaySE SE_CONFIRM_sseq_3
    Message PastoriaCityEastHouse_Text_GuruDone
    WaitABPadPress
    CloseMessage
    GoTo _GuruEVValueLoop

_GuruEVLimitReached:
    Message PastoriaCityEastHouse_Text_GuruStatAtLimit
    WaitABPadPress
    CloseMessage
    GoTo _GuruEVValueLoop

@ ==========================================================
@ ==================== ERRORI E USCITA =====================
@ ==========================================================
_GuruNoMoney:
    Message PastoriaCityEastHouse_Text_GuruNotEnoughMoney
    WaitABPadPress
    CloseMessage
    GoTo _GuruMenu

_GuruNoBadges:
    Message PastoriaCityEastHouse_Text_GuruNeedFourBadges
    WaitABPadPress
    CloseMessage
    GoTo _GuruMenu

_GuruOneAbility:
    Message PastoriaCityEastHouse_Text_GuruOnlyOneAbility
    WaitABPadPress
    CloseMessage
    GoTo _GuruMenu

_GuruEgg:
    Message PastoriaCityEastHouse_Text_GuruEggCantBeHelped
    WaitABPadPress
    CloseMessage
    GoTo _GuruMenu

_GuruCancel:
    Message PastoriaCityEastHouse_Text_GuruComeBackAnytime
    WaitABPadPress
    CloseMessage
    ReleaseAll
    End