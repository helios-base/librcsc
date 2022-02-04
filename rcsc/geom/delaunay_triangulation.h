// -*-c++-*-

/*!
  \file delaunay_triangulation.h
  \brief Delaunay Triangulation class Header File.
*/

/*
 *Copyright:

 Copyright (C) Hidehisa AKIYAMA

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

#ifndef RCSC_GEOM_DELAUNAY_TRIANGULATION_H
#define RCSC_GEOM_DELAUNAY_TRIANGULATION_H

#include <rcsc/geom/rect_2d.h>
#include <rcsc/geom/vector_2d.h>

#include <algorithm>
#include <unordered_map>
#include <vector>
#include <array>

namespace rcsc {

/*!
  \class DelaunayTriangulation
  \brief Delaunay triangulation
*/
class DelaunayTriangulation {
public:

    static const double EPSILON; //!< tolerance threshold

    ////////////////////////////////////////////////////////////////
    /*!
      \enum ContainedType
      \brief containment type in triangles
     */
    enum ContainedType {
        NOT_CONTAINED,
        CONTAINED,
        ONLINE,
        SAME_VERTEX,
    };

    ////////////////////////////////////////////////////////////////
    /*!
      \brief triangle's vertex data.
      This is handled as kernel point for the Voronoi diagram..
     */
    class Vertex {
    private:
        int M_id; //!< Id number of this vertex
        Vector2D M_pos; //!< coordinate of kernel point

    public:
        /*!
          \brief create vertex with Id number 0
         */
        Vertex()
            : M_id( 0 )
          { }

        /*!
          \brief create vertex with Id
          \param id Id number
         */
        explicit
        Vertex( const int id )
            : M_id( id )
          { }

        /*!
          \brief nothing to do
         */
        virtual
        ~Vertex()
          { }

        /*!
          \brief create vertex with Id & coordinates
          \param id Id number
          \param p coordinates of kernel point
         */
        Vertex( const int id,
                const Vector2D & p )
            : M_id( id )
            , M_pos( p )
          { }

        /*!
          \brief create vertex with Id & coordinates
          \param id Id number
          \param x x-coordinates of kernel point
          \param y y-coordinates of kernel point
         */
        Vertex( const int id,
                const double x,
                const double y )
            : M_id( id )
            , M_pos( x, y )
          { }

        /*!
          \brief assign data
          \param id Id number
          \param p coordinates of kernel point
         */
        Vertex & assign( const int id,
                         const Vector2D & p )
          {
              M_id = id;
              M_pos = p;
              return *this;
          }

        /*!
          \brief assign data
          \param id Id number
          \param x x-coordinates of kernel point
          \param y y-coordinates of kernel point
         */
        Vertex & assign( const int id,
                         const double x,
                         const double y )
          {
              M_id = id;
              M_pos.assign( x, y );
              return *this;
          }

        /*!
          \brief get the Id of this vertex
          \return Id number
         */
        int id() const
          {
              return M_id;
          }

        /*!
          \brief get the coordinates of the kernel point
          \return const reference to the vector object.
         */
        const
        Vector2D & pos() const
          {
              return M_pos;
          }
    };

    ////////////////////////////////////////////////////////////////

    class Edge;
    class Triangle;

    typedef Edge* EdgePtr; //!< alias of Edge pointer
    typedef Triangle* TrianglePtr; //!< alias of Triangle pointer

    ////////////////////////////////////////////////////////////////
    /*!
      \brief triangle's edge data.
     */
    class Edge {
    private:
        const int M_id; //!< Id number of this edge
        const Vertex * M_vertices[2]; //!< reference to the vertex of this edge
        TrianglePtr M_triangles[2]; //!< triangles whitch this edge belongs to
    public:

        /*!
          \brief create edge with two vertices. vertices must not be NULL.
          \param id Id number of this edge.
          \param v0 raw pointer to the first vertex
          \param v1 raw pointer to the second vertex
         */
        Edge( const int id,
              const Vertex * v0,
              const Vertex * v1 )
            : M_id( id )
          {
              //std::cout << "Edge() id_" << id << " v0 " << v0 << " v1 " << v1
              //          << std::endl;
              M_vertices[0] = v0;
              M_vertices[1] = v1;
              std::fill_n( M_triangles, 2, nullptr );
          }

