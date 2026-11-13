#include "scrcmd_party.h"

#include <nitro.h>

#include "constants/heap.h"
#include "constants/moves.h"
#include "constants/items.h"
#include "generated/abilities.h"
#include "generated/natures.h"
#include "generated/species.h"

#include "field/field_system.h"
#include "overlay005/daycare.h"
#include "savedata/save_table.h"

#include "bag.h"
#include "field_script_context.h"
#include "field_system.h"
#include "heap.h"
#include "inlines.h"
#include "item.h"
#include "map_header.h"
#include "party.h"
#include "pc_boxes.h"
#include "pokemon.h"
#include "ribbon.h"
#include "save_player.h"
#include "string_template.h"
#include "trainer_info.h"
#include "tv_segment.h"
#include "unk_02017038.h"
#include "unk_02054884.h"
#include "unk_0205DFC4.h"

BOOL ScrCmd_GivePokemon(ScriptContext *ctx)
{
    int metLocation = MapHeader_GetMapLabelTextID(ctx->fieldSystem->location->mapHeaderID);
    int metTerrain = TERRAIN_MAX;
    FieldSystem *fieldSystem = ctx->fieldSystem;
    u16 species = ScriptContext_GetVar(ctx);
    u16 level = ScriptContext_GetVar(ctx);
    u16 heldItem = ScriptContext_GetVar(ctx);
    u16 *success = ScriptContext_GetVarPointer(ctx);

    Party *unused = SaveData_GetParty(fieldSystem->saveData);
    *success = Pokemon_GiveMonFromScript(HEAP_ID_FIELD2, fieldSystem->saveData, species, level, heldItem, metLocation, metTerrain);

    return FALSE;
}

BOOL ScrCmd_GetPartyMonSpecies(ScriptContext *ctx)
{
    FieldSystem *fieldSystem = ctx->fieldSystem;
    u16 *partySlot = ScriptContext_GetVarPointer(ctx);
    u16 *destVar = ScriptContext_GetVarPointer(ctx);

    Pokemon *mon = Party_GetPokemonBySlotIndex(SaveData_GetParty(fieldSystem->saveData), *partySlot);

    if (!Pokemon_GetValue(mon, MON_DATA_IS_EGG, NULL)) {
        *destVar = Pokemon_GetValue(mon, MON_DATA_SPECIES, NULL);
    } else {
        *destVar = SPECIES_NONE;
    }

    return FALSE;
}

BOOL ScrCmd_CheckIsPartyMonOutsider(ScriptContext *ctx)
{
    FieldSystem *fieldSystem = ctx->fieldSystem;
    TrainerInfo *info = SaveData_GetTrainerInfo(FieldSystem_GetSaveData(ctx->fieldSystem));
    u16 *slot = ScriptContext_GetVarPointer(ctx);
    u16 *destVar = ScriptContext_GetVarPointer(ctx);

    Pokemon *mon = Party_GetPokemonBySlotIndex(SaveData_GetParty(fieldSystem->saveData), *slot);
    u16 monOTID = Pokemon_GetValue(mon, MON_DATA_OT_ID, NULL);
    u16 trainerID = TrainerInfo_ID(info);

    if (monOTID == trainerID) {
        *destVar = FALSE;
    } else {
        *destVar = TRUE;
    }

    return FALSE;
}

BOOL ScrCmd_GiveEgg(ScriptContext *ctx)
{
    FieldSystem *fieldSystem = ctx->fieldSystem;
    TrainerInfo *trainer = SaveData_GetTrainerInfo(fieldSystem->saveData);
    u16 species = ScriptContext_GetVar(ctx);
    u16 eggGiver = ScriptContext_GetVar(ctx);

    Party *party = SaveData_GetParty(fieldSystem->saveData);
    u8 partyCount = Party_GetCurrentCount(party);

    if (partyCount < MAX_PARTY_SIZE) {
        Pokemon *egg = Pokemon_New(HEAP_ID_FIELD2);
        Pokemon_Init(egg);

        int specialMetLoc = SpecialMetLoc_GetId(1, eggGiver);
        Egg_CreateEgg(egg, species, 1, trainer, 3, specialMetLoc);

        Party_AddPokemon(party, egg);
        Heap_Free(egg);
    }

    return FALSE;
}

BOOL ScrCmd_ResetPartyMonMoveSlot_Unused(ScriptContext *ctx)
{
    u16 partySlot = ScriptContext_GetVar(ctx);
    u16 moveSlot = ScriptContext_GetVar(ctx);
    u16 moveID = ScriptContext_GetVar(ctx);

    Party_ResetMonMoveSlot(SaveData_GetParty(ctx->fieldSystem->saveData), partySlot, moveSlot, moveID);
    return FALSE;
}

BOOL ScrCmd_CheckPartyMonHasMove(ScriptContext *ctx)
{
    FieldSystem *fieldSystem = ctx->fieldSystem;
    u16 *destVar = ScriptContext_GetVarPointer(ctx);
    u16 move = ScriptContext_GetVar(ctx);
    u16 partySlot = ScriptContext_GetVar(ctx);

    // --- HACK QoL: MN DA ZAINO ---
    u16 hmItem = 0;
    
    // Lista VIP delle MN
    switch(move) {
        case MOVE_CUT: hmItem = ITEM_HM01; break;
        //case MOVE_FLY: hmItem = ITEM_HM02; break;
        //case MOVE_SURF: hmItem = ITEM_HM03; break;
        case MOVE_STRENGTH: hmItem = ITEM_HM04; break;
        //case MOVE_DEFOG: hmItem = ITEM_HM05; break;
        case MOVE_ROCK_SMASH: hmItem = ITEM_HM06; break;
        case MOVE_WATERFALL: hmItem = ITEM_HM07; break;
        case MOVE_ROCK_CLIMB: hmItem = ITEM_HM08; break;
    }
    
    // Se la mossa che lo script sta cercando è una MN...
    if (hmItem != 0) {
        Bag *bag = SaveData_GetBag(fieldSystem->saveData);
        
        // Controlliamo se hai l'oggetto fisicamente nella borsa
        if (Bag_GetItemQuantity(bag, hmItem, HEAP_ID_FIELD1) > 0) {
            
            // Mentiamo spudoratamente allo script!
            // Gli diciamo: "Sì, il Pokémon nello slot 0 (il tuo leader) conosce la mossa!"
            *destVar = TRUE; 
            return FALSE;
        }
    }

    Pokemon *mon = Party_GetPokemonBySlotIndex(SaveData_GetParty(fieldSystem->saveData), partySlot);
    *destVar = FALSE;

    if (Pokemon_GetValue(mon, MON_DATA_IS_EGG, NULL) != FALSE) {
        return FALSE;
    }

    if ((Pokemon_GetValue(mon, MON_DATA_MOVE1, NULL) == move)
        || (Pokemon_GetValue(mon, MON_DATA_MOVE2, NULL) == move)
        || (Pokemon_GetValue(mon, MON_DATA_MOVE3, NULL) == move)
        || (Pokemon_GetValue(mon, MON_DATA_MOVE4, NULL) == move)) {
        *destVar = TRUE;
    }

    return FALSE;
}

