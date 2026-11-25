/*
 * MoP port of AzerothCore mod-reagent-bank (ZhengPeiRu21) — Ling-style reagent bank.
 * Upstream: https://github.com/ZhengPeiRu21/mod-reagent-bank
 * WotLK live used entry 190012; MoP 190012 is MMaps Waypoint — this uses entry 500012.
 */

#include "ScriptMgr.h"
#include "Player.h"
#include "Chat.h"
#include "ScriptedCreature.h"
#include "ScriptedGossip.h"
#include "Item.h"
#include "ItemPrototype.h"
#include "Bag.h"
#include "ObjectMgr.h"
#include "WorldSession.h"
#include "DatabaseEnv.h"
#include "DBCStores.h"
#include "SharedDefines.h"
#include <map>
#include <sstream>
#include <vector>

#define REAGENT_BANK_MAX_OPTIONS 23
#define REAGENT_BANK_MAX_PAGE_NUMBER 700 // Values higher than this are treated as item entry IDs
#define REAGENT_BANK_NPC_TEXT_ID 4259

enum ReagentBankGossip : uint32
{
    REAGENT_BANK_DEPOSIT_ALL = 16,
    REAGENT_BANK_MAIN_MENU   = 17
};

class npc_reagent_banker : public CreatureScript
{
private:
    static std::string GetItemLink(uint32 entry, WorldSession* session)
    {
        int loc_idx = session->GetSessionDbLocaleIndex();
        ItemTemplate const* temp = sObjectMgr->GetItemTemplate(entry);
        if (!temp)
            return "[Unknown]";

        std::string name = temp->Name1;
        if (ItemLocale const* il = sObjectMgr->GetItemLocale(temp->ItemId))
            ObjectMgr::GetLocaleStringOld(il->Name, loc_idx, name);

        std::ostringstream oss;
        oss << "|c" << std::hex << ItemQualityColors[temp->Quality] << std::dec <<
            "|Hitem:" << temp->ItemId << ":0:0:0:0:0:0:0:0:0|h[" << name << "]|h|r";
        return oss.str();
    }

    static std::string GetItemIcon(uint32 entry, uint32 width, uint32 height, int x, int y)
    {
        std::ostringstream ss;
        ss << "|TInterface";
        ItemTemplate const* temp = sObjectMgr->GetItemTemplate(entry);
        ItemDisplayInfoEntry const* dispInfo = nullptr;
        if (temp)
        {
            dispInfo = sItemDisplayInfoStore.LookupEntry(temp->DisplayInfoID);
            if (dispInfo && dispInfo->inventoryIcon[0] && *dispInfo->inventoryIcon[0])
                ss << "/ICONS/" << dispInfo->inventoryIcon[0];
            else
                dispInfo = nullptr;
        }
        if (!dispInfo)
            ss << "/InventoryItems/WoWUnknownItem01";
        ss << ":" << width << ":" << height << ":" << x << ":" << y << "|t";
        return ss.str();
    }

    static void WithdrawItem(Player* player, uint32 entry)
    {
        uint32 guid = player->GetGUID().GetCounter();
        QueryResult result = CharacterDatabase.PQuery(
            "SELECT amount FROM custom_reagent_bank WHERE character_id = %u AND item_entry = %u",
            guid, entry);
        if (!result)
            return;

        uint32 storedAmount = (*result)[0].GetUInt32();
        ItemTemplate const* temp = sObjectMgr->GetItemTemplate(entry);
        if (!temp)
            return;

        uint32 stackSize = temp->GetMaxStackSize();
        uint32 giveAmount = storedAmount <= stackSize ? storedAmount : stackSize;

        ItemPosCountVec dest;
        InventoryResult msg = player->CanStoreNewItem(NULL_BAG, NULL_SLOT, dest, entry, giveAmount);
        if (msg != EQUIP_ERR_OK)
        {
            player->SendEquipError(msg, nullptr, nullptr, entry);
            return;
        }

        if (storedAmount <= stackSize)
            CharacterDatabase.PExecute(
                "DELETE FROM custom_reagent_bank WHERE character_id = %u AND item_entry = %u",
                guid, entry);
        else
            CharacterDatabase.PExecute(
                "UPDATE custom_reagent_bank SET amount = %u WHERE character_id = %u AND item_entry = %u",
                storedAmount - stackSize, guid, entry);

        Item* item = player->StoreNewItem(dest, entry, true);
        player->SendNewItem(item, giveAmount, true, false);
    }

