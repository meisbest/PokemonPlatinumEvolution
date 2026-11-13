#include "trainer_data.h"

#include "constants/battle.h"
#include "constants/pokemon.h"
#include "generated/abilities.h"
#include "generated/natures.h"
#include "generated/trainer_message_types.h"

#include "struct_defs/trainer.h"

#include "data/trainer_class_genders.h"

#include "charcode_util.h"
#include "field_battle_data_transfer.h"
#include "heap.h"
#include "math_util.h"
#include "message.h"
#include "narc.h"
#include "party.h"
#include "pokemon.h"
#include "savedata.h"
#include "savedata_misc.h"
#include "string_gf.h"

static void TrainerData_BuildParty(FieldBattleDTO *dto, int battler, enum HeapID heapID);
static void TrainerData_ApplyEvFlags(Pokemon *mon, u16 evFlags);
static void TrainerData_ApplyAbilitySlot(Pokemon *mon, u16 species, u8 form, u8 abilitySlot);
static u32 TrainerData_ApplyNature(u32 personality, u8 nature);

void Trainer_Encounter(FieldBattleDTO *dto, const SaveData *saveData, enum HeapID heapID)
{
    Trainer trdata;
    MessageLoader *msgLoader = MessageLoader_Init(MSG_LOADER_LOAD_ON_DEMAND, NARC_INDEX_MSGDATA__PL_MSG, TEXT_BANK_NPC_TRAINER_NAMES, heapID);
    const charcode_t *rivalName = MiscSaveBlock_RivalName(SaveData_MiscSaveBlockConst(saveData));

    for (int i = 0; i < MAX_BATTLERS; i++) {
        if (!dto->trainerIDs[i]) {
            continue;
        }

        Trainer_Load(dto->trainerIDs[i], &trdata);
        dto->trainer[i] = trdata;

        if (trdata.header.trainerType == TRAINER_CLASS_RIVAL) {
            CharCode_Copy(dto->trainer[i].name, rivalName);
        } else {
            String *trainerName = MessageLoader_GetNewString(msgLoader, dto->trainerIDs[i]);
            String_ToChars(trainerName, dto->trainer[i].name, TRAINER_NAME_LEN + 1);
            String_Free(trainerName);
        }

        TrainerData_BuildParty(dto, i, heapID);
    }

    dto->battleType |= trdata.header.battleType;
    MessageLoader_Free(msgLoader);
}

u32 Trainer_LoadParam(int trainerID, enum TrainerDataParam paramID)
{
    // TODO: can this be trainerheader?
    u32 result;
    Trainer trdata;

    Trainer_Load(trainerID, &trdata);

    switch (paramID) {
    case TRDATA_TYPE:
        result = trdata.header.monDataType;
        break;

    case TRDATA_CLASS:
        result = trdata.header.trainerType;
        break;

    case TRDATA_SPRITE:
        result = trdata.header.sprite;
        break;

    case TRDATA_PARTY_SIZE:
        result = trdata.header.partySize;
        break;

    case TRDATA_ITEM_1:
    case TRDATA_ITEM_2:
    case TRDATA_ITEM_3:
    case TRDATA_ITEM_4:
        result = trdata.header.items[paramID - TRDATA_ITEM_1];
        break;

    case TRDATA_AI_MASK:
        result = trdata.header.aiMask;
        break;

    case TRDATA_BATTLE_TYPE:
        result = trdata.header.battleType;
        break;
    }

    return result;
}

BOOL Trainer_HasMessageType(int trainerID, enum TrainerMessageType msgType, enum HeapID heapID)
{
    NARC *narc; // must declare up here to match
    u16 offset, data[2];

    BOOL result = FALSE;
    int size = NARC_GetMemberSizeByIndexPair(NARC_INDEX_POKETOOL__TRMSG__TRTBL, 0);
    NARC_ReadFromMemberByIndexPair(&offset, NARC_INDEX_POKETOOL__TRMSG__TRTBLOFS, 0, trainerID * 2, 2);
    narc = NARC_ctor(NARC_INDEX_POKETOOL__TRMSG__TRTBL, heapID);

    while (offset != size) {
        NARC_ReadFromMember(narc, 0, offset, 4, data);

        if (data[0] == trainerID && data[1] == msgType) {
            result = TRUE;
            break;
        }

        if (data[0] != trainerID) {
            break;
        }

        offset += 4;
    }

    NARC_dtor(narc);
    return result;
}

