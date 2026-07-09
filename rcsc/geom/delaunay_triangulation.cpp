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

#include <algorithm>
#include <limits>
#include <utility>

namespace rcsc {

const double DelaunayTriangulation::EPSILON = 1.0e-10;

namespace {

static const std::pair< std::size_t, std::size_t >
edge_pairs[3] = { std::pair< std::size_t, std::size_t >( 0, 1 ),
                  std::pair< std::size_t, std::size_t >( 1, 2 ),
                  std::pair< std::size_t, std::size_t >( 2, 0 ),
};

/*!
  \brief check how 'pos' relates to a single triangle. shared by
  DelaunayTriangulation::exhaustiveFindTriangleContains() (which calls this
  once per triangle) and DelaunayTriangulation::quadTreeFindTriangleContains()
  (which calls this once per candidate returned by the spatial index).
 */
DelaunayTriangulation::ContainedType
test_triangle_contains( const DelaunayTriangulation::TrianglePtr tri,
                        const Vector2D & pos )
{
    if ( std::fabs( tri->circumcenter().x - pos.x )
         > tri->circumradius()
         || std::fabs( tri->circumcenter().y - pos.y )
         > tri->circumradius() )
    {
        // out of circumcircle
        return DelaunayTriangulation::NOT_CONTAINED;
    }

    Vector2D rel0( tri->vertex( 0 )->pos() - pos );
    Vector2D rel1( tri->vertex( 1 )->pos() - pos );
    Vector2D rel2( tri->vertex( 2 )->pos() - pos );

    double outer0 = rel0.outerProduct( rel1 );
    double outer1 = rel1.outerProduct( rel2 );
    double outer2 = rel2.outerProduct( rel0 );

    if ( std::fabs( outer0 ) <= DelaunayTriangulation::EPSILON )
    {
        if ( rel0.x * rel1.x > DelaunayTriangulation::EPSILON
             || rel0.y * rel1.y > DelaunayTriangulation::EPSILON )
        {
            // not online
            return DelaunayTriangulation::NOT_CONTAINED;
        }
        return DelaunayTriangulation::ONLINE;
    }

    if ( std::fabs( outer1 ) <= DelaunayTriangulation::EPSILON )
    {
        if ( rel1.x * rel2.x > DelaunayTriangulation::EPSILON
             || rel1.y * rel2.y > DelaunayTriangulation::EPSILON )
        {
            // not online
            return DelaunayTriangulation::NOT_CONTAINED;
        }
        return DelaunayTriangulation::ONLINE;
    }

    if ( std::fabs( outer2 ) <= DelaunayTriangulation::EPSILON )
    {
        if ( rel2.x * rel0.x > DelaunayTriangulation::EPSILON
             || rel2.y * rel0.y > DelaunayTriangulation::EPSILON )
        {
            // not online
            return DelaunayTriangulation::NOT_CONTAINED;
        }
        return DelaunayTriangulation::ONLINE;
    }

    if ( ( outer0 >= 0.0 && outer1 >= 0.0 && outer2 >= 0.0 )
         || ( outer0 <= 0.0 && outer1 <= 0.0 && outer2 <= 0.0 ) )
    {
        return DelaunayTriangulation::CONTAINED;
    }

    return DelaunayTriangulation::NOT_CONTAINED;
}

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
    M_hint_triangle_id = -1;

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

    clearQuadTree();
    M_quad_tree_dirty = true;
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

