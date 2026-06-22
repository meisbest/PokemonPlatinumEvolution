#include "scrcmd_party.h"

#include <nitro.h>

#include "constants/heap.h"
#include "constants/moves.h"
#include "constants/items.h"
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
    int metLocation = MapHeader_GetMapLabelTextID(ctx->fieldSystem->location->mapId);
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
    u32 location = MapHeader_GetMapLabelTextID(ctx->fieldSystem->location->mapId);

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
    sub_0206DDB8(ctx->fieldSystem->saveData, mon, Ribbon_GetData(ribbonID, RIBBON_DATA_MON_DATA_PARAM));

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

BOOL ScrCmd_CheckAbilitySwap(ScriptContext *ctx)
{
    FieldSystem *fieldSystem = ctx->fieldSystem;
    
    // Lo script ci dice quale Pokémon della squadra ha scelto il giocatore
    u16 partySlot = ScriptContext_GetVar(ctx);
    
    // Variabili di ritorno per lo script
    u16 *statusVar = ScriptContext_GetVarPointer(ctx); // 0 = OK, 1 = Uovo, 2 = Solo 1 abilità
    u16 *newAbilityVar = ScriptContext_GetVarPointer(ctx); // L'ID della nuova abilità da stampare a schermo

    Pokemon *mon = Party_GetPokemonBySlotIndex(SaveData_GetParty(fieldSystem->saveData), partySlot);

    // Controllo Uovo
    if (Pokemon_GetValue(mon, MON_DATA_IS_EGG, NULL) != FALSE) {
        *statusVar = 1; 
        return FALSE;
    }

    u16 species = Pokemon_GetValue(mon, MON_DATA_SPECIES, NULL);
    u32 currAbility = Pokemon_GetValue(mon, MON_DATA_ABILITY, NULL);

    // Leggiamo le abilità base della specie 
    // (Simile a come il file daycare.c leggeva i gruppi uovo!)
    u32 abil1 = SpeciesData_GetSpeciesValue(species, SPECIES_DATA_ABILITY_1);
    u32 abil2 = SpeciesData_GetSpeciesValue(species, SPECIES_DATA_ABILITY_2);

    // Controllo Abilità Unica (Se la 1 è uguale alla 2, o se la 2 è zero)
    if (abil1 == abil2 || abil2 == 0) { 
        *statusVar = 2; 
        return FALSE;
    }

    // È tutto pronto! Calcoliamo qual è quella nuova
    *statusVar = 0; 
    if (currAbility == abil1) {
        *newAbilityVar = abil2;
    } else {
        *newAbilityVar = abil1;
    }

    StringTemplate **strTemplate = FieldSystem_GetScriptMemberPtr(ctx->fieldSystem, SCRIPT_MANAGER_STR_TEMPLATE);
    StringTemplate_SetAbilityName(*strTemplate, 0, *newAbilityVar);

    return FALSE;
}

