#include "stdafx.h"

#include "function_inlining.h"

#include "Compiler/vm.h"
#include "Files/filemanager.h"
#include "Interpreter/lexer.h"
#include "Interpreter/parser.h"
#include "logger.h"

namespace pg
{
    namespace
    {
        const char* DOM = "FunctionInlining";

        // How deep inlined code is inlined again
        constexpr int maxDepth = 4;

        template <typename T>
        std::vector<T> listOf(std::queue<T> q)
        {
            std::vector<T> out;

            while (not q.empty())
            {
                out.push_back(q.front());
                q.pop();
            }

            return out;
        }

        template <typename T>
        std::queue<T> queueOf(const std::vector<T>& v)
        {
            std::queue<T> out;

            for (const auto& e : v)
                out.push(e);

            return out;
        }

        std::string varName(const ExprPtr& e)
        {
            if (e and e->getType() == "Var")
                return std::static_pointer_cast<Var>(e)->name.text;

            return "";
        }

        // ---- reading the program ---------------------------------------------------------------

        struct Program
        {
            std::map<std::string, int> defined;                                // Top-level `fun` definitions by name
            std::map<std::string, std::shared_ptr<FunctionStatement>> functions;
            std::set<std::string> poisoned;                                    // Assigned, declared or a class somewhere
            std::set<std::string> visited;                                     // Files read
            bool complete = true;
        };

        void poisonExpr(const ExprPtr& e, Program& p);
        void poisonStmt(const StatementPtr& s, Program& p);

        void poisonExpr(const ExprPtr& e, Program& p)
        {
            if (not e)
                return;

            const std::string type = e->getType();

            if (type == "Assign")
            {
                auto a = std::static_pointer_cast<Assign>(e);
                p.poisoned.insert(a->name.text);
                poisonExpr(a->expr, p);
            }
            else if (type == "PreFixExpression")
                p.poisoned.insert(std::static_pointer_cast<PreFixExpression>(e)->name.text);
            else if (type == "PostFixExpression")
                p.poisoned.insert(std::static_pointer_cast<PostFixExpression>(e)->name.text);
            else if (type == "BinaryExpression")
            {
                auto b = std::static_pointer_cast<BinaryExpression>(e);
                poisonExpr(b->leftExpr, p);
                poisonExpr(b->rightExpr, p);
            }
            else if (type == "LogicExpression")
            {
                auto l = std::static_pointer_cast<LogicExpression>(e);
                poisonExpr(l->leftExpr, p);
                poisonExpr(l->rightExpr, p);
            }
            else if (type == "UnaryExpression")
                poisonExpr(std::static_pointer_cast<UnaryExpression>(e)->expr, p);
            else if (type == "CompoundAtom")
                poisonExpr(std::static_pointer_cast<CompoundAtom>(e)->expr, p);
            else if (type == "CallExpression")
            {
                auto c = std::static_pointer_cast<CallExpression>(e);
                poisonExpr(c->caller, p);

                for (const auto& a : listOf(c->args))
                    poisonExpr(a, p);
            }
            else if (type == "Get")
                poisonExpr(std::static_pointer_cast<Get>(e)->object, p);
            else if (type == "Set")
            {
                auto s = std::static_pointer_cast<Set>(e);
                poisonExpr(s->object, p);
                poisonExpr(s->value, p);
            }
            else if (type == "IndexGet")
            {
                auto g = std::static_pointer_cast<IndexGet>(e);
                poisonExpr(g->object, p);
                poisonExpr(g->index, p);
            }
            else if (type == "IndexSet")
            {
                auto g = std::static_pointer_cast<IndexSet>(e);
                poisonExpr(g->object, p);
                poisonExpr(g->index, p);
                poisonExpr(g->value, p);
            }
            else if (type == "List")
            {
                for (const auto& entry : listOf(std::static_pointer_cast<List>(e)->entries))
                {
                    poisonExpr(entry.key, p);
                    poisonExpr(entry.value, p);
                }
            }
            else if (type == "AnonymousFunction")
                poisonStmt(std::static_pointer_cast<AnonymousFunction>(e)->body, p);
        }