    static void UpdateItemCount(std::map<uint32, uint32>& entryToAmountMap,
                                std::map<uint32, uint32>& entryToSubclassMap,
                                Item* pItem, Player* player, uint8 bagSlot, uint8 itemSlot)
    {
        uint32 count = pItem->GetCount();
        ItemTemplate const* itemTemplate = pItem->GetTemplate();
        if (!itemTemplate)
            return;

        if (!(itemTemplate->Class == ITEM_CLASS_TRADE_GOODS || itemTemplate->Class == ITEM_CLASS_GEM) ||
            itemTemplate->GetMaxStackSize() <= 1)
            return;

        uint32 itemEntry = itemTemplate->ItemId;
        uint32 itemSubclass = itemTemplate->SubClass;

        // Gems share the Jewelcrafting gossip section
        if (itemTemplate->Class == ITEM_CLASS_GEM)
            itemSubclass = ITEM_SUBCLASS_JEWELCRAFTING;

        if (!entryToAmountMap.count(itemEntry))
        {
            entryToAmountMap[itemEntry] = count;
            entryToSubclassMap[itemEntry] = itemSubclass;
        }
        else
            entryToAmountMap[itemEntry] += count;

        player->DestroyItem(bagSlot, itemSlot, true);
    }

    static void DepositAllReagents(Player* player)
    {
        WorldSession* session = player->GetSession();
        uint32 guid = player->GetGUID().GetCounter();
        std::string query =
            "SELECT item_entry, item_subclass, amount FROM custom_reagent_bank WHERE character_id = " +
            std::to_string(guid);

        session->GetQueryProcessor().AddCallback(
            CharacterDatabase.AsyncQuery(query.c_str()).WithCallback([player, guid](QueryResult result)
        {
            if (!player || !player->GetSession())
                return;

            std::map<uint32, uint32> entryToAmountMap;
            std::map<uint32, uint32> entryToSubclassMap;
            if (result)
            {
                do
                {
                    uint32 itemEntry = (*result)[0].GetUInt32();
                    uint32 itemSubclass = (*result)[1].GetUInt32();
                    uint32 itemAmount = (*result)[2].GetUInt32();
                    entryToAmountMap[itemEntry] = itemAmount;
                    entryToSubclassMap[itemEntry] = itemSubclass;
                } while (result->NextRow());
            }

            for (uint8 i = INVENTORY_SLOT_ITEM_START; i < INVENTORY_SLOT_ITEM_END; ++i)
            {
                if (Item* pItem = player->GetItemByPos(INVENTORY_SLOT_BAG_0, i))
                    UpdateItemCount(entryToAmountMap, entryToSubclassMap, pItem, player, INVENTORY_SLOT_BAG_0, i);
            }

            for (uint8 i = INVENTORY_SLOT_BAG_START; i < INVENTORY_SLOT_BAG_END; ++i)
            {
                Bag* bag = player->GetBagByPos(i);
                if (!bag)
                    continue;
                for (uint8 j = 0; j < bag->GetBagSize(); ++j)
                {
                    if (Item* pItem = player->GetItemByPos(i, j))
                        UpdateItemCount(entryToAmountMap, entryToSubclassMap, pItem, player, i, j);
                }
            }

            if (!entryToAmountMap.empty())
            {
                CharacterDatabaseTransaction trans = CharacterDatabase.BeginTransaction();
                for (auto const& mapEntry : entryToAmountMap)
                {
                    uint32 itemEntry = mapEntry.first;
                    uint32 itemAmount = mapEntry.second;
                    uint32 itemSubclass = entryToSubclassMap[itemEntry];
                    trans->PAppend(
                        "REPLACE INTO custom_reagent_bank (character_id, item_entry, item_subclass, amount) "
                        "VALUES (%u, %u, %u, %u)",
                        guid, itemEntry, itemSubclass, itemAmount);
                }
                CharacterDatabase.CommitTransaction(trans);
            }
        }));

        ChatHandler(player->GetSession()).PSendSysMessage("All reagents deposited successfully.");
        CloseGossipMenuFor(player);
    }

