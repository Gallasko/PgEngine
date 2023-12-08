#include <filesystem>
#include <iostream>
namespace fs = std::filesystem;

#include "gtest/gtest.h"

#include "serialization.h"

#include "mocklogger.h"

namespace pg
{
    struct TestSerializeA
    {
        TestSerializeA(int data) : data(data) {}

        int data = 0;
    };

    template <>
    void serialize(Archive& archive, const TestSerializeA& a)
    {
        archive.startSerialization("Test Serial A");

        serialize(archive, "data", a.data);

        archive.endSerialization();
    }


    namespace test
    {
        // Todo mock serializer or add a serialize to text method !

        // ----------------------------------------------------------------------------------------
        // ---------------------------        Test separator        -------------------------------
        // ----------------------------------------------------------------------------------------
        TEST(serialize_test, initialization)
        {
            Serializer serialize;

            auto map = serialize.getSerializedMap();

            EXPECT_EQ(map.size(), 0);
        }

        // ----------------------------------------------------------------------------------------
        // ---------------------------        Test separator        -------------------------------
        // ----------------------------------------------------------------------------------------
        TEST(serialize_test, serialize_int)
        {
            MockLogger logger;
            fs::remove("tmpSerializeTest.sz");

            Serializer serialize;

            serialize.setFile("tmpSerializeTest.sz");

            int val = 5;

            serialize.serializeObject("test", val);

            auto map = serialize.getSerializedMap();

            EXPECT_EQ(map.size(), 1);

            auto file = UniversalFileAccessor::openTextFile("tmpSerializeTest.sz");

            EXPECT_EQ(file.data, "1.0\ntest: PGSERIALISEDATTRIBUTE int {5}");
        }

        // ----------------------------------------------------------------------------------------
        // ---------------------------        Test separator        -------------------------------
        // ----------------------------------------------------------------------------------------
        TEST(serialize_test, deserialize_int)
        {
            MockLogger<TerminalSink> logger;
            fs::remove("tmpSerializeTest.sz");

            Serializer serialize;

            serialize.setFile("tmpSerializeTest.sz");

            int val = 5;

            serialize.serializeObject("test", val);

            auto map = serialize.getSerializedMap();

            EXPECT_EQ(map.size(), 1);

            auto file = UniversalFileAccessor::openTextFile("tmpSerializeTest.sz");

            EXPECT_EQ(file.data, "1.0\ntest: PGSERIALISEDATTRIBUTE int {5}");

            int ret = serialize.deserializeObject<int>("test");

            EXPECT_EQ(ret, 5);
        }

        // ----------------------------------------------------------------------------------------
        // ---------------------------        Test separator        -------------------------------
        // ----------------------------------------------------------------------------------------
        TEST(serialize_test, serialize_custom)
        {
            MockLogger logger;
            fs::remove("tmpSerializeTest.sz");

            Serializer serialize;

            serialize.setFile("tmpSerializeTest.sz");

            TestSerializeA val = 5;

            serialize.serializeObject("test", val);

            auto map = serialize.getSerializedMap();

            EXPECT_EQ(map.size(), 1);

            auto file = UniversalFileAccessor::openTextFile("tmpSerializeTest.sz");

            EXPECT_EQ(file.data, "1.0\ntest: Test Serial A {\n\tdata: PGSERIALISEDATTRIBUTE int {5}\n}");
        }

    }
}
