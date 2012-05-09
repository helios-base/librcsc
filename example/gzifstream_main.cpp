
#include <string>
#include <rcsc/gz/gzfstream.h>

int
main( int argc, char** argv )
{
    if ( argc < 1 )
    {
        return 1;
    }

    rcsc::gzifstream zin( argv[1] );

    if ( ! zin.is_open() )
    {
        std::cerr << "failed to open " << argv[1] << std::endl;
    }

#if 0
    int counter = 0;
    std::string buf;
    std::cerr << "start to read" << std::endl;
    while ( zin.good() )
    {
        if ( zin.eof() )
        {
            std::cerr << " eof  " << std::endl;
        }
        std::getline( zin, buf );
        std::cerr << "gcount = " << zin.gcount() << std::endl;
        std::cout << "[" << buf << "]" << std::endl;
        break;
        ++counter;
        if ( counter < 0 )
        {
            break;
        }
        if ( counter > 100 )
        {
            zin.seekg( 0 );
            counter = -100;
        }
    }
#elif 1
    // test for rcg
    char buf[4];
    std::cerr << "start to read" << std::endl;
    if ( zin.good() )
    {
        zin.read( buf, 4 );
        std::cerr << "gcount = " << zin.gcount() << std::endl;
        std::cout << "[" << buf[0] << buf[1] << buf[2] << (int)buf[3]
                  << "]" << std::endl;
    }
#endif

    if ( zin.good() )
    {
        std::cerr << "--- good" << std::endl;
    }
    if ( zin.fail() )
    {
        std::cerr << "--- fail" << std::endl;
    }
    if ( zin.bad() )
    {
        std::cerr << "--- bad" << std::endl;
    }
    if ( zin.eof() )
    {
        std::cerr << "--- eof" << std::endl;
    }

    std::cerr << "close file" << std::endl;
    zin.close();

    std::cerr << "end program" << std::endl;
    return 0;
}