    if ( M_search_method == SearchMethod::QUAD_TREE )
    {
        const ContainedType result = quadTreeFindTriangleContains( pos, &tri );
        if ( result == CONTAINED || result == ONLINE )
        {
            return tri;
        }
        // the index could not conclusively resolve pos (e.g. pos outside
        // the triangulated region): fall back to the default search below.
        tri = nullptr;
    }

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
  Point location is the hot path of compute() (called once per inserted
  vertex while the triangle set is being mutated) and of repeated external
  queries (e.g. formation interpolation every cycle). Rather than maintain a
  separate spatial index that would need to be kept in sync with every
  triangle creation/removal, this walks the triangle adjacency graph that
  the triangulation already maintains via Edge::triangle(0/1), starting from
  the triangle found by the previous call. This is the standard
  "visibility/orientation walk" technique for point location in a Delaunay
  triangulation and is expected O(sqrt(N)) instead of O(N) per query.
  Whenever the walk cannot conclusively resolve pos (it reached the
  boundary of the mesh, hit the step budget, or hit a degenerate direction)
  exhaustiveFindTriangleContains() is used instead, so the result is always
  identical to what the plain exhaustive search would have returned.
*/
DelaunayTriangulation::ContainedType
DelaunayTriangulation::findTriangleContains( const Vector2D & pos,
                                             TrianglePtr * sol ) const
{
    if ( M_triangles.empty() )
    {
        return NOT_CONTAINED;
    }

    TriangleCont::const_iterator hint = M_triangles.find( M_hint_triangle_id );
    if ( hint != M_triangles.end() )
    {
        TrianglePtr start = hint->second;
        ContainedType result = walkTriangleContains( pos, start, sol );
        if ( result == CONTAINED || result == ONLINE )
        {
            M_hint_triangle_id = (*sol)->id();
            return result;
        }
    }

    ContainedType result = exhaustiveFindTriangleContains( pos, sol );
    if ( result == CONTAINED || result == ONLINE )
    {
        M_hint_triangle_id = (*sol)->id();
    }
    return result;
}

/*-------------------------------------------------------------------*/
/*!

*/
DelaunayTriangulation::ContainedType
DelaunayTriangulation::walkTriangleContains( const Vector2D & pos,
                                             TrianglePtr start,
                                             TrianglePtr * sol ) const
{
    TrianglePtr tri = start;

    // generous but bounded step budget: a converging walk on a Delaunay
    // triangulation takes O(sqrt(N)) steps on average, so hitting this
    // means the walk is not converging (degenerate input, numerical
    // corner case) and the exhaustive fallback should take over.
    const std::size_t max_steps = M_triangles.size() + 8;

    for ( std::size_t step = 0; tri && step < max_steps; ++step )
    {
        const Vector2D rel0( tri->vertex( 0 )->pos() - pos );
        const Vector2D rel1( tri->vertex( 1 )->pos() - pos );
        const Vector2D rel2( tri->vertex( 2 )->pos() - pos );

        const double outer0 = rel0.outerProduct( rel1 );
        const double outer1 = rel1.outerProduct( rel2 );
        const double outer2 = rel2.outerProduct( rel0 );

        // edge index whose opposite triangle should be visited next if pos
        // turns out not to be online/contained in 'tri'. 3 == "unknown".
        std::size_t cross_edge = 3;

        if ( std::fabs( outer0 ) <= EPSILON )
        {
            if ( ! ( rel0.x * rel1.x > EPSILON || rel0.y * rel1.y > EPSILON ) )
            {
                *sol = tri;
                return ONLINE;
            }
        }
        else if ( std::fabs( outer1 ) <= EPSILON )
        {
            if ( ! ( rel1.x * rel2.x > EPSILON || rel1.y * rel2.y > EPSILON ) )
            {
                *sol = tri;
                return ONLINE;
            }
        }
        else if ( std::fabs( outer2 ) <= EPSILON )
        {
            if ( ! ( rel2.x * rel0.x > EPSILON || rel2.y * rel0.y > EPSILON ) )
            {
                *sol = tri;
                return ONLINE;
            }
        }
        else if ( ( outer0 >= 0.0 && outer1 >= 0.0 && outer2 >= 0.0 )
                  || ( outer0 <= 0.0 && outer1 <= 0.0 && outer2 <= 0.0 ) )
        {
            *sol = tri;
            return CONTAINED;
        }
        else
        {
            // pos is outside 'tri'. vertices 0,1,2 are consistently wound
            // (all CW or all CCW), so the sign of the triangle's own
            // (unsigned by pos) signed area tells us, for each edge, which
            // side is "inside" -- an outer_i disagreeing with that sign
            // means pos is beyond edge i, so cross into its neighbor.
            const double signed_area
                = ( tri->vertex( 1 )->pos() - tri->vertex( 0 )->pos() )
                .outerProduct( tri->vertex( 2 )->pos() - tri->vertex( 0 )->pos() );

            if ( signed_area >= 0.0 )
            {
                if ( outer0 < -EPSILON ) cross_edge = 0;
                else if ( outer1 < -EPSILON ) cross_edge = 1;
                else if ( outer2 < -EPSILON ) cross_edge = 2;
            }
            else
            {
                if ( outer0 > EPSILON ) cross_edge = 0;
                else if ( outer1 > EPSILON ) cross_edge = 1;
                else if ( outer2 > EPSILON ) cross_edge = 2;
            }
        }

        if ( cross_edge == 3 )
        {
            // could not determine a walk direction (should not normally
            // happen); let the caller fall back to the exhaustive search.
            return NOT_CONTAINED;
        }

        const Vertex * va = tri->vertex( cross_edge );
        const Vertex * vb = tri->vertex( ( cross_edge + 1 ) % 3 );
        Edge * e = tri->getEdgeInclude( va, vb );

        // if the neighbor is null, 'e' is on the boundary of the
        // triangulated region and pos is outside it in this direction; the
        // loop condition (tri == nullptr) ends the walk on the next check.
        tri = ( e->triangle( 0 ) == tri ? e->triangle( 1 ) : e->triangle( 0 ) );
    }

    return NOT_CONTAINED;
}

/*-------------------------------------------------------------------*/
/*!

*/
DelaunayTriangulation::ContainedType
DelaunayTriangulation::exhaustiveFindTriangleContains( const Vector2D & pos,
                                                       TrianglePtr * sol ) const
{
    for ( TriangleCont::const_iterator it = M_triangles.begin(), end = M_triangles.end();
          it != end;
          ++it )
    {
        const TrianglePtr tri = it->second;
        const ContainedType result = test_triangle_contains( tri, pos );
        if ( result != NOT_CONTAINED )
        {
            *sol = tri;
            return result;
        }
    }

    return NOT_CONTAINED;
}

/*-------------------------------------------------------------------*/
/*!
  Nested type backing SearchMethod::QUAD_TREE. Defined here (rather than in
  the header) since it is a pure implementation detail: a static index built
  once (lazily, on first use after the triangle set changes) over whichever
  triangles exist at that time, then queried read-only. This is a good
  trade-off only when the triangulation is computed once and then queried
  many times without further mutation -- compute() itself never uses it, so
  there is no cost paid for the frequent create/remove churn of incremental
  construction.
*/
class DelaunayTriangulation::QuadTreeNode {
public:
    static constexpr int MAX_DEPTH = 12;
    static constexpr std::size_t LEAF_CAPACITY = 8;

private:
    Rect2D M_bounds;
    bool M_leaf;
    std::array< QuadTreeNode *, 4 > M_children;
    std::vector< TrianglePtr > M_triangles;

