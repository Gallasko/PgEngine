#include "inventory.h"

#include "UI/listview.h"

#include "UI/ttftext.h" 
#include "UI/prefab.h"

#include "characustomizationscene.h"

namespace pg
{
    void InventorySystem::onEvent(const GainItem& event)
    {
        auto item = event.item;

        if (item.stacksize == 1)
        {
            items[item.type].push_back(item);
            return;
        }

        auto it = std::find(items[item.type].rbegin(), items[item.type].rend(), item);

        if (it != items[item.type].rend())
        {
            // A stack size of -1 means it is infinite
            if (it->stacksize == -1)
            {
                it->nbItems += item.nbItems;
                return;
            }

            int remainingSize = it->stacksize - it->nbItems;

            LOG_INFO("Inventory", remainingSize);

            if (remainingSize > 0)
            {
                if (static_cast<size_t>(remainingSize + 1) > item.nbItems)
                {
                    it->nbItems += item.nbItems;
                    return; 
                }
                else
                {
                    it->nbItems += remainingSize;
                    item.nbItems -= remainingSize;
                }
            }
        }

        if (item.stacksize == -1)
        {
            items[item.type].push_back(item);
            return;
        }

        bool haveRemainingItem = false;

        do
        {
            haveRemainingItem = false;
            auto itemToPush = item;

            if (item.nbItems >= static_cast<size_t>(item.stacksize))
            {
                itemToPush.nbItems = item.stacksize;
                item.nbItems -= item.stacksize;
                haveRemainingItem = true;
            }

            items[item.type].push_back(itemToPush);
        }
        while (haveRemainingItem);
    }

    void InventorySystem::onEvent(const LoseItem& event)
    {
        auto item = event.item;

        auto it = std::find(items[item.type].rbegin(), items[item.type].rend(), item);

        if (it == items[item.type].rend())
        {
            LOG_ERROR("Inventory", "Requested removing of an item that is not in the inventory: " << item.name);
            return;
        }

        it->nbItems -= 1;

        if (it->nbItems <= 0)
        {
            items[item.type].erase(std::next(it).base());
        }
    }

    bool InventorySystem::hasEnough(const Item& item) const
    {
        try
        {
            auto it = items.at(item.type).begin();

            do
            {
                it = std::find(it, items.at(item.type).end(), item);
            
                if (it == items.at(item.type).end())
                    return false;

                size_t nbStoredItem = it->nbItems;

                if (nbStoredItem >= item.nbItems)
                    return true;

                if (it->stacksize == -1)
                    return false;

                std::next(it);

            } while (it != items.at(item.type).end());
        }
        catch (const std::exception& e)
        {
            LOG_MILE("Inventory", "Inventory tab not available, error happend: " << e.what());
        }

        return false;
    }

    struct TitleClicked
    {
        TitleClicked(const std::string& title) : title(title) {}

        std::string title;
    };

    struct ShowCharaList
    {
        ShowCharaList(Item *item) : item(item) {}

        Item *item;
    };

    struct HideCharaList {};

    struct SelectedCharacter
    {
        SelectedCharacter(PlayerCharacter *chara) : chara(chara) {}

        PlayerCharacter *chara;
    };

    struct LearnNewSkillTree {};