BOOL ScrCmd_ApplyAbilitySwap(ScriptContext *ctx)
{
    FieldSystem *fieldSystem = ctx->fieldSystem;
    u16 partySlot = ScriptContext_GetVar(ctx);
    
    // FONDAMENTALE: Usare u32 per impedire letture della RAM sporca!
    u32 newAbility = ScriptContext_GetVar(ctx); 

    Pokemon *mon = Party_GetPokemonBySlotIndex(SaveData_GetParty(fieldSystem->saveData), partySlot);

    Pokemon_SetValue(mon, MON_DATA_ABILITY, &newAbility);
    Pokemon_CalcLevelAndStats(mon); // Aggiorniamo le statistiche per sicurezza

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

    u16 species = Pokemon_GetValue(mon, MON_DATA_SPECIES, NULL);
    u32 oldPid = Pokemon_GetValue(mon, MON_DATA_PERSONALITY, NULL);
    u32 otid = Pokemon_GetValue(mon, MON_DATA_OT_ID, NULL);
    u32 oldGender = Pokemon_GetValue(mon, MON_DATA_GENDER, NULL);
    
    // In Gen 4, lo slot abilità (0 o 1) è il primo bit del PID!
    u8 abilitySlot = oldPid & 1; 

    u8 genderRatio = SpeciesData_GetSpeciesValue(species, SPECIES_DATA_GENDER_RATIO);
    u16 tid = otid & 0xFFFF;
    u16 sid = (otid >> 16) & 0xFFFF;

    u16 oldPLow = oldPid & 0xFFFF;
    u16 oldPHigh = (oldPid >> 16) & 0xFFFF;
    u8 isShiny = ((tid ^ sid ^ oldPLow ^ oldPHigh) < 8);

    // LA MAGIA: Salviamo la sequenza di bit (13-17) che regola il rimescolamento dei blocchi!
    u32 blockShuffleBits = oldPid & 0x3E000;

    u32 newPid;
    
    while (1) {
        newPid = (LCRNG_Next() << 16) | LCRNG_Next();
        
        // 1. Iniettiamo la sequenza originale: questo EVITA la corruzione dei dati!
        newPid = (newPid & ~0x3E000) | blockShuffleBits;

        // 2. Controllo Natura
        if ((newPid % 25) != desiredNature) continue;
        
        // 3. Controllo Slot Abilità
        if ((newPid & 1) != abilitySlot) continue;

        // 4. Controllo Sesso
        u8 newGender;
        if (genderRatio == 255) newGender = 2;
        else if (genderRatio == 254) newGender = 1;
        else if (genderRatio == 0) newGender = 0;
        else newGender = ((newPid & 0xFF) >= genderRatio) ? 0 : 1; 
        
        if (newGender != oldGender) continue;

        // 5. Controllo Cromaticità
        u16 pLow = newPid & 0xFFFF;
        u16 pHigh = (newPid >> 16) & 0xFFFF;
        u8 newShiny = ((tid ^ sid ^ pLow ^ pHigh) < 8);
        if (newShiny != isShiny) continue;

        // Sopravvissuto! Abbiamo il DNA perfetto.
        break; 
    }

    Pokemon_SetValue(mon, MON_DATA_PERSONALITY, &newPid);
    Pokemon_CalcLevelAndStats(mon);

    return FALSE;
}

BOOL ScrCmd_ResetEVs(ScriptContext *ctx)
{
    FieldSystem *fieldSystem = ctx->fieldSystem;
    u16 partySlot = ScriptContext_GetVar(ctx);
    Pokemon *mon = Party_GetPokemonBySlotIndex(SaveData_GetParty(fieldSystem->saveData), partySlot);

    u32 zero = 0;
    Pokemon_SetValue(mon, MON_DATA_HP_EV, &zero);
    Pokemon_SetValue(mon, MON_DATA_ATK_EV, &zero);
    Pokemon_SetValue(mon, MON_DATA_DEF_EV, &zero);
    Pokemon_SetValue(mon, MON_DATA_SPEED_EV, &zero);
    Pokemon_SetValue(mon, MON_DATA_SPATK_EV, &zero);
    Pokemon_SetValue(mon, MON_DATA_SPDEF_EV, &zero);

    Pokemon_CalcLevelAndStats(mon);
    return FALSE;
}

// --- 2. IMPOSTA IVs (0 - 31) ---
BOOL ScrCmd_SetIV(ScriptContext *ctx)
{
    FieldSystem *fieldSystem = ctx->fieldSystem;
    u16 partySlot = ScriptContext_GetVar(ctx);
    u16 statIndex = ScriptContext_GetVar(ctx); // 0=HP, 1=Atk, 2=Def, 3=SpA, 4=SpD, 5=Spe
    u16 value = ScriptContext_ReadHalfWord(ctx);

    Pokemon *mon = Party_GetPokemonBySlotIndex(SaveData_GetParty(fieldSystem->saveData), partySlot);

    int monData = MON_DATA_HP_IV;
    switch(statIndex) {
        case 0: monData = MON_DATA_HP_IV; break;
        case 1: monData = MON_DATA_ATK_IV; break;
        case 2: monData = MON_DATA_DEF_IV; break;
        case 3: monData = MON_DATA_SPATK_IV; break;
        case 4: monData = MON_DATA_SPDEF_IV; break;
        case 5: monData = MON_DATA_SPEED_IV; break;
    }

    u32 val32 = value;
    if (val32 > 31) val32 = 31; // Limite di sicurezza
    Pokemon_SetValue(mon, monData, &val32);
    Pokemon_CalcLevelAndStats(mon);
    return FALSE;
}