BOOL ScrCmd_FindPartySlotWithMove(ScriptContext *ctx)
{
    FieldSystem *fieldSystem = ctx->fieldSystem;
    u16 *destVar = ScriptContext_GetVarPointer(ctx);
    u16 move = ScriptContext_GetVar(ctx);

    // --- HACK QoL: MN DA ZAINO ---
    u16 hmItem = 0;
    
    // Lista VIP delle MN
    switch(move) {
        case MOVE_CUT: hmItem = ITEM_HM01; break;
        //case MOVE_FLY: hmItem = ITEM_HM02; break;
        //case MOVE_SURF: hmItem = ITEM_HM03; break;
        case MOVE_STRENGTH: hmItem = ITEM_HM04; break;
        //case MOVE_DEFOG: hmItem = ITEM_HM05; break;
        case MOVE_ROCK_SMASH: hmItem = ITEM_HM06; break;
        case MOVE_WATERFALL: hmItem = ITEM_HM07; break;
        case MOVE_ROCK_CLIMB: hmItem = ITEM_HM08; break;
    }
    
    // Se la mossa che lo script sta cercando è una MN...
    if (hmItem != 0) {
        Bag *bag = SaveData_GetBag(fieldSystem->saveData);
        
        // Controlliamo se hai l'oggetto fisicamente nella borsa
        if (Bag_GetItemQuantity(bag, hmItem, HEAP_ID_FIELD1) > 0) {
            
            // Mentiamo spudoratamente allo script!
            // Gli diciamo: "Sì, il Pokémon nello slot 0 (il tuo leader) conosce la mossa!"
            *destVar = 0; 
            return FALSE;
        }
    }

    u8 partyCount = Party_GetCurrentCount(SaveData_GetParty(fieldSystem->saveData));

    u8 slot;
    for (slot = 0, *destVar = MAX_PARTY_SIZE; slot < partyCount; slot++) {
        Pokemon *mon = Party_GetPokemonBySlotIndex(SaveData_GetParty(fieldSystem->saveData), slot);

        if (Pokemon_GetValue(mon, MON_DATA_IS_EGG, NULL) != FALSE) {
            continue;
        }

        if ((Pokemon_GetValue(mon, MON_DATA_MOVE1, NULL) == move)
            || (Pokemon_GetValue(mon, MON_DATA_MOVE2, NULL) == move)
            || (Pokemon_GetValue(mon, MON_DATA_MOVE3, NULL) == move)
            || (Pokemon_GetValue(mon, MON_DATA_MOVE4, NULL) == move)) {
            *destVar = slot;
            break;
        }
    }

    return FALSE;
}

BOOL ScrCmd_SurvivePoison(ScriptContext *ctx)
{
    u16 *destVar = ScriptContext_GetVarPointer(ctx);
    u16 partySlot = ScriptContext_GetVar(ctx);
    Pokemon *mon = Party_GetPokemonBySlotIndex(SaveData_GetParty(ctx->fieldSystem->saveData), partySlot);
    *destVar = Pokemon_TrySurvivePoison(mon);

    return FALSE;
}

BOOL ScrCmd_CountPartyMonsBelowLevelThreshold(ScriptContext *ctx)
{
    FieldSystem *fieldSystem = ctx->fieldSystem;
    u16 *destVar = ScriptContext_GetVarPointer(ctx);
    u16 threshold = ScriptContext_GetVar(ctx);
    u8 slot, count;

    u8 partyCount = Party_GetCurrentCount(SaveData_GetParty(fieldSystem->saveData));
    *destVar = 0;

    for (slot = 0, count = 0; slot < partyCount; slot++) {
        Pokemon *mon = Party_GetPokemonBySlotIndex(SaveData_GetParty(fieldSystem->saveData), slot);

        if (!Pokemon_GetValue(mon, MON_DATA_IS_EGG, NULL)) {
            if (Pokemon_GetValue(mon, MON_DATA_LEVEL, NULL) <= threshold) {
                count++;
            }
        }
    }

    *destVar = count;

    return FALSE;
}

BOOL ScrCmd_GetPartyMonLevel(ScriptContext *ctx)
{
    FieldSystem *fieldSystem = ctx->fieldSystem;
    u16 *destVar = ScriptContext_GetVarPointer(ctx);
    u16 slot = ScriptContext_GetVar(ctx);

    Pokemon *mon = Party_GetPokemonBySlotIndex(SaveData_GetParty(fieldSystem->saveData), slot);
    *destVar = 0;

    if (!Pokemon_GetValue(mon, MON_DATA_IS_EGG, NULL)) {
        *destVar = Pokemon_GetValue(mon, MON_DATA_LEVEL, NULL);
    }

    return FALSE;
}

BOOL ScrCmd_GetPartyMonNature(ScriptContext *ctx)
{
    FieldSystem *fieldSystem = ctx->fieldSystem;
    u16 *destVar = ScriptContext_GetVarPointer(ctx);
    u16 slot = ScriptContext_GetVar(ctx);
    u8 partyCount = Party_GetCurrentCount(SaveData_GetParty(fieldSystem->saveData));

    if (slot >= partyCount) {
        *destVar = NATURE_HARDY;
        return FALSE;
    }

    Pokemon *mon = Party_GetPokemonBySlotIndex(SaveData_GetParty(fieldSystem->saveData), slot);

    if (Pokemon_GetValue(mon, MON_DATA_IS_EGG, NULL) != FALSE) {
        *destVar = NATURE_HARDY;
        return FALSE;
    }

    *destVar = Pokemon_GetNature(mon);

    return FALSE;
}

