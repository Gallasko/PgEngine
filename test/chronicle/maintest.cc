#include "stdafx.h"

#include "gtest/gtest.h"

#include <filesystem>
#include <iostream>
#include <string>
#include <system_error>
#include <vector>

#ifdef __EMSCRIPTEN__
    #include <SDL2/SDL.h>
#else
    #ifdef __linux__
        #include <SDL2/SDL.h>
        #include <unistd.h>
    #elif _WIN32
        #include <SDL.h>
        #include <process.h>
    #endif
#endif

#include "compiledrules.h"

#include "Core/rulevm.h"

#include "ECS/entitysystem.h"

#include "logger.h"

namespace pg
{
    namespace test
    {
        namespace
        {
            // Where the rules are compiled to: beside the binary, as "rules" is, one folder a process
            constexpr const char* Prefix = "rules-compiled-";

            std::string compiledDir;

            long processId()
            {
#ifdef _WIN32
                return static_cast<long>(_getpid());
#else
                return static_cast<long>(getpid());
#endif
            }

            // Every error logged while the rules compile (the compiler's own message among them)
            struct CompileErrors : public Logger::LogSink
            {
                std::vector<std::string> seen;

                virtual void processLog(const Logger::Info& log) override
                {
                    if (log.level == Logger::InfoLevel::error or log.level == Logger::InfoLevel::critical)
                        seen.push_back("[" + log.scope + "] " + log.message);
                }
            };

            void removeCompiled(const std::string& dir)
            {
                std::error_code ec;
                std::filesystem::remove_all(dir, ec);
                std::filesystem::remove_all(dir + "-src", ec);   // precompileRules' copy of the sources
            }

            // Folders left by a test process that died before it could remove its own
            void removeOrphans()
            {
#ifdef __linux__
                namespace fs = std::filesystem;

                std::error_code ec;
                const std::string prefix = Prefix;

                for (const auto& entry : fs::directory_iterator(".", ec))
                {
                    std::string name = entry.path().filename().string();

                    if (name.rfind(prefix, 0) != 0)
                        continue;

                    std::string pid = name.substr(prefix.size());

                    if (pid.size() > 4 and pid.compare(pid.size() - 4, 4, "-src") == 0)
                        pid.resize(pid.size() - 4);

                    if (pid.empty() or pid.find_first_not_of("0123456789") != std::string::npos)
                        continue;

                    std::error_code alive;

                    if (not fs::exists("/proc/" + pid, alive))
                        fs::remove_all(entry.path(), ec);
                }
#endif
            }

            // The scripts compiled from their sources, once. Any error stops the run: a test must
            // never read a .pgc left by an earlier compile
            bool compileRules()
            {
                compiledDir = Prefix + std::to_string(processId());

                removeOrphans();
                removeCompiled(compiledDir);

                auto sink = Logger::registerSink<CompileErrors>();
                auto* errorsLogged = static_cast<CompileErrors*>(sink.get());

                std::vector<std::string> errors;
                bool ok = false;

                {
                    EntitySystem ecs;
                    ok = chronicle::precompileRules(&ecs, "rules", compiledDir, errors);
                }

                std::vector<std::string> logged = errorsLogged->seen;
                Logger::removeSink(sink);

                if (ok and logged.empty())
                    return true;

                std::cerr << "\n*** The rule scripts in rules/ do not compile: no test is run ***\n";

                for (const auto& line : logged)
                    std::cerr << "  " << line << "\n";

                for (const auto& line : errors)
                    std::cerr << "  " << line << "\n";

                std::cerr << std::endl;

                removeCompiled(compiledDir);

                return false;
            }
        }

        const std::string& compiledRules()
        {
            return compiledDir;
        }
    }
}

/**
 * Entry point for the test
 */
int main(int argc, char **argv)
{
   std::cout << "Start all the tests" << std::endl;
   ::testing::InitGoogleTest( &argc, argv );

   // The rules compiled once for every fixture (compiledrules.h); a listing of the tests (the
   // build's gtest_discover_tests) runs none, and needs none
   if (not GTEST_FLAG_GET(list_tests) and not pg::test::compileRules())
      return 1;

   int result = RUN_ALL_TESTS();

   if (not pg::test::compiledRules().empty())
      pg::test::removeCompiled(pg::test::compiledRules());

   return result;
}
