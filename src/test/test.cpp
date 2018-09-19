#include "test.h"

// catch2 includes.
#define CATCH_CONFIG_RUNNER
#include "test/catch.hpp"

unsigned int Factorial( unsigned int number )
{
    return number <= 1 ? number : Factorial(number-1)*number;
}

test::test()
{

}

int test::run()
{
    return Catch::Session().run();
}

TEST_CASE( "Factorials are computed", "[factorial]" ) {
    REQUIRE( Factorial(1) == 1 );
    REQUIRE( Factorial(2) == 2 );
    REQUIRE( Factorial(3) == 6 );
    REQUIRE( Factorial(10) == 3628800 );
}