BOOL ScrCmd_FindPartySlotWithNature(ScriptContext *ctx)
{
    FieldSystem *fieldSystem = ctx->fieldSystem;
    u16 *destVar = ScriptContext_GetVarPointer(ctx);
    u16 nature = ScriptContext_GetVar(ctx);

    u8 partyCount = Party_GetCurrentCount(SaveData_GetParty(fieldSystem->saveData));
    *destVar = 0xFF;

    for (u8 slot = 0; slot < partyCount; slot++) {
        Pokemon *mon = Party_GetPokemonBySlotIndex(SaveData_GetParty(fieldSystem->saveData), slot);

        if (!Pokemon_GetValue(mon, MON_DATA_IS_EGG, NULL)) {
            if (Pokemon_GetNature(mon) == nature) {
                *destVar = slot;
                break;
            }
        }
    }

    return FALSE;
}

BOOL ScrCmd_GetPartyMonFriendship(ScriptContext *ctx)
{
    FieldSystem *fieldSystem = ctx->fieldSystem;
    u16 *destVar = ScriptContext_GetVarPointer(ctx);
    u16 slot = ScriptContext_GetVar(ctx);

    Pokemon *mon = Party_GetPokemonBySlotIndex(SaveData_GetParty(fieldSystem->saveData), slot);
    *destVar = Pokemon_GetValue(mon, MON_DATA_FRIENDSHIP, NULL);

    return FALSE;
}

BOOL ScrCmd_IncreasePartyMonFriendship(ScriptContext *ctx)
{
    FieldSystem *fieldSystem = ctx->fieldSystem;
    u16 value = ScriptContext_GetVar(ctx);
    u16 slot = ScriptContext_GetVar(ctx);
    u32 location = MapHeader_GetMapLabelTextID(ctx->fieldSystem->location->mapHeaderID);

    Pokemon *mon = Party_GetPokemonBySlotIndex(SaveData_GetParty(fieldSystem->saveData), slot);
    u16 friendship = Pokemon_GetValue(mon, MON_DATA_FRIENDSHIP, NULL);

    if (value > 0) {
        if (Item_LoadParam(Pokemon_GetValue(mon, MON_DATA_HELD_ITEM, NULL), ITEM_PARAM_HOLD_EFFECT, HEAP_ID_FIELD2) == HOLD_EFFECT_FRIENDSHIP_UP) {
            value = value * 150 / 100;
        }

        if (Pokemon_GetValue(mon, MON_DATA_POKEBALL, NULL) == ITEM_LUXURY_BALL) {
            value++;
        }

        if (Pokemon_GetValue(mon, MON_DATA_EGG_LOCATION, NULL) == location) {
            value++;
        }
    }

    friendship += value;

    if (friendship > MAX_FRIENDSHIP_VALUE) {
        friendship = MAX_FRIENDSHIP_VALUE;
    }

    Pokemon_SetValue(mon, MON_DATA_FRIENDSHIP, &friendship);
    return FALSE;
}

BOOL ScrCmd_DecreasePartyMonFriendship_Unused(ScriptContext *ctx)
{
    FieldSystem *fieldSystem = ctx->fieldSystem;
    u16 value = ScriptContext_GetVar(ctx);
    u16 slot = ScriptContext_GetVar(ctx);

    Pokemon *mon = Party_GetPokemonBySlotIndex(SaveData_GetParty(fieldSystem->saveData), slot);
    u16 friendship = Pokemon_GetValue(mon, MON_DATA_FRIENDSHIP, NULL);

    if (value > friendship) {
        friendship = 0;
    } else {
        friendship -= value;
    }

    Pokemon_SetValue(mon, MON_DATA_FRIENDSHIP, &friendship);
    return FALSE;
}

BOOL ScrCmd_GetPartyMonContestStat(ScriptContext *ctx)
{
    u16 slot = ScriptContext_GetVar(ctx);
    u16 contestType = ScriptContext_GetVar(ctx);
    u16 *destVar = ScriptContext_GetVarPointer(ctx);

    Pokemon *mon = Party_GetPokemonBySlotIndex(SaveData_GetParty(ctx->fieldSystem->saveData), slot);
    *destVar = Pokemon_GetValue(mon, MON_DATA_COOL + contestType, NULL);

    return FALSE;
}

BOOL ScrCmd_GetFirstNonEggInParty(ScriptContext *ctx)
{
    u16 *destVar = ScriptContext_GetVarPointer(ctx);

    *destVar = SaveData_GetFirstNonEggInParty(ctx->fieldSystem->saveData);
    return FALSE;
}

BOOL ScrCmd_GetPartyMonType(ScriptContext *ctx)
{
    FieldSystem *fieldSystem = ctx->fieldSystem;
    u16 *type1DestVar = ScriptContext_GetVarPointer(ctx);
    u16 *type2DestVar = ScriptContext_GetVarPointer(ctx);
    u16 slot = ScriptContext_GetVar(ctx);

    Pokemon *mon = Party_GetPokemonBySlotIndex(SaveData_GetParty(fieldSystem->saveData), slot);
    *type1DestVar = Pokemon_GetValue(mon, MON_DATA_TYPE_1, NULL);
    *type2DestVar = Pokemon_GetValue(mon, MON_DATA_TYPE_2, NULL);

    return FALSE;
}

BOOL ScrCmd_GetPartyCount(ScriptContext *ctx)
{
    FieldSystem *fieldSystem = ctx->fieldSystem;
    u16 *destVar = ScriptContext_GetVarPointer(ctx);

    *destVar = Party_GetCurrentCount(SaveData_GetParty(fieldSystem->saveData));
    return FALSE;
}

