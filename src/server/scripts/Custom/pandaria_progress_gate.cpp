/*
 * Pandaria progress gate — Passage Keeper pattern (mirrors WotLK mod-northrend-gate).
 * Locked characters cannot enter MoP maps (except Wandering Isle) and gain no XP at 85+
 * until they turn in one of every T4–T6 BC raid armor token (54) at the Passage Keeper.
 */

#include "ScriptMgr.h"
#include "Player.h"
#include "Chat.h"
#include "ScriptedCreature.h"
#include "ScriptedGossip.h"
#include "Map.h"
#include "ObjectMgr.h"
#include "WorldSession.h"
#include "DBCStores.h"
#include "SharedDefines.h"
#include "Config.h"
#include <vector>

namespace
{
    uint32 const NPC_PASSAGE_KEEPER = 700021;
    uint32 const QUEST_PASSAGE_TO_PANDARIA = 700021;
    uint32 const MAP_WANDERING_ISLE = 860;
    uint32 const PAUSE_XP_AT_LEVEL = 85;

    // Kick / safe hub: Shattrath Terrace of Light
    uint32 const KICK_MAP = 530;
    float const KICK_X = -1838.16f;
    float const KICK_Y = 5301.79f;
    float const KICK_Z = -12.428f;
    float const KICK_O = 1.57f;

    // One of every T4 Fallen (15) + T5 Vanquished (15) + T6 Forgotten (24) = 54
    uint32 const TOKEN_IDS[] = {
        // T4 Fallen
        29753, 29754, 29755, 29756, 29757, 29758, 29759, 29760, 29761, 29762,
        29763, 29764, 29765, 29766, 29767,
        // T5 Vanquished
        30236, 30237, 30238, 30239, 30240, 30241, 30242, 30243, 30244, 30245,
        30246, 30247, 30248, 30249, 30250,
        // T6 Forgotten (armor + bracers/belt/boots)
        31089, 31090, 31091, 31092, 31093, 31094, 31095, 31096, 31097, 31098,
        31099, 31100, 31101, 31102, 31103,
        34848, 34851, 34852, 34853, 34854, 34855, 34856, 34857, 34858
    };
    size_t const TOKEN_COUNT = sizeof(TOKEN_IDS) / sizeof(TOKEN_IDS[0]);

    bool GateEnabled()
    {
        return sConfigMgr->GetBoolDefault("PandariaGate.Enable", true);
    }

    bool IsExempt(Player* player)
    {
        if (!player || !player->GetSession())
            return true;
        if (player->IsGameMaster())
            return true;
        if (player->GetSession()->GetSecurity() > SEC_PLAYER)
            return true;
        if (player->GetSession()->IsBot())
            return true;
        return false;
    }

    bool IsUnlocked(Player const* player)
    {
        return player && player->GetQuestRewardStatus(QUEST_PASSAGE_TO_PANDARIA);
    }

    bool IsPandariaGatedMap(uint32 mapId)
    {
        if (mapId == MAP_WANDERING_ISLE)
            return false;
        MapEntry const* entry = sMapStore.LookupEntry(mapId);
        return entry && entry->Expansion() == EXPANSION_MISTS_OF_PANDARIA;
    }

    bool IsLocked(Player* player)
    {
        return GateEnabled() && player && !IsExempt(player) && !IsUnlocked(player);
    }

    void KickFromPandaria(Player* player, char const* reason)
    {
        if (!player)
            return;
        ChatHandler(player->GetSession()).PSendSysMessage(
            "|cffff6666Pandaria is sealed until you turn in all 54 T4–T6 tokens at the Passage Keeper in Shattrath.|r (%s)",
            reason ? reason : "locked");
        player->TeleportTo(KICK_MAP, KICK_X, KICK_Y, KICK_Z, KICK_O);
    }

    uint32 CountMissingTokens(Player* player, std::vector<uint32>* missingOut = nullptr)
    {
        uint32 missing = 0;
        for (size_t i = 0; i < TOKEN_COUNT; ++i)
        {
            if (player->GetItemCount(TOKEN_IDS[i], true) < 1)
            {
                ++missing;
                if (missingOut)
                    missingOut->push_back(TOKEN_IDS[i]);
            }
        }
        return missing;
    }

    bool ConsumeAllTokens(Player* player)
    {
        for (size_t i = 0; i < TOKEN_COUNT; ++i)
        {
            if (player->GetItemCount(TOKEN_IDS[i], true) < 1)
                return false;
        }
        for (size_t i = 0; i < TOKEN_COUNT; ++i)
        {
            player->DestroyItemCount(TOKEN_IDS[i], 1, true);
        }
        return true;
    }
}