    void InventoryScene::init()
    {
        std::vector<std::string> tabNames = {"Weapon", "Armor", "Conso", "SkillBook", "Material", "SStones", "KeyItems"};

        auto xBase = 100;

        for (auto tabName : tabNames)
        {
            auto titleTTF = makeTTFText(this, xBase, 20, 0, "res/font/Inter/static/Inter_28pt-Light.ttf", tabName, 0.4);

            attach<MouseLeftClickComponent>(titleTTF.entity, makeCallable<TitleClicked>(tabName));

            xBase += 90;
        }

        listenToEvent<TitleClicked>([this](const TitleClicked& event) {
            std::unordered_map<std::string, ItemType> itemTypeMap = {
                {"Weapon", ItemType::Weapon},
                {"Armor", ItemType::Armor},
                {"Conso", ItemType::Consomable},
                {"SkillBook", ItemType::SkillBook},
                {"Material", ItemType::Material},
                {"SStones", ItemType::SpellStone},
                {"KeyItems", ItemType::Key}};

            try
            {
                populateView(itemTypeMap.at(event.title));
            }
            catch (const std::exception& e)
            {
                LOG_ERROR("Inventory", "Error trying to populate view: " << e.what());
            }
        });

        listenToEvent<ShowCharaList>([this](const ShowCharaList& event) {
            selectedItem = event.item;

            characterList->clear();

            LOG_INFO("Player", "Loading players");

            for (auto player : ecsRef->view<PlayerCharacter>())
            {
                LOG_INFO("Player", "Loading players");

                if (player)
                {
                    LOG_INFO("Player", "Got player: " << player->character.name);

                    auto sys = ecsRef->getSystem<SkillTreeDatabase>();
            
                    try
                    {
                        auto skillTree = sys->database.at(selectedItem->attributes.at("SkillTree").toString());

                        // Only show player that doesn't have the skill tree !
                        auto it = std::find(player->learnedSkillTree.begin(), player->learnedSkillTree.end(), skillTree);
                
                        if (it == player->learnedSkillTree.end())
                        {
                            addPlayerToListView(player);
                        }                            
                    }
                    catch (const std::exception& e)
                    {
                        LOG_ERROR("Inventory", "Item is not a skill tree, error: " << e.what());
                    }
                }
            }

            characterList->setVisibility(true);
        });

        listenToEvent<HideCharaList>([this](const HideCharaList&) {
            characterList->setVisibility(false);

            learnSkillTreeUi->setVisibility(false);
        });

        listenToEvent<SelectedCharacter>([this](const SelectedCharacter& event) {
            selectedCharacter = event.chara;

            learnSkillTreeUi->setVisibility(true);
        });

        listenToEvent<LearnNewSkillTree>([this](const LearnNewSkillTree&) {
            auto sys = ecsRef->getSystem<SkillTreeDatabase>();
            
            try
            {
                auto skillTree = sys->database.at(selectedItem->attributes.at("SkillTree").toString());

                selectedCharacter->learnedSkillTree.emplace_back(skillTree);
            }
            catch (const std::exception& e)
            {
                LOG_ERROR("Inventory", "Item is not a skill tree, error: " << e.what());
            }

            if (selectedItem->attributes.count("ConsumedUponUse") > 0)
            {
                auto item = *selectedItem;

                item.nbItems = 1;

                ecsRef->sendEvent(LoseItem{item});
            }

            // Refresh the inventory (skill tree) tab !
            populateView(ItemType::SkillBook);

            characterList->setVisibility(false);

            learnSkillTreeUi->setVisibility(false);
        });

        auto listView = makeListView(this, 50, 125, 600, 500);

        inventoryUi["InventoryView"] = listView.entity;

        auto listView2 = makeListView(this, 0, 150, 300, 120);

        listView2.get<ListView>()->setVisibility(false);

        characterList = listView2.get<ListView>();

        auto ttf = makeTTFText(this, 0, 350, 0, "res/font/Inter/static/Inter_28pt-Light.ttf", "Learn", 0.4);
        auto ttfUi = ttf.get<PositionComponent>();
        ttfUi->setVisibility(false);

        attach<MouseLeftClickComponent>(ttf.entity, makeCallable<LearnNewSkillTree>());

        learnSkillTreeUi = ttfUi;
    }

    void InventoryScene::startUp()
    {
    }

    void InventoryScene::addPlayerToListView(PlayerCharacter* player)
    {
        auto ttf = makeTTFText(this, 0, 0, 0, "res/font/Inter/static/Inter_28pt-Light.ttf", player->character.name, 0.4);
        auto ttfUi = ttf.get<PositionComponent>();
        ttfUi->setVisibility(false);

        attach<MouseLeftClickComponent>(ttf.entity, makeCallable<SelectedCharacter>(player));

        characterList->addEntity(ttf.entity);
    }