BOOL ScrCmd_CountPartyNonEggs(ScriptContext *ctx)
{
    int count, i;
    FieldSystem *fieldSystem = ctx->fieldSystem;
    u16 *destVar = ScriptContext_GetVarPointer(ctx);
    int partyCount = Party_GetCurrentCount(SaveData_GetParty(fieldSystem->saveData));

    for (i = 0, count = 0; i < partyCount; i++) {
        Pokemon *mon = Party_GetPokemonBySlotIndex(SaveData_GetParty(fieldSystem->saveData), i);

        if (!Pokemon_GetValue(mon, MON_DATA_IS_EGG, NULL)) {
            count++;
        }
    }

    *destVar = count;
    return FALSE;
}

BOOL ScrCmd_CountAliveMonsExcept(ScriptContext *ctx)
{
    int aliveCount, slot;
    FieldSystem *fieldSystem = ctx->fieldSystem;
    u16 *destVar = ScriptContext_GetVarPointer(ctx);
    u16 excludedSlot = ScriptContext_GetVar(ctx);
    int partyCount = Party_GetCurrentCount(SaveData_GetParty(fieldSystem->saveData));

    for (slot = 0, aliveCount = 0; slot < partyCount; slot++) {
        if (slot == excludedSlot) {
            continue;
        }

        Pokemon *mon = Party_GetPokemonBySlotIndex(SaveData_GetParty(fieldSystem->saveData), slot);

        if (!Pokemon_GetValue(mon, MON_DATA_IS_EGG, NULL)
            && Pokemon_GetValue(mon, MON_DATA_HP, NULL) != 0) {
            aliveCount++;
        }
    }

    *destVar = aliveCount;
    return FALSE;
}

BOOL ScrCmd_CountAliveMonsAndBoxMons(ScriptContext *ctx)
{
    int partyCount, count, slot;
    FieldSystem *fieldSystem = ctx->fieldSystem;
    u16 *destVar = ScriptContext_GetVarPointer(ctx);
    PCBoxes *pcBoxes = SaveData_GetPCBoxes(fieldSystem->saveData);

    partyCount = Party_GetCurrentCount(SaveData_GetParty(fieldSystem->saveData));

    for (slot = 0, count = 0; slot < partyCount; slot++) {
        Pokemon *mon = Party_GetPokemonBySlotIndex(SaveData_GetParty(fieldSystem->saveData), slot);

        if (!Pokemon_GetValue(mon, MON_DATA_IS_EGG, NULL)
            && Pokemon_GetValue(mon, MON_DATA_HP, NULL) != 0) {
            count++;
        }
    }

    count += PCBoxes_CountAllNonEggBoxMons(pcBoxes);
    *destVar = count;

    return FALSE;
}

BOOL ScrCmd_CountPartyEggs(ScriptContext *ctx)
{
    int count, slot;
    FieldSystem *fieldSystem = ctx->fieldSystem;
    u16 *destVar = ScriptContext_GetVarPointer(ctx);

    int partyCount = Party_GetCurrentCount(SaveData_GetParty(fieldSystem->saveData));

    for (slot = 0, count = 0; slot < partyCount; slot++) {
        Pokemon *mon = Party_GetPokemonBySlotIndex(SaveData_GetParty(fieldSystem->saveData), slot);

        if (Pokemon_GetValue(mon, MON_DATA_IS_EGG, NULL)) {
            count++;
        }
    }

    *destVar = count;

    return FALSE;
}

BOOL ScrCmd_CheckPartyPokerus(ScriptContext *ctx)
{
    u16 slot, partyCount;
    FieldSystem *fieldSystem = ctx->fieldSystem;
    u16 *destVar = ScriptContext_GetVarPointer(ctx);
    partyCount = Party_GetCurrentCount(SaveData_GetParty(fieldSystem->saveData));
    *destVar = FALSE;

    for (slot = 0; slot < partyCount; slot++) {
        Pokemon *mon = Party_GetPokemonBySlotIndex(SaveData_GetParty(fieldSystem->saveData), slot);

        if (Pokemon_GetValue(mon, MON_DATA_POKERUS, NULL) != FALSE) {
            *destVar = TRUE;
            break;
        }
    }

    return FALSE;
}

BOOL ScrCmd_GetPartyMonGender_Unused(ScriptContext *ctx)
{
    u16 slot = ScriptContext_GetVar(ctx);
    u16 *destVar = ScriptContext_GetVarPointer(ctx);

    Pokemon *mon = Party_GetPokemonBySlotIndex(SaveData_GetParty(ctx->fieldSystem->saveData), slot);
    *destVar = Pokemon_GetValue(mon, MON_DATA_GENDER, NULL);

    return FALSE;
}

BOOL ScrCmd_GetPartyMonMoveCount(ScriptContext *ctx)
{
    FieldSystem *fieldSystem = ctx->fieldSystem;
    u16 *destVar = ScriptContext_GetVarPointer(ctx);
    u16 partySlot = ScriptContext_GetVar(ctx);

    Pokemon *mon = Party_GetPokemonBySlotIndex(SaveData_GetParty(fieldSystem->saveData), partySlot);

    if (Pokemon_GetValue(mon, MON_DATA_IS_EGG, NULL) != FALSE) {
        *destVar = 0;
        return FALSE;
    }

    u8 count = 0;

    if (Pokemon_GetValue(mon, MON_DATA_MOVE1, NULL) != MOVE_NONE) {
        count++;
    }

    if (Pokemon_GetValue(mon, MON_DATA_MOVE2, NULL) != MOVE_NONE) {
        count++;
    }

    if (Pokemon_GetValue(mon, MON_DATA_MOVE3, NULL) != MOVE_NONE) {
        count++;
    }

    if (Pokemon_GetValue(mon, MON_DATA_MOVE4, NULL) != MOVE_NONE) {
        count++;
    }

    *destVar = count;
    return FALSE;
}

BOOL ScrCmd_ClearPartyMonMoveSlot(ScriptContext *ctx)
{
    FieldSystem *fieldSystem = ctx->fieldSystem;
    u16 partySlot = ScriptContext_GetVar(ctx);
    u16 moveSlot = ScriptContext_GetVar(ctx);

    Pokemon *mon = Party_GetPokemonBySlotIndex(SaveData_GetParty(fieldSystem->saveData), partySlot);
    Pokemon_ClearMoveSlot(mon, moveSlot);

    return FALSE;
}