    void ShowReagentItems(Player* player, Creature* creature, uint32 item_subclass, uint16 gossipPageNumber)
    {
        WorldSession* session = player->GetSession();
        uint32 guid = player->GetGUID().GetCounter();
        std::string query =
            "SELECT item_entry, amount FROM custom_reagent_bank WHERE character_id = " +
            std::to_string(guid) + " AND item_subclass = " + std::to_string(item_subclass) +
            " ORDER BY item_entry";

        session->GetQueryProcessor().AddCallback(
            CharacterDatabase.AsyncQuery(query.c_str()).WithCallback(
                [player, creature, item_subclass, gossipPageNumber, session](QueryResult result)
        {
            if (!player || !player->GetSession() || !creature)
                return;

            ClearGossipMenuFor(player);

            uint32 startValue = gossipPageNumber * REAGENT_BANK_MAX_OPTIONS;
            uint32 endValue = (gossipPageNumber + 1) * REAGENT_BANK_MAX_OPTIONS - 1;
            std::map<uint32, uint32> entryToAmountMap;
            std::vector<uint32> itemEntries;
            if (result)
            {
                do
                {
                    uint32 itemEntry = (*result)[0].GetUInt32();
                    uint32 itemAmount = (*result)[1].GetUInt32();
                    entryToAmountMap[itemEntry] = itemAmount;
                    itemEntries.push_back(itemEntry);
                } while (result->NextRow());
            }

            for (uint32 i = startValue; i <= endValue; ++i)
            {
                if (itemEntries.empty() || i > itemEntries.size() - 1)
                    break;
                uint32 itemEntry = itemEntries.at(i);
                AddGossipItemFor(player, GOSSIP_ICON_MONEY_BAG,
                    GetItemIcon(itemEntry, 30, 30, -18, 0) + GetItemLink(itemEntry, session) +
                        " (" + std::to_string(entryToAmountMap[itemEntry]) + ")",
                    itemEntry, gossipPageNumber);
            }

            if (gossipPageNumber > 0)
                AddGossipItemFor(player, GOSSIP_ICON_CHAT, "Previous Page", item_subclass, gossipPageNumber - 1);
            if (endValue < itemEntries.size())
                AddGossipItemFor(player, GOSSIP_ICON_CHAT, "Next Page", item_subclass, gossipPageNumber + 1);

            AddGossipItemFor(player, GOSSIP_ICON_MONEY_BAG,
                "|TInterface/ICONS/Ability_Spy:30:30:-18:0|tBack...",
                REAGENT_BANK_MAIN_MENU, 0);
            SendGossipMenuFor(player, REAGENT_BANK_NPC_TEXT_ID, creature->GetGUID());
        }));
    }

public:
    npc_reagent_banker() : CreatureScript("npc_reagent_banker") { }