class pandaria_progress_gate_player : public PlayerScript
{
public:
    pandaria_progress_gate_player() : PlayerScript("pandaria_progress_gate_player") { }

    void OnLogin(Player* player) override
    {
        if (!IsLocked(player))
            return;
        if (IsPandariaGatedMap(player->GetMapId()))
            KickFromPandaria(player, "login");
    }

    void OnMapChanged(Player* player) override
    {
        if (!IsLocked(player))
            return;
        if (IsPandariaGatedMap(player->GetMapId()))
            KickFromPandaria(player, "map");
    }

    void OnGiveXP(Player* player, uint32& amount, Unit* /*victim*/) override
    {
        if (!amount || !IsLocked(player))
            return;
        if (player->GetLevel() >= PAUSE_XP_AT_LEVEL)
            amount = 0;
    }
};

class npc_pandaria_passage_keeper : public CreatureScript
{
public:
    npc_pandaria_passage_keeper() : CreatureScript("npc_pandaria_passage_keeper") { }

    bool OnGossipHello(Player* player, Creature* creature) override
    {
        ClearGossipMenuFor(player);

        if (!GateEnabled())
        {
            AddGossipItemFor(player, GOSSIP_ICON_CHAT, "The Pandaria gate is currently disabled.", GOSSIP_SENDER_MAIN, 1);
            SendGossipMenuFor(player, DEFAULT_GOSSIP_MESSAGE, creature->GetGUID());
            return true;
        }

        if (IsExempt(player) || IsUnlocked(player))
        {
            AddGossipItemFor(player, GOSSIP_ICON_CHAT, "Your passage to Pandaria is already open.", GOSSIP_SENDER_MAIN, 1);
            SendGossipMenuFor(player, DEFAULT_GOSSIP_MESSAGE, creature->GetGUID());
            return true;
        }

        std::vector<uint32> missing;
        uint32 miss = CountMissingTokens(player, &missing);
        char buf[160];
        if (miss == 0)
        {
            snprintf(buf, sizeof(buf), "Turn in all 54 T4–T6 tokens and open Pandaria");
            AddGossipItemFor(player, GOSSIP_ICON_BATTLE, buf, GOSSIP_SENDER_MAIN, 2);
        }
        else
        {
            snprintf(buf, sizeof(buf), "You still need %u of 54 tokens (Fallen/Vanquished/Forgotten).", miss);
            AddGossipItemFor(player, GOSSIP_ICON_CHAT, buf, GOSSIP_SENDER_MAIN, 1);
            AddGossipItemFor(player, GOSSIP_ICON_CHAT, "Remind me: one of EVERY T4, T5, and T6 armor token.", GOSSIP_SENDER_MAIN, 1);
        }

        SendGossipMenuFor(player, DEFAULT_GOSSIP_MESSAGE, creature->GetGUID());
        return true;
    }

    bool OnGossipSelect(Player* player, Creature* creature, uint32 /*sender*/, uint32 action) override
    {
        ClearGossipMenuFor(player);
        if (action != 2)
        {
            CloseGossipMenuFor(player);
            return true;
        }

        if (!IsLocked(player))
        {
            ChatHandler(player->GetSession()).SendSysMessage("Your passage is already open.");
            CloseGossipMenuFor(player);
            return true;
        }

        if (CountMissingTokens(player) > 0)
        {
            ChatHandler(player->GetSession()).SendSysMessage("You do not have all 54 required tokens.");
            CloseGossipMenuFor(player);
            return true;
        }

        if (!ConsumeAllTokens(player))
        {
            ChatHandler(player->GetSession()).SendSysMessage("Token turn-in failed — check your bags/bank.");
            CloseGossipMenuFor(player);
            return true;
        }

        Quest const* quest = sObjectMgr->GetQuestTemplate(QUEST_PASSAGE_TO_PANDARIA);
        if (!quest)
        {
            ChatHandler(player->GetSession()).SendSysMessage("Passage quest is missing from the database.");
            CloseGossipMenuFor(player);
            return true;
        }

        // Quest is AUTO_COMPLETE — RewardQuest marks it rewarded and persists.
        if (!player->CanRewardQuest(quest, true))
        {
            ChatHandler(player->GetSession()).SendSysMessage("Cannot open passage right now.");
            CloseGossipMenuFor(player);
            return true;
        }
        player->RewardQuest(quest, 0, creature, true);

        ChatHandler(player->GetSession()).SendSysMessage(
            "|cff66ff66The way to Pandaria is open. Safe travels.|r");
        CloseGossipMenuFor(player);
        return true;
    }
};

void AddSC_pandaria_progress_gate()
{
    new pandaria_progress_gate_player();
    new npc_pandaria_passage_keeper();
}
