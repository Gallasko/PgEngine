#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <algorithm>
#include <filesystem>
#include <iostream>
#include <list>
#include <map>
#include <unordered_set>
#include <memory>
#include <string>
#include <vector>

#include "logger.h"

#include "Compiler/chunk.h"
#include "Compiler/vm.h"
#include "ECS/entitysystem.h"
#include "ECS/loggersystem.h"

#include "rulevm.h"

// Chronicle's solver: a PgScript script that plays the game's rules to find out whether every task
// can be done. This host only runs it, and lends it the rule scripts of the game:
//
//   ChronicleSolver <rules dir> <solver.pg> [args...]
//
// The rule scripts cannot be run again with other inputs from PgScript (an import runs once, when
// it is compiled), so each one is loaded here in a VM of its own, as the game loads it, and the
// solver's script asks for a run with a native:
//
//   rule(name, inputs, outputs)
//       name     the script of the rules dir, without ".pg": "windows", "forecast", "resources",
//                "town", "expedition", "achievements", "milestones", "epitaph"
//       inputs   a table: global name -> value, defined before the run (tables and lists are
//                copied whole)
//       outputs  a list of the global names to hand back
//       returns  {ok, errors [text], <each output>: its value, copied whole, none when the run
//                left no such global}
//
// and with a few helpers the CLI does not have:
//
//   clone(value)   a deep copy of a table or a list (a state of the search branches off from it)
//   clockMs()      milliseconds since the host started
//   getArg(i), getArgCount()   as the PgCompiler CLI's: 0 the host, 1 the script, 2... the user's
//
// No window and no scene: the EntitySystem is there for its script registry, as ChronicleRules'.

using namespace pg;

namespace
{
    // A value of `from` made anew in `to`, tables and lists all the way down. Functions and
    // anything else that cannot cross are none.
    Value across(VM& from, Value v, VM& to)
    {
        if (IS_INT(v) or IS_BOOL(v) or IS_DOUBLE(v) or IS_NONE(v))
            return v;

        if (IS_STRING(v))
            return to.createString(from.asString(v));

        if (IS_VECTOR(v))
        {
            Value out = to.createVector();
            ObjVector* list = to.asVector(out);

            const std::vector<Value>& fields = from.asVector(v)->fields;
            list->fields.reserve(fields.size());

            for (Value item : fields)
                list->fields.push_back(across(from, item, to));

            return out;
        }

        if (IS_INSTANCE(v))
        {
            ObjInstance* instance = from.asInstance(v);

            Value out = to.createTable();
            ObjInstance* table = to.asInstance(out);

            for (size_t i = 0; i < instance->fieldValues.size(); ++i)
            {
                const std::string& key = instance->fieldNames[i];

                if (key.empty() or key.rfind("__", 0) == 0)
                    continue;

                table->setField(key, across(from, instance->fieldValues[i], to));
            }

            return out;
        }

        return makeNoneValue();
    }

    // The same within one VM
    Value cloned(VM& vm, Value v)
    {
        return across(vm, v, vm);
    }

    // A value written out the same way whatever order its table was filled in: the key of a call
    void keyOf(VM& vm, Value v, std::string& out)
    {
        if (IS_INT(v))
        {
            out += 'i';
            out += std::to_string(AS_INT(v));
        }
        else if (IS_DOUBLE(v))
        {
            char buf[32];
            std::snprintf(buf, sizeof(buf), "d%.9g", AS_DOUBLE(v));
            out += buf;
        }
        else if (IS_BOOL(v))
            out += AS_BOOL(v) ? "T" : "F";
        else if (IS_NONE(v))
            out += 'N';
        else if (IS_STRING(v))
        {
            const std::string text = vm.asString(v);
            out += 's';
            out += std::to_string(text.size());
            out += ':';
            out += text;
        }
        else if (IS_VECTOR(v))
        {
            out += '[';

            for (Value item : vm.asVector(v)->fields)
            {
                keyOf(vm, item, out);
                out += ',';
            }

            out += ']';
        }
        else if (IS_INSTANCE(v))
        {
            ObjInstance* instance = vm.asInstance(v);
            std::vector<size_t> order;

            for (size_t i = 0; i < instance->fieldNames.size(); ++i)
            {
                if (not instance->fieldNames[i].empty() and instance->fieldNames[i].rfind("__", 0) != 0)
                    order.push_back(i);
            }

            std::sort(order.begin(), order.end(), [instance](size_t a, size_t b) { return instance->fieldNames[a] < instance->fieldNames[b]; });

            out += '{';

            for (size_t i : order)
            {
                out += instance->fieldNames[i];
                out += '=';
                keyOf(vm, instance->fieldValues[i], out);
                out += ';';
            }

            out += '}';
        }
        else
            out += '?';
    }