        void poisonStmt(const StatementPtr& s, Program& p)
        {
            if (not s)
                return;

            const std::string type = s->getType();

            if (type == "ExpressionStatement")
                poisonExpr(std::static_pointer_cast<ExpressionStatement>(s)->expr, p);
            else if (type == "VariableStatement")
            {
                auto v = std::static_pointer_cast<VariableStatement>(s);
                p.poisoned.insert(v->name.text);
                poisonExpr(v->expr, p);
            }
            else if (type == "FunctionStatement")
                poisonStmt(std::static_pointer_cast<FunctionStatement>(s)->body, p);
            else if (type == "ClassStatement")
            {
                auto c = std::static_pointer_cast<ClassStatement>(s);
                p.poisoned.insert(c->name.text);

                for (const auto& m : listOf(c->methods))
                    poisonStmt(m, p);
            }
            else if (type == "BlockStatement")
            {
                for (const auto& b : listOf(std::static_pointer_cast<BlockStatement>(s)->statements))
                    poisonStmt(b, p);
            }
            else if (type == "IfStatement")
            {
                auto i = std::static_pointer_cast<IfStatement>(s);
                poisonExpr(i->condition, p);
                poisonStmt(i->thenBranch, p);
                poisonStmt(i->elseBranch, p);
            }
            else if (type == "WhileStatement")
            {
                auto w = std::static_pointer_cast<WhileStatement>(s);
                poisonExpr(w->condition, p);
                poisonStmt(w->body, p);
            }
            else if (type == "ForStatement")
            {
                auto f = std::static_pointer_cast<ForStatement>(s);
                poisonStmt(f->initializer, p);
                poisonExpr(f->condition, p);
                poisonExpr(f->increment, p);
                poisonStmt(f->body, p);
            }
            else if (type == "ForInStatement")
            {
                auto f = std::static_pointer_cast<ForInStatement>(s);
                p.poisoned.insert(f->varName.text);
                poisonExpr(f->iterable, p);
                poisonStmt(f->body, p);
            }
            else if (type == "ReturnStatement")
                poisonExpr(std::static_pointer_cast<ReturnStatement>(s)->value, p);
            else if (type == "DPrintStatement")
                poisonExpr(std::static_pointer_cast<DPrintStatement>(s)->expr, p);
        }

        void readFile(const std::string& baseDir, const std::string& path, Program& p);

        // A script's statements: its top-level functions counted, every name it writes poisoned,
        // and the modules it imports read in turn
        void readStatements(const std::string& baseDir, const std::vector<StatementPtr>& statements, Program& p)
        {
            for (const auto& s : statements)
            {
                if (not s)
                    continue;

                if (s->getType() == "FunctionStatement")
                {
                    auto f = std::static_pointer_cast<FunctionStatement>(s);
                    ++p.defined[f->name.text];
                    p.functions[f->name.text] = f;
                }
                else if (s->getType() == "ImportStatement")
                {
                    for (const auto& import : listOf(std::static_pointer_cast<ImportStatement>(s)->imports))
                    {
                        std::string name = import->getName();

                        if (name.size() >= 2 and name.front() == '"' and name.back() == '"')
                            name = name.substr(1, name.size() - 2);

                        readFile(baseDir, name, p);
                    }
                }

                poisonStmt(s, p);
            }
        }

        // As CParser::parseImportFile finds a module: beside the main script. Its source is read
        // even when a .pgc stands beside it (the compiler takes that, compiled from the same
        // source); with neither, it is a native module and defines no function of the program
        void readFile(const std::string& baseDir, const std::string& name, Program& p)
        {
            std::string fileName = name;

            if (fileName.find(".pg") == std::string::npos)
                fileName += ".pg";

            const std::string fullPath = baseDir + fileName;

            if (p.visited.count(fullPath))
                return;

            p.visited.insert(fullPath);

            if (not UniversalFileAccessor::exists(fullPath))
            {
                // Bytecode only: what it defines cannot be read
                if (UniversalFileAccessor::exists(fullPath + "c") or UniversalFileAccessor::exists(baseDir + name + ".pgc"))
                    p.complete = false;

                return;
            }

            try
            {
                Lexer lexer;
                lexer.readFromFile(fullPath);

                Parser parser(lexer.getTokens());
                auto statements = parser.parse();

                if (parser.hasError())
                {
                    p.complete = false;
                    return;
                }

                readStatements(baseDir, listOf(statements), p);
            }
            catch (const std::exception&)
            {
                p.complete = false;
            }
        }