BOOL ScrCmd_GetPartyMonMove(ScriptContext *ctx)
{
    FieldSystem *fieldSystem = ctx->fieldSystem;
    u16 *destVar = ScriptContext_GetVarPointer(ctx);
    u16 partySlot = ScriptContext_GetVar(ctx);
    u16 moveSlot = ScriptContext_GetVar(ctx);

    Pokemon *mon = Party_GetPokemonBySlotIndex(SaveData_GetParty(fieldSystem->saveData), partySlot);
    *destVar = Pokemon_GetValue(mon, MON_DATA_MOVE1 + moveSlot, NULL);

    return FALSE;
}

BOOL ScrCmd_GetPartyMonHeldItem_Unused(ScriptContext *ctx)
{
    FieldSystem *fieldSystem = ctx->fieldSystem;
    u16 *destVar = ScriptContext_GetVarPointer(ctx);
    u16 slot = ScriptContext_GetVar(ctx);

    Pokemon *mon = Party_GetPokemonBySlotIndex(SaveData_GetParty(fieldSystem->saveData), slot);
    *destVar = Pokemon_GetValue(mon, MON_DATA_HELD_ITEM, NULL);

    return FALSE;
}

BOOL ScrCmd_DeletePartyMonHeldItem_Unused(ScriptContext *ctx)
{
    FieldSystem *fieldSystem = ctx->fieldSystem;
    u16 slot = ScriptContext_GetVar(ctx);

    Pokemon *mon = Party_GetPokemonBySlotIndex(SaveData_GetParty(fieldSystem->saveData), slot);

    u16 value = ITEM_NONE;
    Pokemon_SetValue(mon, MON_DATA_HELD_ITEM, &value);

    return FALSE;
}

BOOL ScrCmd_CheckPartyHasSpecies(ScriptContext *ctx)
{
    FieldSystem *fieldSystem = ctx->fieldSystem;
    u16 *destVar = ScriptContext_GetVarPointer(ctx);
    u16 species = ScriptContext_GetVar(ctx);

    Party *party = SaveData_GetParty(fieldSystem->saveData);
    u8 partyCount = Party_GetCurrentCount(party);
    *destVar = FALSE;

    for (u8 slot = 0; slot < partyCount; slot++) {
        Pokemon *mon = Party_GetPokemonBySlotIndex(SaveData_GetParty(fieldSystem->saveData), slot);

        if (!Pokemon_GetValue(mon, MON_DATA_IS_EGG, NULL)) {
            u16 partySpecies = Pokemon_GetValue(mon, MON_DATA_SPECIES, NULL);

            if (species == partySpecies) {
                *destVar = TRUE;
                break;
            }
        }
    }

    return FALSE;
}

// If you pass SPECIES_NONE, it returns 1 to check if there's repeated species.
// This is not used in vanilla, however.
BOOL ScrCmd_CountRepeatedSpeciesInParty(ScriptContext *ctx)
{
    FieldSystem *fieldSystem = ctx->fieldSystem;
    u16 *destVar = ScriptContext_GetVarPointer(ctx);
    u16 species = ScriptContext_GetVar(ctx);
    u8 i;
    u16 partySpeciesArray[MAX_PARTY_SIZE];

    for (i = 0; i < MAX_PARTY_SIZE; i++) {
        partySpeciesArray[i] = SPECIES_NONE;
    }

    Party *party = SaveData_GetParty(fieldSystem->saveData);
    u8 partyCount = Party_GetCurrentCount(party);
    *destVar = 0;

    for (i = 0; i < partyCount; i++) {
        Pokemon *mon = Party_GetPokemonBySlotIndex(SaveData_GetParty(fieldSystem->saveData), i);

        if (!Pokemon_GetValue(mon, MON_DATA_IS_EGG, NULL)) {
            u16 partySpecies = Pokemon_GetValue(mon, MON_DATA_SPECIES, NULL);

            if (species == SPECIES_NONE) {
                partySpeciesArray[i] = partySpecies;

                for (u8 j = 0; j < i; j++) {
                    if (partySpeciesArray[j] == partySpeciesArray[i]) {
                        *destVar = 1;
                        return FALSE;
                    }
                }
            } else {
                if (species == partySpecies) {
                    *destVar += 1;
                }
            }
        }
    }

    return FALSE;
}

BOOL ScrCmd_FindPartySlotWithSpecies(ScriptContext *ctx)
{
    FieldSystem *fieldSystem = ctx->fieldSystem;
    u16 *destVar = ScriptContext_GetVarPointer(ctx);
    u16 species = ScriptContext_GetVar(ctx);

    Party *party = SaveData_GetParty(fieldSystem->saveData);
    u8 partyCount = Party_GetCurrentCount(party);
    *destVar = 0xFF;

    for (u8 slot = 0; slot < partyCount; slot++) {
        Pokemon *mon = Party_GetPokemonBySlotIndex(SaveData_GetParty(fieldSystem->saveData), slot);

        if (!Pokemon_GetValue(mon, MON_DATA_IS_EGG, NULL)) {
            u16 partySpecies = Pokemon_GetValue(mon, MON_DATA_SPECIES, NULL);

            if (species == partySpecies) {
                *destVar = slot;
                break;
            }
        }
    }

    return FALSE;
}

BOOL ScrCmd_CountPartyMonRibbons_Unused(ScriptContext *ctx)
{
    u16 *destVar = ScriptContext_GetVarPointer(ctx);
    u16 slot = ScriptContext_GetVar(ctx);
    u16 ribbonID, count;

    Pokemon *mon = Party_GetPokemonBySlotIndex(SaveData_GetParty(ctx->fieldSystem->saveData), slot);

    for (ribbonID = 0, count = 0; ribbonID < RIBBON_MAX; ribbonID++) {
        if (Pokemon_GetValue(mon, Ribbon_GetData(ribbonID, RIBBON_DATA_MON_DATA_PARAM), NULL) != FALSE) {
            count++;
        }
    }

    *destVar = count;

    return FALSE;
}

