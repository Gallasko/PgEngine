#pragma once

#include <string>

namespace pg
{
    namespace test
    {
        // The rule scripts compiled once for this test process (maintest.cc): a folder of .pgc of
        // its own, so a fixture reads them in a few ms instead of compiling every script again,
        // and two test processes never write the same .pgc. The run stops before any test if a
        // script does not compile, so no fixture can read a stale .pgc.
        const std::string& compiledRules();
    }
}