        /*!
          \brief nothing to do
         */
        ~Edge()
          { }

        /*!
          \brief remove pointer to the triangle that this edge belongs to.
          This edge is NOT removed.
          \param tri pointer to the target triangle
         */
        void removeTriangle( TrianglePtr tri )
          {
              if ( M_triangles[0] == tri )
              {
                  //std::cout << "Edge::removeTriangle() edge_id_" << M_id
                  //          << " remove tri_0 " << tri->id() << " "
                  //          << tri << std::endl;
                  M_triangles[0] = nullptr;
              }
              if ( M_triangles[1] == tri )
              {
                  //std::cout << "Edge::removeTriangle() edge_id_" << M_id
                  //          << " remove tri_1 " << tri->id() << " "
                  //          << tri << std::endl;
                  M_triangles[1] = nullptr;
              }
          }

        /*!
          \brief set the triangle that this edge belongs to.
          \param tri raw pointer to the triangle.

          It is not checked whether this edge belongs to that triangle.
          If the target triangle is already set or two triangle is already set,
          this function has no effect.
          So, this method should be called from Triangle's constructor.
         */
        void setTriangle( TrianglePtr tri )
          {
              if ( M_triangles[0] == tri ) return;
              if ( M_triangles[1] == tri ) return;

              if ( ! M_triangles[0] )
              {
                  //std::cout << "Edge::setTriangle() edge_id_" << M_id
                  //          << " set tri_0 " << tri->id() << " "
                  //          << tri << std::endl;
                  M_triangles[0] = tri;
              }
              else if ( ! M_triangles[1] )
              {
                  //std::cout << "Edge::setTriangle() edge_id_" << M_id
                  //          << " set tri_1 " << tri->id() << " "
                  //          << tri << std::endl;
                  M_triangles[1] = tri;
              }
          }

        /*!
          \brief get Id number of this edge
          \return Id number
         */
        int id() const
          {
              return M_id;
          }

        /*!
          \brief get the raw pointer to the vertex that this edge has
          \param i specifies array index
          \return const pointer to the vertex
         */
        const
        Vertex * vertex( const std::size_t i ) const
          {
              return M_vertices[i];
          }

        /*!
          \brief get the raw pointer to the triangle that this edge belongs to
          \param i specifies array index
          \return pointer to the triangle
         */
        Triangle * triangle( const std::size_t i ) const
          {
              return M_triangles[i];
          }

        /*!
          \brief check if this edge has the specified vertex or not.
          \param v raw pointer to the vertex
          \return true if this edge has the specified vertex.
         */
        bool hasVertex( const Vertex * v ) const
          {
              return ( M_vertices[0] == v
                       || M_vertices[1] == v );
          }

    };

    ////////////////////////////////////////////////////////////////
    /*!
      \brief triangle data
     */
    class Triangle {
    private:
        int M_id; //!< Id number of this triangle

        //! vertices of this triangle, but these are pointers to the vertex instance
        std::array< const Vertex *, 3 > M_vertices;
        //! edges of this triangle, but these are pointers to the vertex instance
        std::array< EdgePtr, 3 > M_edges;

        Vector2D M_circumcenter; //!< coordinates of the circumcenter.
        double M_circumradius; //!< radius of the circumcircle.

        Vector2D M_voronoi_vertex; //!< candidate of the voronoi vertex

        // not used
        Triangle() = delete;
    public:

        /*!
          \brief create triangle with index and edges
          \param id Id number of this triangle
          \param e0 raw pointer to the first edge instance
          \param e1 raw pointer to the second edge instance
          \param e2 raw pointer to the third edge instance

          pointers to the vertices are automatically set from edges.
         */
        Triangle( const int id,
                  EdgePtr e0,
                  EdgePtr e1,
                  EdgePtr e2 );

