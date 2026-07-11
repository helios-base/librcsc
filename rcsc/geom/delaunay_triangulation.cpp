// -*-c++-*-

/*!
  \file delaunay_triangulation.cpp
  \brief Delaunay Triangulation class Source File.
*/

/*
 *Copyright:

 Copyright (C) Hidehisa AKIYAMA

 This code is free software; you can redistribute it and/or
 modify it under the terms of the GNU Lesser General Public
 License as published by the Free Software Foundation; either
 version 3 of the License, or (at your option) any later version.

 this library is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 Lesser General Public License for more details.

 You should have received a copy of the GNU Lesser General Public
 License along with this library; if not, write to the Free Software
 Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

 *EndCopyright:
 */

/////////////////////////////////////////////////////////////////////

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "delaunay_triangulation.h"

#include <iostream>

//#define DEBUG

namespace rcsc {

/*-------------------------------------------------------------------*/
DelaunayTriangulation::~DelaunayTriangulation()
{
    clear();
}

/*-------------------------------------------------------------------*/
void
DelaunayTriangulation::clearResults()
{
    DelaunayTriangulationCore::clearResults();
}

/*-------------------------------------------------------------------*/
void
DelaunayTriangulation::compute()
{
    if ( M_vertices.size() < 3 )
    {
        removeInitialVertices();
        return;
    }

    if ( M_triangles.empty()
         || M_triangles.size() > 3 )
    {
        clearResults();
        createInitialTriangle();
    }

    int loop = 0;
    for ( Vertex & v : M_vertices )
    {
        ++loop;
        // find triangle that contains 'vertex'
        TrianglePtr tri = nullptr;
        ContainedType type = findTriangleContainsFast( v.pos(), &tri );

        ////////////////////////////////////////////////////
        if ( ! tri
             || type == NOT_CONTAINED )
        {
            std::cerr << __FILE__ << ':' << __LINE__
                      << " compute()"
                      << " could not determine ContainedType. "
                      << v.pos()
                      << std::endl;
            clearResults();
            return;
        }

        ////////////////////////////////////////////////////
        // new vertex is contained by old triangle
        if ( type == CONTAINED )
        {
            if ( ! updateContainedVertex( &v, tri ) )
            {
                std::cerr << __FILE__ << ':' << __LINE__
                          << " ERROR in updateContainedVertex(). illegal vertex. index=" << loop
                          << std::endl;
                clearResults();
                return;
            }
        }
        else
        {
            // type == ONLINE
            if ( ! updateOnlineVertex( &v, tri ) )
            {
                std::cerr << __FILE__ << ':' << __LINE__
                          << " ERROR in updateOnlineVertex(). illegal vertex. index=" << loop
                          << std::endl;
                clearResults();
                return;
            }
        }
    }

    removeInitialVertices();
}

/*-------------------------------------------------------------------*/
void
DelaunayTriangulation::updateVoronoiVertex()
{
    for ( std::pair< const int, TrianglePtr > & v : M_triangles )
    {
        v.second->updateVoronoiVertex();
    }
}

/*-------------------------------------------------------------------*/
const
DelaunayTriangulation::Triangle *
DelaunayTriangulation::findTriangleContains( const Vector2D & pos ) const
{
    TrianglePtr tri = nullptr;
    findTriangleContainsFast( pos, &tri );
    return tri;
}

}
