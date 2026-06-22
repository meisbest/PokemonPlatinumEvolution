#include "macros/scrcmd.inc"
#include "res/text/bank/pastoria_city_east_house.h"


    ScriptEntry PastoriaCityEastHouse_MoveManiac
    ScriptEntry PastoriaCityEastHouse_Youngster
    ScriptEntry _GuruScript
    ScriptEntryEnd

@ --- MACRO CUSTOM DEL GURU ---
.macro GuruCmd_GetPrice serviceType, destVar
    .short 0x009C
    .short \serviceType
    .short \destVar
.endm

.macro GuruCmd_CheckAbility partySlot, destStatus, destAbility
    .short 0x009D
    .short \partySlot
    .short \destStatus
    .short \destAbility
.endm

.macro GuruCmd_ApplyAbility partySlot, newAbility
    .short 0x009E
    .short \partySlot
    .short \newAbility
.endm

.macro GuruCmd_CheckNature partySlot, destStatus
    .short 0x009F
    .short \partySlot
    .short \destStatus
.endm

.macro GuruCmd_ApplyNature partySlot, newNature
    .short 0x00A0
    .short \partySlot
    .short \newNature
.endm

.macro GuruCmd_ResetEVs partySlot
    .short 0x023F
    .short \partySlot
.endm

.macro GuruCmd_SetIV partySlot, statIndex, value
    .short 0x0240
    .short \partySlot
    .short \statIndex
    .short \value
.endm

.macro GuruCmd_ModifyEV partySlot, statIndex, isAdd, amount, destStatus
    .short 0x0241
    .short \partySlot
    .short \statIndex
    .short \isAdd
    .short \amount
    .short \destStatus
.endm

.macro GuruCmd_CheckMoney priceVar, destStatus
    .short 0x0242
    .short \priceVar
    .short 0
    .short \destStatus
.endm

.macro GuruCmd_TakeMoney priceVar, destStatus
    .short 0x0242
    .short \priceVar
    .short 1
    .short \destStatus
.endm

.macro GuruCmd_InitListMenu anchorX, anchorY, initialCursorPos, canExitWithB, destVar
    .short 0x0045
    .byte \anchorX, \anchorY, \initialCursorPos, \canExitWithB
    .short \destVar
.endm

.macro GuruCmd_WaitAB
    .short 0x0032
.endm
@ -----------------------------

PastoriaCityEastHouse_MoveManiac:
    PlaySE SEQ_SE_CONFIRM
    LockAll
    FacePlayer
    @ BufferItemNameWithArticle 4, ITEM_HEART_SCALE
    GoToIfUnset FLAG_TALKED_TO_PASTORIA_CITY_EAST_HOUSE_MOVE_MANIAC, PastoriaCityEastHouse_CheckHeartScale
    SetVar VAR_0x8008, 5000
    GuruCmd_CheckMoney VAR_0x8008, VAR_RESULT
    GoToIfEq VAR_RESULT, FALSE, PastoriaCityEastHouse_ComeBackWithHeartScale
    GoTo PastoriaCityEastHouse_TryTeachMove