        /*!
          \brief remove this triangle from all edges.
         */
        ~Triangle()
          {
              M_edges[0]->removeTriangle( this );
              M_edges[1]->removeTriangle( this );
              M_edges[2]->removeTriangle( this );
          }

        /*!
          \brief update the voronoi vertex point (intersection of perpendicular bisectors)
         */
        void updateVoronoiVertex();

        /*!
          \brief get the Id of this triangle
          \return Id number
         */
        int id() const
          {
              return M_id;
          }

        /*!
          \brief get the raw pointer to the vertex that this triangle has
          \param i specifies array index
          \return const pointer to the vertex
         */
        const
        Vertex * vertex( std::size_t i ) const
          {
              return M_vertices[i];
          }

        /*!
          \brief get the raw pointer to the edge that this triangle has
          \param i specifies array index
          \return raw pointer to the edge
         */
        Edge * edge( std::size_t i ) const
          {
              return M_edges[i];
          }

        /*!
          \brief get the circumcenter point of this triangle
          \return coordinates of the circumcenter
         */
        const
        Vector2D & circumcenter() const
          {
              return M_circumcenter;
          }

        /*!
          \brief get the radius of the circumcircle of this triangle
          \return radius value
         */
        double circumradius() const
          {
              return M_circumradius;
          }

        /*!
          \brief get the voronoi vertex point
          \return coordinate of the voronoi vertex point. if illegal data, invalid vector is returned.
         */
        const Vector2D & voronoiVertex() const
          {
              return M_voronoi_vertex;
          }

        /*!
          \brief check if *circumcircle* contains the specified point
          \param pos target point
          \return true if target point is contained
         */
        bool contains( const Vector2D & pos ) const
          {
              return pos.dist2( M_circumcenter ) < M_circumradius * M_circumradius;
          }

        /*!
          \brief check if this triangle has the specified vertex.
          \param v raw pointer to the vertex.
          \return true if vertex is contained.
         */
        bool hasVertex( const Vertex * v ) const
          {
              return ( v == M_vertices[0]
                       || v == M_vertices[1]
                       || v == M_vertices[2] );
          }

        /*!
          \brief check if this triangle has the specified edge.
          \param e raw pointer to the edge.
          \return true if edge is contained.
         */
        bool hasEdge( const EdgePtr e ) const
          {
              return ( M_edges[0] == e
                       || M_edges[1] == e
                       || M_edges[2] == e );
          }

        /*!
          \brief get the pointer to the vertex that is different from the specified vertices.
          \param v1 first vertex
          \param v2 second vertex
          \return if exist, const pointer to the vertex. else NULL is returned.
         */
        const
        Vertex * getVertexExclude( const Vertex * v1,
                                   const Vertex * v2 ) const
          {
              for ( std::size_t i = 0; i < 3; ++i )
              {
                  if ( M_vertices[i] != v1
                       && M_vertices[i] != v2 )
                  {
                      return M_vertices[i];
                  }
              }
              return nullptr;
          }

        /*!
          \brief get the pointer to the vertex that does not belong to the specified edge.
          \param edge target edge
          \return if exist, const pointer to the vertex, else NULL is returned.
         */
        const
        Vertex * getVertexExclude( const Edge * edge ) const
          {
              return getVertexExclude( edge->vertex( 0 ),
                                       edge->vertex( 1 ) );
          }

        /*!
          \brief get the pointer to the edge that has the specified vertices.
          \param v1 first vertex
          \param v2 second vertex
          \return if exist, raw pointer to the edge, else NULL is returned.
         */
        Edge * getEdgeInclude( const Vertex * v1,
                               const Vertex * v2 ) const
          {
              for ( std::size_t i = 0; i < 3; ++i )
              {
                  if ( M_edges[i]->hasVertex( v1 )
                       && M_edges[i]->hasVertex( v2 ) )
                  {
                      return M_edges[i];
                  }
              }
              return nullptr;
          }

