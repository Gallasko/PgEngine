/**
 * @file environment.h
 * @author Gallasko
 * @brief Definition of the environment object
 * @version 1.0
 * @date 2022-04-12
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#pragma once

#include <vector>
#include <queue>
#include <unordered_map>
#include <string>

#include "valuable.h"

namespace pg
{
    class VisitorInterpreter;

    /**
     * @class Environment
     * 
     * @brief A class used to represent a scope in the interpreter
     * 
     * This class has the knowledge of it's parent scope and all the variable defined in this current scope.
     * An environment object is then used to interact with stored valuable from it's local list of valuable,
     * or if the valuable is not find locally ask it's parent scope for the valuable.
     * 
     * This class implements various methods to declare, assign and get value from valuable in scope
     * 
     */
    class Environment
    {
        friend class VisitorInterpreter;
    public:
        /**
         * @brief Construct a new Environment object
         * 
         * @param env A pointer to the parent environment object
         */
        Environment(std::shared_ptr<Environment> env = nullptr) : enclosing(env) {}

        virtual ~Environment() {}

        /** Declare a new valuable in the current scope */
        void declareValue(const std::string& name, std::shared_ptr<Valuable> value);

        /** Assign a new value to a valuable in scope */
        void assignValue(const std::string& name, const Token& token, std::shared_ptr<Valuable> value);
        
        /** Get the value of a valuable in scope */
        std::shared_ptr<Valuable> getValue(const std::string& name, const Token& token) const;

        /**
         * @brief Get the parent environment object
         * 
         * @return A reference to the parent environment object
         */
        inline const std::shared_ptr<Environment>& getEnv() const { return enclosing; }

    protected:
        /** A pointer to the parent environment object */
        std::shared_ptr<Environment> enclosing;

        /** A table of the different variable defined in this scope */
        std::unordered_map<std::string, std::shared_ptr<Valuable>> variableTable;
    };

}
