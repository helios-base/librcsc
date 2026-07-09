// -*-c++-*-

/*!
  \file constrained_delaunay_triangulation.h
  \brief Constrained Delaunay Triangulation class Header File.
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

#ifndef RCSC_GEOM_CONSTRAINED_DELAUNAY_TRIANGULATION_H
#define RCSC_GEOM_CONSTRAINED_DELAUNAY_TRIANGULATION_H

#include <rcsc/geom/rect_2d.h>
#include <rcsc/geom/vector_2d.h>

#include <array>
#include <unordered_map>
#include <utility>
#include <vector>

namespace rcsc {

/*!
  \class ConstrainedDelaunayTriangulation
  \brief 2D Constrained Delaunay Triangulation (CDT).

  This class builds a Delaunay triangulation over a set of vertices and then
  forces a given set of "constraint" segments (e.g. polygon/obstacle
  boundaries) to appear as edges of the resulting mesh, even where that
  requires a locally non-Delaunay configuration. Everywhere else the mesh
  remains locally Delaunay.

  This is a standalone, self-contained implementation (incremental point
  insertion to build the unconstrained triangulation, then Sloan-style
  segment recovery by edge flipping to force in the constraint edges).
*/
class ConstrainedDelaunayTriangulation {
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
    };

    ////////////////////////////////////////////////////////////////
    /*!
      \brief triangle's vertex data.
     */
    class Vertex {
    private:
        int M_id; //!< Id number of this vertex
        Vector2D M_pos; //!< coordinate of this vertex

    public:
        /*!
          \brief create vertex with Id number 0
         */
        Vertex()
            : M_id( 0 )
          { }

        /*!
          \brief create vertex with Id & coordinates
          \param id Id number
          \param p coordinates
         */
        Vertex( const int id,
                const Vector2D & p )
            : M_id( id )
            , M_pos( p )
          { }

        /*!
          \brief create vertex with Id & coordinates
          \param id Id number
          \param x x-coordinate
          \param y y-coordinate
         */
        Vertex( const int id,
                const double x,
                const double y )
            : M_id( id )
            , M_pos( x, y )
          { }

        /*!
          \brief get the Id of this vertex
          \return Id number
         */
        int id() const
          {
              return M_id;
          }

        /*!
          \brief get the coordinates of this vertex
          \return const reference to the vector object.
         */
        const Vector2D & pos() const
          {
              return M_pos;
          }
    };

    ////////////////////////////////////////////////////////////////

    class Edge;
    class Triangle;

    typedef Edge * EdgePtr; //!< alias of Edge pointer
    typedef Triangle * TrianglePtr; //!< alias of Triangle pointer

    ////////////////////////////////////////////////////////////////
    /*!
      \brief triangle's edge data.
     */
    class Edge {
    private:
        const int M_id; //!< Id number of this edge
        const Vertex * M_vertices[2]; //!< the two vertices of this edge
        TrianglePtr M_triangles[2]; //!< triangles which this edge belongs to
        bool M_constrained; //!< true if this edge must not be flipped away

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
            : M_id( id ),
              M_constrained( false )
          {
              M_vertices[0] = v0;
              M_vertices[1] = v1;
              M_triangles[0] = nullptr;
              M_triangles[1] = nullptr;
          }

        /*!
          \brief remove pointer to the triangle that this edge belongs to.
          This edge is NOT removed.
          \param tri pointer to the target triangle
         */
        void removeTriangle( TrianglePtr tri )
          {
              if ( M_triangles[0] == tri ) M_triangles[0] = nullptr;
              if ( M_triangles[1] == tri ) M_triangles[1] = nullptr;
          }

        /*!
          \brief set the triangle that this edge belongs to.
          \param tri raw pointer to the triangle.

          It is not checked whether this edge belongs to that triangle.
          If the target triangle is already set or two triangles are already
          set, this function has no effect. So, this method should be called
          only from Triangle's constructor.
         */
        void setTriangle( TrianglePtr tri )
          {
              if ( M_triangles[0] == tri ) return;
              if ( M_triangles[1] == tri ) return;

              if ( ! M_triangles[0] ) M_triangles[0] = tri;
              else if ( ! M_triangles[1] ) M_triangles[1] = tri;
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
        const Vertex * vertex( const std::size_t i ) const
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
              return ( M_vertices[0] == v || M_vertices[1] == v );
          }

        /*!
          \brief check if this edge connects the two specified vertices.
          \param v0 first vertex
          \param v1 second vertex
          \return true if this edge connects v0 and v1.
         */
        bool hasVertices( const Vertex * v0,
                          const Vertex * v1 ) const
          {
              return ( ( M_vertices[0] == v0 && M_vertices[1] == v1 )
                       || ( M_vertices[0] == v1 && M_vertices[1] == v0 ) );
          }

        /*!
          \brief mark/unmark this edge as a constraint edge.
          A constrained edge is never removed by the Delaunay legalization
          (flip) procedure.
          \param on new value
         */
        void setConstrained( const bool on )
          {
              M_constrained = on;
          }

        /*!
          \brief check if this edge is a constraint edge.
          \return true if this edge must be kept as-is.
         */
        bool constrained() const
          {
              return M_constrained;
          }
    };

    ////////////////////////////////////////////////////////////////
    /*!
      \brief triangle data
     */
    class Triangle {
    private:
        int M_id; //!< Id number of this triangle

        std::array< const Vertex *, 3 > M_vertices; //!< vertices of this triangle
        std::array< EdgePtr, 3 > M_edges; //!< edges of this triangle

        Vector2D M_circumcenter; //!< coordinates of the circumcenter.
        double M_circumradius; //!< radius of the circumcircle.

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
        const Vertex * vertex( std::size_t i ) const
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
        const Vector2D & circumcenter() const
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
              return ( v == M_vertices[0] || v == M_vertices[1] || v == M_vertices[2] );
          }

        /*!
          \brief get the pointer to the vertex that is different from the specified vertices.
          \param v1 first vertex
          \param v2 second vertex
          \return if exist, const pointer to the vertex. else NULL is returned.
         */
        const Vertex * getVertexExclude( const Vertex * v1,
                                         const Vertex * v2 ) const
          {
              for ( std::size_t i = 0; i < 3; ++i )
              {
                  if ( M_vertices[i] != v1 && M_vertices[i] != v2 )
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
        const Vertex * getVertexExclude( const Edge * edge ) const
          {
              return getVertexExclude( edge->vertex( 0 ), edge->vertex( 1 ) );
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
                  if ( M_edges[i]->hasVertex( v1 ) && M_edges[i]->hasVertex( v2 ) )
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
    typedef std::pair< int, int > ConstraintSegment; //!< constraint segment (pair of vertex Id)
    typedef std::vector< ConstraintSegment > ConstraintCont; //!< constraint segment container type

private:

    int M_edge_count = 0; //!< counter to assign Id to edges
    int M_tri_count = 0; //!< counter to assign Id to triangles

    Vertex M_initial_vertex[3]; //!< vertex instances of the initial super triangle

    VertexCont M_vertices; //!< instances of the input vertices. referred to by edges and triangles.
    EdgeCont M_edges; //!< edge instance holder. key: id
    TriangleCont M_triangles; //!< triangle instance holder. key: id

    ConstraintCont M_constraints; //!< input constraint segments (pairs of vertex Id)

    // not used
    ConstrainedDelaunayTriangulation( const ConstrainedDelaunayTriangulation & ) = delete;
    ConstrainedDelaunayTriangulation & operator=( const ConstrainedDelaunayTriangulation & ) = delete;

public:

    /*!
      \brief nothing to do
    */
    ConstrainedDelaunayTriangulation() = default;

    /*!
      \brief construct with considerable rectangle region
      \param region considerable rectangle region.

      All vertices must be included in region.
    */
    explicit
    ConstrainedDelaunayTriangulation( const Rect2D & region )
      {
          createInitialTriangle( region );
      }

    /*!
      \brief destruct
     */
    ~ConstrainedDelaunayTriangulation();

    /*!
      \brief initialize with target field rectangle data.
      All data are cleared. Initial triangle is created.
     */
    void init( const Rect2D & region )
      {
          clear();
          createInitialTriangle( region );
      }

    /*!
      \brief clear all vertices, constraints and all computed results.
     */
    void clear();

    /*!
      \brief clear all computed results. Input vertices and constraints are kept.
     */
    void clearResults();

    /*!
      \brief get vertices
      \return const reference to the vertices container
     */
    const VertexCont & vertices() const
      {
          return M_vertices;
      }

    /*!
      \brief get edge set
      \return const reference to the map container. key=id, value=raw pointer
     */
    const EdgeCont & edges() const
      {
          return M_edges;
      }

    /*!
      \brief get triangle set
      \return const reference to the map container. key=id, value=raw pointer
     */
    const TriangleCont & triangles() const
      {
          return M_triangles;
      }

    /*!
      \brief get the input constraint segments
      \return const reference to the constraint container
     */
    const ConstraintCont & constraints() const
      {
          return M_constraints;
      }

    /*!
      \brief add new vertex
      \param x coordinate x
      \param y coordinate y
      \return assigned id value. -1 if a vertex already exists at (nearly) the same coordinates.
     */
    int addVertex( const double x,
                   const double y );

    /*!
      \brief add new vertex
      \param p added point
      \return assigned id value. -1 if a vertex already exists at (nearly) the same coordinates.
     */
    int addVertex( const Vector2D & p )
      {
          return addVertex( p.x, p.y );
      }

    /*!
      \brief add new vertices
      \param v container of vertices.
     */
    void addVertices( const std::vector< Vector2D > & v );

    /*!
      \brief get the const pointer to vertex specified by Id number.
      \param id wanted vertex Id number.
      \return const pointer to the vertex instance. if no vertex, NULL is returned.
     */
    const Vertex * getVertex( const int id ) const;

    /*!
      \brief register a constraint segment between two already added vertices.
      The segment is only enforced when compute() is called afterward.
      \param id0 Id of the first vertex (as returned by addVertex()).
      \param id1 Id of the second vertex.
      \return false if id0/id1 do not refer to distinct existing vertices.
     */
    bool addConstraint( const int id0,
                        const int id1 );

    /*!
      \brief register a constraint segment, adding its endpoints as vertices
      first if necessary (an existing vertex within the merge tolerance is
      reused instead of creating a duplicate).
      \param p0 first endpoint
      \param p1 second endpoint
      \return false if p0 and p1 are (nearly) the same coordinate.
     */
    bool addConstraint( const Vector2D & p0,
                        const Vector2D & p1 );

    /*!
      \brief register the segments of a polyline (or, if closed==true, a
      closed polygon boundary) as constraints, adding vertices as necessary.
      \param points ordered points of the polyline.
      \param closed if true, an additional constraint segment connecting the
      last point back to the first one is also added.
      \return number of successfully added constraint segments.
     */
    std::size_t addConstraint( const std::vector< Vector2D > & points,
                               const bool closed );

    /*!
      \brief compute the Constrained Delaunay Triangulation.

      First an (unconstrained) Delaunay triangulation of all the added
      vertices is built by incremental insertion. Then every registered
      constraint segment is forced into the mesh by flipping the edges that
      cross it (Sloan-style segment recovery), after which the affected
      region is re-legalized (Delaunay edge flip) without ever touching a
      constrained edge. A constraint that geometrically crosses another
      already-inserted constraint cannot be honored and is skipped with a
      diagnostic message; every other requested constraint is still applied.
    */
    void compute();

    /*!
      \brief find triangle that contains pos from the computed triangle set.
      \param pos coordinates of the target point
      \return const pointer to the found triangle. if no triangle, NULL is returned.
     */
    const Triangle * findTriangleContains( const Vector2D & pos ) const;

    /*!
      \brief find the vertex nearest to the specified point
      \param pos coordinates of the target point
      \return const pointer to the found vertex, if no vertex, NULL is returned.
     */
    const Vertex * findNearestVertex( const Vector2D & pos ) const;

    /*!
      \brief check whether the edge connecting the two vertices is registered
      as a constraint edge in the computed result.
      \param id0 Id of the first vertex.
      \param id1 Id of the second vertex.
      \return true if such an edge exists and is constrained.
     */
    bool isConstrainedEdge( const int id0,
                            const int id1 ) const;

private:

    /*!
      \brief clear old triangles and create initial triangle that includes region.
      \param region considerable region
    */
    void createInitialTriangle( const Rect2D & region );

    /*!
      \brief create region using the stored vertices.
    */
    void createInitialTriangle();

    /*!
      \brief remove initial vertices and edges/triangles connected to them.
     */
    void removeInitialVertices();

    /*!
      \brief find or add a vertex at (nearly) the given coordinates.
      \param p target coordinates
      \return Id of the existing or newly added vertex.
     */
    int findOrCreateVertex( const Vector2D & p );

    /*!
      \brief update triangles by new vertex contained in the interior of 'tri'.
      \param vertex const pointer to the new vertex
      \param tri pointer to the triangle that contains vertex.
      \return false if an unrecoverable numerical error was detected.
     */
    bool updateContainedVertex( const Vertex * vertex,
                                const TrianglePtr tri );

    /*!
      \brief update triangles by new vertex that lies on an edge of 'tri'.
      \param vertex const pointer to the new vertex
      \param tri pointer to the triangle that vertex is online to.
      \return false if an unrecoverable numerical error was detected.
     */
    bool updateOnlineVertex( const Vertex * vertex,
                             const TrianglePtr tri );

    /*!
      \brief flip the diagonal of the quadrilateral formed by the two
      triangles sharing 'edge'. 'edge' and its two triangles are destroyed
      and replaced by a new edge (connecting the two triangles' opposite
      vertices) and two new triangles. Never called on a constrained edge or
      on a boundary edge (an edge with only one adjacent triangle).
      \param edge the edge to flip. must have two valid adjacent triangles.
      \param outer_edges on success, filled with the (unchanged, pre-existing)
      four edges of the two new triangles other than the new diagonal, in an
      unspecified order.
      \param new_edge on success, set to the newly created diagonal edge.
      \return false if an unrecoverable numerical error was detected.
     */
    bool flipEdge( EdgePtr edge,
                   EdgePtr outer_edges[4],
                   EdgePtr * new_edge );

    /*!
      \brief check whether 'edge' is already locally Delaunay-legal, i.e. it
      needs no flip. This is true when the edge is constrained, is a
      boundary edge (only one adjacent triangle), or the incircle test
      between its two adjacent triangles passes.
      \param edge the edge to check.
      \return true if no flip of 'edge' is required.
     */
    bool isLocallyDelaunay( const EdgePtr edge ) const;

    /*!
      \brief restore the Delaunay condition for 'edge' if it is violated,
      flipping it (and recursively any edge affected by that flip) as
      needed. Never flips a constrained edge, and is a no-op on a boundary
      edge or on an already-legal edge.
      \param edge the edge to check.
      \return false if an unrecoverable numerical error was detected.
     */
    bool legalizeEdge( EdgePtr edge );

    /*!
      \brief repeatedly sweep every edge currently in the mesh and
      legalizeEdge() it, until a full sweep finds nothing left to fix (or a
      generous sweep budget is exceeded). A single sweep is not enough in
      general: fixing one edge can re-illegalize an edge that was already
      checked earlier in the same sweep, so convergence needs to be
      re-verified. Used to restore the Delaunay property (outside of
      constrained edges) after a constraint segment has been forced into the
      mesh.
      \return false if an unrecoverable numerical error was detected, or the
      sweep budget was exceeded.
     */
    bool legalizeAll();

    /*!
      \brief force the constraint segment between v0 and v1 into the mesh,
      flipping any edge that crosses it, then re-legalize the affected
      region. If v0-v1 already exists as an edge, it is simply marked
      constrained.
      \param id0 Id of the first vertex.
      \param id1 Id of the second vertex.
      \return false if the constraint could not be honored (e.g. it crosses
      an already-placed constraint) or an unrecoverable numerical error was
      detected. The mesh is left in a consistent (if partially unlegalized)
      state either way.
     */
    bool insertConstraint( const int id0,
                           const int id1 );

    /*!
      \brief find triangle that contains pos from the computed triangle set,
      by exhaustively checking every triangle. O(N).
      \param pos coordinates of the target point
      \param sol pointer to the solution variable.
      \return how the point is contained.
     */
    ContainedType findTriangleContains( const Vector2D & pos,
                                        TrianglePtr * sol ) const;

    /*!
      \brief remove the specified edge from the edge set
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
      \brief remove the specified edge from the edge set
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
      \brief remove the specified triangle from the triangle set
      \param id Id number of the removed triangle.
     */
    void removeTriangle( int id )
      {
          TriangleCont::iterator it = M_triangles.find( id );
          if ( it != M_triangles.end() )
          {
              delete it->second;
              M_triangles.erase( it );
          }
      }

    /*!
      \brief remove the specified triangle from the triangle set
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
      \brief create new edge from two vertices, and register it in the edge set.
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
      \brief create new triangle from three edges, and register it in the triangle set.
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
