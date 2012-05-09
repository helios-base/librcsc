
#include <rcsc/gz/gzfstream.h>

int
main( int argc, char** argv )
{
    rcsc::gzofstream zout( "out.gz" );

    if ( ! zout.is_open() )
    {
        std::cerr << "failed to open " << std::endl;
    }

    std::cerr << "start to write" << std::endl;
    std::string write_line = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ\n";

    for ( int i = 0; i < 8000; ++i )
    {
        zout << i << ":" << write_line;
    }

    if ( zout.good() )
    {
        std::cerr << "--- good" << std::endl;
    }
    if ( zout.fail() )
    {
        std::cerr << "--- fail" << std::endl;
    }
    if ( zout.bad() )
    {
        std::cerr << "--- bad" << std::endl;
    }
    if ( zout.eof() )
    {
        std::cerr << "--- eof" << std::endl;
    }

    std::cerr << "close file" << std::endl;
    zout.close();

    std::cerr << "end program" << std::endl;
    return 0;
}