BOOL ScrCmd_CountPartyRibbons(ScriptContext *ctx)
{
    u16 *destVar = ScriptContext_GetVarPointer(ctx);

    u16 partyCount = Party_GetCurrentCount(SaveData_GetParty(ctx->fieldSystem->saveData));
    Party *party = SaveData_GetParty(ctx->fieldSystem->saveData);

    u16 ribbonID, count;
    for (ribbonID = 0, count = 0; ribbonID < RIBBON_MAX; ribbonID++) {
        for (u16 slot = 0; slot < partyCount; slot++) {
            Pokemon *mon = Party_GetPokemonBySlotIndex(party, slot);

            if (!Pokemon_GetValue(mon, MON_DATA_IS_EGG, NULL)
                && Pokemon_GetValue(mon, Ribbon_GetData(ribbonID, RIBBON_DATA_MON_DATA_PARAM), NULL) != FALSE) {
                count++;
                break;
            }
        }
    }

    *destVar = count;

    return FALSE;
}

BOOL ScrCmd_GetPartyMonRibbon(ScriptContext *ctx)
{
    u16 *hasRibbonVar = ScriptContext_GetVarPointer(ctx);
    u16 slot = ScriptContext_GetVar(ctx);
    u16 ribbonID = ScriptContext_GetVar(ctx);
    u8 unused = TRUE;

    Pokemon *mon = Party_GetPokemonBySlotIndex(SaveData_GetParty(ctx->fieldSystem->saveData), slot);
    *hasRibbonVar = Pokemon_GetValue(mon, Ribbon_GetData(ribbonID, RIBBON_DATA_MON_DATA_PARAM), NULL);

    return FALSE;
}

BOOL ScrCmd_SetPartyMonRibbon(ScriptContext *ctx)
{
    u16 slot = ScriptContext_GetVar(ctx);
    u16 ribbonID = ScriptContext_GetVar(ctx);
    u8 value = TRUE;

    Pokemon *mon = Party_GetPokemonBySlotIndex(SaveData_GetParty(ctx->fieldSystem->saveData), slot);

    Pokemon_SetValue(mon, Ribbon_GetData(ribbonID, RIBBON_DATA_MON_DATA_PARAM), &value);
    TVSegment_Save_RibbonMaster(ctx->fieldSystem->saveData, mon, Ribbon_GetData(ribbonID, RIBBON_DATA_MON_DATA_PARAM));

    return FALSE;
}

BOOL ScrCmd_CheckPartyHasBadEgg(ScriptContext *ctx)
{
    u16 *destVar = ScriptContext_GetVarPointer(ctx);

    u16 partyCount = Party_GetCurrentCount(SaveData_GetParty(ctx->fieldSystem->saveData));
    Party *party = SaveData_GetParty(ctx->fieldSystem->saveData);

    // bug: left in from copying ribbon code
    for (u16 i = 0; i < RIBBON_MAX; i++) {
        for (u16 slot = 0; slot < partyCount; slot++) {
            Pokemon *mon = Party_GetPokemonBySlotIndex(party, slot);

            if (Pokemon_GetValue(mon, MON_DATA_IS_EGG, NULL)) {
                if (Pokemon_GetValue(mon, MON_DATA_CHECKSUM_FAILED, NULL)) {
                    *destVar = TRUE;
                    return FALSE;
                }
            }
        }
    }

    *destVar = FALSE;

    return FALSE;
}

BOOL ScrCmd_Dummy0A0(ScriptContext *ctx)
{
    return FALSE;
}

BOOL ScrCmd_FindPartySlotWithFatefulEncounterSpecies(ScriptContext *ctx)
{
    u8 partyCount;
    FieldSystem *fieldSystem = ctx->fieldSystem;
    u16 *destVar = ScriptContext_GetVarPointer(ctx);
    u16 species = ScriptContext_GetVar(ctx);

    Party *party = SaveData_GetParty(fieldSystem->saveData);
    partyCount = Party_GetCurrentCount(party);
    *destVar = 0xFF;

    for (u8 slot = 0; slot < partyCount; slot++) {
        Pokemon *mon = Party_GetPokemonBySlotIndex(SaveData_GetParty(fieldSystem->saveData), slot);

        if (!Pokemon_GetValue(mon, MON_DATA_IS_EGG, NULL)) {
            u16 partySpecies = Pokemon_GetValue(mon, MON_DATA_SPECIES, NULL);

            if (species == partySpecies) {
                if (Pokemon_GetValue(mon, MON_DATA_FATEFUL_ENCOUNTER, NULL) == TRUE) {
                    *destVar = slot;
                    break;
                }
            }
        }
    }

    return FALSE;
}

BOOL ScrCmd_CheckPartyHasHeldItem(ScriptContext *ctx)
{
    int slot;
    FieldSystem *fieldSystem = ctx->fieldSystem;
    u16 item = ScriptContext_GetVar(ctx);
    u16 *destVar = ScriptContext_GetVarPointer(ctx);

    Party *party = SaveData_GetParty(fieldSystem->saveData);
    int partyCount = Party_GetCurrentCount(party);
    *destVar = FALSE;

    for (slot = 0; slot < partyCount; slot++) {
        Pokemon *mon = Party_GetPokemonBySlotIndex(SaveData_GetParty(fieldSystem->saveData), slot);

        if (Pokemon_GetValue(mon, MON_DATA_HELD_ITEM, NULL) == item) {
            *destVar = TRUE;
            break;
        }
    }

    return FALSE;
}


// --- SEZIONE GURU LOTTA

BOOL ScrCmd_GetGuruPrice(ScriptContext *ctx)
{
    FieldSystem *fieldSystem = ctx->fieldSystem;
    
    // Lo script ci passa il servizio (0 = Abilità/IV, 1 = Natura, 2 = EV)
    u16 serviceType = ScriptContext_ReadHalfWord(ctx);
    
    // Lo script ci passa la variabile dove salvare il prezzo finale
    u16 *priceDest = ScriptContext_GetVarPointer(ctx); 

    TrainerInfo *trainerInfo = SaveData_GetTrainerInfo(fieldSystem->saveData);

    // 1. Ha battuto la lega? Tutto gratis!
    if (TrainerInfo_IsMainStoryCleared(trainerInfo) == TRUE) {
        *priceDest = 0;
        return FALSE;
    }

    // 2. Contiamo le medaglie
    int badges = TrainerInfo_BadgeCount(trainerInfo);

    // 3. Ha meno di 4 medaglie? Prezzo "proibitivo" per bloccarlo
    if (badges < 4) {
        *priceDest = 65000; // Lo script gestirà il rifiuto se non ha i soldi
        return FALSE;
    }

    // 4. Calcolo dello sconto dinamico
    switch (serviceType) {
        case 0: // Abilità e IVs (Base 10.000, -2000 a medaglia)
            *priceDest = 10000 - ((badges - 4) * 2000);
            break;
        case 1: // Natura (Base 15.000, -3000 a medaglia)
            *priceDest = 15000 - ((badges - 4) * 3000);
            break;
        case 2: // EVs (Base 35.000, -5000 a medaglia)
            *priceDest = 35000 - ((badges - 4) * 5000);
            break;
        case 3: // Reset EVs
            *priceDest = 0;
            break;
    }

    return FALSE;
}

