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

#include <rcsc/geom/triangle_2d.h>

namespace rcsc {

const double DelaunayTriangulation::EPSILON = 1.0e-10;

namespace {

static const std::pair< std::size_t, std::size_t >
edge_pairs[3] = { std::pair< std::size_t, std::size_t >( 0, 1 ),
                  std::pair< std::size_t, std::size_t >( 1, 2 ),
                  std::pair< std::size_t, std::size_t >( 2, 0 ),
};

}

//#define DEBUG
//#define DEBUG2

/*-------------------------------------------------------------------*/
/*!

*/
DelaunayTriangulation::Triangle::Triangle( const int id,
                                           EdgePtr e0,
                                           EdgePtr e1,
                                           EdgePtr e2 )
    : M_id( id ),
      M_voronoi_vertex( Vector2D::INVALIDATED )
{
    //std::cout << "Triangle() start id = " << id << std::endl;

    //std::cout << "Triangle() edge0 "
    //          << e0->vertex( 0 )->pos() << e0->vertex( 1 )->pos()
    //          << " edge1 " << e1->vertex( 0 )->pos() << e1->vertex( 1 )->pos()
    //          << " edge2 " << e2->vertex( 0 )->pos() << e2->vertex( 1 )->pos()
    //          << std::endl;

    //S_tri_construct_counter++;

    M_edges[0] = e0;
    M_edges[1] = e1;
    M_edges[2] = e2;

    //std::cout << "Triangle() setTriangle for edges" << std::endl;
    // set this pointer to edges
    for ( std::size_t i = 0; i < 3; ++i )
    {
        M_edges[i]->setTriangle( this );
    }

    /*
    for ( std::size_t i = 0; i < 3; ++i )
    {
        //std::cout << "Triangle() M_edge " << i
        //          << "  p= "<< M_edges[i]
        //          << "  v0 " << M_edges[i]->vertex( 0 )
        //          << "  v1 " << M_edges[i]->vertex( 1 )
        //          << std::endl;
    }
    */

    // set vertices
    M_vertices[0] = M_edges[0]->vertex( 0 );
    M_vertices[1] = M_edges[0]->vertex( 1 );
    //std::cout << "Triangle() M_vertices[0] " << M_vertices[0]
    //          << " M_vertices[1] " << M_vertices[1]
    //          << std::endl;
    M_vertices[2] = ( ( M_vertices[0] != M_edges[1]->vertex( 0 )
                        && M_vertices[1] != M_edges[1]->vertex( 0 ) )
                      ? M_edges[1]->vertex( 0 )
                      : M_edges[1]->vertex( 1 ) );

    //std::cout << "Triangle() create circumcenter of "
    //          << M_vertices[0]->pos()
    //          << M_vertices[1]->pos()
    //          << M_vertices[2]->pos()
    //          << std::endl;

    // set circumcircle data
    M_circumcenter = Triangle2D::circumcenter( M_vertices[0]->pos(),
                                               M_vertices[1]->pos(),
                                               M_vertices[2]->pos() );

    M_circumradius = M_circumcenter.dist( M_vertices[0]->pos() );

    //std::cout << "Triangle() circumcenter " << M_circumcenter
    //          << " radius " << M_circumradius
    //          << std::endl;

    //std::cout << "Triangle() end" << std::endl;
}


/*-------------------------------------------------------------------*/
/*!

*/
void
DelaunayTriangulation::Triangle::updateVoronoiVertex()
{
    Line2D l1 = Line2D::perpendicular_bisector( M_vertices[0]->pos(),
                                                M_vertices[1]->pos() );
    Line2D l2 = Line2D::perpendicular_bisector( M_vertices[1]->pos(),
                                                M_vertices[2]->pos() );

    M_voronoi_vertex = l1.intersection( l2 );
    if ( ! M_voronoi_vertex.isValid() )
    {
        l2 = Line2D::perpendicular_bisector( M_vertices[0]->pos(),
                                             M_vertices[2]->pos() );
        M_voronoi_vertex = l1.intersection( l2 );

        if ( ! M_voronoi_vertex.isValid() )
        {
            std::cerr <<"(DelaunayTriangulation::Triangle::updateVoronoiVertex):"
                      << " Could not calculate the vertex candidate point."
                      << std::endl;
        }
    }
}

/*-------------------------------------------------------------------*/
/*!

*/
DelaunayTriangulation::~DelaunayTriangulation()
{
    //std::cout << "~DelaunayTriangulation() start" << std::endl;
    clear();
    //std::cout << "~DelaunayTriangulation() end destructed." << std::endl;
}

/*-------------------------------------------------------------------*/
/*!

*/
void
DelaunayTriangulation::clear()
{
    clearResults();
    M_vertices.clear();
}

/*-------------------------------------------------------------------*/
/*!

*/
void
DelaunayTriangulation::clearResults()
{
    M_edge_count = M_tri_count = 0;

    for ( TriangleCont::iterator it = M_triangles.begin();
          it != M_triangles.end();
          ++it )
    {
        delete it->second;
    }

    for ( EdgeCont::iterator it = M_edges.begin();
          it != M_edges.end();
          ++it )
    {
        delete it->second;
    }

    M_triangles.clear();
    M_edges.clear();
}

/*-------------------------------------------------------------------*/
/*!

*/
int
DelaunayTriangulation::addVertex( const double x,
                                  const double y )
{
    for ( VertexCont::iterator it = M_vertices.begin(), end = M_vertices.end();
          it != end;
          ++it )
    {
        if ( std::pow( it->pos().x - x, 2 ) + std::pow( it->pos().y - y,2) < 1.0e-6 )
        {
            // detect same coordinate vertex
            return -1;
        }
    }

    int id = M_vertices.size();
    M_vertices.emplace_back( id, x, y );
    return id;
}

/*-------------------------------------------------------------------*/
/*!

*/
void
DelaunayTriangulation::addVertices( const std::vector< Vector2D > & v )
{
    M_vertices.reserve( M_vertices.size() + v.size() );

    int id = M_vertices.size();

    for ( const Vector2D & d : v )
    {
        M_vertices.emplace_back( id, d.x, d.y );
        ++id;
    }
}

/*-------------------------------------------------------------------*/
/*!

*/
void
DelaunayTriangulation::createInitialTriangle( const Rect2D & region )
{
    //std::cout << "createInitialTriangle(region) start" << std::endl;

    // reset all result data
    clearResults();

    //std::cout << "createInitialTriangle(region) cleared" << std::endl;

    // create double size rectanble
    //Vector2D top_left = region.topLeft();

    double max_size = std::max( region.size().length() + 1.0,
                                region.size().width() + 1.0 );
    //std::cerr << "region max size = " << max_size << std::endl;
    Vector2D center = region.center();

    M_initial_vertex[0].assign( -1,
                                center.x + std::max( 1000.0 * max_size, 1000.0 ),
                                center.y );

    M_initial_vertex[1].assign( -2,
                                center.x,
                                center.y + std::max( 1000.0 * max_size, 1000.0 ) );

    M_initial_vertex[2].assign( -3,
                                center.x - std::max( 1000.0 * max_size, 1000.0 ),
                                center.y - std::max( 1000.0 * max_size, 1000.0 ) );

    //std::cout << "createInitialTriangle(region) create super triangle edges" << std::endl;

    EdgePtr edge0 = createEdge( &M_initial_vertex[0],
                                &M_initial_vertex[1] );
    EdgePtr edge1 = createEdge( &M_initial_vertex[1],
                                &M_initial_vertex[2] );
    EdgePtr edge2 = createEdge( &M_initial_vertex[2],
                                &M_initial_vertex[0] );

    //std::cout << "createInitialTriangle(region) create super triangle" << std::endl;
    createTriangle( edge0, edge1, edge2 );

    // std::cout << "createInitialTriangle(region) end" << std::endl;
}

/*-------------------------------------------------------------------*/
/*!

*/
void
DelaunayTriangulation::createInitialTriangle()
{
    // std::cout << "createInitialTriangle() vertex size = "
    //           << M_vertices.size() << std::endl;
    if ( M_vertices.empty() )
    {
        return;
    }

    VertexCont::iterator vit = M_vertices.begin();

    double min_x = vit->pos().x;
    double max_x = vit->pos().x;
    double min_y = vit->pos().y;
    double max_y = vit->pos().y;

    ++vit;
    const VertexCont::iterator vend = M_vertices.end();
    for ( ; vit != vend; ++vit )
    {
        if ( vit->pos().x < min_x ) min_x = vit->pos().x;
        else if ( max_x < vit->pos().x ) max_x = vit->pos().x;

        if ( vit->pos().y < min_y ) min_y = vit->pos().y;
        else if ( max_y < vit->pos().y ) max_y = vit->pos().y;
    }

    // std::cout << __FILE__": createInitialTriangle() min="
    //           << min_x << " " << min_y
    //           << " max=" << max_x << " " << max_y
    //           << std::endl;

    createInitialTriangle( Rect2D( Vector2D( min_x - 1.0, min_y - 1.0 ),
                                   Vector2D( min_x + 1.0, min_y + 1.0 ) ) );
}

/*-------------------------------------------------------------------*/
/*!

*/
void
DelaunayTriangulation::removeInitialVertices()
{
    std::vector< EdgePtr > removed_edges;

    // search removed edges that has initial vertex
    for ( EdgeCont::iterator it = M_edges.begin(), end = M_edges.end();
          it != end;
          ++it )
    {
        for ( std::size_t i = 0; i < 3; ++i )
        {
            if ( it->second->vertex( 0 ) == &M_initial_vertex[i]
                 || it->second->vertex( 1 ) == &M_initial_vertex[i] )
            {
                removed_edges.push_back( it->second );
                break;
            }
        }
    }

    // remove edges and triangles
    for ( std::vector< EdgePtr >::iterator it = removed_edges.begin(), end = removed_edges.end();
          it != end;
          ++it )
    {
        removeTriangle( (*it)->triangle( 0 ) );
        removeTriangle( (*it)->triangle( 1 ) );

        removeEdge( (*it)->id() );
    }
}

/*-------------------------------------------------------------------*/
/*!

*/
const
DelaunayTriangulation::Vertex *
DelaunayTriangulation::getVertex( const int id ) const
{
    if ( M_vertices.empty()
         || id < 0
         || static_cast< int >( M_vertices.size() ) < id )
    {
        return nullptr;
    }
    return &M_vertices[id];
}

/*-------------------------------------------------------------------*/
/*!

*/
const
DelaunayTriangulation::Triangle *
DelaunayTriangulation::findTriangleContains( const Vector2D & pos ) const
{
    Triangle * tri = nullptr;
    findTriangleContains( pos, &tri );
    return tri;
}

/*-------------------------------------------------------------------*/
/*!

*/
const
DelaunayTriangulation::Vertex *
DelaunayTriangulation::findNearestVertex( const Vector2D & pos ) const
{
    const Vertex * candidate = nullptr;

    double min_dist2 = 10000000.0;
    for ( VertexCont::const_iterator it = M_vertices.begin(), end = M_vertices.end();
          it != end;
          ++it )
    {
        double d2 = it->pos().dist2( pos );
        if ( d2 < min_dist2 )
        {
            candidate = &(*it);
            min_dist2 = d2;
        }
    }

    return candidate;
}

/*-------------------------------------------------------------------*/
/*!

*/
void
DelaunayTriangulation::compute()
{
    //std::cout << "compute() start " << std::endl;
    if ( M_vertices.size() < 3 )
    {
        //std::cout << __FILE__ << ": compute() too few vertices" << std::endl;
        removeInitialVertices();
        return;
    }

    if ( M_triangles.empty()
         || M_triangles.size() > 3 )
    {
        // std::cout << __FILE__ << ": compute() create initial triangle no arg" << std::endl;
        createInitialTriangle();
    }

    // std::cout << "********** compute() start **********"
    //           << " vertex size = " << vertices().size()
    //           << std::endl;

    int loop = 0;
    for ( VertexCont::iterator vit = M_vertices.begin(), end = M_vertices.end();
          vit != end;
          ++vit )
    {
        ++loop;
        //std::cout << "compute() ********** vertex loop " << loop
        //          << vit->pos() << std::endl;
        // find triangle that contains 'vertex'
        TrianglePtr tri = nullptr;
        ContainedType type = findTriangleContains( vit->pos(), &tri );

        ////////////////////////////////////////////////////
        if ( ! tri
             || type == NOT_CONTAINED )
        {
            std::cerr << __FILE__ << ':' << __LINE__
                      << " compute()"
                      << " could not determine ContainedType. "
                      << vit->pos()
                      << std::endl;
            clearResults();
            return;
        }

        ////////////////////////////////////////////////////
        // new vertex is contained by old triangle
        if ( type == CONTAINED )
        {
            if ( ! updateContainedVertex( &(*vit), tri ) )
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
            if ( ! updateOnlineVertex( &(*vit), tri ) )
            {
                std::cerr << __FILE__ << ':' << __LINE__
                          << " ERROR in updateOnlineVertex(). illegal vertex. index=" << loop
                          << std::endl;
                clearResults();
                return;
            }
        }

#ifdef DEBUG
        std::cout << __FILE__ << ':' << __LINE__
                  << " ----- result of loop " << loop
                  << " edge num= " << M_edges.size()
                  << " triangle num= " << M_triangles.size()
                  << std::endl;
        for ( TriangleCont::iterator it = M_triangles.begin();
              it != M_triangles.end();
              ++it )
        {
            std::cout << "  triangle " << it->second->id()
                      << it->second->vertex( 0 )->pos()
                      << it->second->vertex( 1 )->pos()
                      << it->second->vertex( 2 )->pos()
                      << std::endl;
        }
        std::cout << "--------------------------------------" << std::endl;
#endif
    }

    removeInitialVertices();
#ifdef DEBUG
    std::cout << __FILE__ << ':' << __LINE__
              << " compute() end\n"
              << "----- result of trianglation "
              << " edge num= " << M_edges.size()
              << " triangle num= " << M_triangles.size()
              << std::endl;

    for ( TriangleCont::iterator it = M_triangles.begin();
          it != M_triangles.end();
          ++it )
    {
        std::cout << "  triangle " << it->second->id()
                  << it->second->vertex( 0 )->pos()
                  << it->second->vertex( 1 )->pos()
                  << it->second->vertex( 2 )->pos()
                  << std::endl;
    }
#endif
}

/*-------------------------------------------------------------------*/
/*!

*/
void
DelaunayTriangulation::updateVoronoiVertex()
{
    for ( TriangleCont::iterator it = M_triangles.begin(), end = M_triangles.end();
          it != end;
          ++it )
    {
        it->second->updateVoronoiVertex();
    }
}

/*-------------------------------------------------------------------*/
/*!

*/
bool
DelaunayTriangulation::updateContainedVertex( const Vertex * new_vertex,
                                              const TrianglePtr tri )
{
#ifdef DEBUG
    std::cout << __FILE__ << ':' << __LINE__
              << " updateContainedVertex() start  tri_id=" << tri->id()
              << std::endl;
#endif
    // split 'tri' to 3 pieces
    // --> create new 3 triangle in 'tri'

    // create new edge
    EdgePtr new_edges[3];
    for ( std::size_t i = 0; i < 3; ++i )
    {
        //std::cout << "updateContainedVertex() create edge new_v "
        //          << new_vertex << new_vertex.pos()
        //          << "  tri_v " << tri->vertex( i ) << tri->vertex( i )->pos()
        //          << std::endl;
        // *** tri->vertex(i) must be the second argument!!
        new_edges[i] = createEdge( new_vertex, tri->vertex( i ) );
    }

    // create child triangles
    //std::cout << "updateContainedVertex() create child triangle start" << std::endl;

    EdgePtr old_edges[3]; // edges of 'tri'
    TrianglePtr new_tri[3];

    for ( std::size_t i = 0; i < 3; ++i )
    {
        old_edges[i]
            = tri->getEdgeInclude( new_edges[ edge_pairs[i].first ]->vertex( 1 ),
                                   new_edges[ edge_pairs[i].second ]->vertex( 1 ) );
        //std::cout << "updateContainedVertex() remove old triangle " << i
        //          << std::endl;
        old_edges[i]->removeTriangle( tri );
        new_tri[i] = createTriangle( old_edges[i],
                                     new_edges[ edge_pairs[i].first ],
                                     new_edges[ edge_pairs[i].second ] );
        if ( ! new_tri[i]->circumcenter().isValid() )
        {
            std::cerr << __FILE__ << ':' << __LINE__
                      << " updateContainedVertex() detect illegal vertex\n"
                      << new_tri[i] << '\n'
                      << old_edges[i]->vertex( 0 )->pos()
                      << old_edges[i]->vertex( 1 )->pos() << "\n"
                      << new_edges[ edge_pairs[i].first ]->vertex( 0 )->pos()
                      << new_edges[ edge_pairs[i].first ]->vertex( 1 )->pos() << "\n"
                      << new_edges[ edge_pairs[i].second ]->vertex( 0 )->pos()
                      << new_edges[ edge_pairs[i].second ]->vertex( 1 )->pos() << "\n"
                      << std::endl;
            return false;
        }
    }

    //std::cout << "updateContainedVertex() create child triangle end " << std::endl;

    // remove old triangle
    removeTriangle( tri );

    //std::cout << "updateContainedVertex() removed old triangle " << std::endl;

    // legalize new triangles
    for ( std::size_t i = 0; i < 3; ++i )
    {
        if ( ! legalizeEdge( new_tri[i],
                             new_vertex,
                             old_edges[i] ) )
        {
            return false;
        }
    }
    //std::cout << "updateContainedVertex() end " << std::endl;
    return true;
}

/*-------------------------------------------------------------------*/
/*!

*/
bool
DelaunayTriangulation::updateOnlineVertex( const Vertex * new_vertex,
                                           const TrianglePtr tri )
{
#ifdef DEBUG
    std::cout << __FILE__ << ':' << __LINE__
              << "updateOnlineVertex() start tri_id=" << tri->id()
              << std::endl;
#endif

    // find edge that vertex is on-line
    int online_count = 0;
    EdgePtr online_edge = nullptr;
    for ( std::size_t i = 0; i < 3; ++i )
    {
        Vector2D rel0( tri->edge( i )->vertex( 0 )->pos() - new_vertex->pos() );
        Vector2D rel1( tri->edge( i )->vertex( 1 )->pos() - new_vertex->pos() );
        // check area value of sub triangle
        if ( std::fabs( rel0.outerProduct( rel1 ) ) <= EPSILON )
        {
            online_edge = tri->edge( i );
            ++online_count;
        }
    }

    if ( online_count >= 2 )
    {
        std::cerr << __FILE__ << ':' << __LINE__
                  << " ***ERROR*** updateOnlineVertex()."
                  << " detect the same vertex in old triangle="
                  << tri->vertex( 0 )->pos()
                  << tri->vertex( 1 )->pos()
                  << tri->vertex( 2 )->pos()
                  << " illegal_vertex=" << new_vertex->pos()
                  << std::endl;
        return false;
    }

    if ( ! online_edge )
    {
        std::cerr << __FILE__ << ':' << __LINE__
                  << " ***ERROR*** updateOnlineVertex()."
                  << " failed to find online edge."
                  << " illegal_vertex=" << new_vertex->pos()
                  << std::endl;
        return false;
    }
#ifdef DEBUG
    std::cout << __FILE__ << ':' << __LINE__
              << " updateOnlineVertex() find adjacent\n"
              << "  online_edge_id_" << online_edge->id()
              << online_edge->vertex( 0 )->pos()
              << online_edge->vertex( 1 )->pos()
              << "  tri_0 " << online_edge->triangle( 0 )
              << "  tri_1 " << online_edge->triangle( 1 )
              << std::endl;
#endif
    ////////////////////////////////////////////////////////////////

    // create child edge of 'online_edge'
    EdgePtr new_edge[2]; // edges that is shared by 'tri' and 'adjacent_tri'
    for ( std::size_t i = 0; i < 2; ++i )
    {
        new_edge[i] = createEdge( new_vertex, online_edge->vertex( i ) );
    }

    ////////////////////////////////////////////////////////////////

    TrianglePtr new_tri_in_tri[2];
    EdgePtr old_edge_in_tri[2];

    // create new child triangles in 'tri'
    {
#ifdef DEBUG
        std::cout << __FILE__ << ':' << __LINE__
                  << " updateOnlineVertex() create new child triangle(1)"
                  << std::endl;
#endif
        // get vertex that is not on the online_edge.
        const Vertex * tri_vertex = tri->getVertexExclude( online_edge );

        if ( ! tri_vertex )
        {
            std::cerr << __FILE__ << ':' << __LINE__
                      << " updateOnlineVertex()."
                      << " failed to find vertex of tri."
                      << std::endl;
            return false;
        }

        EdgePtr new_edge_in_tri = createEdge( new_vertex, tri_vertex );

        // create new child triangles in tri
        for ( std::size_t i = 0; i < 2; ++i )
        {
            old_edge_in_tri[i] = tri->getEdgeInclude( new_edge[i]->vertex( 1 ),
                                                      tri_vertex );
            // remove old triangle from old edge
            old_edge_in_tri[i]->removeTriangle( tri );
            // first argument edge must be old existing edge
            new_tri_in_tri[i] = createTriangle( old_edge_in_tri[i],
                                                new_edge[i],
                                                new_edge_in_tri );
            if ( ! new_tri_in_tri[i]->circumcenter().isValid() )
            {
                std::cerr << __FILE__ << ':' << __LINE__
                          << " updateOnlineVertex() detect illegal vertex normal."
                          << "\n  tri=" << new_tri_in_tri[i]->vertex( 0 )->pos()
                          << new_tri_in_tri[i]->vertex( 1 )->pos()
                          << new_tri_in_tri[i]->vertex( 2 )->pos()
                          << "\n  new vertex pos=" << new_vertex->pos()
                          << "\n  old_edge:" << old_edge_in_tri[i]->vertex( 0 )->pos()
                          << "-" << old_edge_in_tri[i]->vertex( 1 )->pos()
                          << "\n  new_edge 1:" << new_edge[i]->vertex( 0 )->pos()
                          << "-" << new_edge[i]->vertex( 1 )->pos()
                          << "\n  edge 3:" << new_edge_in_tri->vertex( 0 )->pos()
                          << "-" << new_edge_in_tri->vertex( 1 )->pos()
                          << std::endl;
                return false;
            }
        }
    }

    ////////////////////////////////////////////////////////////////

    // get adjacent triangle that shares online_edge
    Triangle * adjacent = ( online_edge->triangle( 0 ) == tri
                            ? online_edge->triangle( 1 )
                            : online_edge->triangle( 0 ) );
    const bool exist_adjacent = ( adjacent ? true : false );
#ifdef DEBUG
    std::cout << __FILE__ << ':' << __LINE__
              << " updateOnlineVertex() find adjacent\n"
              << "  online_edge "
              << online_edge->vertex( 0 )->pos()
              << online_edge->vertex( 1 )->pos() << "\n"
              << "  tri = " << tri << "\n"
              << "  online_edge_tri_0 " << online_edge->triangle( 0 )
              << "  online_edge_tri_1 " << online_edge->triangle( 1 )
              << "\n  adjacent = " << adjacent
              << std::endl;
#endif
    TrianglePtr new_tri_in_adjacent[2];
    EdgePtr old_edge_in_adjacent[2];

    // create new child triangles in 'adjacent'
    if ( exist_adjacent )
    {
#ifdef DEBUG
        std::cout << __FILE__ << ':' << __LINE__
                  << " updateOnlineVertex() create new child triangle(1)"
                  << std::endl;
#endif
        const Vertex * adjacent_vertex = adjacent->getVertexExclude( online_edge );

        if ( ! adjacent_vertex )
        {
            std::cerr << __FILE__ << ':' << __LINE__
                      << " updateOnliePoint()."
                      << " failed to find vertex of adjacent."
                      << std::endl;
            return false;
        }

        EdgePtr new_edge_in_adjacent = createEdge( new_vertex, adjacent_vertex );

        // create new child triangles in tri
        for ( std::size_t i = 0; i < 2; ++i )
        {
            old_edge_in_adjacent[i]
                = adjacent->getEdgeInclude( new_edge[i]->vertex( 1 ),
                                            adjacent_vertex );
            // remove old triangle from old edge
            old_edge_in_adjacent[i]->removeTriangle( adjacent );
            // first argument edge must be old existing edge
            new_tri_in_adjacent[i] = createTriangle( old_edge_in_adjacent[i],
                                                     new_edge[i],
                                                     new_edge_in_adjacent );
            if ( ! new_tri_in_adjacent[i]->circumcenter().isValid() )
            {
                std::cerr << __FILE__ << ':' << __LINE__
                          << " updateOnlineVertex() detect illegal vertex adjacent"
                          << "\n  tri=" << new_tri_in_adjacent[i]->vertex( 0 )->pos()
                          << new_tri_in_adjacent[i]->vertex( 1 )->pos()
                          << new_tri_in_adjacent[i]->vertex( 2 )->pos()
                          << "\n  new vertex pos=" << new_vertex->pos()
                          << "\n  edge 1:" << old_edge_in_tri[i]->vertex( 0 )->pos()
                          << "-" << old_edge_in_adjacent[i]->vertex( 1 )->pos()
                          << "\n  edge 2:" << new_edge[i]->vertex( 0 )->pos()
                          << "-" << new_edge[i]->vertex( 1 )->pos()
                          << "\n  edge 3:" << new_edge_in_adjacent->vertex( 0 )->pos()
                          << "-" << new_edge_in_adjacent->vertex( 1 )->pos()
                          << std::endl;
                return false;
            }
        }
    }

    ////////////////////////////////////////////////////////////////

    // remove old triangles & edge from memory map
    removeTriangle( tri );
    if ( exist_adjacent )
    {
        removeTriangle( adjacent );
    }
    removeEdge( online_edge );

    ////////////////////////////////////////////////////////////////

    // legalize new triangles
    //std::cout << "updateOnlineVertex() legalize normal start " << std::endl;
    for ( std::size_t i = 0; i < 2; ++i )
    {
        //std::cout << "updateOnlineVertex() legalize normal i=" << i << std::endl;
        if ( ! legalizeEdge( new_tri_in_tri[i],
                             new_vertex,
                             old_edge_in_tri[i] ) )
        {
            return false;
        }
    }
    //std::cout << "updateOnlineVertex() legalize normal end " << std::endl;

    if ( exist_adjacent )
    {
        //std::cout << "updateOnlineVertex() legalize adjacent start " << std::endl;
        for ( std::size_t i = 0; i < 2; ++i )
        {
            //std::cout << "updateOnlineVertex() legalize adjacent i=" << i << std::endl;
            if ( ! legalizeEdge( new_tri_in_adjacent[i],
                                 new_vertex,
                                 old_edge_in_adjacent[i] ) )
            {
                return false;
            }
        }
        //std::cout << "updateOnlineVertex() legalize adjacent end " << std::endl;
    }

    //std::cout << "updateOnlineVertex() end " << std::endl;
    return true;
}

/*-------------------------------------------------------------------*/
/*!

*/
bool
DelaunayTriangulation::legalizeEdge( TrianglePtr new_tri,
                                     const Vertex * new_vertex,
                                     EdgePtr shared_edge )
{
    //std::cout << "legalizeEdge() start " << std::endl;
    if ( ! new_tri )
    {
        return false;
    }

    TrianglePtr adjacent = ( shared_edge->triangle( 0 ) == new_tri
                             ? shared_edge->triangle( 1 )
                             : shared_edge->triangle( 0 ) );
    if ( ! adjacent )
    {
        // no adjacent triangle
#ifdef DEBUG2
        std::cout << "legalizeEdge() no adjacent\n"
                  << "   added triangle id " << new_tri->id()
                  << new_tri->vertex( 0 )->pos()
                  << new_tri->vertex( 1 )->pos()
                  << new_tri->vertex( 2 )->pos()
                  << std::endl;
#endif
        return true;
    }

    if ( ! adjacent->contains( new_vertex->pos() ) )
    {
        // legal triangle
#ifdef DEBUG2
        std::cout << "legalizeEdge() this is a legal triangle\n"
                  << "   added triangle id " << new_tri->id()
                  << new_tri->vertex( 0 )->pos()
                  << new_tri->vertex( 1 )->pos()
                  << new_tri->vertex( 2 )->pos()
                  << std::endl;
#endif
        return true;
    }

    ////////////////////////////////////////////////////////////////
    // detect illegal triangle
    // shared_edge must be flipped.

    //std::cout << "legalizeEdge() flip adjacent tri = "
    //          << adjacent->id()
    //          << std::endl;

    const Vertex * adjacent_vertex = adjacent->getVertexExclude( shared_edge );

    // find no changed edges from two triangles
    EdgePtr edge_in_new_tri[2];
    EdgePtr edge_in_adjacent[2];
    {
        std::size_t idx = 0;
        for ( std::size_t i = 0; i < 3; ++i )
        {
            if ( new_tri->edge( i ) != shared_edge )
            {
                edge_in_new_tri[idx] = new_tri->edge( i );
                // remove triangle reference
                edge_in_new_tri[idx]->removeTriangle( new_tri );
                ++idx;
            }
        }

        idx = 0;
        for ( std::size_t i = 0; i < 3; ++i )
        {
            if ( adjacent->edge( i ) != shared_edge )
            {
                edge_in_adjacent[idx] = adjacent->edge( i );
                // remove triangle reference
                edge_in_adjacent[idx]->removeTriangle( adjacent );
                ++idx;
            }
        }

        // adjast edge index. paired edge must have same vertex.
        if ( edge_in_adjacent[1]->hasVertex( edge_in_new_tri[0]->vertex( 0 ) )
             || edge_in_adjacent[1]->hasVertex( edge_in_new_tri[0]->vertex( 1 ) ) )
        {
            std::swap( edge_in_adjacent[0], edge_in_adjacent[1] );
        }
    }

    ////////////////////////////////////////////////////////////////
    // create new edge for flip

    EdgePtr new_edge = createEdge( new_vertex, adjacent_vertex );

    // create new triangle
    TrianglePtr flipped_tri[2];
    for ( std::size_t i = 0; i < 2; ++i )
    {
        flipped_tri[i] = createTriangle( new_edge,
                                         edge_in_new_tri[i],
                                         edge_in_adjacent[i] );
        if ( ! flipped_tri[i]->circumcenter().isValid() )
        {
            std::cerr << __FILE__ << ':' << __LINE__
                      << " legalizeEdge() detect illegal vertex \n"
                      << flipped_tri[i] << '\n'
                      << new_edge->vertex( 0 )->pos()
                      << new_edge->vertex( 1 )->pos() << " : new edge\n"
                      << edge_in_new_tri[i]->vertex( 0 )->pos()
                      << edge_in_new_tri[i]->vertex( 1 )->pos() << " : edge_in_new_tri\n"
                      << edge_in_adjacent[i]->vertex( 0 )->pos()
                      << edge_in_adjacent[i]->vertex( 1 )->pos() << " : edge_in_adjacent\n"
                      << std::flush;
            return false;
        }
    }


    // remove old triangles and old shared edge
    removeTriangle( new_tri );
    removeTriangle( adjacent );
    removeEdge( shared_edge );

    for ( std::size_t i = 0; i < 2; ++i )
    {
        // new shared edge is one of old adjacent edge.
        if ( ! legalizeEdge( flipped_tri[i],
                             new_vertex,
                             edge_in_adjacent[i] ) )
        {
            return false;
        }
    }

    //std::cout << "legalizeEdge() end " << std::endl;
    return true;
}

/*-------------------------------------------------------------------*/
/*!

*/
DelaunayTriangulation::ContainedType
DelaunayTriangulation::findTriangleContains( const Vector2D & pos,
                                             TrianglePtr * sol ) const
{
    for ( TriangleCont::const_iterator it = M_triangles.begin(), end = M_triangles.end();
          it != end;
          ++it )
    {
        const TrianglePtr tri = it->second;

        if ( std::fabs( tri->circumcenter().x - pos.x )
             > tri->circumradius()
             || std::fabs( tri->circumcenter().y - pos.y )
             > tri->circumradius() )
        {
            // out of circumcircle
            continue;
        }

        Vector2D rel0( tri->vertex( 0 )->pos() - pos );
        Vector2D rel1( tri->vertex( 1 )->pos() - pos );
        Vector2D rel2( tri->vertex( 2 )->pos() - pos );

        double outer0 = rel0.outerProduct( rel1 );
        double outer1 = rel1.outerProduct( rel2 );
        double outer2 = rel2.outerProduct( rel0 );

        //std::cout << "findTriangleContains() found online\n"
        //          << " tri0 = " << tri->vertex( 0 )->pos()
        //          << " tri0 = " << tri->vertex( 1 )->pos()
        //          << " tri0 = " << tri->vertex( 2 )->pos()
        //          << "\n  outer0 = " << outer0
        //          << "  outer1 = " << outer1
        //          << "  outer2 = " << outer2
        //          << std::endl;

        if ( std::fabs( outer0 ) <= EPSILON )
        {
            if ( rel0.x * rel1.x > EPSILON
                 || rel0.y * rel1.y > EPSILON )
            {
                // not online
                continue;
            }
            //std::cout << "findTriangleContains() found online on 0-1 " << std::endl;
            *sol = tri;
            return ONLINE;
        }

        if ( std::fabs( outer1 ) <= EPSILON )
        {
            if ( rel1.x * rel2.x > EPSILON
                 || rel1.y * rel2.y > EPSILON )
            {
                // not online
                continue;
            }
            //std::cout << "findTriangleContains() found online on 1-2 " << std::endl;
            *sol = tri;
            return ONLINE;
        }

        if ( std::fabs( outer2 ) <= EPSILON )
        {
            if ( rel2.x * rel0.x > EPSILON
                 || rel2.y * rel0.y > EPSILON )
            {
                // not online
                continue;
            }
            //std::cout << "findTriangleContains() found online on 2-0 " << std::endl;
            *sol = tri;
            return ONLINE;
        }

        if ( ( outer0 >= 0.0 && outer1 >= 0.0 && outer2 >= 0.0 )
             || ( outer0 <= 0.0 && outer1 <= 0.0 && outer2 <= 0.0 ) )
        {
#ifdef DEBUG
            std::cout << __FILE__ << ':' << __LINE__
                      << " findTriangleContains() found contained "
                      << " pos" << pos
                      << " triangle"
                      << tri->vertex( 0 )->pos()
                      << tri->vertex( 1 )->pos()
                      << tri->vertex( 2 )->pos()
                      << std::endl;
#endif
            *sol = tri;
            return CONTAINED;
        }
    }

    //std::cout << "findTriangleContains() end not found " << std::endl;
    return NOT_CONTAINED;
}

}
