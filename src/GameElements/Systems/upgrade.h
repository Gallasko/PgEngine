#pragma once

#include <string>
#include <vector>
#include <unordered_map>

#include "ressources.h"

class UpgradeManager
{
public:
    class Upgrade
    {
    friend class UpgradeManager;
    
    public:
        Upgrade();
    
    protected:
        inline void setName(const std::string& name) { this->name = name; }
        inline void setDescription(const std::string& description) { this->description = description; }

        inline void setPrerequisites(const std::string& requirements);

    private:
        std::string name;
        std::string description;

        std::vector<Effect> initialEffect; // List of the effect of the upgrade
        //Curve effectCurve; // Math to calculate the increase of effect when buying this upgrade past the first one

        std::vector<std::string> requirements; // List of the name of the required upgrade to unlock this one, an empty list means no requirements
        
        std::vector<Ressources> initialCost; // List of the ressources to be paid for the upgrade
        unsigned int numberOfMaxBuy = 1; // Number of time this upgrade can be bought -> set to 0 for unlimited buy;
        //Curve buyingCurve; // Math to calculate the increase of ressources when buying this upgrade past the first one

        unsigned int nbBought = 0; // Number of time this upgrade was bought
    };

public:
    UpgradeManager();

    /**
     * @brief Get a list of all the current Upgrade that can be bought
     * 
     * @return std::vector<Upgrade> A list of available Upgrade
     */
    std::vector<Upgrade> availableUpgrades() const;

private:
    std::unordered_map<std::string, Upgrade> upgradeRefs;
    std::vector<Upgrade> boughtUpgrades;
};