    bool OnGossipHello(Player* player, Creature* creature) override
    {
        ClearGossipMenuFor(player);
        AddGossipItemFor(player, GOSSIP_ICON_MONEY_BAG, GetItemIcon(4359, 30, 30, -18, 0) + "Parts", ITEM_SUBCLASS_PARTS, 0);
        AddGossipItemFor(player, GOSSIP_ICON_MONEY_BAG, GetItemIcon(4358, 30, 30, -18, 0) + "Explosives", ITEM_SUBCLASS_EXPLOSIVES, 0);
        AddGossipItemFor(player, GOSSIP_ICON_MONEY_BAG, GetItemIcon(4388, 30, 30, -18, 0) + "Devices", ITEM_SUBCLASS_DEVICES, 0);
        AddGossipItemFor(player, GOSSIP_ICON_MONEY_BAG, GetItemIcon(1206, 30, 30, -18, 0) + "Jewelcrafting", ITEM_SUBCLASS_JEWELCRAFTING, 0);
        AddGossipItemFor(player, GOSSIP_ICON_MONEY_BAG, GetItemIcon(2589, 30, 30, -18, 0) + "Cloth", ITEM_SUBCLASS_CLOTH, 0);
        AddGossipItemFor(player, GOSSIP_ICON_MONEY_BAG, GetItemIcon(2318, 30, 30, -18, 0) + "Leather", ITEM_SUBCLASS_LEATHER, 0);
        AddGossipItemFor(player, GOSSIP_ICON_MONEY_BAG, GetItemIcon(2772, 30, 30, -18, 0) + "Metal & Stone", ITEM_SUBCLASS_METAL_STONE, 0);
        AddGossipItemFor(player, GOSSIP_ICON_MONEY_BAG, GetItemIcon(12208, 30, 30, -18, 0) + "Meat", ITEM_SUBCLASS_MEAT, 0);
        AddGossipItemFor(player, GOSSIP_ICON_MONEY_BAG, GetItemIcon(2453, 30, 30, -18, 0) + "Herb", ITEM_SUBCLASS_HERB, 0);
        AddGossipItemFor(player, GOSSIP_ICON_MONEY_BAG, GetItemIcon(7068, 30, 30, -18, 0) + "Elemental", ITEM_SUBCLASS_ELEMENTAL, 0);
        AddGossipItemFor(player, GOSSIP_ICON_MONEY_BAG, GetItemIcon(10940, 30, 30, -18, 0) + "Enchanting", ITEM_SUBCLASS_ENCHANTING, 0);
        AddGossipItemFor(player, GOSSIP_ICON_MONEY_BAG, GetItemIcon(23572, 30, 30, -18, 0) + "Materials", ITEM_SUBCLASS_MATERIAL, 0);
        AddGossipItemFor(player, GOSSIP_ICON_MONEY_BAG, GetItemIcon(2604, 30, 30, -18, 0) + "Other Trade Goods", ITEM_SUBCLASS_TRADE_GOODS_OTHER, 0);
        // MoP collapsed armor/weapon vellum into ITEM_SUBCLASS_ENCHANTMENT
        AddGossipItemFor(player, GOSSIP_ICON_MONEY_BAG, GetItemIcon(38682, 30, 30, -18, 0) + "Enchantment (Vellum)", ITEM_SUBCLASS_ENCHANTMENT, 0);
        AddGossipItemFor(player, GOSSIP_ICON_MONEY_BAG, "Deposit All Reagents", REAGENT_BANK_DEPOSIT_ALL, 0);
        SendGossipMenuFor(player, REAGENT_BANK_NPC_TEXT_ID, creature->GetGUID());
        return true;
    }

    bool OnGossipSelect(Player* player, Creature* creature, uint32 sender, uint32 action) override
    {
        ClearGossipMenuFor(player);

        if (sender > REAGENT_BANK_MAX_PAGE_NUMBER)
        {
            ItemTemplate const* temp = sObjectMgr->GetItemTemplate(sender);
            WithdrawItem(player, sender);
            if (temp && temp->Class == ITEM_CLASS_GEM)
                ShowReagentItems(player, creature, ITEM_SUBCLASS_JEWELCRAFTING, action);
            else if (temp)
                ShowReagentItems(player, creature, temp->SubClass, action);
            return true;
        }

        if (sender == REAGENT_BANK_DEPOSIT_ALL)
        {
            DepositAllReagents(player);
            return true;
        }

        if (sender == REAGENT_BANK_MAIN_MENU)
            return OnGossipHello(player, creature);

        ShowReagentItems(player, creature, sender, action);
        return true;
    }
};

void AddSC_npc_reagent_bank()
{
    new npc_reagent_banker();
}
