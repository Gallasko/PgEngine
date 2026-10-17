#include <atomic>
#include <chrono>
#include <condition_variable>
#include <cstdio>
#include <cstdlib>
#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <list>
#include <map>
#include <mutex>
#include <thread>
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
//   ChronicleSolver <rules dir> <solver.pg> [args...] [threads=N]
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
//   scalarsOf(table), scalarsOfEach(list)   a table's numbers, booleans and strings only, of one
//                  table or of each of a list's (what the C++ hands a rule script of its tables)
//   chronicleCharacter(stats, townKnown, town, fund, seen, know), chronicleTakeStats(stats, know,
//                  after)   the character between the solver's state and the rules, as the Life
//                  screen's C++ makes it and takes it back (life.pg's character and takeStats)
//   clockMs()      milliseconds since the host started
//   getArg(i), getArgCount()   as the PgCompiler CLI's: 0 the host, 1 the script, 2... the user's
//
// Threads (threads=N, the cores by default; threads=0: none). The solver's script is compiled once
// and run by N more VMs, the workers, each with rule scripts of its own, made when the main script
// first wants them (startWorkers(), or its first parallel); their script starts in the role
// "worker" (it reads the tables and waits). The main script hands them work:
//
//   parallel(function, jobs, setup, version)
//       function  the name of a global function of the script, called with one job in a worker; it
//                 leaves its answer in the global `jobResult`
//       jobs      a list, each copied whole into whichever worker takes it
//       setup     a table of globals defined in every worker before its first job of the call,
//                 when `version` differs from the last one it was given
//       returns   the answers, in the jobs' order, copied whole
//   solverRole()   "main" or "worker"
//   threadCount()  the workers there are, or will be (0: parallel cannot be called)
//   startWorkers() the workers made and started now
//
// The rules' answers are kept for all of them alike: the same inputs bring the same answer.
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

            for (size_t i = 0; i < instance->fieldCount(); ++i)
            {
                const std::string& key = instance->fieldName(i);

                if (key.empty() or key.rfind("__", 0) == 0)
                    continue;

                table->setField(key, across(from, instance->fieldValue(i), to));
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

    // A value outside any VM, as `across` copies one: what crosses between threads (a job, its
    // answer, a kept answer of the rules)
    struct Tree
    {
        enum class Kind : uint8_t { None, Int, Double, Bool, String, List, Table };

        Kind kind = Kind::None;
        int64_t whole = 0;               // Int; Bool as 0 or 1
        double real = 0.0;               // Double
        std::string text;                // String
        std::vector<Tree> items;         // A list's items; a table's values
        std::vector<std::string> keys;   // A table's keys, as `items`
    };

    void treeOf(VM& vm, Value v, Tree& out)
    {
        if (IS_INT(v))
        {
            out.kind = Tree::Kind::Int;
            out.whole = AS_INT(v);
        }
        else if (IS_DOUBLE(v))
        {
            out.kind = Tree::Kind::Double;
            out.real = AS_DOUBLE(v);
        }
        else if (IS_BOOL(v))
        {
            out.kind = Tree::Kind::Bool;
            out.whole = AS_BOOL(v) ? 1 : 0;
        }
        else if (IS_STRING(v))
        {
            out.kind = Tree::Kind::String;
            out.text = vm.asString(v);
        }
        else if (IS_VECTOR(v))
        {
            out.kind = Tree::Kind::List;

            const std::vector<Value>& fields = vm.asVector(v)->fields;
            out.items.resize(fields.size());

            for (size_t i = 0; i < fields.size(); ++i)
                treeOf(vm, fields[i], out.items[i]);
        }
        else if (IS_INSTANCE(v))
        {
            out.kind = Tree::Kind::Table;

            ObjInstance* instance = vm.asInstance(v);
            out.items.reserve(instance->fieldCount());
            out.keys.reserve(instance->fieldCount());

            for (size_t i = 0; i < instance->fieldCount(); ++i)
            {
                const std::string& key = instance->fieldName(i);

                if (key.empty() or key.rfind("__", 0) == 0)
                    continue;

                out.keys.push_back(key);
                out.items.emplace_back();
                treeOf(vm, instance->fieldValue(i), out.items.back());
            }
        }
        else
            out.kind = Tree::Kind::None;
    }

    Value valueOf(VM& vm, const Tree& t)
    {
        switch (t.kind)
        {
        case Tree::Kind::Int:
            return makeIntValue(t.whole);

        case Tree::Kind::Double:
            return makeDoubleValue(t.real);

        case Tree::Kind::Bool:
            return makeBoolValue(t.whole != 0);

        case Tree::Kind::String:
            return vm.createString(t.text);

        case Tree::Kind::List:
        {
            Value out = vm.createVector();
            ObjVector* list = vm.asVector(out);
            list->fields.reserve(t.items.size());

            for (const Tree& item : t.items)
                list->fields.push_back(valueOf(vm, item));

            return out;
        }

        case Tree::Kind::Table:
        {
            Value out = vm.createTable();
            ObjInstance* table = vm.asInstance(out);

            for (size_t i = 0; i < t.keys.size(); ++i)
                table->setField(t.keys[i], valueOf(vm, t.items[i]));

            return out;
        }

        case Tree::Kind::None:
        default:
            return makeNoneValue();
        }
    }

    // A table's numbers, booleans and strings, in a table of their own: what the C++ hands a rule
    // script of a table (an ElementMap). Anything else is an empty table
    Value scalarsOf(VM& vm, Value v)
    {
        Value out = vm.createTable();

        if (not IS_INSTANCE(v))
            return out;

        ObjInstance* from = vm.asInstance(v);
        ObjInstance* table = vm.asInstance(out);

        for (size_t i = 0; i < from->fieldCount(); ++i)
        {
            const Value field = from->fieldValue(i);

            if (IS_INT(field) or IS_DOUBLE(field) or IS_BOOL(field) or IS_STRING(field))
                table->setField(from->fieldName(i), vm.retainValue(field));
        }

        return out;
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

            for (size_t i = 0; i < instance->fieldCount(); ++i)
            {
                if (not instance->fieldName(i).empty() and instance->fieldName(i).rfind("__", 0) != 0)
                    order.push_back(i);
            }

            std::sort(order.begin(), order.end(), [instance](size_t a, size_t b) { return instance->fieldName(a) < instance->fieldName(b); });

            out += '{';

            for (size_t i : order)
            {
                out += instance->fieldName(i);
                out += '=';
                keyOf(vm, instance->fieldValue(i), out);
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
    // are an input, so the same inputs always bring the same answer. Kept outside any VM, for every
    // thread alike; the oldest go once a script holds `most` of them
    struct Kept
    {
        size_t most = 0;
        std::unordered_map<std::string, std::shared_ptr<const Tree>> answers;
        std::list<std::string> order;
        std::unordered_set<std::string> pending;   // Being run by a thread: the others wait for it
        long hits = 0;
    };

    // ---- Chronicle's own: the character between the solver's state and the rules -------------
    // What the Life screen's C++ does (LifeSave::character, LifeScene::takeStats + takeTown), with
    // the solver's state as tables: done here, a walk step or a month at a time, rather than key by
    // key in PgScript

    // A whole number as the Life screen makes it of what a script hands back: cut toward zero
    int64_t asWhole(Value v)
    {
        if (IS_INT(v))
            return AS_INT(v);

        if (IS_DOUBLE(v))
            return static_cast<int64_t>(AS_DOUBLE(v));

        if (IS_BOOL(v))
            return AS_BOOL(v) ? 1 : 0;

        return 0;
    }

    bool startsWith(const std::string& text, const char* prefix)
    {
        return text.rfind(prefix, 0) == 0;
    }

    // chronicleCharacter(stats, townKnown, town, fund, seen, know): his stats, then the town and
    // what the world knows, as the rules read them ("town.<place>", "fund.<key>", "seen.<place>",
    // "know.<key>")
    Value chronicleCharacter(VM& vm, Value* args)
    {
        Value out = vm.createTable();
        ObjInstance* c = vm.asInstance(out);

        if (IS_INSTANCE(args[0]))
        {
            ObjInstance* stats = vm.asInstance(args[0]);

            for (size_t i = 0; i < stats->fieldCount(); ++i)
                c->setField(stats->fieldName(i), vm.retainValue(stats->fieldValue(i)));
        }

        c->setField("town_known", makeIntValue(IS_BOOL(args[1]) and AS_BOOL(args[1]) ? 1 : 0));

        struct Part { int arg; const char* prefix; bool one; };

        for (const Part& part : {Part{2, "town.", false}, Part{3, "fund.", false}, Part{4, "seen.", true}, Part{5, "know.", false}})
        {
            if (not IS_INSTANCE(args[part.arg]))
                continue;

            ObjInstance* from = vm.asInstance(args[part.arg]);

            for (size_t i = 0; i < from->fieldCount(); ++i)
                c->setField(part.prefix + from->fieldName(i), part.one ? makeIntValue(1) : vm.retainValue(from->fieldValue(i)));
        }

        return out;
    }

    // chronicleTakeStats(stats, know, after): what a script handed back made the character's.
    // Every value a whole number; "know.<key>" is the world's knowledge; "town.", "fund.", "seen."
    // are the world's and left alone (the solver keeps them, as the C++ does). Writes into `stats`
    // and `know`; true when the town has come to be known
    Value chronicleTakeStats(VM& vm, Value* args)
    {
        if (not IS_INSTANCE(args[0]) or not IS_INSTANCE(args[1]) or not IS_INSTANCE(args[2]))
            throw std::runtime_error("chronicleTakeStats expects (stats, know, after) tables");

        ObjInstance* stats = vm.asInstance(args[0]);
        ObjInstance* know = vm.asInstance(args[1]);
        ObjInstance* after = vm.asInstance(args[2]);
        bool townKnown = false;

        for (size_t i = 0; i < after->fieldCount(); ++i)
        {
            const std::string& key = after->fieldName(i);
            const Value v = makeIntValue(asWhole(after->fieldValue(i)));

            if (key == "town_known")
            {
                if (AS_INT(v) > 0)
                    townKnown = true;
            }
            else if (startsWith(key, "know."))
            {
                know->setField(key.substr(5), v, &vm, true);
            }
            else if (not startsWith(key, "town.") and not startsWith(key, "fund.") and not startsWith(key, "seen."))
            {
                stats->setField(key, v, &vm, true);
            }
        }

        return makeBoolValue(townKnown);
    }

    // The rule scripts a worker may ask for: loaded before the threads start (the script registry
    // is not for several threads)
    const char* const workerRules[] = {"windows", "forecast", "resources", "town", "expedition"};

    // A VM running the solver's script, with rule scripts of its own
    struct Machine
    {
        std::unique_ptr<VM> vm;
        ObjFunction* fn = nullptr;
        bool worker = false;
        bool sealed = false;   // Its rule scripts loaded: it loads no other (a worker, on its thread)
        std::map<std::string, std::unique_ptr<chronicle::RuleScript>> scripts;
        std::map<std::string, Spent> spent;
        long setupVersion = -1;
        std::string failure;   // A worker's script that did not start
    };

    // One call of `parallel`: its jobs, taken by the workers one at a time, and their answers
    struct Batch
    {
        std::string function;
        std::shared_ptr<const Tree> setup;
        long setupVersion = 0;
        std::vector<Tree> jobs;
        std::vector<Tree> answers;
        std::vector<std::string> failures;
        std::vector<double> jobMs;
        std::atomic<size_t> next{0};
        size_t workersDone = 0;
    };

    struct Host;

    // The natives every VM running the solver's script has (below)
    void defineNatives(Host* h, Machine* m, VM& vm);

    struct Host
    {
        std::map<std::string, Kept> kept;
        std::mutex keptLock;
        std::condition_variable keptReady;
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

        bool profiling = std::getenv("CHRONICLE_SOLVER_PROFILE") != nullptr;
        std::string vmProfileDir = []{ const char* d = std::getenv("CHRONICLE_SOLVER_VMPROFILE"); return d ? std::string(d) : std::string(); }();

        EntitySystem ecs;
        std::string rulesDir;
        std::string scriptPath;
        std::vector<std::string> argv;
        std::chrono::steady_clock::time_point started = std::chrono::steady_clock::now();

        Machine main;
        std::vector<std::unique_ptr<Machine>> workers;

        // ---- the workers' threads -------------------------------------------------------------
        std::vector<std::thread> threads;
        std::mutex poolLock;
        std::condition_variable wake;
        std::condition_variable finished;
        Batch* batch = nullptr;
        long generation = 0;
        bool stopping = false;
        double waited = 0.0;      // Milliseconds the main script spent in `parallel`
        double sending = 0.0;     // Of it: its jobs written out of the main VM
        double working = 0.0;     // The workers at them
        double taking = 0.0;      // Their answers made in the main VM
        // By job function: its calls, jobs, every job's time summed, the slowest job of each call
        // summed (what no number of threads goes under), the longest job, the main script's wait
        struct Work
        {
            long calls = 0;
            long jobs = 0;
            double jobsMs = 0.0;
            double slowestSum = 0.0;
            double longestMs = 0.0;
            double waitedMs = 0.0;
        };

        std::map<std::string, Work> work;
        long batches = 0;
        long jobsRun = 0;

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

        // The main VM's and its rule scripts' (the workers' are not written)
        void dumpProfiles() const
        {
            if (vmProfileDir.empty())
                return;

            std::filesystem::create_directories(vmProfileDir);

            for (const auto& [name, rs] : main.scripts)
                dumpProfile(*rs->machine(), vmProfileDir + "/" + name + ".tsv");

            dumpProfile(*main.vm, vmProfileDir + "/solver.tsv");
            std::fprintf(stderr, "VM profiles written to %s\n", vmProfileDir.c_str());
        }

        chronicle::RuleScript* script(Machine& m, const std::string& name)
        {
            auto it = m.scripts.find(name);

            if (it != m.scripts.end())
                return it->second.get();

            // A worker loads nothing on its thread: what it may ask for was loaded before
            if (m.sealed)
                return nullptr;

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
            if (not vmProfileDir.empty() and not m.worker)
                loaded->machine()->enableProfiling();

            return (m.scripts[name] = std::move(loaded)).get();
        }

        Value rule(Machine& m, VM* vm, int argCount, Value* args)
        {
            if (argCount != 3 or not IS_STRING(args[0]) or not IS_INSTANCE(args[1]) or not IS_VECTOR(args[2]))
                throw std::runtime_error("rule expects (name, inputs table, outputs list)");

            const std::string name = vm->asString(args[0]);

            Value answer = vm->createTable();
            ObjInstance* out = vm->asInstance(answer);

            Value errors = vm->createVector();
            ObjVector* errorList = vm->asVector(errors);

            chronicle::RuleScript* rs = script(m, name);

            if (not rs)
            {
                errorList->fields.push_back(vm->createString("rule script `" + name + "` does not load"));
                out->setField("ok", makeBoolValue(false));
                out->setField("errors", errors);
                return answer;
            }

            VM& rvm = *rs->machine();
            Spent& cost = m.spent[name];
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

            bool running = false;   // This thread runs it: the others asking the same wait for it

            if (caching)
            {
                std::shared_ptr<const Tree> hit;

                {
                    std::unique_lock<std::mutex> lock(keptLock);
                    Kept& store = kept[name];

                    if (store.most == 0)
                        store.most = mostFor(name);

                    // Asked by another thread at the same time: its answer, once it has it
                    keptReady.wait(lock, [&] { return store.pending.count(key) == 0; });

                    auto found = store.answers.find(key);

                    if (found != store.answers.end())
                    {
                        ++store.hits;
                        hit = found->second;
                    }
                    else
                    {
                        store.pending.insert(key);
                        running = true;
                    }
                }

                if (hit)
                {
                    ++cost.repeats;

                    // (the answer we built for this call goes: a copy of the kept one stands for it)
                    vm->releaseAndDelete(answer);
                    vm->releaseAndDelete(errors);

                    return valueOf(*vm, *hit);
                }
            }

            // Whatever happens below, the threads waiting for this answer are let go
            struct Release
            {
                Host* host;
                const std::string& name;
                const std::string& key;
                bool& running;

                ~Release()
                {
                    if (not running)
                        return;

                    {
                        std::lock_guard<std::mutex> lock(host->keptLock);
                        host->kept[name].pending.erase(key);
                    }

                    host->keptReady.notify_all();
                }
            } release{this, name, key, running};

            auto t0 = std::chrono::steady_clock::now();

            ObjInstance* inputs = vm->asInstance(args[1]);

            for (size_t i = 0; i < inputs->fieldCount(); ++i)
            {
                const std::string& key = inputs->fieldName(i);

                if (key.empty() or key.rfind("__", 0) == 0)
                    continue;

                rvm.defineGlobal(key, across(*vm, inputs->fieldValue(i), rvm));
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

            // Kept for the next time it is asked, by any thread, when it is a true answer
            if (running)
            {
                std::shared_ptr<Tree> tree;

                if (ok)
                {
                    tree = std::make_shared<Tree>();
                    treeOf(*vm, answer, *tree);
                }

                {
                    std::lock_guard<std::mutex> lock(keptLock);
                    Kept& store = kept[name];

                    if (tree and store.answers.emplace(key, std::move(tree)).second)
                    {
                        store.order.push_back(key);

                        while (store.order.size() > store.most)
                        {
                            store.answers.erase(store.order.front());
                            store.order.pop_front();
                        }
                    }

                    store.pending.erase(key);
                }

                running = false;
                keptReady.notify_all();
            }

            return answer;
        }

        // ---- parallel ---------------------------------------------------------------------------

        // One job in worker `w`: its function called with the job, its `jobResult` the answer
        void runJob(Machine& w, Batch& b, size_t i)
        {
            VM& vm = *w.vm;
            const Value fn = vm.findGlobal(b.function);

            if (not IS_CLOSURE(fn))
            {
                b.failures[i] = "no function `" + b.function + "` in the worker";
                return;
            }

            vm.defineGlobal("jobResult", makeNoneValue());

            vm.push(vm.retainValue(fn));
            vm.push(valueOf(vm, b.jobs[i]));

            InterpretResult result = InterpretResult::RUNTIME_ERROR;

            try
            {
                if (vm.call(vm.asClosure(fn), 1))
                    result = vm.run();
            }
            catch (const std::exception& e)
            {
                LOG_ERROR("ChronicleSolver", "worker: " << e.what());
                vm.resetStack();
            }

            if (result != InterpretResult::OK)
            {
                vm.resetStack();
                b.failures[i] = "job " + std::to_string(i) + " of `" + b.function + "` ended in an error (the VM's message is in the log above)";
                return;
            }

            treeOf(vm, vm.findGlobal("jobResult"), b.answers[i]);
            vm.defineGlobal("jobResult", makeNoneValue());
        }

        void workerLoop(Machine& w)
        {
            // Its script first: the role "worker" reads the tables, then waits
            const InterpretResult started = w.vm->runPreparedFunction(w.fn, 0, scriptPath);

            if (started != InterpretResult::OK)
                w.failure = "a worker's script did not start (the VM's message is in the log above)";

            long seen = 0;

            while (true)
            {
                Batch* b = nullptr;

                {
                    std::unique_lock<std::mutex> lock(poolLock);
                    wake.wait(lock, [&] { return stopping or generation != seen; });

                    if (stopping)
                        return;

                    seen = generation;
                    b = batch;
                }

                if (w.failure.empty() and w.setupVersion != b->setupVersion and b->setup and b->setup->kind == Tree::Kind::Table)
                {
                    for (size_t i = 0; i < b->setup->keys.size(); ++i)
                        w.vm->defineGlobal(b->setup->keys[i], valueOf(*w.vm, b->setup->items[i]));

                    w.setupVersion = b->setupVersion;
                }

                while (true)
                {
                    const size_t i = b->next.fetch_add(1);

                    if (i >= b->jobs.size())
                        break;

                    auto tj = std::chrono::steady_clock::now();

                    if (not w.failure.empty())
                        b->failures[i] = w.failure;
                    else
                        runJob(w, *b, i);

                    b->jobMs[i] = since(tj);
                }

                {
                    std::lock_guard<std::mutex> lock(poolLock);
                    ++b->workersDone;
                }

                finished.notify_all();
            }
        }

        // parallel(function, jobs, setup, version): see the top of the file
        std::shared_ptr<const Tree> lastSetup;
        long lastSetupVersion = -1;

        Value parallel(VM* vm, int argCount, Value* args)
        {
            if (argCount != 4 or not IS_STRING(args[0]) or not IS_VECTOR(args[1]) or not IS_INT(args[3]))
                throw std::runtime_error("parallel expects (function name, jobs list, setup table, version)");

            startWorkers();

            if (threads.empty())
                throw std::runtime_error("parallel: there are no workers (threads=0)");

            auto t0 = std::chrono::steady_clock::now();

            Batch b;
            b.function = vm->asString(args[0]);
            b.setupVersion = AS_INT(args[3]);

            if (b.setupVersion != lastSetupVersion or not lastSetup)
            {
                auto setup = std::make_shared<Tree>();
                treeOf(*vm, args[2], *setup);
                lastSetup = setup;
                lastSetupVersion = b.setupVersion;
            }

            b.setup = lastSetup;

            const std::vector<Value>& jobs = vm->asVector(args[1])->fields;
            b.jobs.resize(jobs.size());
            b.answers.resize(jobs.size());
            b.failures.resize(jobs.size());
            b.jobMs.resize(jobs.size());

            for (size_t i = 0; i < jobs.size(); ++i)
                treeOf(*vm, jobs[i], b.jobs[i]);

            sending += since(t0);
            auto t1 = std::chrono::steady_clock::now();

            {
                std::unique_lock<std::mutex> lock(poolLock);
                batch = &b;
                ++generation;
                wake.notify_all();

                finished.wait(lock, [&] { return b.workersDone == threads.size(); });
                batch = nullptr;
            }

            working += since(t1);

            Work& done = work[b.function];
            ++done.calls;
            done.jobs += static_cast<long>(b.jobMs.size());
            done.waitedMs += since(t1);

            for (double ms : b.jobMs)
            {
                done.jobsMs += ms;
                done.longestMs = std::max(done.longestMs, ms);
            }

            if (not b.jobMs.empty())
                done.slowestSum += *std::max_element(b.jobMs.begin(), b.jobMs.end());
            auto t2 = std::chrono::steady_clock::now();

            ++batches;
            jobsRun += static_cast<long>(jobs.size());

            for (const std::string& failure : b.failures)
            {
                if (not failure.empty())
                    throw std::runtime_error("parallel: " + failure);
            }

            Value out = vm->createVector();
            ObjVector* list = vm->asVector(out);
            list->fields.reserve(b.answers.size());

            for (const Tree& answer : b.answers)
                list->fields.push_back(valueOf(*vm, answer));

            taking += since(t2);
            waited += since(t0);

            return out;
        }

        // The workers, made on the main thread when the script first wants them (startWorkers or
        // parallel): each starts its script on its own thread at once
        std::vector<char> bytes;
        int wantedWorkers = 0;

        void startWorkers()
        {
            if (not workers.empty() or wantedWorkers <= 0)
                return;

            for (int i = 0; i < wantedWorkers; ++i)
            {
                auto w = std::make_unique<Machine>();
                w->worker = true;
                w->vm = std::make_unique<VM>();
                ecs.setupVm(*w->vm);
                defineNatives(this, w.get(), *w->vm);

                for (const char* name : workerRules)
                    script(*w, name);

                w->sealed = true;
                w->fn = w->vm->prepareCachedFunction(bytes, scriptPath);
                workers.push_back(std::move(w));
            }

            for (auto& w : workers)
            {
                Machine* m = w.get();
                threads.emplace_back([this, m] { workerLoop(*m); });
            }
        }

        void stopWorkers()
        {
            {
                std::lock_guard<std::mutex> lock(poolLock);
                stopping = true;
            }

            wake.notify_all();

            for (std::thread& t : threads)
                t.join();

            threads.clear();
        }

        void report() const
        {
            const double total = since(started);
            double inRules = 0.0;

            std::map<std::string, Spent> all;

            auto add = [&all](const Machine& m) {
                for (const auto& [name, c] : m.spent)
                {
                    Spent& s = all[name];
                    s.calls += c.calls;
                    s.repeats += c.repeats;
                    s.keying += c.keying;
                    s.copyIn += c.copyIn;
                    s.run += c.run;
                    s.copyOut += c.copyOut;
                }
            };

            add(main);

            for (const auto& w : workers)
                add(*w);

            std::fprintf(stderr, "\n%-14s %8s %8s %10s %10s %10s %9s %8s\n", "rule script", "calls", caching ? "cached" : "repeats", "copy in s", "run s", "copy out s", "ms/call", "key s");

            for (const auto& [name, c] : all)
            {
                const double spentHere = c.copyIn + c.run + c.copyOut;
                inRules += spentHere;
                std::fprintf(stderr, "%-14s %8ld %8ld %10.1f %10.1f %10.1f %9.2f %8.1f\n", name.c_str(), c.calls, c.repeats, c.copyIn / 1000.0, c.run / 1000.0, c.copyOut / 1000.0, c.calls ? spentHere / c.calls : 0.0, c.keying / 1000.0);
            }

            if (workers.empty())
            {
                std::fprintf(stderr, "in the rules %.1f s of %.1f s; the solver's own PgScript %.1f s\n", inRules / 1000.0, total / 1000.0, (total - inRules) / 1000.0);
            }
            else
            {
                std::fprintf(stderr, "%zu workers: %ld calls of parallel, %ld jobs, the main script waited %.1f s of %.1f s (the rules' times are summed over every thread: %.1f s)\n",
                             workers.size(), batches, jobsRun, waited / 1000.0, total / 1000.0, inRules / 1000.0);
                std::fprintf(stderr, "in parallel: %.1f s writing the jobs out, %.1f s the workers at them, %.1f s taking the answers in\n",
                             sending / 1000.0, working / 1000.0, taking / 1000.0);
                std::fprintf(stderr, "\n%-14s %8s %8s %10s %10s %12s %10s\n", "job", "calls", "jobs", "waited s", "jobs s", "slowest s", "longest s");

                for (const auto& [function, wk] : work)
                    std::fprintf(stderr, "%-14s %8ld %8ld %10.1f %10.1f %12.1f %10.2f\n", function.c_str(), wk.calls, wk.jobs, wk.waitedMs / 1000.0, wk.jobsMs / 1000.0, wk.slowestSum / 1000.0, wk.longestMs / 1000.0);
            }
        }
    };

    // The natives every VM running the solver's script has; `rule` is its machine's
    void defineNatives(Host* h, Machine* m, VM& vm)
    {
        vm.defineNative("rule", [h, m](VM* v, int argCount, Value* args) -> Value {
            return h->rule(*m, v, argCount, args);
        });

        vm.defineNative("clone", [](VM* v, int argCount, Value* args) -> Value {
            if (argCount != 1)
                throw std::runtime_error("clone expects one argument");

            return cloned(*v, args[0]);
        });

        // scalarsOf(table): its numbers, booleans and strings only; scalarsOfEach(list): the same of
        // each of its tables (a walk's state, as the C++ hands it to expedition.pg)
        vm.defineNative("scalarsOf", [](VM* v, int argCount, Value* args) -> Value {
            if (argCount != 1)
                throw std::runtime_error("scalarsOf expects one argument");

            return scalarsOf(*v, args[0]);
        });

        vm.defineNative("scalarsOfEach", [](VM* v, int argCount, Value* args) -> Value {
            if (argCount != 1)
                throw std::runtime_error("scalarsOfEach expects one argument");

            Value out = v->createVector();

            if (IS_VECTOR(args[0]))
            {
                ObjVector* list = v->asVector(out);

                for (Value item : v->asVector(args[0])->fields)
                    list->fields.push_back(scalarsOf(*v, item));
            }

            return out;
        });

        vm.defineNative("chronicleCharacter", [](VM* v, int argCount, Value* args) -> Value {
            if (argCount != 6)
                throw std::runtime_error("chronicleCharacter expects (stats, townKnown, town, fund, seen, know)");

            return chronicleCharacter(*v, args);
        });

        vm.defineNative("chronicleTakeStats", [](VM* v, int argCount, Value* args) -> Value {
            if (argCount != 3)
                throw std::runtime_error("chronicleTakeStats expects (stats, know, after)");

            return chronicleTakeStats(*v, args);
        });

        vm.defineNative("clockMs", [h](VM*, int, Value*) -> Value {
            const auto since = std::chrono::steady_clock::now() - h->started;
            return makeIntValue(std::chrono::duration_cast<std::chrono::milliseconds>(since).count());
        });

        vm.defineNative("getArg", [h](VM* v, int argCount, Value* args) -> Value {
            if (argCount != 1 or not IS_INT(args[0]))
                throw std::runtime_error("getArg expects an integer index");

            const int64_t index = AS_INT(args[0]);

            if (index < 0 or index >= static_cast<int64_t>(h->argv.size()))
                return v->createString("");

            return v->createString(h->argv[index]);
        });

        vm.defineNative("getArgCount", [h](VM*, int, Value*) -> Value {
            return makeIntValue(static_cast<int64_t>(h->argv.size()));
        });

        vm.defineNative("solverRole", [m](VM* v, int, Value*) -> Value {
            return v->createString(m->worker ? "worker" : "main");
        });

        vm.defineNative("threadCount", [h](VM*, int, Value*) -> Value {
            return makeIntValue(static_cast<int64_t>(h->wantedWorkers));
        });

        // startWorkers(): the workers begin their script now, while the main one goes on
        vm.defineNative("startWorkers", [h, m](VM*, int, Value*) -> Value {
            if (not m->worker)
                h->startWorkers();

            return makeNoneValue();
        });

        vm.defineNative("parallel", [h, m](VM* v, int argCount, Value* args) -> Value {
            if (m->worker)
                throw std::runtime_error("parallel is the main script's");

            return h->parallel(v, argCount, args);
        });

        // A worker says nothing: the main script speaks for the run
        const bool quiet = m->worker;

        vm.defineNative("log", [quiet](VM* v, int argCount, Value* args) -> Value {
            if (quiet)
                return makeNoneValue();

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
    }

    // The script compiled once, as the bytecode every VM prepares
    bool compileScript(Host& host, std::vector<char>& bytes)
    {
        VM compiler;
        host.ecs.setupVm(compiler);

        Machine names;
        defineNatives(&host, &names, compiler);

        const auto stamp = std::chrono::steady_clock::now().time_since_epoch().count();
        const std::filesystem::path out = std::filesystem::temp_directory_path() / ("chronicle-solver-" + std::to_string(stamp) + ".pgc");

        const InterpretResult r = compiler.interpretFromFile(host.scriptPath, true, out.string());

        if (r != InterpretResult::OK)
            return false;

        std::ifstream in(out.string(), std::ios::binary);
        bytes.assign(std::istreambuf_iterator<char>(in), std::istreambuf_iterator<char>());
        in.close();

        std::error_code ec;
        std::filesystem::remove(out, ec);

        return not bytes.empty();
    }
}

int main(int argc, char* argv[])
{
    if (argc < 3)
    {
        std::fprintf(stderr, "usage: ChronicleSolver <rules dir> <solver.pg> [args...] [threads=N]\n");
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
    host->scriptPath = argv[2];

    // As the CLI hands them: 0 the program, 1 the script, then the user's
    host->argv.push_back(argv[0]);

    int threadCount = static_cast<int>(std::max(1u, std::thread::hardware_concurrency()));

    for (int i = 2; i < argc; ++i)
    {
        const std::string arg = argv[i];

        if (arg.rfind("threads=", 0) == 0)
            threadCount = std::max(0, std::atoi(arg.c_str() + 8));

        host->argv.push_back(arg);
    }

    std::vector<char> bytes;

    if (not compileScript(*host, bytes))
    {
        std::fprintf(stderr, "%s does not compile (the VM's message is in the log above)\n", argv[2]);
        return 1;
    }

    Host* h = host.get();

    // The main VM, and the workers': made here, on this thread, before any thread runs
    host->main.vm = std::make_unique<VM>();
    host->ecs.setupVm(*host->main.vm);

    if (not host->vmProfileDir.empty())
        host->main.vm->enableProfiling();

    defineNatives(h, &host->main, *host->main.vm);
    host->main.fn = host->main.vm->prepareCachedFunction(bytes, host->scriptPath);

    host->bytes = std::move(bytes);
    host->wantedWorkers = threadCount;

    InterpretResult result = InterpretResult::COMPILE_ERROR;

    if (host->main.fn)
        result = host->main.vm->runPreparedFunction(host->main.fn, 0, host->scriptPath);

    std::cout.flush();

    host->stopWorkers();

    if (std::getenv("CHRONICLE_SOLVER_PROFILE"))
        host->report();

    host->dumpProfiles();

    return result == InterpretResult::OK ? 0 : 1;
}
