#include "gtest/gtest.h"

#include "Compiler/vm.h"
#include "Compiler/value_nanbox.h"
#include "Compiler/chunk.h"

#include <cmath>
#include <limits>
#include <string>
#include <vector>

// Chunk::addConstant / addConstantIndex find a duplicate through a hash index (findConstant).
// These pin that they give the index the old scan gave: the first constant valuesEqual finds equal.

namespace pg
{
    namespace test
    {
        namespace
        {
            // The scan addConstant made before the index: the first equal constant, or -1
            int64_t scannedIndex(const Chunk& chunk, Value value)
            {
                for (size_t i = 0; i < chunk.constants.size(); i++)
                {
                    if (chunk.valuesEqual(chunk.constants[i], value, chunk.stringPool))
                        return static_cast<int64_t>(i);
                }

                return -1;
            }

            // Adds the value and checks it lands where the scan says: on its first twin, or at the end
            uint8_t addChecked(Chunk& chunk, Value value)
            {
                const int64_t scanned = scannedIndex(chunk, value);
                const size_t expected = scanned >= 0 ? static_cast<size_t>(scanned) : chunk.constants.size();

                const uint8_t index = chunk.addConstantIndex(value);

                EXPECT_EQ(index, expected) << "value bits " << std::hex << value;

                return index;
            }

            // Two long strings of the same text at two places of the pool (createString would share one)
            Value longString(VM& vm, const std::string& text)
            {
                auto [ptr, index] = vm.pools.stringPool.allocateWithIndex(text);
                (void)ptr;

                return makeStringValue(static_cast<uint32_t>(index));
            }
        }

        TEST(chunk_constants_test, dedupe_follows_values_equal)
        {
            VM vm;
            Chunk chunk;
            chunk.stringPool = &vm.pools.stringPool;

            // NaN twice: the same bits are one constant
            const Value nan = makeDoubleValue(std::numeric_limits<double>::quiet_NaN());
            const uint8_t nanIndex = addChecked(chunk, nan);
            EXPECT_EQ(addChecked(chunk, nan), nanIndex);

            // 0.0 and -0.0 are not merged (bits differ)
            const uint8_t zero = addChecked(chunk, makeDoubleValue(0.0));
            const uint8_t negZero = addChecked(chunk, makeDoubleValue(-0.0));
            EXPECT_NE(zero, negZero);
            EXPECT_EQ(addChecked(chunk, makeDoubleValue(-0.0)), negZero);

            // An int and a float of the same value are not merged
            const uint8_t intOne = addChecked(chunk, makeIntValue(1));
            const uint8_t floatOne = addChecked(chunk, makeFloatValue(1.0));
            EXPECT_NE(intOne, floatOne);
            EXPECT_EQ(addChecked(chunk, makeIntValue(1)), intOne);
            EXPECT_EQ(addChecked(chunk, makeFloatValue(1.0)), floatOne);

            // Small strings by their characters, even when the bytes past their length differ
            const Value small = makeSmallStringValue("ab", 2);
            const Value smallNoisy = small | (static_cast<uint64_t>(0x7A) << 24);
            ASSERT_NE(small, smallNoisy);
            const uint8_t smallIndex = addChecked(chunk, small);
            EXPECT_EQ(addChecked(chunk, smallNoisy), smallIndex);
            EXPECT_NE(addChecked(chunk, makeSmallStringValue("abc", 3)), smallIndex);

            // Interned strings by their index
            const uint8_t interned3 = addChecked(chunk, makeInternedStringValue(3));
            EXPECT_EQ(addChecked(chunk, makeInternedStringValue(3)), interned3);
            EXPECT_NE(addChecked(chunk, makeInternedStringValue(4)), interned3);

            // Long strings by their text in the pool, wherever they stand there
            const Value longA = longString(vm, "a long string constant");
            const Value longB = longString(vm, "a long string constant");
            ASSERT_NE(longA, longB);
            const uint8_t longIndex = addChecked(chunk, longA);
            EXPECT_EQ(addChecked(chunk, longB), longIndex);
            EXPECT_NE(addChecked(chunk, longString(vm, "another long string")), longIndex);

            // Strings of different kinds are never merged, even of the same text
            const Value longAb = longString(vm, "ab");
            EXPECT_NE(addChecked(chunk, longAb), smallIndex);
        }

        TEST(chunk_constants_test, a_duplicate_takes_its_lowest_twin)
        {
            VM vm;
            Chunk chunk;
            chunk.stringPool = &vm.pools.stringPool;

            addChecked(chunk, makeIntValue(10));

            // Twins pushed from outside, as a .pgc read does: the first of them is the one found
            const Value twin = longString(vm, "the same text twice");
            chunk.constants.push_back(makeIntValue(20));
            chunk.constants.push_back(twin);
            chunk.constants.push_back(longString(vm, "the same text twice"));
            chunk.constants.push_back(twin);

            EXPECT_EQ(addChecked(chunk, twin), 2);
            EXPECT_EQ(addChecked(chunk, longString(vm, "the same text twice")), 2);
            EXPECT_EQ(addChecked(chunk, makeIntValue(20)), 1);
            EXPECT_EQ(addChecked(chunk, makeIntValue(10)), 0);

            // addConstant emits the twin's index too
            const size_t at = chunk.addConstant(twin, 1);
            EXPECT_EQ(chunk.code[at - 1], static_cast<uint8_t>(OpCode::OP_Constant));
            EXPECT_EQ(chunk.code[at], 2);
            EXPECT_EQ(chunk.constants.size(), 5u);
        }

        TEST(chunk_constants_test, the_index_follows_a_cleared_chunk_and_a_new_pool)
        {
            VM vm;
            Chunk chunk;
            chunk.stringPool = &vm.pools.stringPool;

            addChecked(chunk, makeIntValue(1));
            addChecked(chunk, makeIntValue(2));

            // Cleared then refilled from outside: what was indexed before is not found
            chunk.clear();
            chunk.constants.push_back(makeIntValue(2));
            EXPECT_EQ(addChecked(chunk, makeIntValue(1)), 1);
            EXPECT_EQ(addChecked(chunk, makeIntValue(2)), 0);

            // Without a pool long strings are told apart by their bits only, as valuesEqual does
            Chunk bare;
            const Value longA = longString(vm, "a long string constant");
            const Value longB = longString(vm, "a long string constant");
            EXPECT_EQ(addChecked(bare, longA), 0);
            EXPECT_EQ(addChecked(bare, longB), 1);

            // Given the pool afterwards, they are one constant again
            bare.stringPool = &vm.pools.stringPool;
            EXPECT_EQ(addChecked(bare, longB), 0);
        }
    }
}
