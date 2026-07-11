// -*-c++-*-

/*!
  \file delaunay_triangulation_core.h
  \brief shared planar triangle mesh (vertex/edge/triangle graph plus
  incremental point insertion and edge-flip legalization) Header File.
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

#ifndef RCSC_GEOM_DELAUNAY_TRIANGULATION_CORE_H
#define RCSC_GEOM_DELAUNAY_TRIANGULATION_CORE_H

#include <rcsc/geom/rect_2d.h>
#include <rcsc/geom/vector_2d.h>

#include <array>
#include <unordered_map>
#include <vector>

namespace rcsc {

/*!
  \class DelaunayTriangulationCore
  \brief common vertex/edge/triangle graph and algorithms shared by
  rcsc::DelaunayTriangulation and rcsc::ConstrainedDelaunayTriangulation.

  This holds the mesh data structure (a doubly-linked vertex/edge/triangle
  graph, in the same style as Guibas & Stolfi's quad-edge / Lawson-flip
  incremental construction), the mechanics of splitting a triangle around a
  newly inserted vertex (both the case where the vertex lands strictly
  inside a triangle and the case where it lands exactly on an existing
  edge), generic Lawson-flip Delaunay legalization (which stops propagating
  at any edge marked "constrained" -- unused by DelaunayTriangulation, which
  never constrains an edge, so this reduces to plain Delaunay legalization
  for it), and the two fast point-location search strategies shared by both
  subclasses: a hint-based adjacency walk (WALK, default) and an optional
  quadtree spatial index (QUAD_TREE).

  This class is not intended to be used polymorphically (no virtual
  functions, protected non-virtual destructor): it exists purely so its two
  subclasses can share an implementation, not as a public abstraction in its
  own right.
*/
class DelaunayTriangulationCore {
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
      \enum SearchMethod
      \brief strategy used by findTriangleContains() to locate the triangle
      that contains a query point.
     */
    enum class SearchMethod {
        WALK,      //!< walk the triangle adjacency graph from a cached hint
                   //!< triangle (default). cheap to keep correct while the
                   //!< triangle set is being mutated, so compute() always
                   //!< uses this internally.
        QUAD_TREE, //!< static quadtree spatial index over the current
                   //!< triangle set, (re)built lazily the first time it is
                   //!< queried after the triangle set changes. faster than
                   //!< WALK when a triangulation is computed once and then
                   //!< queried many times without further mutation.
    };