PastoriaCityEastHouse_CheckHeartScale:
    SetFlag FLAG_TALKED_TO_PASTORIA_CITY_EAST_HOUSE_MOVE_MANIAC
    Message PastoriaCityEastHouse_Text_TeachMoveForHeartScale
    @CheckItem ITEM_HEART_SCALE, 1, VAR_RESULT
    SetVar VAR_0x8008, 5000
    GuruCmd_CheckMoney VAR_0x8008, VAR_RESULT
    GoToIfEq VAR_RESULT, 0, PastoriaCityEastHouse_ComeBackWithHeartScale
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
    @RemoveItem ITEM_HEART_SCALE, 1, VAR_RESULT
    SetVar VAR_0x8008, 5000
    GuruCmd_TakeMoney VAR_0x8008, VAR_RESULT
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
    Message 18
    
    GuruCmd_InitListMenu 17, 1, 0, 1, VAR_RESULT
    AddListMenuEntry 28, 0, 255
    AddListMenuEntry 29, 1, 255
    AddListMenuEntry 30, 2, 255
    AddListMenuEntry 31, 3, 255
    AddListMenuEntry 32, 4, 255
    AddListMenuEntry 33, 5, 255
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
    GuruCmd_GetPrice 0, VAR_0x8008
    GoToIfEq VAR_0x8008, 65000, _GuruNoBadges
    GuruCmd_CheckMoney VAR_0x8008, VAR_RESULT
    GoToIfEq VAR_RESULT, 0, _GuruNoMoney

    Message 19 
    CloseMessage
    FadeScreenOut
    WaitFadeScreen
    SelectMoveTutorPokemon
    GetSelectedPartySlot VAR_0x8005
    ReturnToField
    FadeScreenIn
    WaitFadeScreen
    
    GoToIfEq VAR_0x8005, 255, _GuruMenu

    GuruCmd_CheckAbility VAR_0x8005, VAR_0x8006, VAR_0x8007
    GoToIfEq VAR_0x8006, 1, _GuruEgg
    GoToIfEq VAR_0x8006, 2, _GuruOneAbility
    
    GoToIfEq VAR_0x8008, 0, _GuruAbilityFree
    BufferNumber 1, VAR_0x8008 
    Message 20 
    ShowYesNoMenu VAR_RESULT
    CloseMessage
    GoToIfEq VAR_RESULT, 0, _GuruAbilityPay
    GoTo _GuruMenu

_GuruAbilityFree:
    Message 21
    ShowYesNoMenu VAR_RESULT
    CloseMessage
    GoToIfEq VAR_RESULT, 0, _GuruExecuteAbility
    GoTo _GuruMenu

_GuruAbilityPay:
    GuruCmd_TakeMoney VAR_0x8008, VAR_RESULT
    Message 85
    GuruCmd_WaitAB
    CloseMessage
    GoTo _GuruExecuteAbility

_GuruExecuteAbility:
    GuruCmd_ApplyAbility VAR_0x8005, VAR_0x8007
    PlayFanfare SEQ_SE_CONFIRM
    Message 22
    GuruCmd_WaitAB
    CloseMessage
    GoTo _GuruMenu

@ ==========================================================
@ ======================== NATURE ==========================
@ ==========================================================
_GuruNature:
    GuruCmd_GetPrice 1, VAR_0x8008
    GoToIfEq VAR_0x8008, 65000, _GuruNoBadges
    GuruCmd_CheckMoney VAR_0x8008, VAR_RESULT
    GoToIfEq VAR_RESULT, 0, _GuruNoMoney

    Message 19 
    CloseMessage
    FadeScreenOut
    WaitFadeScreen
    SelectMoveTutorPokemon
    GetSelectedPartySlot VAR_0x8005
    ReturnToField
    FadeScreenIn
    WaitFadeScreen
    
    GoToIfEq VAR_0x8005, 255, _GuruMenu

    GuruCmd_CheckNature VAR_0x8005, VAR_0x8006
    GoToIfEq VAR_0x8006, 1, _GuruEgg

    Message 34
    GuruCmd_InitListMenu 17, 1, 0, 1, VAR_0x8007
    AddListMenuEntry 35, 0, 255
    AddListMenuEntry 36, 1, 255
    AddListMenuEntry 37, 2, 255
    AddListMenuEntry 38, 3, 255
    AddListMenuEntry 39, 4, 255
    AddListMenuEntry 40, 5, 255
    AddListMenuEntry 41, 6, 255
    AddListMenuEntry 42, 7, 255
    AddListMenuEntry 43, 8, 255
    AddListMenuEntry 44, 9, 255
    AddListMenuEntry 45, 10, 255
    AddListMenuEntry 46, 11, 255
    AddListMenuEntry 47, 12, 255
    AddListMenuEntry 48, 13, 255
    AddListMenuEntry 49, 14, 255
    AddListMenuEntry 50, 15, 255
    AddListMenuEntry 51, 16, 255
    AddListMenuEntry 52, 17, 255
    AddListMenuEntry 53, 18, 255
    AddListMenuEntry 54, 19, 255
    AddListMenuEntry 55, 20, 255
    AddListMenuEntry 56, 21, 255
    AddListMenuEntry 57, 22, 255
    AddListMenuEntry 58, 23, 255
    AddListMenuEntry 59, 24, 255
    ShowListMenu
    CloseMessage

    GoToIfEq VAR_0x8007, 255, _GuruMenu
    GoToIfEq VAR_0x8007, 65535, _GuruMenu

    GoToIfEq VAR_0x8008, 0, _GuruNatureFree

    BufferNumber 1, VAR_0x8008 
    Message 60 
    ShowYesNoMenu VAR_RESULT
    CloseMessage
    GoToIfEq VAR_RESULT, 0, _GuruNaturePay
    GoTo _GuruMenu