        // ---- what can be inlined ----------------------------------------------------------------

        // Only operators, constants, variables, field and index reads, table literals and calls
        bool inlinableExpr(const ExprPtr& e, FunctionInliningPass::Inlinable& def, const std::set<std::string>& params)
        {
            if (not e)
                return true;

            const std::string type = e->getType();

            if (type == "Atom" or type == "NoneAtom")
                return true;

            if (type == "Var")
            {
                const std::string name = varName(e);

                if (params.count(name))
                    ++def.uses[name];
                else
                    def.freeNames.insert(name);

                return true;
            }

            if (type == "BinaryExpression")
            {
                auto b = std::static_pointer_cast<BinaryExpression>(e);
                return inlinableExpr(b->leftExpr, def, params) and inlinableExpr(b->rightExpr, def, params);
            }

            if (type == "LogicExpression")
            {
                auto l = std::static_pointer_cast<LogicExpression>(e);
                return inlinableExpr(l->leftExpr, def, params) and inlinableExpr(l->rightExpr, def, params);
            }

            if (type == "UnaryExpression")
                return inlinableExpr(std::static_pointer_cast<UnaryExpression>(e)->expr, def, params);

            if (type == "CompoundAtom")
                return inlinableExpr(std::static_pointer_cast<CompoundAtom>(e)->expr, def, params);

            if (type == "Get")
                return inlinableExpr(std::static_pointer_cast<Get>(e)->object, def, params);

            if (type == "IndexGet")
            {
                auto g = std::static_pointer_cast<IndexGet>(e);
                return inlinableExpr(g->object, def, params) and inlinableExpr(g->index, def, params);
            }

            if (type == "CallExpression")
            {
                auto c = std::static_pointer_cast<CallExpression>(e);

                if (not inlinableExpr(c->caller, def, params))
                    return false;

                for (const auto& a : listOf(c->args))
                {
                    if (not inlinableExpr(a, def, params))
                        return false;
                }

                return true;
            }

            if (type == "List")
            {
                for (const auto& entry : listOf(std::static_pointer_cast<List>(e)->entries))
                {
                    // An explicit key is a name, not a variable
                    if (not entry.explicitKey and not inlinableExpr(entry.key, def, params))
                        return false;

                    if (not inlinableExpr(entry.value, def, params))
                        return false;
                }

                return true;
            }

            // Assign, Set, IndexSet, prefix and postfix operators, this, anonymous functions
            return false;
        }

        // Whether an expression may run script code (which may write any global or field) rather
        // than only natives: a call to a function of the program, to a name it assigns anywhere, to a
        // parameter, or to anything but a plain name
        bool callsScript(const ExprPtr& e, const std::set<std::string>& scriptNames, const std::set<std::string>& params)
        {
            if (not e)
                return false;

            const std::string type = e->getType();

            if (type == "CallExpression")
            {
                auto c = std::static_pointer_cast<CallExpression>(e);
                const std::string name = varName(c->caller);

                if (name.empty() or scriptNames.count(name) or params.count(name))
                    return true;

                for (const auto& a : listOf(c->args))
                {
                    if (callsScript(a, scriptNames, params))
                        return true;
                }

                return false;
            }

            if (type == "BinaryExpression")
            {
                auto b = std::static_pointer_cast<BinaryExpression>(e);
                return callsScript(b->leftExpr, scriptNames, params) or callsScript(b->rightExpr, scriptNames, params);
            }

            if (type == "LogicExpression")
            {
                auto l = std::static_pointer_cast<LogicExpression>(e);
                return callsScript(l->leftExpr, scriptNames, params) or callsScript(l->rightExpr, scriptNames, params);
            }

            if (type == "UnaryExpression")
                return callsScript(std::static_pointer_cast<UnaryExpression>(e)->expr, scriptNames, params);

            if (type == "CompoundAtom")
                return callsScript(std::static_pointer_cast<CompoundAtom>(e)->expr, scriptNames, params);

            if (type == "Get")
                return callsScript(std::static_pointer_cast<Get>(e)->object, scriptNames, params);

            if (type == "IndexGet")
            {
                auto g = std::static_pointer_cast<IndexGet>(e);
                return callsScript(g->object, scriptNames, params) or callsScript(g->index, scriptNames, params);
            }

            if (type == "List")
            {
                for (const auto& entry : listOf(std::static_pointer_cast<List>(e)->entries))
                {
                    if (callsScript(entry.value, scriptNames, params) or (not entry.explicitKey and callsScript(entry.key, scriptNames, params)))
                        return true;
                }
            }

            return false;
        }

