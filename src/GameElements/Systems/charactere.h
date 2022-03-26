#pragma once

#include <string>
#include <vector>
#include <algorithm>

#include "gametime.h"
#include "inventory.h"

#include "../../constant.h"

using namespace pg::constant;

class WorkingTime
{
public:
    struct WorkingPeriod
    {
        GameHour startTime;
        GameHour endTime;
    };

    WorkingTime() {}
    WorkingTime(const GameHour& startTime, const GameHour& endTime) { workingPeriods.emplace_back(WorkingPeriod{startTime, endTime}); }
    WorkingTime(const WorkingPeriod& period) { workingPeriods.emplace_back(period); }

    WorkingTime& operator()(const GameHour& startTime, const GameHour& endTime) { workingPeriods.emplace_back(WorkingPeriod{startTime, endTime}); return *this; }
    WorkingTime& operator()(const WorkingPeriod& period) { workingPeriods.emplace_back(period); return *this; }

    // Working time
    // bool inWorkingPeriods() const { return workingPeriods.}

private:
    std::vector<WorkingPeriod> workingPeriods;
};

/**
 * @class Character
 * @brief A class that create character entities
 * 
 * Need to create a system of promotion when a character does well enough -> he can evolve into another class
 */
class Character
{
    /**
     * @struct CharacterInfo
     * @brief A structure holding all of the character information
     */
    struct CharacterInfo
    {
        const unsigned int id;            ///< Id of the character
        std::string name;                 ///< Name of the character
        double speed;                     ///< Speed of the character
        unsigned int nbHoldableObjects;   ///< Number of maximum holdable objects
        WorkingTime workingHours;         ///< Working time of the character

        bool hasHouse;                    ///< Boolean to know if the character has currently a house
        constant::Vector2D housePos;      ///< Position of the house of the character
    };

public:
    static Character createCharacter(const std::string& name = "");

    void setManager(int managerId) { managerId = managerId; }

    // void setPath();

    // void move(double elapsedTime); //Elapsed Time -> how much time pass to calculate how much does the character need to move

    bool grabItem(const Item& item) {
        if(heldItems.size() < info.nbHoldableObjects)
        {
            heldItems.push_back(item);
            return true;
        }
        return false; }

    bool depositItem(const Item& item) {
        //auto it = std::find(heldItems.begin(), heldItems.end(), item);
        //if(it != heldItems.end())
        //{
        //    heldItems.erase(it);
        //    return true;
        //}

        return false; }

private:
    Character(const CharacterInfo& info) : info(info), managerId(-1), elapsedTimeOnPath(0.0f) {}

    /** Struct holding all the basic information about a character */
    CharacterInfo info;

    /** The id of the manager that owns this character: -1 => no manager, any positive integer is a manager id */
    int managerId;

    /** Current position of the character on the map */
    constant::Vector2D currentPos;
    /** Current path that the character is following */
    std::vector<constant::Vector2D> currentPath;

    /** Value to know how much the character moved on the path */
    double elapsedTimeOnPath;

    /** Current items held by the character */
    std::vector<Item> heldItems;
};

class Manager
{
    struct ManagerInfo
    {
        unsigned int id;
        unsigned int nbManageableCharacters;
        WorkingTime workingHours;
    };
    
public:
    Manager(const ManagerInfo& info) : info(info) {}

    bool manage(Character* character) {
        if(managedCharacters.size() < info.nbManageableCharacters)
        {
            character->setManager(info.id);
            managedCharacters.push_back(character);
            return true;
        }
        return false;}

private:
    ManagerInfo info;
    std::vector<Character*> managedCharacters;
};