    QuadTreeNode( const QuadTreeNode & ) = delete;
    QuadTreeNode & operator=( const QuadTreeNode & ) = delete;

public:
    explicit
    QuadTreeNode( const Rect2D & bounds )
        : M_bounds( bounds ),
          M_leaf( true )
      {
          M_children.fill( nullptr );
      }

    ~QuadTreeNode()
      {
          for ( QuadTreeNode * c : M_children )
          {
              delete c;
          }
      }

    /*!
      \brief build a (sub)tree covering 'bounds' that indexes 'items'
      (triangle + its axis-aligned bounding box).
     */
    static
    QuadTreeNode * build( const Rect2D & bounds,
                         const std::vector< std::pair< TrianglePtr, Rect2D > > & items,
                         const int depth )
      {
          QuadTreeNode * node = new QuadTreeNode( bounds );

          if ( items.size() <= LEAF_CAPACITY || depth >= MAX_DEPTH )
          {
              node->M_triangles.reserve( items.size() );
              for ( const auto & item : items )
              {
                  node->M_triangles.push_back( item.first );
              }
              return node;
          }

          const Vector2D c = bounds.center();
          const Rect2D quad_bounds[4] = {
              Rect2D( bounds.topLeft(), c ),                                       // x<=c.x, y<=c.y
              Rect2D( Vector2D( c.x, bounds.top() ), Vector2D( bounds.right(), c.y ) ), // x>c.x, y<=c.y
              Rect2D( Vector2D( bounds.left(), c.y ), Vector2D( c.x, bounds.bottom() ) ), // x<=c.x, y>c.y
              Rect2D( c, bounds.bottomRight() ),                                   // x>c.x, y>c.y
          };

          std::array< std::vector< std::pair< TrianglePtr, Rect2D > >, 4 > buckets;
          for ( const auto & item : items )
          {
              for ( int q = 0; q < 4; ++q )
              {
                  if ( quad_bounds[q].left() <= item.second.right()
                       && item.second.left() <= quad_bounds[q].right()
                       && quad_bounds[q].top() <= item.second.bottom()
                       && item.second.top() <= quad_bounds[q].bottom() )
                  {
                      buckets[q].push_back( item );
                  }
              }
          }

          bool shrank = false;
          for ( int q = 0; q < 4; ++q )
          {
              if ( buckets[q].size() < items.size() )
              {
                  shrank = true;
                  break;
              }
          }

          if ( ! shrank )
          {
              // subdividing did not shrink the candidate set in any
              // quadrant (e.g. every remaining triangle straddles the
              // center): stop here instead of recursing without end.
              node->M_triangles.reserve( items.size() );
              for ( const auto & item : items )
              {
                  node->M_triangles.push_back( item.first );
              }
              return node;
          }

          node->M_leaf = false;
          for ( int q = 0; q < 4; ++q )
          {
              if ( ! buckets[q].empty() )
              {
                  node->M_children[q] = build( quad_bounds[q], buckets[q], depth + 1 );
              }
          }
          return node;
      }