        // ---- copying an expression, its parameters replaced ------------------------------------

        ExprPtr copyOf(const ExprPtr& e, const std::map<std::string, ExprPtr>* args)
        {
            if (not e)
                return nullptr;

            const std::string type = e->getType();

            if (type == "Atom")
                return std::make_shared<Atom>(std::static_pointer_cast<Atom>(e)->value);

            if (type == "NoneAtom")
                return std::make_shared<NoneAtom>();

            if (type == "Var")
            {
                auto v = std::static_pointer_cast<Var>(e);

                if (args)
                {
                    auto it = args->find(v->name.text);

                    // The argument, copied as it is (its own names are the caller's)
                    if (it != args->end())
                        return copyOf(it->second, nullptr);
                }

                return std::make_shared<Var>(v->name);
            }

            if (type == "BinaryExpression")
            {
                auto b = std::static_pointer_cast<BinaryExpression>(e);
                return std::make_shared<BinaryExpression>(copyOf(b->leftExpr, args), b->op, copyOf(b->rightExpr, args));
            }

            if (type == "LogicExpression")
            {
                auto l = std::static_pointer_cast<LogicExpression>(e);
                return std::make_shared<LogicExpression>(copyOf(l->leftExpr, args), l->op, copyOf(l->rightExpr, args));
            }

            if (type == "UnaryExpression")
            {
                auto u = std::static_pointer_cast<UnaryExpression>(e);
                return std::make_shared<UnaryExpression>(copyOf(u->expr, args), u->op);
            }

            if (type == "CompoundAtom")
                return std::make_shared<CompoundAtom>(copyOf(std::static_pointer_cast<CompoundAtom>(e)->expr, args));

            if (type == "Get")
            {
                auto g = std::static_pointer_cast<Get>(e);
                return std::make_shared<Get>(copyOf(g->object, args), g->name);
            }

            if (type == "IndexGet")
            {
                auto g = std::static_pointer_cast<IndexGet>(e);
                return std::make_shared<IndexGet>(copyOf(g->object, args), copyOf(g->index, args), g->bracket);
            }

            if (type == "CallExpression")
            {
                auto c = std::static_pointer_cast<CallExpression>(e);
                std::vector<ExprPtr> callArgs;

                for (const auto& a : listOf(c->args))
                    callArgs.push_back(copyOf(a, args));

                return std::make_shared<CallExpression>(copyOf(c->caller, args), c->paren, queueOf(callArgs));
            }

            if (type == "List")
            {
                auto l = std::static_pointer_cast<List>(e);
                std::vector<ListElement> entries;

                for (auto entry : listOf(l->entries))
                {
                    ListElement copy;
                    copy.explicitKey = entry.explicitKey;
                    copy.key = entry.explicitKey ? copyOf(entry.key, nullptr) : copyOf(entry.key, args);
                    copy.value = copyOf(entry.value, args);
                    entries.push_back(copy);
                }

                auto out = std::make_shared<List>(copyOf(l->self, args), l->squareBracket, queueOf(entries));
                out->braceForm = l->braceForm;
                out->hasExplicitKeys = l->hasExplicitKeys;

                return out;
            }

            return e;
        }

        // ---- the rewrite ---------------------------------------------------------------------------

