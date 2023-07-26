#pragma once

#include "ECS/entitysystem.h"

#include "interpreter.h"

#include <map>
#include <unordered_map>

namespace pg
{
    struct ExecuteFileScriptEvent
    {
        std::string filename;
    };

    struct ExecuteCodeScriptEvent
    {
        std::string data;
    };
    class PgInterpreter : public System<Listener<ExecuteFileScriptEvent>, Listener<ExecuteCodeScriptEvent>, StoragePolicy, NamedSystem>
    {
    public:
        void interpretFromText(const std::string& scriptText);
        void interpretFromFile(const std::string& scriptFile);
        void interpretFromFile(const TextFile& scriptFile);

        virtual std::string getSystemName() const override { return "Pg Interpreter"; }

        virtual void onEvent(const ExecuteFileScriptEvent& event) override
        {
            interpretFromFile(event.filename);
        }

        virtual void onEvent(const ExecuteCodeScriptEvent& event) override
        {
            interpretFromText(event.data);
        }

    protected:
        typedef void(*sysFunction)(Interpreter*, const std::string&);

        template <typename Functional>
        void addSystemFunction(const std::string& name)
        {
            sysFunctionTable.emplace(name, [](Interpreter *interpreter, const std::string& sysName){ interpreter->defineSystemFunction<Functional>(sysName); });
        }

        std::map<std::string, sysFunction> sysFunctionTable;

    private:
        ScriptImport getAst(const std::string& script, const std::string& filePath = "");

        ScriptImport generateAST(const std::string& data);
        ScriptImport generateASTFromFile(const std::string& filename);
        ScriptImport generateASTFromFile(const TextFile& file);

        void _interpret(const ScriptImport& script);

        // Todo store the absolute path of the custom made script file
        // to avoid using the AST of another file when trying to import a script
        std::unordered_map<std::string, ScriptImport> importedScripts;
    };
}