// --- 3. MODIFICA INTELLIGENTE EVs ---
BOOL ScrCmd_ModifyEV(ScriptContext *ctx)
{
    FieldSystem *fieldSystem = ctx->fieldSystem;
    u16 partySlot = ScriptContext_GetVar(ctx);
    u16 statIndex = ScriptContext_GetVar(ctx);
    u16 isAdd = ScriptContext_ReadHalfWord(ctx); // 1 = Aggiungi, 0 = Sottrai
    u16 amount = ScriptContext_ReadHalfWord(ctx); // Se 0 legge e basta
    u16 *statusVar = ScriptContext_GetVarPointer(ctx); // 0 = Ok, 1 = Bloccato dal Limite

    Pokemon *mon = Party_GetPokemonBySlotIndex(SaveData_GetParty(fieldSystem->saveData), partySlot);

    int monData = MON_DATA_HP_EV;
    switch(statIndex) {
        case 0: monData = MON_DATA_HP_EV; break;
        case 1: monData = MON_DATA_ATK_EV; break;
        case 2: monData = MON_DATA_DEF_EV; break;
        case 3: monData = MON_DATA_SPATK_EV; break;
        case 4: monData = MON_DATA_SPDEF_EV; break;
        case 5: monData = MON_DATA_SPEED_EV; break;
    }

    u32 currentEV = Pokemon_GetValue(mon, monData, NULL);

    if (amount == 0) {
        *statusVar = currentEV;
        return FALSE;
    }

    u32 totalEV = Pokemon_GetValue(mon, MON_DATA_HP_EV, NULL) +
                  Pokemon_GetValue(mon, MON_DATA_ATK_EV, NULL) +
                  Pokemon_GetValue(mon, MON_DATA_DEF_EV, NULL) +
                  Pokemon_GetValue(mon, MON_DATA_SPATK_EV, NULL) +
                  Pokemon_GetValue(mon, MON_DATA_SPDEF_EV, NULL) +
                  Pokemon_GetValue(mon, MON_DATA_SPEED_EV, NULL);

    *statusVar = 0; // Tutto OK di default

    if (isAdd) {
        u32 availableStat = (currentEV < 252) ? (252 - currentEV) : 0;
        u32 availableTotal = (totalEV < 510) ? (510 - totalEV) : 0;
        u32 maxAdd = (availableStat < availableTotal) ? availableStat : availableTotal;

        if (amount > maxAdd) {
            amount = maxAdd;
            *statusVar = 1; // Ha sbattuto contro il limite!
        }
        u32 newVal = currentEV + amount;
        Pokemon_SetValue(mon, monData, &newVal);
    } else {
        if (amount > currentEV) {
            amount = currentEV;
            *statusVar = 1; // Non può scendere sotto 0!
        }
        u32 newVal = currentEV - amount;
        Pokemon_SetValue(mon, monData, &newVal);
    }

    Pokemon_CalcLevelAndStats(mon);
    return FALSE;
}

BOOL ScrCmd_ProcessGuruMoney(ScriptContext *ctx)
{
    FieldSystem *fieldSystem = ctx->fieldSystem;
    u16 price = ScriptContext_GetVar(ctx);
    u16 actionType = ScriptContext_ReadHalfWord(ctx); // 0 = Controlla, 1 = Paga
    u16 *statusVar = ScriptContext_GetVarPointer(ctx);

    TrainerInfo *trainerInfo = SaveData_GetTrainerInfo(fieldSystem->saveData);
    u32 currentMoney = TrainerInfo_Money(trainerInfo);

    if (currentMoney >= price) {
        if (actionType == 1) { // Se è 1, preleva anche i soldi!
            TrainerInfo_SetMoney(trainerInfo, currentMoney - price);
        }
        *statusVar = 1; // Tutto ok
    } else {
        *statusVar = 0; // Soldi insufficienti
    }
    return FALSE;
}