        // The names a function body (or a script's nested blocks) declares: its locals
        void declsOf(const StatementPtr& s, std::set<std::string>& out, bool topLevel)
        {
            if (not s)
                return;

            const std::string type = s->getType();

            if (type == "VariableStatement")
            {
                // (a top-level `var` is a global, not a local)
                if (not topLevel)
                    out.insert(std::static_pointer_cast<VariableStatement>(s)->name.text);
            }
            else if (type == "FunctionStatement")
            {
                if (not topLevel)
                    out.insert(std::static_pointer_cast<FunctionStatement>(s)->name.text);
            }
            else if (type == "BlockStatement")
            {
                for (const auto& b : listOf(std::static_pointer_cast<BlockStatement>(s)->statements))
                    declsOf(b, out, false);
            }
            else if (type == "IfStatement")
            {
                auto i = std::static_pointer_cast<IfStatement>(s);
                declsOf(i->thenBranch, out, false);
                declsOf(i->elseBranch, out, false);
            }
            else if (type == "WhileStatement")
                declsOf(std::static_pointer_cast<WhileStatement>(s)->body, out, false);
            else if (type == "ForStatement")
            {
                auto f = std::static_pointer_cast<ForStatement>(s);
                declsOf(f->initializer, out, false);
                declsOf(f->body, out, false);
            }
            else if (type == "ForInStatement")
            {
                auto f = std::static_pointer_cast<ForInStatement>(s);
                out.insert(f->varName.text);
                declsOf(f->body, out, false);
            }
        }

        struct Rewriter
        {
            const std::map<std::string, FunctionInliningPass::Inlinable>& inlinable;
            std::vector<std::set<std::string>> scopes;
            long inlined = 0;

            bool shadowed(const std::string& name) const
            {
                for (const auto& s : scopes)
                {
                    if (s.count(name))
                        return true;
                }

                return false;
            }

            // A chain of field reads on a variable: `req.stat`, `a.b.c`
            static bool getChain(const ExprPtr& e)
            {
                if (not e)
                    return false;

                if (e->getType() == "Var")
                    return true;

                if (e->getType() == "Get")
                    return getChain(std::static_pointer_cast<Get>(e)->object);

                return false;
            }

            static std::string rootName(const ExprPtr& e)
            {
                if (e->getType() == "Var")
                    return varName(e);

                return rootName(std::static_pointer_cast<Get>(e)->object);
            }

            ExprPtr tryInline(const std::shared_ptr<CallExpression>& call, int depth)
            {
                const std::string name = varName(call->caller);

                if (name.empty() or depth >= maxDepth or shadowed(name))
                    return nullptr;

                auto it = inlinable.find(name);

                if (it == inlinable.end())
                    return nullptr;

                const auto& def = it->second;
                auto args = listOf(call->args);

                if (args.size() != def.params.size())
                    return nullptr;

                for (const auto& free : def.freeNames)
                {
                    if (shadowed(free))
                        return nullptr;
                }

                std::map<std::string, ExprPtr> byParam;

                for (size_t i = 0; i < args.size(); ++i)
                {
                    const ExprPtr& arg = args[i];
                    const std::string type = arg->getType();
                    auto used = def.uses.find(def.params[i]);
                    const int uses = used == def.uses.end() ? 0 : used->second;

                    if (type == "Atom" or type == "NoneAtom")
                    {
                        // Nothing to evaluate twice, nothing to miss
                    }
                    else if (type == "Var")
                    {
                        // Script code the expression runs before reading it could write it (a global,
                        // or a local through a closure)
                        if (def.callsScript)
                            return nullptr;
                    }
                    else if (getChain(arg))
                    {
                        // Read once, where the expression reads it
                        if (uses != 1 or def.callsScript)
                            return nullptr;
                    }
                    else
                    {
                        return nullptr;
                    }

                    byParam[def.params[i]] = arg;
                }

                ++inlined;

                return copyOf(def.body, &byParam);
            }