_GuruNatureFree:
    Message 21
    ShowYesNoMenu VAR_RESULT
    CloseMessage
    GoToIfEq VAR_RESULT, 0, _GuruExecuteNature
    GoTo _GuruMenu

_GuruNaturePay:
    GuruCmd_TakeMoney VAR_0x8008, VAR_RESULT
    Message 85
    GuruCmd_WaitAB
    CloseMessage
    GoTo _GuruExecuteNature

_GuruExecuteNature:
    GuruCmd_ApplyNature VAR_0x8005, VAR_0x8007
    PlayFanfare SEQ_SE_CONFIRM
    Message 22
    GuruCmd_WaitAB
    CloseMessage
    GoTo _GuruMenu

@ ==========================================================
@ ================= RESET EVs (GRATIS) =====================
@ ==========================================================
_GuruResetEVs:
    Message 19 
    CloseMessage
    FadeScreenOut
    WaitFadeScreen
    SelectMoveTutorPokemon
    GetSelectedPartySlot VAR_0x8005
    ReturnToField
    FadeScreenIn
    WaitFadeScreen
    
    GoToIfEq VAR_0x8005, 255, _GuruMenu

    GuruCmd_CheckNature VAR_0x8005, VAR_0x8006
    GoToIfEq VAR_0x8006, 1, _GuruEgg

    GuruCmd_ResetEVs VAR_0x8005
    PlayFanfare SEQ_SE_CONFIRM
    Message 84 
    GuruCmd_WaitAB
    CloseMessage
    GoTo _GuruMenu

@ ==========================================================
@ ======================== SET IVs =========================
@ ==========================================================
_GuruIVs:
    GuruCmd_GetPrice 0, VAR_0x8008
    GoToIfEq VAR_0x8008, 65000, _GuruNoBadges
    GuruCmd_CheckMoney VAR_0x8008, VAR_RESULT
    GoToIfEq VAR_RESULT, 0, _GuruNoMoney

    Message 19 
    CloseMessage
    FadeScreenOut
    WaitFadeScreen
    SelectMoveTutorPokemon
    GetSelectedPartySlot VAR_0x8005
    ReturnToField
    FadeScreenIn
    WaitFadeScreen
    
    GoToIfEq VAR_0x8005, 255, _GuruMenu

    GuruCmd_CheckNature VAR_0x8005, VAR_0x8006
    GoToIfEq VAR_0x8006, 1, _GuruEgg

    GoToIfEq VAR_0x8008, 0, _GuruIVLoop 

    BufferNumber 1, VAR_0x8008
    Message 82 
    ShowYesNoMenu VAR_RESULT
    CloseMessage
    GoToIfEq VAR_RESULT, 0, _GuruIVPay
    GoTo _GuruMenu