    /*!
      \brief append every triangle indexed by the leaf cell that contains
      'pos' to 'candidates'. 'candidates' is a superset of triangles that
      might contain 'pos'; the caller must still verify each one.
     */
    void collectCandidates( const Vector2D & pos,
                            std::vector< TrianglePtr > & candidates ) const
      {
          if ( M_leaf )
          {
              candidates.insert( candidates.end(), M_triangles.begin(), M_triangles.end() );
              return;
          }

          const Vector2D c = M_bounds.center();
          const int q = ( pos.x > c.x ? 1 : 0 ) + ( pos.y > c.y ? 2 : 0 );
          if ( M_children[q] )
          {
              M_children[q]->collectCandidates( pos, candidates );
          }
      }
};

/*-------------------------------------------------------------------*/
/*!

*/
void
DelaunayTriangulation::buildQuadTree() const
{
    clearQuadTree();
    M_quad_tree_dirty = false;

    if ( M_triangles.empty() )
    {
        return;
    }

    double min_x = std::numeric_limits< double >::max();
    double max_x = std::numeric_limits< double >::lowest();
    double min_y = std::numeric_limits< double >::max();
    double max_y = std::numeric_limits< double >::lowest();

    std::vector< std::pair< TrianglePtr, Rect2D > > items;
    items.reserve( M_triangles.size() );

    for ( TriangleCont::const_iterator it = M_triangles.begin(), end = M_triangles.end();
          it != end;
          ++it )
    {
        const TrianglePtr tri = it->second;

        double tminx = tri->vertex( 0 )->pos().x;
        double tmaxx = tminx;
        double tminy = tri->vertex( 0 )->pos().y;
        double tmaxy = tminy;
        for ( std::size_t i = 1; i < 3; ++i )
        {
            const Vector2D & p = tri->vertex( i )->pos();
            tminx = std::min( tminx, p.x );
            tmaxx = std::max( tmaxx, p.x );
            tminy = std::min( tminy, p.y );
            tmaxy = std::max( tmaxy, p.y );
        }

        min_x = std::min( min_x, tminx );
        max_x = std::max( max_x, tmaxx );
        min_y = std::min( min_y, tminy );
        max_y = std::max( max_y, tmaxy );

        items.emplace_back( tri, Rect2D( Vector2D( tminx, tminy ), Vector2D( tmaxx, tmaxy ) ) );
    }

    // small padding so a query point exactly on the outer boundary still
    // falls strictly inside the root cell.
    const double pad = 1.0;
    const Rect2D root_bounds( Vector2D( min_x - pad, min_y - pad ),
                              Vector2D( max_x + pad, max_y + pad ) );

    M_quad_tree_root = QuadTreeNode::build( root_bounds, items, 0 );
}

/*-------------------------------------------------------------------*/
/*!

*/
void
DelaunayTriangulation::clearQuadTree() const
{
    delete M_quad_tree_root;
    M_quad_tree_root = nullptr;
}

/*-------------------------------------------------------------------*/
/*!

*/
DelaunayTriangulation::ContainedType
DelaunayTriangulation::quadTreeFindTriangleContains( const Vector2D & pos,
                                                     TrianglePtr * sol ) const
{
    if ( M_quad_tree_dirty || ! M_quad_tree_root )
    {
        buildQuadTree();
    }

    if ( ! M_quad_tree_root )
    {
        return NOT_CONTAINED;
    }

    M_quad_tree_candidates.clear();
    M_quad_tree_root->collectCandidates( pos, M_quad_tree_candidates );

    for ( TrianglePtr tri : M_quad_tree_candidates )
    {
        const ContainedType result = test_triangle_contains( tri, pos );
        if ( result != NOT_CONTAINED )
        {
            *sol = tri;
            return result;
        }
    }

    return NOT_CONTAINED;
}

}