            ExprPtr expr(const ExprPtr& e, int depth)
            {
                if (not e)
                    return e;

                const std::string type = e->getType();

                if (type == "BinaryExpression")
                {
                    auto b = std::static_pointer_cast<BinaryExpression>(e);
                    b->leftExpr = expr(b->leftExpr, depth);
                    b->rightExpr = expr(b->rightExpr, depth);
                }
                else if (type == "LogicExpression")
                {
                    auto l = std::static_pointer_cast<LogicExpression>(e);
                    l->leftExpr = expr(l->leftExpr, depth);
                    l->rightExpr = expr(l->rightExpr, depth);
                }
                else if (type == "UnaryExpression")
                {
                    auto u = std::static_pointer_cast<UnaryExpression>(e);
                    u->expr = expr(u->expr, depth);
                }
                else if (type == "CompoundAtom")
                {
                    auto c = std::static_pointer_cast<CompoundAtom>(e);
                    c->expr = expr(c->expr, depth);
                }
                else if (type == "Assign")
                {
                    auto a = std::static_pointer_cast<Assign>(e);
                    a->expr = expr(a->expr, depth);
                }
                else if (type == "Get")
                {
                    auto g = std::static_pointer_cast<Get>(e);
                    g->object = expr(g->object, depth);
                }
                else if (type == "Set")
                {
                    auto s = std::static_pointer_cast<Set>(e);
                    s->object = expr(s->object, depth);
                    s->value = expr(s->value, depth);
                }
                else if (type == "IndexGet")
                {
                    auto g = std::static_pointer_cast<IndexGet>(e);
                    g->object = expr(g->object, depth);
                    g->index = expr(g->index, depth);
                }
                else if (type == "IndexSet")
                {
                    auto g = std::static_pointer_cast<IndexSet>(e);
                    g->object = expr(g->object, depth);
                    g->index = expr(g->index, depth);
                    g->value = expr(g->value, depth);
                }
                else if (type == "List")
                {
                    auto l = std::static_pointer_cast<List>(e);
                    std::vector<ListElement> entries;

                    for (auto entry : listOf(l->entries))
                    {
                        if (not entry.explicitKey)
                            entry.key = expr(entry.key, depth);

                        entry.value = expr(entry.value, depth);
                        entries.push_back(entry);
                    }

                    l->entries = queueOf(entries);
                }
                else if (type == "AnonymousFunction")
                {
                    auto f = std::static_pointer_cast<AnonymousFunction>(e);
                    std::set<std::string> locals;

                    for (const auto& p : listOf(f->parameters))
                        locals.insert(p->getName());

                    declsOf(f->body, locals, false);
                    scopes.push_back(locals);
                    stmt(f->body);
                    scopes.pop_back();
                }
                else if (type == "CallExpression")
                {
                    auto c = std::static_pointer_cast<CallExpression>(e);
                    c->caller = expr(c->caller, depth);

                    std::vector<ExprPtr> args;

                    for (const auto& a : listOf(c->args))
                        args.push_back(expr(a, depth));

                    c->args = queueOf(args);

                    if (auto folded = tryInline(c, depth))
                        return expr(folded, depth + 1);
                }

                return e;
            }

            void stmt(const StatementPtr& s)
            {
                if (not s)
                    return;

                const std::string type = s->getType();

                if (type == "ExpressionStatement")
                {
                    auto x = std::static_pointer_cast<ExpressionStatement>(s);
                    x->expr = expr(x->expr, 0);
                }
                else if (type == "VariableStatement")
                {
                    auto v = std::static_pointer_cast<VariableStatement>(s);
                    v->expr = expr(v->expr, 0);
                }
                else if (type == "ReturnStatement")
                {
                    auto r = std::static_pointer_cast<ReturnStatement>(s);
                    r->value = expr(r->value, 0);
                }
                else if (type == "DPrintStatement")
                {
                    auto d = std::static_pointer_cast<DPrintStatement>(s);
                    d->expr = expr(d->expr, 0);
                }
                else if (type == "IfStatement")
                {
                    auto i = std::static_pointer_cast<IfStatement>(s);
                    i->condition = expr(i->condition, 0);
                    stmt(i->thenBranch);
                    stmt(i->elseBranch);
                }
                else if (type == "WhileStatement")
                {
                    auto w = std::static_pointer_cast<WhileStatement>(s);
                    w->condition = expr(w->condition, 0);
                    stmt(w->body);
                }
                else if (type == "ForStatement")
                {
                    auto f = std::static_pointer_cast<ForStatement>(s);
                    stmt(f->initializer);
                    f->condition = expr(f->condition, 0);
                    f->increment = expr(f->increment, 0);
                    stmt(f->body);
                }
                else if (type == "ForInStatement")
                {
                    auto f = std::static_pointer_cast<ForInStatement>(s);
                    f->iterable = expr(f->iterable, 0);
                    stmt(f->body);
                }
                else if (type == "BlockStatement")
                {
                    for (const auto& b : listOf(std::static_pointer_cast<BlockStatement>(s)->statements))
                        stmt(b);
                }
                else if (type == "FunctionStatement")
                {
                    auto f = std::static_pointer_cast<FunctionStatement>(s);
                    std::set<std::string> locals;

                    for (const auto& p : listOf(f->parameters))
                        locals.insert(p->getName());

                    declsOf(f->body, locals, false);
                    scopes.push_back(locals);
                    stmt(f->body);
                    scopes.pop_back();
                }
                else if (type == "ClassStatement")
                {
                    for (const auto& m : listOf(std::static_pointer_cast<ClassStatement>(s)->methods))
                        stmt(m);
                }
            }
        };
    }