    // Where the time goes, a rule script at a time: `CHRONICLE_SOLVER_PROFILE=1` prints it at the end
    struct Spent
    {
        long calls = 0;
        long repeats = 0;      // Calls whose inputs were seen before: what a cache would spare
        double keying = 0.0;   // Milliseconds spent writing the keys
        std::unordered_set<std::string> seen;
        double copyIn = 0.0;   // Milliseconds: the inputs copied into the rule's VM
        double run = 0.0;      // The script itself
        double copyOut = 0.0;  // The outputs copied back
    };

    double since(std::chrono::steady_clock::time_point t)
    {
        return std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - t).count();
    }

    // The answers already given, by script: a rule script keeps nothing between runs and its dice
    // are an input, so the same inputs always bring the same answer. Kept in the solver's VM, as a
    // copy of their own; the oldest go once a script holds `most` of them
    struct Kept
    {
        size_t most = 0;
        std::unordered_map<std::string, Value> answers;
        std::list<std::string> order;
        long hits = 0;
    };

    struct Host
    {
        std::map<std::string, Kept> kept;
        bool caching = []{ const char* c = std::getenv("CHRONICLE_SOLVER_CACHE"); return not c or std::string(c) != "0"; }();

        // How many answers each script keeps: an activity list is large, a month is small
        size_t mostFor(const std::string& name) const
        {
            if (name == "windows")
                return 600;

            if (name == "forecast")
                return 20000;

            if (name == "expedition")
                return 30000;

            return 60000;
        }

        std::map<std::string, Spent> spent;
        bool profiling = std::getenv("CHRONICLE_SOLVER_PROFILE") != nullptr;
        std::string vmProfileDir = []{ const char* d = std::getenv("CHRONICLE_SOLVER_VMPROFILE"); return d ? std::string(d) : std::string(); }();

        // Every instruction the profiler saw in `vm`: function, offset, opcode, count, nanoseconds
        static void dumpProfile(const VM& vm, const std::string& file)
        {
            FILE* out = std::fopen(file.c_str(), "w");

            if (not out)
                return;

            std::fprintf(out, "function\toffset\tline\topcode\tcount\tns\n");

            for (const auto& p : vm.profiler.getResultsSortedByTime())
            {
                // The source line of the instruction, from its chunk (alive while its script is)
                const Chunk* chunk = static_cast<const Chunk*>(p.chunkPtr);
                const int line = chunk and p.instructionOffset < chunk->lines.size() ? chunk->lines[p.instructionOffset] : -1;

                std::fprintf(out, "%s\t%zu\t%d\t%s\t%llu\t%llu\n", p.functionName.c_str(), p.instructionOffset, line, p.opcodeName.c_str(),
                             static_cast<unsigned long long>(p.executionCount), static_cast<unsigned long long>(p.totalNanoseconds));
            }

            std::fclose(out);
        }

        void dumpProfiles(const VM& solverVm) const
        {
            if (vmProfileDir.empty())
                return;

            std::filesystem::create_directories(vmProfileDir);

            for (const auto& [name, rs] : scripts)
                dumpProfile(*rs->machine(), vmProfileDir + "/" + name + ".tsv");

            dumpProfile(solverVm, vmProfileDir + "/solver.tsv");
            std::fprintf(stderr, "VM profiles written to %s\n", vmProfileDir.c_str());
        }

        EntitySystem ecs;
        std::string rulesDir;
        std::map<std::string, std::unique_ptr<chronicle::RuleScript>> scripts;
        std::vector<std::string> argv;
        std::chrono::steady_clock::time_point started = std::chrono::steady_clock::now();

        chronicle::RuleScript* script(const std::string& name)
        {
            auto it = scripts.find(name);

            if (it != scripts.end())
                return it->second.get();

            // The game's way: the source when there is one, else what was compiled of it
            std::string path = rulesDir + "/" + name + ".pg";

            if (not std::filesystem::exists(path) and std::filesystem::exists(path + "c"))
                path += "c";

            auto loaded = std::make_unique<chronicle::RuleScript>();

            if (not loaded->load(&ecs, path))
            {
                for (const auto& e : loaded->errors())
                    std::fprintf(stderr, "%s\n", e.c_str());

                return nullptr;
            }

            // The VM's own profiler, instruction by instruction (CHRONICLE_SOLVER_VMPROFILE=<dir>)
            if (not vmProfileDir.empty())
                loaded->machine()->enableProfiling();

            return (scripts[name] = std::move(loaded)).get();
        }

        Value rule(VM* vm, int argCount, Value* args)
        {
            if (argCount != 3 or not IS_STRING(args[0]) or not IS_INSTANCE(args[1]) or not IS_VECTOR(args[2]))
                throw std::runtime_error("rule expects (name, inputs table, outputs list)");

            const std::string name = vm->asString(args[0]);

            Value answer = vm->createTable();
            ObjInstance* out = vm->asInstance(answer);

            Value errors = vm->createVector();
            ObjVector* errorList = vm->asVector(errors);

            chronicle::RuleScript* rs = script(name);

            if (not rs)
            {
                errorList->fields.push_back(vm->createString("rule script `" + name + "` does not load"));
                out->setField("ok", makeBoolValue(false));
                out->setField("errors", errors);
                return answer;
            }

            VM& rvm = *rs->machine();
            Spent& cost = spent[name];
            ++cost.calls;

            std::string key;

            if (profiling or caching)
            {
                auto tk = std::chrono::steady_clock::now();
                keyOf(*vm, args[1], key);
                key += '|';
                keyOf(*vm, args[2], key);

                if (profiling and not caching and not cost.seen.insert(key).second)
                    ++cost.repeats;

                cost.keying += since(tk);
            }

            Kept* store = nullptr;

            if (caching)
            {
                store = &kept[name];

                if (store->most == 0)
                    store->most = mostFor(name);

                auto hit = store->answers.find(key);

                if (hit != store->answers.end())
                {
                    ++store->hits;
                    ++cost.repeats;

                    // (the answer we built for this call goes: the copy of the kept one stands for it)
                    vm->releaseAndDelete(answer);
                    vm->releaseAndDelete(errors);

                    return cloned(*vm, hit->second);
                }
            }

            auto t0 = std::chrono::steady_clock::now();

            ObjInstance* inputs = vm->asInstance(args[1]);

            for (size_t i = 0; i < inputs->fieldValues.size(); ++i)
            {
                const std::string& key = inputs->fieldNames[i];

                if (key.empty() or key.rfind("__", 0) == 0)
                    continue;

                rvm.defineGlobal(key, across(*vm, inputs->fieldValues[i], rvm));
            }

            cost.copyIn += since(t0);

            auto t1 = std::chrono::steady_clock::now();

            rs->clearErrors();
            const bool ok = rs->run();

            cost.run += since(t1);

            auto t2 = std::chrono::steady_clock::now();

            for (const auto& e : rs->errors())
                errorList->fields.push_back(vm->createString(e));

            for (Value wanted : vm->asVector(args[2])->fields)
            {
                if (not IS_STRING(wanted))
                    continue;

                const std::string global = vm->asString(wanted);
                const Value v = rvm.findGlobal(global);

                out->setField(global, IS_UNDEFINED(v) ? makeNoneValue() : across(rvm, v, *vm));
            }

            out->setField("ok", makeBoolValue(ok));
            out->setField("errors", errors);

            cost.copyOut += since(t2);

            // Kept for the next time it is asked, when it is a true answer
            if (store and ok)
            {
                store->answers[key] = cloned(*vm, answer);
                store->order.push_back(key);

                while (store->order.size() > store->most)
                {
                    auto oldest = store->answers.find(store->order.front());

                    if (oldest != store->answers.end())
                    {
                        vm->releaseAndDelete(oldest->second);
                        store->answers.erase(oldest);
                    }

                    store->order.pop_front();
                }
            }

            return answer;
        }

        void report() const
        {
            const double total = since(started);
            double inRules = 0.0;

            std::fprintf(stderr, "\n%-14s %8s %8s %10s %10s %10s %9s %8s\n", "rule script", "calls", caching ? "cached" : "repeats", "copy in s", "run s", "copy out s", "ms/call", "key s");

            for (const auto& [name, c] : spent)
            {
                const double all = c.copyIn + c.run + c.copyOut;
                inRules += all;
                std::fprintf(stderr, "%-14s %8ld %8ld %10.1f %10.1f %10.1f %9.2f %8.1f\n", name.c_str(), c.calls, c.repeats, c.copyIn / 1000.0, c.run / 1000.0, c.copyOut / 1000.0, c.calls ? all / c.calls : 0.0, c.keying / 1000.0);
            }

            std::fprintf(stderr, "in the rules %.1f s of %.1f s; the solver's own PgScript %.1f s\n", inRules / 1000.0, total / 1000.0, (total - inRules) / 1000.0);
        }
    };
}