// PPE: abilità della specie, tenendo conto della forma (come Pokemon_CalcAbility)
static void Guru_GetSpeciesAbilities(Pokemon *mon, u32 *ability1, u32 *ability2)
{
    int species = Pokemon_GetValue(mon, MON_DATA_SPECIES, NULL);
    int form = Pokemon_GetValue(mon, MON_DATA_FORM, NULL);

    *ability1 = SpeciesData_GetFormValue(species, form, SPECIES_DATA_ABILITY_1);
    *ability2 = SpeciesData_GetFormValue(species, form, SPECIES_DATA_ABILITY_2);
}

// PPE: slot (0 o 1) dell'abilità che il Pokémon ha ADESSO.
// Non si usa il bit 0 del PID perché i Pokémon cambiati dalla vecchia versione
// del Guru hanno l'abilità scritta a mano, non coerente con il PID.
static u8 Guru_GetCurrentAbilitySlot(Pokemon *mon)
{
    u32 ability1, ability2;
    Guru_GetSpeciesAbilities(mon, &ability1, &ability2);

    if (ability2 != ABILITY_NONE && Pokemon_GetValue(mon, MON_DATA_ABILITY, NULL) == ability2) {
        return 1;
    }

    return 0;
}

// PPE: sceglie un nuovo PID con la natura e lo slot di abilità richiesti,
// tenendo uguali sesso, cromaticità e ordine dei blocchi dati, poi ricalcola
// abilità e statistiche. L'abilità dipende dal bit 0 del PID: upstream la
// ricalcola da lì all'evoluzione (Pokemon_CalcAbility in evolution.c), quindi
// cambiarla solo con MON_DATA_ABILITY non basta.
static void Guru_SetPersonality(Pokemon *mon, u8 desiredNature, u8 abilitySlot)
{
    u16 species = Pokemon_GetValue(mon, MON_DATA_SPECIES, NULL);
    u32 oldPid = Pokemon_GetValue(mon, MON_DATA_PERSONALITY, NULL);
    u32 otid = Pokemon_GetValue(mon, MON_DATA_OT_ID, NULL);
    u8 oldGender = Pokemon_GetValue(mon, MON_DATA_GENDER, NULL);
    u8 isShiny = Pokemon_IsPersonalityShiny(otid, oldPid);

    // Caricati una volta sola: il ciclo può fare centinaia di tentativi
    SpeciesData *speciesData = SpeciesData_FromMonSpecies(species, HEAP_ID_SYSTEM);

    // I bit 13-17 del PID decidono l'ordine dei blocchi dati cifrati:
    // tenerli uguali evita di dover rimescolare i blocchi.
    u32 blockShuffleBits = oldPid & 0x3E000;

    u32 newPid;

    while (TRUE) {
        newPid = (LCRNG_Next() << 16) | LCRNG_Next();
        newPid = (newPid & ~0x3E000) | blockShuffleBits;

        if (Pokemon_GetNatureOf(newPid) != desiredNature) continue;
        if ((newPid & 1) != abilitySlot) continue;
        if (SpeciesData_GetGenderOf(speciesData, species, newPid) != oldGender) continue;
        if (Pokemon_IsPersonalityShiny(otid, newPid) != isShiny) continue;

        break;
    }

    SpeciesData_Free(speciesData);

    Pokemon_SetValue(mon, MON_DATA_PERSONALITY, &newPid);
    Pokemon_CalcAbility(mon);
    Pokemon_CalcLevelAndStats(mon);
}

BOOL ScrCmd_CheckAbilitySwap(ScriptContext *ctx)
{
    FieldSystem *fieldSystem = ctx->fieldSystem;
    u16 partySlot = ScriptContext_GetVar(ctx);
    u16 *statusVar = ScriptContext_GetVarPointer(ctx); // 0 = OK, 1 = Uovo, 2 = Solo 1 abilità
    u16 *newAbilityVar = ScriptContext_GetVarPointer(ctx); // nuova abilità, da mostrare a schermo

    Pokemon *mon = Party_GetPokemonBySlotIndex(SaveData_GetParty(fieldSystem->saveData), partySlot);

    if (Pokemon_GetValue(mon, MON_DATA_IS_EGG, NULL) != FALSE) {
        *statusVar = 1;
        return FALSE;
    }

    u32 ability1, ability2;
    Guru_GetSpeciesAbilities(mon, &ability1, &ability2);

    if (ability1 == ability2 || ability2 == ABILITY_NONE) {
        *statusVar = 2;
        return FALSE;
    }

    *statusVar = 0;
    *newAbilityVar = Guru_GetCurrentAbilitySlot(mon) == 0 ? ability2 : ability1;

    StringTemplate **strTemplate = FieldSystem_GetScriptMemberPtr(ctx->fieldSystem, SCRIPT_MANAGER_STR_TEMPLATE);
    StringTemplate_SetAbilityName(*strTemplate, 0, *newAbilityVar);

    return FALSE;
}

BOOL ScrCmd_ApplyAbilitySwap(ScriptContext *ctx)
{
    FieldSystem *fieldSystem = ctx->fieldSystem;
    u16 partySlot = ScriptContext_GetVar(ctx);
    u16 newAbility = ScriptContext_GetVar(ctx);

    Pokemon *mon = Party_GetPokemonBySlotIndex(SaveData_GetParty(fieldSystem->saveData), partySlot);

    u32 ability1, ability2;
    Guru_GetSpeciesAbilities(mon, &ability1, &ability2);
    u8 newSlot = (ability2 != ABILITY_NONE && newAbility == ability2) ? 1 : 0;

    Guru_SetPersonality(mon, Pokemon_GetNature(mon), newSlot);

    return FALSE;
}