void Trainer_LoadMessage(int trainerID, enum TrainerMessageType msgType, String *string, enum HeapID heapID)
{
    NARC *narc; // must declare up here to match
    u16 offset, data[2];

    int size = NARC_GetMemberSizeByIndexPair(NARC_INDEX_POKETOOL__TRMSG__TRTBL, 0);
    NARC_ReadFromMemberByIndexPair(&offset, NARC_INDEX_POKETOOL__TRMSG__TRTBLOFS, 0, trainerID * 2, 2);
    narc = NARC_ctor(NARC_INDEX_POKETOOL__TRMSG__TRTBL, heapID);

    while (offset != size) {
        NARC_ReadFromMember(narc, 0, offset, 4, data);

        if (data[0] == trainerID && data[1] == msgType) {
            MessageBank_GetStringFromNARC(NARC_INDEX_MSGDATA__PL_MSG, TEXT_BANK_NPC_TRAINER_MESSAGES, offset / 4, heapID, string);
            break;
        }

        offset += 4;
    }

    NARC_dtor(narc);

    if (offset == size) {
        String_Clear(string);
    }
}

void Trainer_Load(int trainerID, Trainer *trdata)
{
    NARC_ReadWholeMemberByIndexPair(trdata, NARC_INDEX_POKETOOL__TRAINER__TRDATA, trainerID);
}

void Trainer_LoadParty(int trainerID, void *trparty)
{
    NARC_ReadWholeMemberByIndexPair(trparty, NARC_INDEX_POKETOOL__TRAINER__TRPOKE, trainerID);
}

u8 TrainerClass_Gender(int trclass)
{
    return sTrainerClassGender[trclass];
}

static void TrainerData_ApplyEvFlags(Pokemon *mon, u16 evFlags)
{
    static const int sEvParams[] = {
        MON_DATA_HP_EV,
        MON_DATA_ATK_EV,
        MON_DATA_DEF_EV,
        MON_DATA_SPEED_EV,
        MON_DATA_SPATK_EV,
        MON_DATA_SPDEF_EV,
    };
    u8 evCount, evVal, k;

    if (!evFlags) {
        return;
    }

    evCount = 0;
    for (k = 0; k < 6; k++) {
        if (evFlags & (1 << k)) {
            evCount++;
        }
    }
    evVal = (510 / evCount) > 255 ? 255 : (u8)(510 / evCount);

    for (k = 0; k < 6; k++) {
        if (evFlags & (1 << k)) {
            Pokemon_SetValue(mon, sEvParams[k], &evVal);
        }
    }

    Pokemon_CalcLevelAndStats(mon);
}

// PPE: abilità scelta nel JSON (0 = prima, 1 = seconda). Se la specie ne ha una
// sola, si usa quella invece di lasciare il Pokémon senza abilità.
static void TrainerData_ApplyAbilitySlot(Pokemon *mon, u16 species, u8 form, u8 abilitySlot)
{
    u32 abilityVal;

    if (abilitySlot == TRAINER_MON_ABILITY_NONE) {
        return;
    }

    abilityVal = SpeciesData_GetFormValue(species, form, SPECIES_DATA_ABILITY_1);

    if (abilitySlot == 1) {
        u32 ability2 = SpeciesData_GetFormValue(species, form, SPECIES_DATA_ABILITY_2);

        if (ability2 != ABILITY_NONE) {
            abilityVal = ability2;
        }
    }

    Pokemon_SetValue(mon, MON_DATA_ABILITY, &abilityVal);
}

