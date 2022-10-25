#include "gtest/gtest.h"

#include "Files/filemanager.h"

namespace pg
{
    namespace test
    {
        // ----------------------------------------------------------------------------------------
        // ---------------------------        Test separator        -------------------------------
        // ----------------------------------------------------------------------------------------
        TEST(universal_file_accessor_test, open_file_in_exe_directory)
        {
            auto file = UniversalFileAccessor::openTextFile("testfile.txt");

            EXPECT_EQ(file.filepath, "testfile.txt");
            EXPECT_EQ(file.data, "This is a test file !");
        }

        // ----------------------------------------------------------------------------------------
        // ---------------------------        Test separator        -------------------------------
        // ----------------------------------------------------------------------------------------
        TEST(universal_file_accessor_test, file_info_in_exe_directory)
        {
            auto file = UniversalFileAccessor::openTextFile("testfile.txt");

            EXPECT_EQ(file.filepath, "testfile.txt");
            EXPECT_EQ(file.data, "This is a test file !");

            auto filename = UniversalFileAccessor::getFileName(file);
            auto folderpath = UniversalFileAccessor::getFoldername(file);

            EXPECT_EQ(filename, "testfile");
            EXPECT_EQ(folderpath, ".");
        }

    } // namespace test

} // namespace pg