        /*!
          \brief get the pointer to the edge that does not have the specified vertex.
          \param v target vertex
          \return if exist, raw pointer to the edge, else NULL is returned.
         */
        Edge * getEdgeExclude( const Vertex * v ) const
          {
              for ( std::size_t i = 0; i < 3; ++i )
              {
                  if ( ! M_edges[i]->hasVertex( v ) )
                  {
                      return M_edges[i];
                  }
              }
              return nullptr;
          }

    };

    ////////////////////////////////////////////////////////////////

    typedef std::vector< Vertex > VertexCont; //!< vertex container type
    typedef std::unordered_map< int, EdgePtr > EdgeCont; //!< edge pointer container type
    typedef std::unordered_map< int, TrianglePtr > TriangleCont; //!< triangle pointer container type

private:

    //! counter to set Id to edges
    int M_edge_count;
    //! counter to set Id to triangles
    int M_tri_count;

    //! vertex instance of initial super triangle
    Vertex M_initial_vertex[3];

    //! edge reference of inital super triangle
    EdgePtr M_initial_edge[3];

    //! instance of vertices. these are refered by edge and triangle.
    VertexCont M_vertices;

    //! edge instance holder. key: id
    EdgeCont M_edges;

    //! triangle instance holder. key: id
    TriangleCont M_triangles;

    // not used
    DelaunayTriangulation & operator=( const DelaunayTriangulation & ) = delete;

public:

    /*!
      \brief nothing to do
    */
    DelaunayTriangulation() = default;

    /*!
      \brief construct with considerable rectangle region
      \param region considerable rectangle region.

      All verteices must be included in region.
    */
    explicit
    DelaunayTriangulation( const Rect2D & region )
      {
          //std::cout << "create with rect" << std::endl;
          createInitialTriangle( region );
      }

    /*!
      \brief destruct
     */
    ~DelaunayTriangulation();

    /*!
      \brief initialize with target field rectangle data.
      All data are cleared.
      Initial triangle is crated.
     */
    void init( const Rect2D & region )
      {
          clear();
          createInitialTriangle( region );
      }

    /*!
      \brief clear all vertices and all computed results.
     */
    void clear();

    /*!
      \brief clear all computed results
     */
    void clearResults();

    /*!
      \brief get vertices
      \return const reference to the vertices container
     */
    const
    VertexCont & vertices() const
      {
          return M_vertices;
      }

    /*!
      \brief get edge set
      \return const referenct to the map container. key=id, value=raw pointer
     */
    const
    EdgeCont & edges() const
      {
          return M_edges;
      }

    /*!
      \brief get triangle set
      \return const referenct to the map container. key=id, value=raw pointer
     */
    const
    TriangleCont & triangles() const
      {
          return M_triangles;
      }

    /*!
      \brief add new vertex
      \param x coordinate x
      \param y coordinate y
      \return assigned id value
     */
    int addVertex( const double x,
                   const double y );

    /*!
      \brief add new vertex
      \param p added point
      \return assigned id value
     */
    int addVertex( const Vector2D & p )
      {
          return addVertex( p.x, p.y );
      }

    /*!
      \brief set vertices.
      \param v container of vertices.
     */
    void addVertices( const std::vector< Vector2D > & v );

    /*!
      \brief get the const pointer to vertex specified by Id number.
      \param id wanted vertex Id number.
      \return const pointer to the vertex instance. if no vertex, NULL is returned.
     */
    const
    Vertex * getVertex( const int id ) const;

    /*!
      \brief compute the Delaunay Triangulation
    */
    void compute();

    /*!
      \brief calculate voronoi vertex point for each triangle
    */
    void updateVoronoiVertex();

    /*!
      \brief find triangle that contains pos from the computed triangle set.
      \param pos coordinates of the target point
      \return const pointer to the found triangle. if no triangle, NULL is returned.
     */
    const
    Triangle * findTriangleContains( const Vector2D & pos ) const;

    /*!
      \brief find the vertex nearest to the specified point
      \param pos coordinates of the target point
      \return const pointer to the found vertex, if no vertex, NULL is returned.
     */
    const
    Vertex * findNearestVertex( const Vector2D & pos ) const;

private:

    /*!
      \brief clear old triangles and create initial triangle that include region.
      \param region considerable region
    */
    void createInitialTriangle( const Rect2D & region );

    /*!
      \brief create region using the stored vertices.
    */
    void createInitialTriangle();

    /*!
      \brief remove initial vertices and edges connected to them.
     */
    void removeInitialVertices();

    /*!
      \brief update triangles by new vertex.
      \param vertex const pointer to the new vertex
      \param tri pointer to the triangle that contains vertex.
     */
    bool updateContainedVertex( const Vertex * vertex,
                                const TrianglePtr tri );

    /*!
      \brief update triangles by new vertex.
      \param vertex const pointer to the new vertex
      \param tri pointer to the triangle that vertex is online.
      \return the status of vertex insertion result
     */
    bool updateOnlineVertex( const Vertex * vertex,
                             const TrianglePtr tri );

    /*!
      \brief check if new triangle satisfies delaunay triangle condition.
      this function is used recursively.
      \param new_tri new triangle added by new vertex
      \param new_vertex new added vertex
      \param old_edge edge that exists before vertex is added.
      \return operation result. if error occurs, false is returned.
    */
    bool legalizeEdge( TrianglePtr new_tri,
                       const Vertex * new_vertex,
                       EdgePtr old_edge );

    /*!
      \brief find triangle that contains pos from the computed triangle set.
      \param pos coordinates of the target point
      \param sol pointer to the solution variable.
      \return how the vertex is contained.
     */
    ContainedType findTriangleContains( const Vector2D & pos,
                                        TrianglePtr * sol ) const;

    /*!
      \brief remove the specified edge from edge set
      \param id Id number of the removed edge.
     */
    void removeEdge( int id )
      {
          EdgeCont::iterator it = M_edges.find( id );
          if ( it != M_edges.end() )
          {
              delete it->second;
              M_edges.erase( it );
          }
      }

    /*!
      \brief remove the specified edge from edge set
      \param edge pointer to the removed edge.
     */
    void removeEdge( Edge * edge )
      {
          if ( edge )
          {
              removeEdge( edge->id() );
          }
      }

    /*!
      \brief remove the specified triangle from triangle set
      \param id Id number of the removed triangle.
     */
    void removeTriangle( int id )
      {
          TriangleCont::iterator it = M_triangles.find( id );
          if ( it != M_triangles.end() )
          {
              //std::cout << "remove triangle " << id
              //          << it->second->vertex( 0 )->pos()
              //          << it->second->vertex( 1 )->pos()
              //          << it->second->vertex( 2 )->pos()
              //          << std::endl;
              delete it->second;
              M_triangles.erase( it );
          }
      }

    /*!
      \brief remove the specified triangle from triangle set
      \param tri pointer to the removed triangle.
     */
    void removeTriangle( TrianglePtr tri )
      {
          if ( tri )
          {
              removeTriangle( tri->id() );
          }
      }

    /*!
      \brief create new edge from two vertices, and register it to the edge set.
      \param v0 first vertex
      \param v1 second vertex
      \return pointer to the new edge instance.
     */
    EdgePtr createEdge( const Vertex * v0,
                        const Vertex * v1 )
      {
          EdgePtr ptr = new Edge( M_edge_count++, v0, v1 );
          M_edges.insert( EdgeCont::value_type( ptr->id(), ptr ) );
          return ptr;
      }

    /*!
      \brief create new triangle from three edges, and register it to the triangle set.
      \param e0 first edge
      \param e1 second edge
      \param e2 third edge
      \return pointer to the new triangle instance.
     */
    TrianglePtr createTriangle( Edge * e0,
                                Edge * e1,
                                Edge * e2 )
      {
          // triangle is set to edges in the constructor of Triangle
          TrianglePtr ptr = new Triangle( M_tri_count++, e0, e1, e2 );
          M_triangles.insert( TriangleCont::value_type( ptr->id(), ptr ) );
          return ptr;
      }

};

}

#endif
