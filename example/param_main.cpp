

#include <rcsc/param/param_map.h>
#include <rcsc/param/cmd_line_parser.h>
#include <rcsc/param/conf_file_parser.h>

using namespace rcsc;

/*-------------------------------------------------------------------*/
int
main( int argc, char ** argv )
{
    ParamMap param_map;

    int iparam = 0;
    //double dparam = 0.0;
    bool bparam = false;
    bool switch_param = false;

    param_map.add()
        ( "iparam", "i",  &iparam, "integer parameter" )
        ( "bparam", "b",  &bparam,"boolean parameter" )
        ( "switch_param", "s",  BoolSwitch( &switch_param ),"switch parameter" );

    param_map.printHelp( std::cout );

    std::cout << std::boolalpha;
    std::cout << "Before iparam = " << iparam << std::endl;
    std::cout << "Before bparam = " << bparam << std::endl;
    std::cout << "Before switch = " << switch_param << std::endl;


    //////////////////////////////////
    ConfFileParser confparser( "test.conf" );

    if ( ! confparser.parse( param_map ) )
    {
        std::cerr << "Invalid config file" << std::endl;
        return 1;
    }

    //////////////////////////////////
    CmdLineParser cmdparser( argc, argv );

    if ( ! cmdparser.parse( param_map ) )
    {
        std::cerr << "Invalid command line argument" << std::endl;
        return 1;
    }

    //////////////////////////////////
    std::cout << "After  iparam = " << iparam << std::endl;
    std::cout << "After  bparam = " << bparam << std::endl;
    std::cout << "After  switch = " << switch_param << std::endl;

    return 0;
}