BOOL ScrCmd_CheckNatureSwap(ScriptContext *ctx)
{
    FieldSystem *fieldSystem = ctx->fieldSystem;
    u16 partySlot = ScriptContext_GetVar(ctx);
    u16 *statusVar = ScriptContext_GetVarPointer(ctx);

    Pokemon *mon = Party_GetPokemonBySlotIndex(SaveData_GetParty(fieldSystem->saveData), partySlot);

    if (Pokemon_GetValue(mon, MON_DATA_IS_EGG, NULL) != FALSE) {
        *statusVar = 1; // È un uovo
    } else {
        *statusVar = 0; // Ok!
    }

    return FALSE;
}

BOOL ScrCmd_ApplyNatureSwap(ScriptContext *ctx)
{
    FieldSystem *fieldSystem = ctx->fieldSystem;
    u16 partySlot = ScriptContext_GetVar(ctx);
    u16 desiredNature = ScriptContext_GetVar(ctx);

    Pokemon *mon = Party_GetPokemonBySlotIndex(SaveData_GetParty(fieldSystem->saveData), partySlot);

    Guru_SetPersonality(mon, desiredNature, Guru_GetCurrentAbilitySlot(mon));

    return FALSE;
}

// PPE: il Guru porta gli EV al massimo utile (252), non al limite tecnico
// MAX_EVS_SINGLE_STAT (255): con 255 l'ultimo punto non dà statistiche.
#define GURU_MAX_EVS_SINGLE_STAT 252

// Gli indici delle statistiche nello script sono i valori di enum PokemonStat
// (STAT_HP...STAT_SPECIAL_DEFENSE). I campi MON_DATA_*_EV e MON_DATA_*_IV sono
// nello stesso ordine, quindi basta sommare l'indice al primo campo.
static u32 Guru_GetEVTotal(Pokemon *mon)
{
    u32 total = 0;

    for (int stat = STAT_HP; stat < STAT_MAX; stat++) {
        total += Pokemon_GetValue(mon, MON_DATA_HP_EV + stat, NULL);
    }

    return total;
}

BOOL ScrCmd_ResetEVs(ScriptContext *ctx)
{
    FieldSystem *fieldSystem = ctx->fieldSystem;
    u16 partySlot = ScriptContext_GetVar(ctx);
    Pokemon *mon = Party_GetPokemonBySlotIndex(SaveData_GetParty(fieldSystem->saveData), partySlot);

    u32 zero = 0;

    for (int stat = STAT_HP; stat < STAT_MAX; stat++) {
        Pokemon_SetValue(mon, MON_DATA_HP_EV + stat, &zero);
    }

    Pokemon_CalcLevelAndStats(mon);
    return FALSE;
}

// Imposta un IV (0-31). statIndex = STAT_*
BOOL ScrCmd_SetIV(ScriptContext *ctx)
{
    FieldSystem *fieldSystem = ctx->fieldSystem;
    u16 partySlot = ScriptContext_GetVar(ctx);
    u16 statIndex = ScriptContext_GetVar(ctx);
    u16 value = ScriptContext_ReadHalfWord(ctx);

    if (statIndex >= STAT_MAX) {
        return FALSE;
    }

    Pokemon *mon = Party_GetPokemonBySlotIndex(SaveData_GetParty(fieldSystem->saveData), partySlot);

    u32 iv = value;
    if (iv > MAX_IVS_SINGLE_STAT) {
        iv = MAX_IVS_SINGLE_STAT;
    }

    Pokemon_SetValue(mon, MON_DATA_HP_IV + statIndex, &iv);
    Pokemon_CalcLevelAndStats(mon);
    return FALSE;
}

// Aggiunge o toglie EV. statIndex = STAT_*. Con amount 0 legge soltanto:
// statusVar riceve il valore attuale. Altrimenti statusVar = 1 se il limite
// (252 per statistica, MAX_EVS_ALL_STATS in totale, o 0) ha ridotto la modifica.
BOOL ScrCmd_ModifyEV(ScriptContext *ctx)
{
    FieldSystem *fieldSystem = ctx->fieldSystem;
    u16 partySlot = ScriptContext_GetVar(ctx);
    u16 statIndex = ScriptContext_GetVar(ctx);
    u16 isAdd = ScriptContext_ReadHalfWord(ctx);
    u16 amount = ScriptContext_ReadHalfWord(ctx);
    u16 *statusVar = ScriptContext_GetVarPointer(ctx);

    if (statIndex >= STAT_MAX) {
        *statusVar = 0;
        return FALSE;
    }

    Pokemon *mon = Party_GetPokemonBySlotIndex(SaveData_GetParty(fieldSystem->saveData), partySlot);
    int monData = MON_DATA_HP_EV + statIndex;
    u32 currentEV = Pokemon_GetValue(mon, monData, NULL);

    if (amount == 0) {
        *statusVar = currentEV;
        return FALSE;
    }

    u32 totalEV = Guru_GetEVTotal(mon);
    u32 newVal;

    *statusVar = 0;

    if (isAdd) {
        u32 availableStat = (currentEV < GURU_MAX_EVS_SINGLE_STAT) ? (GURU_MAX_EVS_SINGLE_STAT - currentEV) : 0;
        u32 availableTotal = (totalEV < MAX_EVS_ALL_STATS) ? (MAX_EVS_ALL_STATS - totalEV) : 0;
        u32 maxAdd = (availableStat < availableTotal) ? availableStat : availableTotal;

        if (amount > maxAdd) {
            amount = maxAdd;
            *statusVar = 1;
        }

        newVal = currentEV + amount;
    } else {
        if (amount > currentEV) {
            amount = currentEV;
            *statusVar = 1;
        }

        newVal = currentEV - amount;
    }

    Pokemon_SetValue(mon, monData, &newVal);
    Pokemon_CalcLevelAndStats(mon);
    return FALSE;
}