_GuruIVPay:
    GuruCmd_TakeMoney VAR_0x8008, VAR_RESULT
    Message 85
    GuruCmd_WaitAB
    CloseMessage
    GoTo _GuruIVLoop

_GuruIVLoop:
    Message 61 
    GuruCmd_InitListMenu 17, 1, 0, 1, VAR_0x8009
    AddListMenuEntry 62, 0, 255 
    AddListMenuEntry 63, 1, 255 
    AddListMenuEntry 64, 2, 255 
    AddListMenuEntry 65, 3, 255 
    AddListMenuEntry 66, 4, 255 
    AddListMenuEntry 67, 5, 255 
    AddListMenuEntry 68, 6, 255 
    ShowListMenu
    CloseMessage

    @ LISTA BIANCA: Solo 0-5 vanno avanti!
    GoToIfEq VAR_0x8009, 0, _GuruIVValueLoop
    GoToIfEq VAR_0x8009, 1, _GuruIVValueLoop
    GoToIfEq VAR_0x8009, 2, _GuruIVValueLoop
    GoToIfEq VAR_0x8009, 3, _GuruIVValueLoop
    GoToIfEq VAR_0x8009, 4, _GuruIVValueLoop
    GoToIfEq VAR_0x8009, 5, _GuruIVValueLoop
    
    @ PARACADUTE: Tasto B o "Back" (6) tornano al menù principale
    GoTo _GuruMenu

_GuruIVValueLoop:
    Message 69 
    GuruCmd_InitListMenu 17, 1, 0, 1, VAR_RESULT
    AddListMenuEntry 70, 0, 255 
    AddListMenuEntry 71, 1, 255 
    AddListMenuEntry 72, 2, 255 
    AddListMenuEntry 73, 3, 255 
    AddListMenuEntry 68, 4, 255 
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
    GuruCmd_SetIV VAR_0x8005, VAR_0x8009, 31
    GoTo _IVSuccess
_IV30:
    GuruCmd_SetIV VAR_0x8005, VAR_0x8009, 30
    GoTo _IVSuccess
_IV1:
    GuruCmd_SetIV VAR_0x8005, VAR_0x8009, 1
    GoTo _IVSuccess
_IV0:
    GuruCmd_SetIV VAR_0x8005, VAR_0x8009, 0
    GoTo _IVSuccess

_IVSuccess:
    PlayFanfare SEQ_SE_CONFIRM
    Message 22
    GuruCmd_WaitAB
    CloseMessage
    GoTo _GuruIVLoop

@ ==========================================================
@ ======================== SET EVs =========================
@ ==========================================================
_GuruEVs:
    GuruCmd_GetPrice 2, VAR_0x8008
    GoToIfEq VAR_0x8008, 65000, _GuruNoBadges
    GuruCmd_CheckMoney VAR_0x8008, VAR_RESULT
    GoToIfEq VAR_RESULT, 0, _GuruNoMoney

    Message 19 
    CloseMessage
    FadeScreenOut
    WaitFadeScreen
    SelectMoveTutorPokemon
    GetSelectedPartySlot VAR_0x8005
    ReturnToField
    FadeScreenIn
    WaitFadeScreen
    
    GoToIfEq VAR_0x8005, 255, _GuruMenu

    GuruCmd_CheckNature VAR_0x8005, VAR_0x8006
    GoToIfEq VAR_0x8006, 1, _GuruEgg

    GoToIfEq VAR_0x8008, 0, _GuruEVLoop

    BufferNumber 1, VAR_0x8008
    Message 83 
    ShowYesNoMenu VAR_RESULT
    CloseMessage
    GoToIfEq VAR_RESULT, 0, _GuruEVPay
    GoTo _GuruMenu

_GuruEVPay:
    GuruCmd_TakeMoney VAR_0x8008, VAR_RESULT
    Message 85
    GuruCmd_WaitAB
    CloseMessage
    GoTo _GuruEVLoop

