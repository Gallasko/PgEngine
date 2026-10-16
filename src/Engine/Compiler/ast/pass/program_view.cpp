#include "stdafx.h"

#include "program_view.h"

#include "Compiler/vm.h"
#include "Files/filemanager.h"
#include "Interpreter/lexer.h"
#include "Interpreter/parser.h"

namespace pg
{
    namespace ast
    {
        namespace
        {
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

            void poisonExpr(const ExprPtr& e, ProgramView& p);
            void poisonStmt(const StatementPtr& s, ProgramView& p);

            void poisonExpr(const ExprPtr& e, ProgramView& p)
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

            void poisonStmt(const StatementPtr& s, ProgramView& p)
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

            void readFile(const std::string& baseDir, const std::string& path, ProgramView& p);

            // A script's statements: its top-level functions counted, every name it writes poisoned,
            // and the modules it imports read in turn
            void readStatements(const std::string& baseDir, const std::vector<StatementPtr>& statements, ProgramView& p)
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
            void readFile(const std::string& baseDir, const std::string& name, ProgramView& p)
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
        }

        ProgramView readProgram(VM* vm, const std::queue<StatementPtr>& statements)
        {
            ProgramView program;
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

            return program;
        }
    }
}