    void InventoryScene::populateView(const ItemType& type)
    {
        auto view = inventoryUi["InventoryView"].get<ListView>();

        view->clear();

        auto sys = ecsRef->getSystem<InventorySystem>();

        for (auto& item : sys->items[type])
        {
            auto prefab = makeAnchoredPrefab(this, 0, 0);
            auto prefabAnchor = prefab.get<UiAnchor>();
            prefab.get<Prefab>()->setVisibility(false);

            // Careful here we don't pass this but ecsRef because when using a Prefab we don't want the underlaying element to be deleted by the scene
            // before deleting it ourselves in the prefab
            auto itemTex = makeUiTexture(ecsRef, 125 * 3, 21 * 3, "Header2");

            itemTex.get<UiAnchor>()->setTopAnchor(prefabAnchor->top);
            itemTex.get<UiAnchor>()->setLeftAnchor(prefabAnchor->left);

            prefabAnchor->setWidthConstrain(PosConstrain{itemTex.entity.id, AnchorType::Width});
            prefabAnchor->setHeightConstrain(PosConstrain{itemTex.entity.id, AnchorType::Height});

            prefab.get<Prefab>()->addToPrefab(itemTex.entity);

            auto iconTex = makeUiTexture(ecsRef, 15 * 3, 15 * 3, item.icon);
            auto iconTexAnchor = iconTex.get<UiAnchor>();

            iconTexAnchor->setTopAnchor(prefabAnchor->top);
            iconTexAnchor->setTopMargin(8);
            iconTexAnchor->setLeftAnchor(prefabAnchor->left);
            iconTexAnchor->setLeftMargin(12);
            iconTexAnchor->setZConstrain(PosConstrain{itemTex.entity.id, AnchorType::Z, PosOpType::Add, 1.0f});

            prefab.get<Prefab>()->addToPrefab(iconTex.entity);

            auto itemUi = makeTTFText(ecsRef, 0, 0, 2, "res/font/Inter/static/Inter_28pt-Light.ttf", item.name, 0.4);
            auto itemUiAnchor = itemUi.get<UiAnchor>();

            itemUiAnchor->setTopAnchor(iconTexAnchor->top);
            itemUiAnchor->setTopMargin(17);
            itemUiAnchor->setLeftAnchor(iconTexAnchor->right);
            itemUiAnchor->setLeftMargin(8);

            prefab.get<Prefab>()->addToPrefab(itemUi.entity);

            auto itemNb = makeTTFText(ecsRef, 0, 0, 0, "res/font/Inter/static/Inter_28pt-Light.ttf", std::to_string(item.nbItems), 0.4);
            auto itemNbAnchor = itemNb.get<UiAnchor>();

            itemNbAnchor->setZConstrain(PosConstrain{iconTex.entity.id, AnchorType::Z, PosOpType::Add, 1});
            itemNbAnchor->setBottomAnchor(prefabAnchor->bottom);
            itemNbAnchor->setBottomMargin(5);
            itemNbAnchor->setLeftAnchor(prefabAnchor->left);
            // Todo make it vertical centered
            itemNbAnchor->setLeftMargin(iconTex.get<PositionComponent>()->width / 2.0f - itemNb.get<PositionComponent>()->width / 2.0f);

            prefab.get<Prefab>()->addToPrefab(itemNb.entity);

            if (item.attributes.count("UsableOnCharacter") > 0 and item.attributes.at("UsableOnCharacter") == true)
            {
                LOG_INFO("Inventory", "Item " << item.name << " is usable !");
                attach<MouseLeftClickComponent>(itemTex.entity, makeCallable<ShowCharaList>(&item));
            }

            view->addEntity(prefab.entity);
        }
    }
}