    bool FunctionInliningPass::runPass(VM* vm, std::queue<StatementPtr>& statements)
    {
        // The whole program: the script compiled (outermost) and every module it imports. A module
        // compiled while it is (deeper) belongs to that program
        if (not vm or vm->astCompileDepth <= 1)
        {
            inlinable.clear();

            Program program;
            std::string baseDir;
            const std::string mainFile = vm ? vm->currentFileName : std::string();

            if (not mainFile.empty())
            {
                program.visited.insert(mainFile);

                const size_t slash = mainFile.find_last_of("/\\");

                if (slash != std::string::npos)
                    baseDir = mainFile.substr(0, slash + 1);
            }

            readStatements(baseDir, listOf(statements), program);
            std::set<std::string> scriptNames = program.poisoned;

            for (const auto& [name, count] : program.defined)
                scriptNames.insert(name);

            if (program.complete)
            {
                for (const auto& [name, count] : program.defined)
                {
                    if (count != 1 or program.poisoned.count(name))
                        continue;

                    auto f = program.functions[name];

                    if (not f->body or f->body->getType() != "BlockStatement")
                        continue;

                    auto body = listOf(std::static_pointer_cast<BlockStatement>(f->body)->statements);

                    if (body.size() != 1 or not body[0] or body[0]->getType() != "ReturnStatement")
                        continue;

                    auto ret = std::static_pointer_cast<ReturnStatement>(body[0]);

                    if (not ret->value)
                        continue;

                    Inlinable def;
                    std::set<std::string> params;
                    bool ok = true;

                    for (const auto& p : listOf(f->parameters))
                    {
                        if (p->getType() != "Var" or params.count(p->getName()))
                            ok = false;

                        def.params.push_back(p->getName());
                        params.insert(p->getName());
                    }

                    if (not ok or not inlinableExpr(ret->value, def, params) or def.freeNames.count(name))
                        continue;

                    // A copy: the rewrite below changes the function's own statements in place
                    def.body = copyOf(ret->value, nullptr);
                    inlinable[name] = def;
                }
            }

            // What they call may be script code that writes: unless it is one of them, as none of
            // them writes anything. Whichever calls one that may write may write too, until no more do
            std::set<std::string> mayWrite = scriptNames;

            for (const auto& [name, def] : inlinable)
                mayWrite.erase(name);

            bool grew = true;

            while (grew)
            {
                grew = false;

                for (auto& [name, def] : inlinable)
                {
                    if (def.callsScript)
                        continue;

                    std::set<std::string> params(def.params.begin(), def.params.end());

                    if (callsScript(def.body, mayWrite, params))
                    {
                        def.callsScript = true;
                        mayWrite.insert(name);
                        grew = true;
                    }
                }
            }

            LOG_INFO(DOM, "Inlinable functions: " << inlinable.size() << (program.complete ? "" : " (a module could not be read: none)"));
        }

        if (inlinable.empty())
            return false;

        Rewriter rewriter{inlinable, {}, 0};

        // The script's own nested blocks declare locals too
        std::set<std::string> topLocals;

        for (const auto& s : listOf(statements))
        {
            if (s and s->getType() != "FunctionStatement" and s->getType() != "ClassStatement")
                declsOf(s, topLocals, true);
        }

        rewriter.scopes.push_back(topLocals);

        std::vector<StatementPtr> out;

        for (const auto& s : listOf(statements))
        {
            rewriter.stmt(s);
            out.push_back(s);
        }

        statements = queueOf(out);

        LOG_INFO(DOM, "Inlined " << rewriter.inlined << " calls");

        return rewriter.inlined > 0;
    }
}