_GuruEVLoop:
    Message 61 
    GuruCmd_InitListMenu 17, 1, 0, 1, VAR_0x8009
    AddListMenuEntry 62, 0, 255
    AddListMenuEntry 63, 1, 255
    AddListMenuEntry 64, 2, 255
    AddListMenuEntry 65, 3, 255
    AddListMenuEntry 66, 4, 255
    AddListMenuEntry 67, 5, 255
    AddListMenuEntry 68, 6, 255
    ShowListMenu
    CloseMessage

    @ LISTA BIANCA
    GoToIfEq VAR_0x8009, 0, _GuruEVValueLoop
    GoToIfEq VAR_0x8009, 1, _GuruEVValueLoop
    GoToIfEq VAR_0x8009, 2, _GuruEVValueLoop
    GoToIfEq VAR_0x8009, 3, _GuruEVValueLoop
    GoToIfEq VAR_0x8009, 4, _GuruEVValueLoop
    GoToIfEq VAR_0x8009, 5, _GuruEVValueLoop
    
    @ PARACADUTE
    GoTo _GuruMenu

_GuruEVValueLoop:
    GuruCmd_ModifyEV VAR_0x8005, VAR_0x8009, 1, 0, VAR_0x800A
    BufferNumber 0, VAR_0x800A

    Message 74 
    GuruCmd_InitListMenu 17, 1, 0, 1, VAR_RESULT
    AddListMenuEntry 75, 0, 255 
    AddListMenuEntry 76, 1, 255 
    AddListMenuEntry 77, 2, 255 
    AddListMenuEntry 78, 3, 255 
    AddListMenuEntry 79, 4, 255 
    AddListMenuEntry 80, 5, 255 
    AddListMenuEntry 68, 6, 255 
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
    GuruCmd_ModifyEV VAR_0x8005, VAR_0x8009, 1, 252, VAR_0x8006
    GoTo _EVCheckLimit
_EVAdd10:
    GuruCmd_ModifyEV VAR_0x8005, VAR_0x8009, 1, 10, VAR_0x8006
    GoTo _EVCheckLimit
_EVAdd1:
    GuruCmd_ModifyEV VAR_0x8005, VAR_0x8009, 1, 1, VAR_0x8006
    GoTo _EVCheckLimit
_EVSub1:
    GuruCmd_ModifyEV VAR_0x8005, VAR_0x8009, 0, 1, VAR_0x8006
    GoTo _EVCheckLimit
_EVSub10:
    GuruCmd_ModifyEV VAR_0x8005, VAR_0x8009, 0, 10, VAR_0x8006
    GoTo _EVCheckLimit
_EVSub252:
    GuruCmd_ModifyEV VAR_0x8005, VAR_0x8009, 0, 252, VAR_0x8006
    GoTo _EVCheckLimit

_EVCheckLimit:
    GoToIfEq VAR_0x8006, 1, _EVLimitReaced
    PlayFanfare SEQ_SE_CONFIRM
    Message 22
    GuruCmd_WaitAB
    CloseMessage
    GoTo _GuruEVValueLoop

_EVLimitReaced:
    Message 81 
    GuruCmd_WaitAB
    CloseMessage
    GoTo _GuruEVValueLoop

@ ==========================================================
@ ==================== ERRORI E USCITA =====================
@ ==========================================================
_GuruNoMoney:
    Message 23
    GuruCmd_WaitAB
    CloseMessage
    GoTo _GuruMenu

_GuruNoBadges:
    Message 24
    GuruCmd_WaitAB
    CloseMessage
    GoTo _GuruMenu

_GuruOneAbility:
    Message 25
    GuruCmd_WaitAB
    CloseMessage
    GoTo _GuruMenu

_GuruEgg:
    Message 26
    GuruCmd_WaitAB
    CloseMessage
    GoTo _GuruMenu

_GuruCancel:
    Message 27
    GuruCmd_WaitAB
    CloseMessage
    ReleaseAll
    End