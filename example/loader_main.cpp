
#include <netinet/in.h>
#include <iostream>
#include <fstream>
#include <rcsc/gz/gzfstream.h>
#include <rcsc/rcg/factory.h>
#include <rcsc/rcg/holder.h>
#include <rcsc/rcg/loader.h>

class AHolder
    : public rcsc::rcg::Holder {

    //! pure virtual. register showinfo_t (rcg v2, monitor v1)
    virtual
    bool addShowInfo( const rcsc::rcg::showinfo_t& show )
      {
          return true;
      }
    //! pure virtual. register showinfo_t2 (monitor v2 only)
    virtual
    bool addShowInfo2( const rcsc::rcg::showinfo_t2& show )
      {
          return true;
      }
    //! pure virtual. register short_showinfo_t2 (rcg v3 only)
    virtual
    bool addShortShowInfo2( const rcsc::rcg::short_showinfo_t2& show2 )
      {
          return true;
      }

    //! pure virtual. register msginfo_t
    virtual
    bool addMsgInfo( const short board, const std::string& msg )
      {
          return true;
      }
    //! pure virtual. register drawinfo_t(rcg v1:v2, monitor v1)
    virtual
    bool addDrawInfo( const rcsc::rcg::drawinfo_t& draw )
      {
          return true;
      }

    //! pure virtual. register playmode
    virtual
    bool addPlayMode( const char pmode )
      {
          std::cerr << "add playmode " << (int)pmode << std::endl;
          return true;
      }
    //! pure virtual. register team_t * 2
    virtual
    bool addTeamInfo( const rcsc::rcg::team_t& team_l,
                      const rcsc::rcg::team_t& team_r )
      {
          std::cerr << "add team info. left=" << team_l.name
                    << " " << (int)ntohs(team_l.score)
                    << " - "
                    << "right=" << team_r.name
                    << " " << (int)ntohs(team_l.score)
                    << std::endl;
          return true;
      }

    //! pure virtual. register player_type_t
    virtual
    bool addPlayerType( const rcsc::rcg::player_type_t& ptinfo )
      {
          std::cerr << "add player type" << std::endl;
          return true;
      }
    //! pure virtual. register server_params_t
    virtual
    bool addServerParam( const rcsc::rcg::server_params_t& sparams )
      {
          std::cerr << "add server param" << std::endl;
          return true;
      }
    //! pure virtual. register player_param_t
    virtual
    bool addPlayerParam( const rcsc::rcg::player_params_t& pparams )
      {
          std::cerr << "add player param" << std::endl;
          return true;
      }
};


int
main( int argc, char** argv )
{
    if ( argc < 1 )
    {
        std::cerr << "Please input the name of rcg file." << std::endl;
        return 0;
    }

    rcsc::gzifstream fin( argv[1] );

    if ( ! fin.is_open() )
    {
        std::cerr << "Failed to open file : " << argv[1] << std::endl;
        return 0;
    }

    const rcsc::rcg::ParserPtr parser = rcsc::rcg::make_parser( fin );

    if ( ! parser )
    {
        std::cerr << "Failed to create rcg parser." << std::endl;
        return 0;
    }

    std::cerr << "main. used parser version=" << parser->version()
              << ": pointer count = " << parser.use_count()
              << std::endl;


    AHolder h;
    rcsc::rcg::Loader l( h );
    parser->parse( fin, l );

    return 0;
}
