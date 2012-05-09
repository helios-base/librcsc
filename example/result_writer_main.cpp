
#include <iostream>
#include <fstream>
#include <rcsc/rcg/factory.h>
#include "result_writer.h"

int
main( int argc, char** argv )
{
    if ( argc < 1 )
    {
        std::cerr << "Please input the name of rcg file." << std::endl;
        return 0;
    }

    std::ifstream fin( argv[1], std::ios_base::in | std::ios_base::binary );

    if ( ! fin.is_open() )
    {
        std::cerr << "Failed to open file : " << argv[1] << std::endl;
        return 0;
    }

    rcsc::rcg::ParserPtr parser = rcsc::rcg::make_parser( fin );

    if ( ! parser )
    {
        std::cerr << "Failed to create rcg parser." << std::endl;
        return 0;
    }

    std::cerr << parser->version() << ": pointer count = "
              << parser.use_count() << std::endl;


    fin.seekg( 0 );

    ResultWriter w;
    parser->parse( fin, w );

    return 0;
}
