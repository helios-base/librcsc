// A program that uses the installed librcsc, built and run by the
// "find_package" test. It includes some of the main header files, calls
// functions of the library, and checks the version of the library.

#include <rcsc/player/world_model.h>
#include <rcsc/time/time_stamp.h>
#include <rcsc/geom/vector_2d.h>
#include <rcsc/version.h>

#include <cstring>
#include <iostream>

int
main()
{
    rcsc::WorldModel wm;
    const rcsc::Vector2D v( 3.0, 4.0 );

    std::cout << "librcsc " << rcsc::version()
              << " time=" << wm.time()
              << " |(3,4)|=" << v.r() << std::endl;

#ifdef LIBRCSC_EXPECTED_VERSION
    if ( std::strcmp( rcsc::version(), LIBRCSC_EXPECTED_VERSION ) != 0 )
    {
        std::cerr << "unexpected version: " << rcsc::version()
                  << " (expected " << LIBRCSC_EXPECTED_VERSION << ")" << std::endl;
        return 1;
    }
#endif

    return 0;
}
