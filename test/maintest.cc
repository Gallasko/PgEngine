#include "stdafx.h"

#include "gtest/gtest.h"

#include <iostream>

#include <SDL.h>

/**
 * Entry point for the test
 */
int main(int argc, char **argv)
{
   std::cout << "Start all the tests" << std::endl;
   ::testing::InitGoogleTest( &argc, argv );
   return RUN_ALL_TESTS();
}