// PPE: porta il PID alla natura scelta nel JSON senza toccare il byte basso, che
// decide sesso (genderMod) e abilità (bit 0). Si aggiunge k * 256: 256 vale 6
// modulo 25 e 6 * 21 vale 1 modulo 25, quindi k = differenza * 21 (mod 25).
static u32 TrainerData_ApplyNature(u32 personality, u8 nature)
{
    u32 diff;

    if (nature == TRAINER_MON_NATURE_NONE) {
        return personality;
    }

    diff = (nature + NATURE_COUNT - personality % NATURE_COUNT) % NATURE_COUNT;

    return personality + ((diff * 21) % NATURE_COUNT) * 256;
}

/**
 * @brief Build the party for a trainer as loaded in the FieldBattleDTO struct.
 *
 * @param dto  The parent FieldBattleDTO struct containing trainer data.
 * @param battler       Which battler's party is to be loaded.
 * @param heapID        Heap on which to perform any allocations.
 */
static void TrainerData_BuildParty(FieldBattleDTO *dto, int battler, enum HeapID heapID)
{
    // must make declarations C89-style to match
    void *buf;
    int i, j;
    u32 genderMod, rnd, oldSeed;
    u8 ivs;
    Pokemon *mon;

    oldSeed = LCRNG_GetSeed();

    // alloc enough space to support the maximum possible data size
    Party_InitWithCapacity(dto->parties[battler], MAX_PARTY_SIZE);
    buf = Heap_Alloc(heapID, sizeof(TrainerMonWithMovesAndItem) * MAX_PARTY_SIZE);
    mon = Pokemon_New(heapID);

    Trainer_LoadParty(dto->trainerIDs[battler], buf);

    // determine which magic gender-specific modifier to use for the RNG function
    genderMod = TrainerClass_Gender(dto->trainer[battler].header.trainerType) == GENDER_FEMALE
        ? 120
        : 136;

    switch (dto->trainer[battler].header.monDataType) {
    case TRDATATYPE_BASE: {
        TrainerMonBase *trmon = (TrainerMonBase *)buf;
        for (i = 0; i < dto->trainer[battler].header.partySize; i++) {
            u16 species = trmon[i].species & 0x3FF;
            u8 form = (trmon[i].species & 0xFC00) >> TRAINER_MON_FORM_SHIFT;

            rnd = trmon[i].ivScale + trmon[i].level + species + dto->trainerIDs[battler];
            LCRNG_SetSeed(rnd);

            for (j = 0; j < dto->trainer[battler].header.trainerType; j++) {
                rnd = LCRNG_Next();
            }

            rnd = (rnd << 8) + genderMod;
            rnd = TrainerData_ApplyNature(rnd, trmon[i].nature);
            ivs = trmon[i].ivScale * MAX_IVS_SINGLE_STAT / MAX_IV_SCALE;

            Pokemon_InitWith(mon, species, trmon[i].level, ivs, TRUE, rnd, OTID_NOT_SHINY, 0);
            TrainerData_ApplyEvFlags(mon, trmon[i].evFlags);
            TrainerData_ApplyAbilitySlot(mon, species, form, trmon[i].abilitySlot);
            Pokemon_SetBallSeal(trmon[i].cbSeal, mon, heapID);
            Pokemon_SetValue(mon, MON_DATA_FORM, &form);
            Party_AddPokemon(dto->parties[battler], mon);
        }

        break;
    }

    case TRDATATYPE_WITH_MOVES: {
        TrainerMonWithMoves *trmon = (TrainerMonWithMoves *)buf;
        for (i = 0; i < dto->trainer[battler].header.partySize; i++) {
            u16 species = trmon[i].species & 0x3FF;
            u8 form = (trmon[i].species & 0xFC00) >> TRAINER_MON_FORM_SHIFT;

            rnd = trmon[i].ivScale + trmon[i].level + species + dto->trainerIDs[battler];
            LCRNG_SetSeed(rnd);

            for (j = 0; j < dto->trainer[battler].header.trainerType; j++) {
                rnd = LCRNG_Next();
            }

            rnd = (rnd << 8) + genderMod;
            rnd = TrainerData_ApplyNature(rnd, trmon[i].nature);
            ivs = trmon[i].ivScale * MAX_IVS_SINGLE_STAT / MAX_IV_SCALE;

            Pokemon_InitWith(mon, species, trmon[i].level, ivs, TRUE, rnd, OTID_NOT_SHINY, 0);
            TrainerData_ApplyEvFlags(mon, trmon[i].evFlags);
            TrainerData_ApplyAbilitySlot(mon, species, form, trmon[i].abilitySlot);

            for (j = 0; j < 4; j++) {
                Pokemon_SetMoveSlot(mon, trmon[i].moves[j], j);
            }

            Pokemon_SetBallSeal(trmon[i].cbSeal, mon, heapID);
            Pokemon_SetValue(mon, MON_DATA_FORM, &form);
            Party_AddPokemon(dto->parties[battler], mon);
        }

        break;
    }

    case TRDATATYPE_WITH_ITEM: {
        TrainerMonWithItem *trmon = (TrainerMonWithItem *)buf;
        for (i = 0; i < dto->trainer[battler].header.partySize; i++) {
            u16 species = trmon[i].species & 0x3FF;
            u8 form = (trmon[i].species & 0xFC00) >> TRAINER_MON_FORM_SHIFT;

            rnd = trmon[i].ivScale + trmon[i].level + species + dto->trainerIDs[battler];
            LCRNG_SetSeed(rnd);

            for (j = 0; j < dto->trainer[battler].header.trainerType; j++) {
                rnd = LCRNG_Next();
            }

            rnd = (rnd << 8) + genderMod;
            rnd = TrainerData_ApplyNature(rnd, trmon[i].nature);
            ivs = trmon[i].ivScale * MAX_IVS_SINGLE_STAT / MAX_IV_SCALE;

            Pokemon_InitWith(mon, species, trmon[i].level, ivs, TRUE, rnd, OTID_NOT_SHINY, 0);
            TrainerData_ApplyEvFlags(mon, trmon[i].evFlags);
            TrainerData_ApplyAbilitySlot(mon, species, form, trmon[i].abilitySlot);
            Pokemon_SetValue(mon, MON_DATA_HELD_ITEM, &trmon[i].item);
            Pokemon_SetBallSeal(trmon[i].cbSeal, mon, heapID);
            Pokemon_SetValue(mon, MON_DATA_FORM, &form);
            Party_AddPokemon(dto->parties[battler], mon);
        }

        break;
    }

    case TRDATATYPE_WITH_MOVES_AND_ITEM: {
        TrainerMonWithMovesAndItem *trmon = (TrainerMonWithMovesAndItem *)buf;
        for (i = 0; i < dto->trainer[battler].header.partySize; i++) {
            u16 species = trmon[i].species & 0x3FF;
            u8 form = (trmon[i].species & 0xFC00) >> TRAINER_MON_FORM_SHIFT;

            rnd = trmon[i].ivScale + trmon[i].level + species + dto->trainerIDs[battler];
            LCRNG_SetSeed(rnd);

            for (j = 0; j < dto->trainer[battler].header.trainerType; j++) {
                rnd = LCRNG_Next();
            }

            rnd = (rnd << 8) + genderMod;
            rnd = TrainerData_ApplyNature(rnd, trmon[i].nature);
            ivs = trmon[i].ivScale * MAX_IVS_SINGLE_STAT / MAX_IV_SCALE;

            Pokemon_InitWith(mon, species, trmon[i].level, ivs, TRUE, rnd, OTID_NOT_SHINY, 0);
            TrainerData_ApplyEvFlags(mon, trmon[i].evFlags);
            TrainerData_ApplyAbilitySlot(mon, species, form, trmon[i].abilitySlot);
            Pokemon_SetValue(mon, MON_DATA_HELD_ITEM, &trmon[i].item);

            for (j = 0; j < 4; j++) {
                Pokemon_SetMoveSlot(mon, trmon[i].moves[j], j);
            }

            Pokemon_SetBallSeal(trmon[i].cbSeal, mon, heapID);
            Pokemon_SetValue(mon, MON_DATA_FORM, &form);
            Party_AddPokemon(dto->parties[battler], mon);
        }

        break;
    }
    }

    Heap_Free(buf);
    Heap_Free(mon);
    LCRNG_SetSeed(oldSeed);
}
