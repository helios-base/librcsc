

#ifndef SOCCER_ADVISER_H
#define SOCCER_ADVISER_H

#include <iostream>

#include <rcsc/coach/global_world_model.h>
#include <rcsc/coach/clang_manager.h>


class SoccerAdviser {
private:

public:
    virtual
    ~SoccerAdviser()
      { }


    virtual
    bool execute( const GlobalWorldModel & world,
                  CLangManager & manager ) = 0;
    virtual
    bool isAdviceTiming( const GlobalWorldModel & world,
                         const CLangManager & manager ) const = 0;
    virtual
    std::ostream & toCLang( std::ostream & o ) = 0;

};


#endif
