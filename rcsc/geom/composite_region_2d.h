// -*-c++-*-

/*!
  \file composite_region_2d.h
  \brief composite 2D region class Header File.
*/

/*
 *Copyright:

 Copyright (C) Hidehisa Akiyama

 This code is free software; you can redistribute it and/or
 modify it under the terms of the GNU Lesser General Public
 License as published by the Free Software Foundation; either
 version 3 of the License, or (at your option) any later version.

 This library is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 Lesser General Public License for more details.

 You should have received a copy of the GNU Lesser General Public
 License along with this library; if not, write to the Free Software
 Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

 *EndCopyright:
 */

/////////////////////////////////////////////////////////////////////

#ifndef RCSC_GEOM_COMPOSITE_REGION2D_H
#define RCSC_GEOM_COMPOSITE_REGION2D_H

#include <rcsc/geom/region_2d.h>

#include <memory>
#include <vector>

namespace rcsc {

/*!
  \class UnitedRegion2D
  \brief union set of 2D regions
*/
class UnitedRegion2D
    : public Region2D {
private:
    //! the set of regions
    std::vector< std::shared_ptr< const Region2D > > M_regions;
public:

    /*!
      \brief create empty region set
    */
    UnitedRegion2D()
      { }

    /*!
      \brief construct with 2 regions. all arguments must be a dynamically allocated object.
      \param r1 1st region
      \param r2 2nd region
    */
    UnitedRegion2D( const Region2D * r1 ,
                    const Region2D * r2 )
      {
          M_regions.push_back( std::shared_ptr< const Region2D >( r1 ) );
          M_regions.push_back( std::shared_ptr< const Region2D >( r2 ) );
      }

    /*!
      \brief construct with regions. all arguments must be a dynamically allocated object.
      \param r1 1st region
      \param r2 2nd region
      \param r3 3rd region
    */
    UnitedRegion2D( const Region2D * r1 ,
                    const Region2D * r2,
                    const Region2D * r3 )
      {
          M_regions.push_back( std::shared_ptr< const Region2D >( r1 ) );
          M_regions.push_back( std::shared_ptr< const Region2D >( r2 ) );
          M_regions.push_back( std::shared_ptr< const Region2D >( r3 ) );
      }

    /*!
      \brief construct with regions. all arguments must be a dynamically allocated object.
      \param r1 1st region
      \param r2 2nd region
      \param r3 3rd region
      \param r4 4th region
    */
    UnitedRegion2D( const Region2D * r1 ,
                    const Region2D * r2,
                    const Region2D * r3,
                    const Region2D * r4 )
      {
          M_regions.push_back( std::shared_ptr< const Region2D >( r1 ) );
          M_regions.push_back( std::shared_ptr< const Region2D >( r2 ) );
          M_regions.push_back( std::shared_ptr< const Region2D >( r3 ) );
          M_regions.push_back( std::shared_ptr< const Region2D >( r4 ) );
      }

    /*!
      \brief construct with regions. all arguments must be a dynamically allocated object.
      \param r1 1st region
      \param r2 2nd region
      \param r3 3rd region
      \param r4 4th region
      \param r5 5th region
    */
    UnitedRegion2D( const Region2D * r1 ,
                    const Region2D * r2,
                    const Region2D * r3,
                    const Region2D * r4,
                    const Region2D * r5 )
      {
          M_regions.push_back( std::shared_ptr< const Region2D >( r1 ) );
          M_regions.push_back( std::shared_ptr< const Region2D >( r2 ) );
          M_regions.push_back( std::shared_ptr< const Region2D >( r3 ) );
          M_regions.push_back( std::shared_ptr< const Region2D >( r4 ) );
          M_regions.push_back( std::shared_ptr< const Region2D >( r5 ) );
      }

    /*!
      \brief construct with regions. all arguments must be a dynamically allocated object.
      \param r1 1st region
      \param r2 2nd region
      \param r3 3rd region
      \param r4 4th region
      \param r5 5th region
      \param r6 6th region
    */
    UnitedRegion2D( const Region2D * r1 ,
                    const Region2D * r2,
                    const Region2D * r3,
                    const Region2D * r4,
                    const Region2D * r5,
                    const Region2D * r6 )
      {
          M_regions.push_back( std::shared_ptr< const Region2D >( r1 ) );
          M_regions.push_back( std::shared_ptr< const Region2D >( r2 ) );
          M_regions.push_back( std::shared_ptr< const Region2D >( r3 ) );
          M_regions.push_back( std::shared_ptr< const Region2D >( r4 ) );
          M_regions.push_back( std::shared_ptr< const Region2D >( r5 ) );
          M_regions.push_back( std::shared_ptr< const Region2D >( r6 ) );
      }

    /*!
      \brief construct with regions. all arguments must be a dynamically allocated object.
      \param r1 1st region
      \param r2 2nd region
      \param r3 3rd region
      \param r4 4th region
      \param r5 5th region
      \param r6 6th region
      \param r7 7th region
    */
    UnitedRegion2D( const Region2D * r1 ,
                    const Region2D * r2,
                    const Region2D * r3,
                    const Region2D * r4,
                    const Region2D * r5,
                    const Region2D * r6,
                    const Region2D * r7 )
      {
          M_regions.push_back( std::shared_ptr< const Region2D >( r1 ) );
          M_regions.push_back( std::shared_ptr< const Region2D >( r2 ) );
          M_regions.push_back( std::shared_ptr< const Region2D >( r3 ) );
          M_regions.push_back( std::shared_ptr< const Region2D >( r4 ) );
          M_regions.push_back( std::shared_ptr< const Region2D >( r5 ) );
          M_regions.push_back( std::shared_ptr< const Region2D >( r6 ) );
          M_regions.push_back( std::shared_ptr< const Region2D >( r7 ) );
      }

    /*!
      \brief construct with regions. all arguments must be a dynamically allocated object.
      \param r1 1st region
      \param r2 2nd region
      \param r3 3rd region
      \param r4 4th region
      \param r5 5th region
      \param r6 6th region
      \param r7 7th region
      \param r8 8th region
    */
    UnitedRegion2D( const Region2D * r1 ,
                    const Region2D * r2,
                    const Region2D * r3,
                    const Region2D * r4,
                    const Region2D * r5,
                    const Region2D * r6,
                    const Region2D * r7,
                    const Region2D * r8 )
      {
          M_regions.push_back( std::shared_ptr< const Region2D >( r1 ) );
          M_regions.push_back( std::shared_ptr< const Region2D >( r2 ) );
          M_regions.push_back( std::shared_ptr< const Region2D >( r3 ) );
          M_regions.push_back( std::shared_ptr< const Region2D >( r4 ) );
          M_regions.push_back( std::shared_ptr< const Region2D >( r5 ) );
          M_regions.push_back( std::shared_ptr< const Region2D >( r6 ) );
          M_regions.push_back( std::shared_ptr< const Region2D >( r7 ) );
          M_regions.push_back( std::shared_ptr< const Region2D >( r8 ) );
      }

    /*!
      \brief add new region.
      \param r new region, which must be a dynamically allocated object.
      \return reference to itself
    */
    UnitedRegion2D & add( const Region2D * r )
      {
          M_regions.push_back( std::shared_ptr< const Region2D >( r ) );
          return *this;
      }

    /*!
      \brief not implemented yet.
      \return always 0
    */
    virtual
    double area() const;

    /*!
      \brief check if union region contains 'point'.
      \param point considerd point
      \return true or false
    */
    virtual
    bool contains( const Vector2D & point ) const;
};


/*!
  \class IntersectedRegion2D
  \brief inersection set of 2D regions
*/
class IntersectedRegion2D
    : public Region2D {
private:
    //! the set of regions
    std::vector< std::shared_ptr< const Region2D > > M_regions;
public:

    /*!
      \brief construct with 2 regions. all arguments must be a dynamically allocated object.
      \param r1 1st region
      \param r2 2nd region
    */
    IntersectedRegion2D( const Region2D * r1 ,
                         const Region2D * r2 )
      {
          M_regions.push_back( std::shared_ptr< const Region2D >( r1 ) );
          M_regions.push_back( std::shared_ptr< const Region2D >( r2 ) );
      }

    /*!
      \brief construct with regions. all arguments must be a dynamically allocated object.
      \param r1 1st region
      \param r2 2nd region
      \param r3 3rd region
    */
    IntersectedRegion2D( const Region2D * r1 ,
                         const Region2D * r2,
                         const Region2D * r3 )
      {
          M_regions.push_back( std::shared_ptr< const Region2D >( r1 ) );
          M_regions.push_back( std::shared_ptr< const Region2D >( r2 ) );
          M_regions.push_back( std::shared_ptr< const Region2D >( r3 ) );
      }

    /*!
      \brief construct with regions. all arguments must be a dynamically allocated object.
      \param r1 1st region
      \param r2 2nd region
      \param r3 3rd region
      \param r4 4th region
    */
    IntersectedRegion2D( const Region2D * r1 ,
                         const Region2D * r2,
                         const Region2D * r3,
                         const Region2D * r4 )
      {
          M_regions.push_back( std::shared_ptr< const Region2D >( r1 ) );
          M_regions.push_back( std::shared_ptr< const Region2D >( r2 ) );
          M_regions.push_back( std::shared_ptr< const Region2D >( r3 ) );
          M_regions.push_back( std::shared_ptr< const Region2D >( r4 ) );
      }

    /*!
      \brief construct with regions. all arguments must be a dynamically allocated object.
      \param r1 1st region
      \param r2 2nd region
      \param r3 3rd region
      \param r4 4th region
      \param r5 5th region
    */
    IntersectedRegion2D( const Region2D * r1 ,
                         const Region2D * r2,
                         const Region2D * r3,
                         const Region2D * r4,
                         const Region2D * r5 )
      {
          M_regions.push_back( std::shared_ptr< const Region2D >( r1 ) );
          M_regions.push_back( std::shared_ptr< const Region2D >( r2 ) );
          M_regions.push_back( std::shared_ptr< const Region2D >( r3 ) );
          M_regions.push_back( std::shared_ptr< const Region2D >( r4 ) );
          M_regions.push_back( std::shared_ptr< const Region2D >( r5 ) );
      }

    /*!
      \brief construct with regions. all arguments must be a dynamically allocated object.
      \param r1 1st region
      \param r2 2nd region
      \param r3 3rd region
      \param r4 4th region
      \param r5 5th region
      \param r6 6th region
    */
    IntersectedRegion2D( const Region2D * r1 ,
                         const Region2D * r2,
                         const Region2D * r3,
                         const Region2D * r4,
                         const Region2D * r5,
                         const Region2D * r6 )
      {
          M_regions.push_back( std::shared_ptr< const Region2D >( r1 ) );
          M_regions.push_back( std::shared_ptr< const Region2D >( r2 ) );
          M_regions.push_back( std::shared_ptr< const Region2D >( r3 ) );
          M_regions.push_back( std::shared_ptr< const Region2D >( r4 ) );
          M_regions.push_back( std::shared_ptr< const Region2D >( r5 ) );
          M_regions.push_back( std::shared_ptr< const Region2D >( r6 ) );
      }

    /*!
      \brief construct with regions. all arguments must be a dynamically allocated object.
      \param r1 1st region
      \param r2 2nd region
      \param r3 3rd region
      \param r4 4th region
      \param r5 5th region
      \param r6 6th region
      \param r7 7th region
    */
    IntersectedRegion2D( const Region2D * r1 ,
                         const Region2D * r2,
                         const Region2D * r3,
                         const Region2D * r4,
                         const Region2D * r5,
                         const Region2D * r6,
                         const Region2D * r7 )
      {
          M_regions.push_back( std::shared_ptr< const Region2D >( r1 ) );
          M_regions.push_back( std::shared_ptr< const Region2D >( r2 ) );
          M_regions.push_back( std::shared_ptr< const Region2D >( r3 ) );
          M_regions.push_back( std::shared_ptr< const Region2D >( r4 ) );
          M_regions.push_back( std::shared_ptr< const Region2D >( r5 ) );
          M_regions.push_back( std::shared_ptr< const Region2D >( r6 ) );
          M_regions.push_back( std::shared_ptr< const Region2D >( r7 ) );
      }

    /*!
      \brief construct with regions. all arguments must be a dynamically allocated object.
      \param r1 1st region
      \param r2 2nd region
      \param r3 3rd region
      \param r4 4th region
      \param r5 5th region
      \param r6 6th region
      \param r7 7th region
      \param r8 8th region
    */
    IntersectedRegion2D( const Region2D * r1 ,
                         const Region2D * r2,
                         const Region2D * r3,
                         const Region2D * r4,
                         const Region2D * r5,
                         const Region2D * r6,
                         const Region2D * r7,
                         const Region2D * r8 )
      {
          M_regions.push_back( std::shared_ptr< const Region2D >( r1 ) );
          M_regions.push_back( std::shared_ptr< const Region2D >( r2 ) );
          M_regions.push_back( std::shared_ptr< const Region2D >( r3 ) );
          M_regions.push_back( std::shared_ptr< const Region2D >( r4 ) );
          M_regions.push_back( std::shared_ptr< const Region2D >( r5 ) );
          M_regions.push_back( std::shared_ptr< const Region2D >( r6 ) );
          M_regions.push_back( std::shared_ptr< const Region2D >( r7 ) );
          M_regions.push_back( std::shared_ptr< const Region2D >( r8 ) );
      }

    /*!
      \brief add new region.
      \param r new region, which must be a dynamically allocated object.
      \return reference to itself
    */
    IntersectedRegion2D & add( const Region2D * r )
      {
          M_regions.push_back( std::shared_ptr< const Region2D >( r ) );
          return *this;
      }

    /*!
      \brief not implemented yet.
      \return always 0
    */
    virtual
    double area() const;


    /*!
      \brief check if union region contains 'point'.
      \param point considerd point
      \return true or false
    */
    virtual
    bool contains( const Vector2D & point ) const;

};

}

#endif