    ////////////////////////////////////////////////////////////////
    /*!
      \brief triangle's vertex data.
      This is handled as kernel point for the Voronoi diagram.
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
        const Vector2D & pos() const
          {
              return M_pos;
          }
    };

    ////////////////////////////////////////////////////////////////

    class Edge;
    class Triangle;

    using EdgePtr = Edge*; //!< alias of Edge pointer
    using TrianglePtr = Triangle*; //!< alias of Triangle pointer

    ////////////////////////////////////////////////////////////////
    /*!
      \brief triangle's edge data.
     */
    class Edge {
    private:
        const int M_id; //!< Id number of this edge
        const Vertex * M_vertices[2]; //!< reference to the vertex of this edge
        TrianglePtr M_triangles[2]; //!< triangles whitch this edge belongs to
        bool M_constrained; //!< true if this edge must never be flipped away
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
                  M_triangles[0] = nullptr;
              }
              if ( M_triangles[1] == tri )
              {
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
                  M_triangles[0] = tri;
              }
              else if ( ! M_triangles[1] )
              {
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
              return ( M_vertices[0] == v
                       || M_vertices[1] == v );
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
          \brief mark/unmark this edge as a constraint edge. A constrained
          edge is never flipped away by legalizeEdge(). Unused by (plain)
          Delaunay triangulation, which never constrains an edge.
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
        const Vertex * getVertexExclude( const Vertex * v1,
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
        const Vertex * getVertexExclude( const Edge * edge ) const
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

    using VertexCont = std::vector< Vertex >; //!< vertex container type
    using EdgeCont = std::unordered_map< int, EdgePtr >; //!< edge pointer container type
    using TriangleCont = std::unordered_map< int, TrianglePtr >; //!< triangle pointer container type

protected:

    //! counter to set Id to edges
    int M_edge_count = 0;
    //! counter to set Id to triangles
    int M_tri_count = 0;

    //! vertex instance of initial super triangle
    Vertex M_initial_vertex[3];

    //! instance of vertices. these are refered by edge and triangle.
    VertexCont M_vertices;

    //! edge instance holder. key: id
    EdgeCont M_edges;

    //! triangle instance holder. key: id
    TriangleCont M_triangles;

    //! set whenever a triangle is created or removed (including by
    //! clearResults()). Used to know when the quadtree spatial index has
    //! gone stale and needs to be rebuilt before the next query.
    mutable bool M_topology_dirty = true;

    //! id of the triangle found by the previous findTriangleContainsFast()
    //! call. used as the start triangle ("hint") of the next point location
    //! walk, since queries tend to be spatially coherent.
    mutable int M_hint_triangle_id = -1;

    //! search strategy used by findTriangleContainsFast().
    SearchMethod M_search_method = SearchMethod::WALK;

    //! opaque quadtree node type, defined in the .cpp file.
    class QuadTreeNode;

    //! root of the quadtree spatial index. only built/used when
    //! M_search_method == SearchMethod::QUAD_TREE. owning raw pointer,
    //! released by clearQuadTree().
    mutable QuadTreeNode * M_quad_tree_root = nullptr;

    //! reused across quadTreeFindTriangleContains() calls to avoid a heap
    //! allocation per query.
    mutable std::vector< TrianglePtr > M_quad_tree_candidates;

    DelaunayTriangulationCore() = default;

    //! not meant to be used/destroyed polymorphically: this class exists
    //! purely to share an implementation between its two subclasses.
    ~DelaunayTriangulationCore() = default;

    // not used
    DelaunayTriangulationCore( const DelaunayTriangulationCore & ) = delete;
    DelaunayTriangulationCore & operator=( const DelaunayTriangulationCore & ) = delete;

public:

    /*!
      \brief clear all data and reset the state.
     */
    void clear();

    /*!
      \brief select the search strategy used by findTriangleContainsFast().
      Does not affect the search compute() performs internally while building
      the triangulation, which always uses exhaustiveFindTriangleContains().
      \param method the new search strategy.
     */
    void setSearchMethod( SearchMethod method )
      {
          M_search_method = method;
      }

    /*!
      \brief get the search strategy currently used by
      findTriangleContainsFast().
      \return the current search strategy
     */
    SearchMethod searchMethod() const
      {
          return M_search_method;
      }

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
      \return const referenct to the map container. key=id, value=raw pointer
     */
    const EdgeCont & edges() const
      {
          return M_edges;
      }

    /*!
      \brief get triangle set
      \return const referenct to the map container. key=id, value=raw pointer
     */
    const TriangleCont & triangles() const
      {
          return M_triangles;
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
      \brief set vertices.
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
      \brief find the vertex nearest to the specified point
      \param pos coordinates of the target point
      \return const pointer to the found vertex, if no vertex, NULL is returned.
     */
    const Vertex * findNearestVertex( const Vector2D & pos ) const;

protected:

    /*!
      \brief reset the counters and delete every edge/triangle. Vertices and
      any subclass-only state (e.g. a cached spatial index, registered
      constraints) are untouched -- subclasses that own such state must
      define their own clearResults() that calls this one and then also
      resets that extra state.
     */
    void clearResults();

    /*!
      \brief clear old triangles and create initial triangle that includes region.
      Does not itself call clearResults(): callers that need a clean slate
      (as opposed to bootstrapping a brand new, already-empty mesh) must call
      it first.
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
      \brief check whether 'edge' is already locally Delaunay-legal, i.e. it
      needs no flip. True when the edge is constrained, is a boundary edge
      (only one adjacent triangle), or the incircle test between its two
      adjacent triangles passes.
      \param edge the edge to check.
      \return true if no flip of 'edge' is required.
     */
    bool isLocallyDelaunay( const EdgePtr edge ) const;

    /*!
      \brief restore the Delaunay condition for 'edge' if it is violated,
      flipping it (and recursively any edge affected by that flip) as
      needed. Never flips a constrained edge, and is a no-op on a boundary
      edge or on an already-legal edge. This is the classic Lawson-flip
      recursion: used both to legalize the neighborhood of a freshly
      inserted point, and (by ConstrainedDelaunayTriangulation) to restore
      the Delaunay property after a constraint segment has been forced into
      the mesh.

      \param edge the edge to check.
      \param new_vertex when non-null, 'edge' is assumed to have just arisen
      from inserting this vertex (directly, or via a chain of flips that
      trace back to it), and exactly one of edge's two adjacent triangles is
      assumed to already contain it. After a flip, recursion then follows
      the textbook point-insertion algorithm exactly: only the two edges of
      the triangle that does *not* contain new_vertex are re-checked (the
      two that do are provably already legal, by induction on this same
      invariant). This asymmetric recursion isn't just an optimization: for
      inputs with a non-unique Delaunay triangulation (e.g. near-cocircular
      points), it is what selects the same tie-break as the reference
      algorithm out of several simultaneously-"locally legal" results. When
      new_vertex is null (or, defensively, if the invariant doesn't hold),
      all four edges affected by a flip are re-checked instead -- always
      correct, just not guaranteed to match that specific tie-break.
      \return false if an unrecoverable numerical error was detected.
     */
    bool legalizeEdge( EdgePtr edge,
                       const Vertex * new_vertex = nullptr );

    /*!
      \brief flip the diagonal of the quadrilateral formed by the two
      triangles sharing 'edge'. 'edge' and its two triangles are destroyed
      and replaced by a new edge (connecting the two triangles' opposite
      vertices) and two new triangles. Must not be called on a constrained
      edge, on a boundary edge (an edge with only one adjacent triangle), or
      when the resulting quadrilateral would not be convex.
      \param edge the edge to flip. must have two valid adjacent triangles.
      \param outer_edges on success, filled with the (unchanged, pre-existing)
      four edges of the two new triangles other than the new diagonal:
      outer_edges[0]/[1] belong to the new triangle built on the same side
      as new_vertex (if given and found), outer_edges[2]/[3] to the other
      one; when new_vertex is null (or not found on either side) the split
      between [0]/[1] and [2]/[3] still holds, just without that meaning.
      \param new_edge on success, set to the newly created diagonal edge.
      \param new_vertex when non-null and equal to one of the two triangles'
      opposite vertices, canonicalizes which triangle is treated as "first"
      so that the new diagonal edge and the two new triangles are built with
      new_vertex ordered first -- see legalizeEdge() for why this matters.
      \return false if an unrecoverable numerical error was detected.
     */
    bool flipEdge( EdgePtr edge,
                   EdgePtr outer_edges[4],
                   EdgePtr * new_edge,
                   const Vertex * new_vertex = nullptr );

    /*!
      \brief classify how 'pos' relates to a single triangle (point-in-
      triangle test, with a fast reject using the fact that the circumcircle
      always encloses the triangle).
      \param tri the triangle to test.
      \param pos the point to classify.
      \return NOT_CONTAINED, CONTAINED (strictly inside), or ONLINE (on the
      boundary).
     */
    static
    ContainedType classifyPoint( const TrianglePtr tri,
                                 const Vector2D & pos );

    /*!
      \brief find the triangle that contains pos by exhaustively checking
      every triangle in the current triangle set. O(N), but always
      conclusive.
      \param pos coordinates of the target point
      \param sol pointer to the solution variable.
      \return how the point is contained.
     */
    ContainedType exhaustiveFindTriangleContains( const Vector2D & pos,
                                                  TrianglePtr * sol ) const;

    /*!
      \brief find triangle that contains pos using the selected search
      strategy (WALK or QUAD_TREE), falling back to
      exhaustiveFindTriangleContains() when needed. This is the shared
      implementation used by both DelaunayTriangulation and
      ConstrainedDelaunayTriangulation for their public
      findTriangleContains().
      \param pos coordinates of the target point
      \param sol pointer to the solution variable.
      \return how the point is contained.
     */
    ContainedType findTriangleContainsFast( const Vector2D & pos,
                                            TrianglePtr * sol ) const;

    /*!
      \brief try to find the triangle that contains pos by walking through
      triangle adjacency, starting from 'start'.
      \param pos coordinates of the target point
      \param start triangle to start the walk from
      \param sol pointer to the solution variable.
      \return CONTAINED or ONLINE if conclusively found. NOT_CONTAINED means
      either pos is confirmed outside the triangulated region, or the walk
      could not reach a conclusive answer — callers must fall back to
      exhaustiveFindTriangleContains().
     */
    ContainedType walkTriangleContains( const Vector2D & pos,
                                        TrianglePtr start,
                                        TrianglePtr * sol ) const;

    /*!
      \brief find triangle that contains pos using the quadtree spatial
      index, (re)building it first if it is missing or stale.
      \param pos coordinates of the target point
      \param sol pointer to the solution variable.
      \return CONTAINED or ONLINE if conclusively found via the index.
      NOT_CONTAINED means the index could not conclusively resolve pos.
     */
    ContainedType quadTreeFindTriangleContains( const Vector2D & pos,
                                                TrianglePtr * sol ) const;

    /*!
      \brief build (or rebuild) the quadtree spatial index from the current
      triangle set. Marks M_topology_dirty false on success.
     */
    void buildQuadTree() const;

    /*!
      \brief release the quadtree spatial index.
     */
    void clearQuadTree() const;

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
              delete it->second;
              M_triangles.erase( it );
              M_topology_dirty = true;
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
          M_topology_dirty = true;
          return ptr;
      }

};

}

#endif