int main(int argc, char* argv[])
{
    if (argc < 3)
    {
        std::fprintf(stderr, "usage: ChronicleSolver <rules dir> <solver.pg> [args...]\n");
        return 2;
    }

    // Errors only: the rule scripts' loading and the VM are talkative
    auto terminalSink = std::shared_ptr<Logger::LogSink>(Logger::registerSink<TerminalSink>());

    if (auto* sink = dynamic_cast<TerminalSink*>(terminalSink.get()))
        sink->setVerboseInfo(false);

    terminalSink->addFilter("log", new Logger::LogSink::FilterLogLevel(Logger::InfoLevel::log));
    terminalSink->addFilter("info", new Logger::LogSink::FilterLogLevel(Logger::InfoLevel::info));
    terminalSink->addFilter("mile", new Logger::LogSink::FilterLogLevel(Logger::InfoLevel::mile));
    terminalSink->addFilter("test", new Logger::LogSink::FilterLogLevel(Logger::InfoLevel::test));
    terminalSink->addFilter("warn", new Logger::LogSink::FilterLogLevel(Logger::InfoLevel::warning));

    auto host = std::make_unique<Host>();
    host->rulesDir = argv[1];

    // As the CLI hands them: 0 the program, 1 the script, then the user's
    host->argv.push_back(argv[0]);

    for (int i = 2; i < argc; ++i)
        host->argv.push_back(argv[i]);

    auto vm = std::make_unique<VM>();
    host->ecs.setupVm(*vm);

    if (not host->vmProfileDir.empty())
        vm->enableProfiling();

    Host* h = host.get();

    vm->defineNative("rule", [h](VM* v, int argCount, Value* args) -> Value {
        return h->rule(v, argCount, args);
    });

    vm->defineNative("clone", [](VM* v, int argCount, Value* args) -> Value {
        if (argCount != 1)
            throw std::runtime_error("clone expects one argument");

        return cloned(*v, args[0]);
    });

    vm->defineNative("clockMs", [h](VM*, int, Value*) -> Value {
        const auto since = std::chrono::steady_clock::now() - h->started;
        return makeIntValue(std::chrono::duration_cast<std::chrono::milliseconds>(since).count());
    });

    vm->defineNative("getArg", [h](VM* v, int argCount, Value* args) -> Value {
        if (argCount != 1 or not IS_INT(args[0]))
            throw std::runtime_error("getArg expects an integer index");

        const int64_t index = AS_INT(args[0]);

        if (index < 0 or index >= static_cast<int64_t>(h->argv.size()))
            return v->createString("");

        return v->createString(h->argv[index]);
    });

    vm->defineNative("getArgCount", [h](VM*, int, Value*) -> Value {
        return makeIntValue(static_cast<int64_t>(h->argv.size()));
    });

    vm->defineNative("log", [](VM* v, int argCount, Value* args) -> Value {
        for (int i = 0; i < argCount; ++i)
        {
            if (i > 0)
                std::cout << " ";

            if (IS_STRING(args[i]))
                std::cout << v->asString(args[i]);
            else if (IS_INT(args[i]))
                std::cout << AS_INT(args[i]);
            else if (IS_DOUBLE(args[i]))
                std::cout << AS_DOUBLE(args[i]);
            else if (IS_BOOL(args[i]))
                std::cout << (AS_BOOL(args[i]) ? "true" : "false");
            else
                std::cout << "none";
        }

        std::cout << std::endl;
        return makeNoneValue();
    });

    const InterpretResult result = vm->interpretFromFile(argv[2]);

    std::cout.flush();

    if (std::getenv("CHRONICLE_SOLVER_PROFILE"))
        host->report();

    host->dumpProfiles(*vm);

    return result == InterpretResult::OK ? 0 : 1;
}
