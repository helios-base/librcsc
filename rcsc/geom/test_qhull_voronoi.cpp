
#include "qhull/src/qhull_a.h"

#include <boost/random.hpp>

#include <cstdio>
#include <csetjmp>
#include <iostream>


namespace {

const double noise_2d[8] = { +1.0e-8, +1.0e-8,
                             +1.0e-8, -1.0e-8,
                             -1.0e-8, -1.0e-8,
                             +1.0e-8, -1.0e-8 };
int noise_index = 0;

}

/*
  enum qh_PRINT {
  qh_PRINTnone = 0,
  qh_PRINTarea = 1,       // 'Fa'
  qh_PRINTaverage = 2,    // 'FV'
  qh_PRINTcoplanars = 3,  // 'Fc'
  qh_PRINTcentrums = 4,   // 'FC'
  qh_PRINTfacets = 5,     // 'f'
  qh_PRINTfacets_xridge = 6, // 'FF'
  qh_PRINTgeom = 7,       // 'G'
  qh_PRINTids = 8,        // 'FI'
  qh_PRINTinner = 9,      // 'Fi'
  qh_PRINTneighbors = 10, // 'Fn'
  qh_PRINTnormals = 11,   // 'n'
  qh_PRINTouter = 12,     // 'Fo'
  qh_PRINTmaple = 13,     // 'i'
  qh_PRINTincidences = 14, // 'm'
  qh_PRINTmathematica = 15, // 'Fm'
  qh_PRINTmerges = 16,    // 'FM'
  qh_PRINToff = 17,       // 'o'
  qh_PRINToptions = 18,   // 'FO'
  qh_PRINTpointintersect = 19, // 'Fp'
  qh_PRINTpointnearest = 20, // 'FP'
  qh_PRINTpoints = 21,    // 'p'
  qh_PRINTqhull = 22,     // 'FQ'
  qh_PRINTsize = 23,      // 'FS'
  qh_PRINTsummary = 24,   // 'Fs'
  qh_PRINTtriangles = 25, // 'Ft'
  qh_PRINTvertices = 26,  // 'Fv'
  qh_PRINTvneighbors = 27, // 'FN'
  qh_PRINTextremes = 28,  // 'Fx'
  qh_PRINTEND = 29,
*/


const char *format_string[] = {
    "qh_PRINTnone",
    "Fa qh_PRINTarea",
    "FV qh_PRINTaverage", // 'FV'
    "Fc qh_PRINTcoplanars", // 'Fc'
    "FC qh_PRINTcentrums", // 'FC'
    "f  qh_PRINTfacets",  // 'f'
    "FF qh_PRINTfacets_xridge",  // 'FF'
    "G  qh_PRINTgeom", // 'G'
    "FI qh_PRINTids", // 'FI'
    "Fi qh_PRINTinner", // 'Fi'
    "Fn qh_PRINTneighbors",  // 'Fn'
    "n  qh_PRINTnormals", // 'n'
    "Fo qh_PRINTouter", // 'Fo'
    "i  qh_PRINTmaple", // 'i'
    "m  qh_PRINTincidences", // 'm'
    "Fm qh_PRINTmathematica", // 'Fm'
    "FM qh_PRINTmerges",    // 'FM'
    "o  qh_PRINToff",       // 'o'
    "FO qh_PRINToptions",   // 'FO'
    "Fp qh_PRINTpointintersect", // 'Fp'
    "FP qh_PRINTpointnearest", // 'FP'
    "p  qh_PRINTpoints",    // 'p'
    "FQ  qh_PRINTqhull",     // 'FQ'
    "FS qh_PRINTsize",      // 'FS'
    "Fs qh_PRINTsummary",   // 'Fs'
    "Ft qh_PRINTtriangles", // 'Ft'
    "Fv qh_PRINTvertices",  // 'Fv'
    "FN qh_PRINTvneighbors", // 'FN'
    "Fx qh_PRINTextremes",  // 'Fx'
    "qh_PRINTEND" };


/*-<a                             href="qh-io.htm#TOC"
  >-------------------------------</a><a name="detvnorm">-</a>

  qh_detvnorm( vertex, vertexA, centers, offset )
  compute separating plane of the Voronoi diagram for a pair of input sites
  centers= set of facets (i.e., Voronoi vertices)
  facet->visitid= 0 if vertex-at-infinity (i.e., unbounded)

  assumes:
  qh_ASvoronoi and qh_vertexneighbors() already set

  returns:
  norm
  a pointer into qh.gm_matrix to qh.hull_dim-1 reals
  copy the data before reusing qh.gm_matrix
  offset
  if 'QVn'
  sign adjusted so that qh.GOODvertexp is inside
  else
  sign adjusted so that vertex is inside

  qh.gm_matrix= simplex of points from centers relative to first center

  notes:
  in io.c so that code for 'v Tv' can be removed by removing io.c
  returns pointer into qh.gm_matrix to avoid tracking of temporary memory

  design:
  determine midpoint of input sites
  build points as the set of Voronoi vertices
  select a simplex from points (if necessary)
  include midpoint if the Voronoi region is unbounded
  relocate the first vertex of the simplex to the origin
  compute the normalized hyperplane through the simplex
  orient the hyperplane toward 'QVn' or 'vertex'
  if 'Tv' or 'Ts'
  if bounded
  test that hyperplane is the perpendicular bisector of the input sites
  test that Voronoi vertices not in the simplex are still on the hyperplane
  free up temporary memory
*/
pointT *
my_qh_detvnorm( vertexT * vertex,
                vertexT * vertexA,
                setT * centers,
                realT * offsetp )
{
    facetT * facet;
    facetT ** facetp;
    int  i, k, pointid, pointidA, point_i, point_n;
    setT *simplex= NULL;
    pointT *point, **pointp, *point0, *midpoint, *normal, *inpoint;
    coordT *coord, *gmcoord, *normalp;
    setT * points = qh_settemp( qh TEMPsize );
    boolT nearzero = False;
    boolT unbounded = False;
    int numcenters = 0;
    int dim = qh hull_dim - 1;
    realT dist, offset, angle, zero= 0.0;

    midpoint = qh gm_matrix + qh hull_dim * qh hull_dim;  /* last row */
    for ( k = 0; k < dim; ++k )
    {
        midpoint[k] = ( vertex->point[k] + vertexA->point[k] ) / 2;
    }

    FOREACHfacet_( centers )
    {
        ++numcenters;
        if ( ! facet->visitid )
        {
            unbounded = True;
        }
        else
        {
            if ( ! facet->center )
            {
                facet->center = qh_facetcenter( facet->vertices );
            }
            qh_setappend(&points, facet->center);
        }
    }

    if ( numcenters > dim )
    {
        simplex = qh_settemp( qh TEMPsize );
        qh_setappend(&simplex, vertex->point);
        if ( unbounded )
            qh_setappend(&simplex, midpoint);
        qh_maxsimplex(dim, points, NULL, 0, &simplex);
        qh_setdelnth(simplex, 0);
    }
    else if ( numcenters == dim )
    {
        if ( unbounded )
        {
            qh_setappend( &points, midpoint );
        }
        simplex = points;
    }
    else
    {
        qh_fprintf( qh ferr, 6216, "qhull internal error (qh_detvnorm): too few points(%d) to compute separating plane\n", numcenters );
        qh_errexit( qh_ERRqhull, NULL, NULL );
    }

    i = 0;
    gmcoord = qh gm_matrix;
    point0 = SETfirstt_( simplex, pointT );

    FOREACHpoint_( simplex )
    {
        if ( qh IStracing >= 4 )
        {
            qh_printmatrix( qh ferr, "qh_detvnorm: Voronoi vertex or midpoint",
                            &point, 1, dim );
        }

        if ( point != point0 )
        {
            qh gm_row[i++] = gmcoord;
            coord = point0;
            for ( k = dim; k--; )
            {
                *(gmcoord++) = *point++ - *coord++;
            }
        }
    }
    qh gm_row[i] = gmcoord;  /* does not overlap midpoint, may be used later for qh_areasimplex */
    normal= gmcoord;
    qh_sethyperplane_gauss(dim, qh gm_row, point0, True,
                           normal, &offset, &nearzero);
    if (qh GOODvertexp == vertexA->point)
    {
        inpoint= vertexA->point;
    }
    else
    {
        inpoint= vertex->point;
    }

    zinc_(Zdistio);
    dist = qh_distnorm(dim, inpoint, normal, &offset);
    if (dist > 0)
    {
        offset= -offset;
        normalp= normal;
        for (k=dim; k--; ) {
            *normalp= -(*normalp);
            normalp++;
        }
    }

    if (qh VERIFYoutput || qh PRINTstatistics)
    {
        pointid= qh_pointid(vertex->point);
        pointidA= qh_pointid(vertexA->point);
        if (!unbounded)
        {
            zinc_(Zdiststat);
            dist= qh_distnorm(dim, midpoint, normal, &offset);
            if (dist < 0)
                dist= -dist;
            zzinc_(Zridgemid);
            wwmax_(Wridgemidmax, dist);
            wwadd_(Wridgemid, dist);
            trace4((qh ferr, 4014, "qh_detvnorm: points %d %d midpoint dist %2.2g\n",
                    pointid, pointidA, dist));
            for (k=0; k < dim; k++)
                midpoint[k]= vertexA->point[k] - vertex->point[k];  /* overwrites midpoint! */
            qh_normalize(midpoint, dim, False);
            angle= qh_distnorm(dim, midpoint, normal, &zero); /* qh_detangle uses dim+1 */
            if (angle < 0.0)
                angle= angle + 1.0;
            else
                angle= angle - 1.0;
            if (angle < 0.0)
                angle -= angle;
            trace4((qh ferr, 4015, "qh_detvnorm: points %d %d angle %2.2g nearzero %d\n",
                    pointid, pointidA, angle, nearzero));
            if (nearzero) {
                zzinc_(Zridge0);
                wwmax_(Wridge0max, angle);
                wwadd_(Wridge0, angle);
            }else {
                zzinc_(Zridgeok)
                    wwmax_(Wridgeokmax, angle);
                wwadd_(Wridgeok, angle);
            }
        }

        if ( simplex != points )
        {
            FOREACHpoint_i_( points )
            {
                if ( ! qh_setin( simplex, point ) )
                {
                    facet = SETelemt_( centers, point_i, facetT );
                    zinc_( Zdiststat );
                    dist= qh_distnorm(dim, point, normal, &offset);
                    if (dist < 0)
                    {
                        dist= -dist;
                    }
                    zzinc_(Zridge);
                    wwmax_(Wridgemax, dist);
                    wwadd_(Wridge, dist);
                    trace4((qh ferr, 4016, "qh_detvnorm: points %d %d Voronoi vertex %d dist %2.2g\n",
                            pointid, pointidA, facet->visitid, dist));
                }
            }
        }
    }
    *offsetp= offset;

    if (simplex != points)
    {
        qh_settempfree(&simplex);
    }
    qh_settempfree(&points);
    return normal;
} /* detvnorm */


/*-<a                             href="qh-io.htm#TOC"
  >-------------------------------</a><a name="printvnorm">-</a>

  qh_printvnorm( fp, vertex, vertexA, centers, unbounded )
  print one separating plane of the Voronoi diagram for a pair of input sites
  unbounded==True if centers includes vertex-at-infinity

  assumes:
  qh_ASvoronoi and qh_vertexneighbors() already set

  note:
  parameter unbounded is UNUSED by this callback

  see:
  qh_printvdiagram()
  qh_eachvoronoi()
*/
void
my_qh_printvnorm( FILE * fp,
                  vertexT * vertex,
                  vertexT * vertexA,
                  setT * centers,
                  boolT unbounded )
{
    QHULL_UNUSED(unbounded);

    realT offset;
    pointT * normal = my_qh_detvnorm( vertex, vertexA, centers, &offset );

    qh_fprintf( fp, 9271, "%d %d %d ",
                2 + qh hull_dim,
                qh_pointid( vertex->point ),
                qh_pointid( vertexA->point ) );
    for ( int k = 0; k < qh hull_dim - 1; ++k )
    {
        qh_fprintf( fp, 9272, qh_REAL_1, normal[k] );
    }
    qh_fprintf( fp, 9273, qh_REAL_1, offset );
    qh_fprintf( fp, 9274, "\n" );

}



/*-<a                             href="qh-io.htm#TOC"
  >-------------------------------</a><a name="markvoronoi">-</a>

  qh_markvoronoi( facetlist, facets, printall, isLower, numcenters )
  mark voronoi vertices for printing by site pairs

  returns:
  temporary set of vertices indexed by pointid
  isLower set if printing lower hull (i.e., at least one facet is lower hull)
  numcenters= total number of Voronoi vertices
  bumps qh.printoutnum for vertex-at-infinity
  clears all facet->seen and sets facet->seen2

  if selected
  facet->visitid= Voronoi vertex id
  else if upper hull (or 'Qu' and lower hull)
  facet->visitid= 0
  else
  facet->visitid >= qh num_facets

  notes:
  ignores qh.ATinfinity, if defined
*/
setT *
my_qh_markvoronoi( facetT * facetlist,
                   setT * facets,
                   boolT printall,
                   boolT * isLowerp,
                   int * numcentersp )
{
    int numcenters = 0;
    facetT *facet, **facetp;
    setT *vertices;
    boolT isLower= False;

    qh printoutnum++;
    qh_clearcenters(qh_ASvoronoi);  /* in case, qh_printvdiagram2 called by user */
    qh_vertexneighbors();
    vertices= qh_pointvertex();

    if ( qh ATinfinity )
    {
        SETelem_(vertices, qh num_points-1)= NULL;
    }
    qh visit_id++;

    maximize_( qh visit_id, (unsigned) qh num_facets );

    FORALLfacet_(facetlist)
    {
        if (printall || !qh_skipfacet(facet)) {
            if (!facet->upperdelaunay) {
                isLower= True;
                break;
            }
        }
    }

    FOREACHfacet_( facets )
    {
        if ( printall
             || ! qh_skipfacet(facet) )
        {
            if ( ! facet->upperdelaunay )
            {
                isLower = True;
                break;
            }
        }
    }

    FORALLfacets
    {
        if ( facet->normal
             && facet->upperdelaunay == isLower )
        {
            facet->visitid = 0;  /* facetlist or facets may overwrite */
        }
        else
        {
            facet->visitid = qh visit_id;
        }
        facet->seen= False;
        facet->seen2= True;
    }
    numcenters++;  /* qh_INFINITE */

    FORALLfacet_(facetlist) {
        if (printall || !qh_skipfacet(facet))
            facet->visitid= numcenters++;
    }

    FOREACHfacet_( facets )
    {
        if (printall || !qh_skipfacet(facet))
            facet->visitid= numcenters++;
    }

    *isLowerp = isLower;
    *numcentersp = numcenters;
    trace2((qh ferr, 2007, "qh_markvoronoi: isLower %d numcenters %d\n", isLower, numcenters));

    return vertices;
} /* markvoronoi */



/*-<a                             href="qh-io.htm#TOC"
  >-------------------------------</a><a name="eachvoronoi">-</a>

  qh_eachvoronoi( fp, printvridge, vertex, visitall, innerouter, inorder )
  if visitall,
  visit all Voronoi ridges for vertex (i.e., an input site)
  else
  visit all unvisited Voronoi ridges for vertex
  all vertex->seen= False if unvisited
  assumes
  all facet->seen= False
  all facet->seen2= True (for qh_detvridge3)
  all facet->visitid == 0 if vertex_at_infinity
  == index of Voronoi vertex
  >= qh.num_facets if ignored
  innerouter:
  qh_RIDGEall--  both inner (bounded) and outer(unbounded) ridges
  qh_RIDGEinner- only inner
  qh_RIDGEouter- only outer

  if inorder
  orders vertices for 3-d Voronoi diagrams

  returns:
  number of visited ridges (does not include previously visited ridges)

  if printvridge,
  calls printvridge( fp, vertex, vertexA, centers)
  fp== any pointer (assumes FILE*)
  vertex,vertexA= pair of input sites that define a Voronoi ridge
  centers= set of facets (i.e., Voronoi vertices)
  ->visitid == index or 0 if vertex_at_infinity
  ordered for 3-d Voronoi diagram
  notes:
  uses qh.vertex_visit

  see:
  qh_eachvoronoi_all()

  design:
  mark selected neighbors of atvertex
  for each selected neighbor (either Voronoi vertex or vertex-at-infinity)
  for each unvisited vertex
  if atvertex and vertex share more than d-1 neighbors
  bump totalcount
  if printvridge defined
  build the set of shared neighbors (i.e., Voronoi vertices)
  call printvridge
*/
int
my_qh_eachvoronoi( FILE * fp,
                   printvridgeT printvridge,
                   vertexT * atvertex,
                   boolT visitall,
                   qh_RIDGE innerouter,
                   boolT inorder )
{
    boolT unbounded;
    int count;
    facetT *neighbor, **neighborp, *neighborA, **neighborAp;
    setT *centers;
    setT *tricenters = qh_settemp(qh TEMPsize);

    vertexT *vertex, **vertexp;
    boolT firstinf;
    unsigned int numfacets= (unsigned int)qh num_facets;
    int totridges= 0;

    qh vertex_visit++;
    atvertex->seen = True;
    if ( visitall )
    {
        FORALLvertices
        {
            vertex->seen = False;
        }
    }

    FOREACHneighbor_( atvertex )
    {
        if ( neighbor->visitid < numfacets )
        {
            neighbor->seen = True;
        }
    }

    FOREACHneighbor_( atvertex )
    {
        if ( neighbor->seen )
        {
            FOREACHvertex_( neighbor->vertices )
            {
                if ( vertex->visitid != qh vertex_visit
                     && !vertex->seen )
                {
                    vertex->visitid = qh vertex_visit;
                    count= 0;
                    firstinf= True;
                    qh_settruncate( tricenters, 0 );
                    FOREACHneighborA_(vertex)
                    {
                        if (neighborA->seen)
                        {
                            if (neighborA->visitid)
                            {
                                if ( ! neighborA->tricoplanar
                                     || qh_setunique(&tricenters, neighborA->center) )
                                {
                                    count++;
                                }
                            }
                            else if ( firstinf )
                            {
                                count++;
                                firstinf= False;
                            }
                        }
                    }

                    if ( count >= qh hull_dim - 1 )
                    {  /* e.g., 3 for 3-d Voronoi */
                        if (firstinf)
                        {
                            if (innerouter == qh_RIDGEouter)
                            {
                                continue;
                            }
                            unbounded= False;
                        }
                        else
                        {
                            if (innerouter == qh_RIDGEinner)
                            {
                                continue;
                            }
                            unbounded = True;
                        }
                        totridges++;
                        trace4((qh ferr, 4017, "qh_eachvoronoi: Voronoi ridge of %d vertices between sites %d and %d\n",
                                count, qh_pointid(atvertex->point), qh_pointid(vertex->point)));
                        if ( printvridge
                             && fp )
                        {
                            if ( inorder
                                 && qh hull_dim == 3+1 ) /* 3-d Voronoi diagram */
                            {
                                centers = qh_detvridge3 (atvertex, vertex);
                            }
                            else
                            {
                                centers = qh_detvridge( vertex );
                            }

                            (*printvridge) ( fp, atvertex, vertex, centers, unbounded );
                            qh_settempfree(&centers);
                        }
                    }
                }
            }
        }
    }

    FOREACHneighbor_(atvertex)
    {
        neighbor->seen= False;
    }

    qh_settempfree(&tricenters);
    return totridges;
} /* eachvoronoi */


/*-<a                             href="qh-io.htm#TOC"
  >-------------------------------</a><a name="printvdiagram2">-</a>

  qh_printvdiagram2( fp, printvridge, vertices, innerouter, inorder )
  visit all pairs of input sites (vertices) for selected Voronoi vertices
  vertices may include NULLs

  innerouter:
  qh_RIDGEall   print inner ridges(bounded) and outer ridges(unbounded)
  qh_RIDGEinner print only inner ridges
  qh_RIDGEouter print only outer ridges

  inorder:
  print 3-d Voronoi vertices in order

  assumes:
  qh_markvoronoi marked facet->visitid for Voronoi vertices
  all facet->seen= False
  all facet->seen2= True

  returns:
  total number of Voronoi ridges
  if printvridge,
  calls printvridge( fp, vertex, vertexA, centers) for each ridge
  [see qh_eachvoronoi()]

  see:
  qh_eachvoronoi_all()
*/
int
my_qh_printvdiagram2( FILE * fp,
                      printvridgeT printvridge,
                      setT * vertices,
                      qh_RIDGE innerouter,
                      boolT inorder )
{
    int totcount = 0;
    int vertex_i, vertex_n;

    vertexT * vertex;
    FORALLvertices
    {
        vertex->seen = False;
    }

    FOREACHvertex_i_( vertices )
    {
        if ( vertex )
        {
            if ( qh GOODvertex > 0
                 && qh_pointid( vertex->point ) + 1 != qh GOODvertex )
            {
                continue;
            }
            totcount += my_qh_eachvoronoi( fp, printvridge, vertex, ! qh_ALL, innerouter, inorder );
        }
    }
    return totcount;
} /* printvdiagram2 */

/*-<a                             href="qh-io.htm#TOC"
  >-------------------------------</a><a name="printvdiagram">-</a>

  qh_printvdiagram( fp, format, facetlist, facets, printall )
  print voronoi diagram
  # of pairs of input sites
  #indices site1 site2 vertex1 ...

  sites indexed by input point id
  point 0 is the first input point
  vertices indexed by 'o' and 'p' order
  vertex 0 is the 'vertex-at-infinity'
  vertex 1 is the first Voronoi vertex

  see:
  qh_printvoronoi()
  qh_eachvoronoi_all()

  notes:
  if all facets are upperdelaunay,
  prints upper hull (furthest-site Voronoi diagram)
*/
void
my_qh_printvdiagram( FILE * fp,
                     qh_PRINT format,
                     facetT * facetlist,
                     setT * facets,
                     boolT printall )
{
    qh_RIDGE innerouter = qh_RIDGEall;
    printvridgeT printvridge = NULL; // function pointer

    if ( format == qh_PRINTvertices )
    {
        innerouter = qh_RIDGEall;
        printvridge = qh_printvridge;
    }
    else if ( format == qh_PRINTinner )
    {
        innerouter = qh_RIDGEinner;
        printvridge = my_qh_printvnorm;
    }
    else if ( format == qh_PRINTouter )
    {
        innerouter = qh_RIDGEouter;
        printvridge = my_qh_printvnorm;
    }
    else
    {
        std::cerr << "Qhull internal error (qh_printvdiagram): unknown print format "
                  << format << std::endl;
    }

    boolT isLower;
    int numcenters;
    setT * vertices = my_qh_markvoronoi( facetlist, facets, printall, &isLower, &numcenters );
    int totcount = my_qh_printvdiagram2( NULL, NULL, vertices, innerouter, False );
    qh_fprintf( fp, 9231, "%d\n", totcount );
    totcount = my_qh_printvdiagram2( fp, printvridge, vertices, innerouter, True /* inorder*/ );

    qh_settempfree( &vertices );
}


void
my_qh_printcenter( FILE *fp,
                   qh_PRINT format,
                   const char *string,
                   facetT * facet )
{
    int num = 0;

    if ( qh CENTERtype != qh_ASvoronoi
         && qh CENTERtype != qh_AScentrum )
    {
        return;
    }

    if ( string )
    {
        qh_fprintf(fp, 9066, string);
    }

    if ( qh CENTERtype == qh_ASvoronoi )
    {
        //std::cerr << "my_qh_printcenter voronoi" << std::endl;
        num = qh hull_dim - 1;
        if ( ! facet->normal
             || ! facet->upperdelaunay
             || ! qh ATinfinity )
        {
            if ( ! facet->center )
            {
                facet->center = qh_facetcenter( facet->vertices );
            }

            for ( int k = 0; k < num; k++ )
            {
                qh_fprintf( fp, 9067, qh_REAL_1, facet->center[k] );
            }
        }
        else
        {
            for ( int k=0; k < num; k++)
            {
                qh_fprintf( fp, 9068, qh_REAL_1, qh_INFINITE );
            }
        }
    }
    else /* qh CENTERtype == qh_AScentrum */
    {
        std::cerr << "my_qh_printcenter centrum" << std::endl;
        num = qh hull_dim;

        if ( format == qh_PRINTtriangles
             && qh DELAUNAY )
        {
            --num;
        }

        if ( ! facet->center )
        {
            facet->center= qh_getcentrum(facet);
        }

        for ( int k = 0; k < num; k++ )
        {
            qh_fprintf(fp, 9069, qh_REAL_1, facet->center[k]);
        }
    }

    if ( format == qh_PRINTgeom && num == 2 )
        qh_fprintf(fp, 9070, " 0\n");
    else
        qh_fprintf(fp, 9071, "\n");
} /* printcenter */


/*-<a                             href="qh-io.htm#TOC"
  >-------------------------------</a><a name="printvoronoi">-</a>

  qh_printvoronoi( fp, format, facetlist, facets, printall )
  print voronoi diagram in 'o' or 'G' format
  for 'o' format
  prints voronoi centers for each facet and for infinity
  for each vertex, lists ids of printed facets or infinity
  assumes facetlist and facets are disjoint
  for 'G' format
  prints an OFF object
  adds a 0 coordinate to center
  prints infinity but does not list in vertices

  see:
  qh_printvdiagram()

  notes:
  if 'o',
  prints a line for each point except "at-infinity"
  if all facets are upperdelaunay,
  reverses lower and upper hull
*/
void
my_qh_printvoronoi( FILE * fp,
                    qh_PRINT format,
                    facetT * facetlist,
                    setT * facets,
                    boolT printall )
{
    int numcenters;
    int numvertices = 0;
    int numneighbors;
    int numinf;
    int vid = 1;
    int vertex_i, vertex_n;
    facetT * facet;
    facetT ** facetp;
    facetT * neighbor;
    facetT ** neighborp;
    boolT isLower;
    unsigned int numfacets = (unsigned int) qh num_facets;

    setT * vertices = my_qh_markvoronoi( facetlist, facets, printall, &isLower, &numcenters );

    vertexT * vertex;
    FOREACHvertex_i_( vertices )
    {
        if ( vertex )
        {
            ++numvertices;
            numneighbors = numinf = 0;
            FOREACHneighbor_( vertex )
            {
                if ( neighbor->visitid == 0 )
                {
                    numinf = 1;
                }
                else if ( neighbor->visitid < numfacets )
                {
                    ++numneighbors;
                }
            }

            if ( numinf
                 && ! numneighbors )
            {
                SETelem_( vertices, vertex_i ) = NULL;
                --numvertices;
            }
        }
    }

    if ( format == qh_PRINTgeom )
    {
        qh_fprintf( fp, 9254, "{appearance {+edge -face} OFF %d %d 1 # Voronoi centers and cells\n",
                    numcenters, numvertices );
    }
    else
    {
        qh_fprintf(fp, 9255, "%d\n%d %d 1\n", qh hull_dim-1, numcenters, qh_setsize(vertices));
    }

    if ( format == qh_PRINTgeom )
    {
        for ( int k = qh hull_dim - 1; k--; )
        {
            qh_fprintf(fp, 9256, qh_REAL_1, 0.0);
        }
        qh_fprintf(fp, 9257, " 0 # infinity not used\n");
    }
    else
    {
        for ( int k = qh hull_dim - 1; k--; )
        {
            qh_fprintf(fp, 9258, qh_REAL_1, qh_INFINITE);
        }
        qh_fprintf(fp, 9259, "\n");
    }

    //std::cerr << "print voronoi vertices" << std::endl;
    FORALLfacet_( facetlist )
    {
        if ( facet->visitid
             && facet->visitid < numfacets )
        {
            if ( format == qh_PRINTgeom )
            {
                qh_fprintf(fp, 9260, "# %d f%d\n", vid++, facet->id);
            }
            my_qh_printcenter( fp, format, NULL, facet );
        }
    }

    //std::cerr << "foreach facets" << std::endl;
    FOREACHfacet_( facets )
    {
        if ( facet->visitid
             && facet->visitid < numfacets )
        {
            if ( format == qh_PRINTgeom )
            {
                qh_fprintf( fp, 9261, "# %d f%d\n", vid++, facet->id );
            }
            my_qh_printcenter( fp, format, NULL, facet );
        }
    }

    //std::cerr << "foreach vertex_i" << std::endl;
    FOREACHvertex_i_( vertices )
    {
        numneighbors = 0;
        numinf = 0;
        if ( vertex )
        {
            if ( qh hull_dim == 3 )
            {
                qh_order_vertexneighbors(vertex);
            }
            else if ( qh hull_dim >= 4 )
            {
                qsort(SETaddr_(vertex->neighbors, vertexT),
                      (size_t)qh_setsize(vertex->neighbors),
                      sizeof(facetT *), qh_compare_facetvisit);
            }

            FOREACHneighbor_( vertex )
            {
                if ( neighbor->visitid == 0 )
                {
                    numinf = 1;
                }
                else if ( neighbor->visitid < numfacets )
                {
                    ++numneighbors;
                }
            }
        }

        if ( format == qh_PRINTgeom )
        {
            if ( vertex )
            {
                qh_fprintf(fp, 9262, "%d", numneighbors);
                if ( vertex )
                {
                    FOREACHneighbor_( vertex )
                    {
                        if ( neighbor->visitid
                             && neighbor->visitid < numfacets )
                        {
                            qh_fprintf( fp, 9263, " %d", neighbor->visitid );
                        }
                    }
                }
                qh_fprintf( fp, 9264, " # p%d(v%d)\n", vertex_i, vertex->id );
            }
            else
            {
                qh_fprintf(fp, 9265, " # p%d is coplanar or isolated\n", vertex_i);
            }
        }
        else
        {
            if ( numinf )
            {
                numneighbors++;
            }

            qh_fprintf( fp, 9266, "%d", numneighbors );
            if ( vertex )
            {
                FOREACHneighbor_( vertex )
                {
                    if ( neighbor->visitid == 0 )
                    {
                        if (numinf)
                        {
                            numinf= 0;
                            qh_fprintf(fp, 9267, " %d", neighbor->visitid);
                        }
                    }
                    else if ( neighbor->visitid < numfacets )
                    {
                        qh_fprintf( fp, 9268, " %d", neighbor->visitid );
                    }
                }
            }
            qh_fprintf( fp, 9269, "\n" );
        }
    }

    if ( format == qh_PRINTgeom )
    {
        qh_fprintf( fp, 9270, "}\n" );
    }

    qh_settempfree( &vertices );
} /* printvoronoi */




/*-<a                             href="qh-io.htm#TOC"
  >-------------------------------</a><a name="printfacets">-</a>

  qh_printfacets( fp, format, facetlist, facets, printall )
  prints facetlist and/or facet set in output format

  notes:
  also used for specialized formats ('FO' and summary)
  turns off 'Rn' option since want actual numbers
*/
void
my_qh_printfacets( FILE * fp,
                   qh_PRINT format,
                   facetT * facetlist,
                   setT * facets,
                   boolT printall )
{
    int numfacets, numsimplicial, numridges, totneighbors, numcoplanars, numtricoplanars;
    facetT * facet;
    facetT ** facetp;
    setT * vertices;
    coordT * center;
    realT outerplane, innerplane;

    qh old_randomdist = qh RANDOMdist;
    qh RANDOMdist = False;

    if ( qh CDDoutput
         && (format == qh_PRINTcentrums
             || format == qh_PRINTpointintersect
             || format == qh_PRINToff ) )
    {
        qh_fprintf(qh ferr, 7056, "qhull warning: CDD format is not available for centrums, halfspace\nintersections, and OFF file format.\n");
    }

    if ( format == qh_PRINTnone )
    {
        //std::cerr << "qh_PRINTnone" << std::endl;; /* print nothing */
    }
    else if ( format == qh_PRINTaverage )
    {
        vertices = qh_facetvertices( facetlist, facets, printall );
        center = qh_getcenter( vertices );
        qh_fprintf( fp, 9186, "%d 1\n", qh hull_dim );
        qh_printpointid( fp, NULL, qh hull_dim, center, -1 );
        qh_memfree( center, qh normal_size );
        qh_settempfree( &vertices );
    }
    else if ( format == qh_PRINTextremes )
    {
        if ( qh DELAUNAY )
        {
            qh_printextremes_d( fp, facetlist, facets, printall );
        }
        else if ( qh hull_dim == 2 )
        {
            qh_printextremes_2d( fp, facetlist, facets, printall );
        }
        else
        {
            qh_printextremes( fp, facetlist, facets, printall );
        }
    }
    else if ( format == qh_PRINToptions )
    {
        qh_fprintf(fp, 9187, "Options selected for Qhull %s:\n%s\n", qh_version, qh qhull_options);
    }
    else if ( format == qh_PRINTpoints
              && ! qh VORONOI )
    {
        qh_printpoints_out(fp, facetlist, facets, printall);
    }
    else if (format == qh_PRINTqhull)
    {
        qh_fprintf(fp, 9188, "%s | %s\n", qh rbox_command, qh qhull_command);
    }
    else if ( format == qh_PRINTsize )
    {
        qh_fprintf(fp, 9189, "0\n2 ");
        qh_fprintf(fp, 9190, qh_REAL_1, qh totarea);
        qh_fprintf(fp, 9191, qh_REAL_1, qh totvol);
        qh_fprintf(fp, 9192, "\n");
    }
    else if ( format == qh_PRINTsummary )
    {
        qh_countfacets( facetlist, facets, printall, &numfacets, &numsimplicial,
                        &totneighbors, &numridges, &numcoplanars, &numtricoplanars );
        vertices= qh_facetvertices(facetlist, facets, printall);
        qh_fprintf( fp, 9193, "10 %d %d %d %d %d %d %d %d %d %d\n2 ", qh hull_dim,
                    qh num_points + qh_setsize(qh other_points),
                    qh num_vertices, qh num_facets - qh num_visible,
                    qh_setsize(vertices), numfacets, numcoplanars,
                    numfacets - numsimplicial, zzval_(Zdelvertextot),
                    numtricoplanars );
        qh_settempfree( &vertices );
        qh_outerinner( NULL, &outerplane, &innerplane );
        qh_fprintf( fp, 9194, qh_REAL_2n, outerplane, innerplane );
    }
    else if ( format == qh_PRINTvneighbors )
    {
        qh_printvneighbors( fp, facetlist, facets, printall );
    }
    else if ( qh VORONOI
              && format == qh_PRINToff )
    {
        std::cerr << "qh VORONOI && format == qh_PRINToff" << std::endl;
        my_qh_printvoronoi( fp, format, facetlist, facets, printall );
    }
    else if ( qh VORONOI
              && format == qh_PRINTgeom )
    {
        std::cerr << "qh VORONOI && format == qh_PRINTgeom" << std::endl;
        qh_printbegin( fp, format, facetlist, facets, printall );
        qh_printvoronoi( fp, format, facetlist, facets, printall );
        qh_printend( fp, format, facetlist, facets, printall );
    }
    else if ( qh VORONOI
              && ( format == qh_PRINTvertices
                   || format == qh_PRINTinner
                   || format == qh_PRINTouter ) )
    {
        std::cerr << " qh VORONOI \n"
                  << " && ( format == qh_PRINTvertices \n"
                  << "     || format == qh_PRINTinner \n"
                  << "     || format == qh_PRINTouter )" << std::endl;
        my_qh_printvdiagram( fp, format, facetlist, facets, printall );
    }
    else
    {
        std::cerr << "other pattern" << std::endl;
        qh_printbegin(fp, format, facetlist, facets, printall);
        FORALLfacet_(facetlist)
            qh_printafacet(fp, format, facet, printall);
        FOREACHfacet_(facets)
            qh_printafacet(fp, format, facet, printall);
        qh_printend(fp, format, facetlist, facets, printall);
    }

    qh RANDOMdist = qh old_randomdist;
} /* printfacets */



void
print_voronoi_output_impl()
{
    std::cerr << "----- print facets ----- " << std::endl;
    for ( int i = 0; i < qh_PRINTEND; ++i )
    {
        std::cerr << "===== call qh_printfacets " << i
                  << " format = " << qh PRINTout[i] << ' '
                  << format_string[ qh PRINTout[i] ]
                  << std::endl;
        //qh_printfacets( qh fout, qh PRINTout[i], qh facet_list, NULL, ! qh_ALL );
        my_qh_printfacets( stdout, qh PRINTout[i], qh facet_list, NULL, ! qh_ALL );
    }
}

void
print_voronoi_output()
{
    int tempsize = qh_setsize( qhmem.tempstack );

    qh_prepare_output();
    print_voronoi_output_impl();

    if ( qh_setsize( qhmem.tempstack ) != tempsize )
    {
        std::cerr << "qhull internal error (qh_produce_output): temporary sets not empty("
                  << qh_setsize( qhmem.tempstack )
                  << ")" << std::endl;
    }
}

void
add_point( std::vector< coordT > & points,
           const double x,
           const double y )
{
    points.push_back( x + noise_2d[noise_index] );
    if ( ++noise_index > 3 ) noise_index = 0;

    points.push_back( y + noise_2d[noise_index] );
    if ( ++noise_index > 3 ) noise_index = 0;
}


int
main()
{
    const int dim = 2;
    const boolT ismalloc = False;
    std::vector< coordT > points;
#if 0
    add_point( points, 0.0, 0.0 );
    add_point( points, -100.0, -100.0 );
    add_point( points, -100.0, +100.0 );
    add_point( points, +100.0, +100.0 );
    add_point( points, +100.0, -100.0 );
#else
    add_point( points, 0.0,   0.0 );
    add_point( points, +10.0, +10.0 );
    add_point( points, -10.0, +10.0 );
    add_point( points, -10.0, -10.0 );
    add_point( points, +10.0, -10.0 );
    add_point( points, +20.0,   0.0 );
    add_point( points,  0.0, +20.0 );
    add_point( points, -20.0,   0.0 );
    add_point( points,  0.0, -20.0 );
#endif
    const int numpoints = points.size() / 2;

    for ( int i = 0; i < numpoints; ++i )
    {
        for ( int j = 0; j < dim; ++j )
        {
            std::cout << points[i*dim + j] << ' ';
        }
        std::cout << std::endl;
    }
    std::cerr << "----------" << std::endl;

    char command[250];
    snprintf( command, 250, "qhull v Qbb o Fi Fo" );

    /*
      http://www.qhull.org/html/qvoronoi.htm

      'qvoronoi' is equivalant to 'qhull v Qbb'

      p : (voronoi vertices)
      __ print the coordinates of the Voronoi vertices.
      __ The first line is the dimension.
      __ The second line is the number of vertices.
      __ Each remaining line is a Voronoi vertex.
      o : (voronoi vertices, voronoi region)
      __ print the Voronoi regions in OFF format.
      __ The first line is the dimension.
      __ The second line is the number of vertices, the number of input sites, and "1".
      __ The third line represents the vertex-at-infinity. Its coordinates are "-10.101".
      __ The next lines are the coordinates of the Voronoi vertices.
      __ Each remaining line starts with the number of Voronoi vertices in a Voronoi region.
      __ In 2-d, the vertices are listed in adjacency order (unoriented).
      Fi :
      __ print separating hyperplanes for inner, bounded Voronoi regions.
      __ The first number is the number of separating hyperplanes.
      __ Each remaining line starts with 3+dim.
      __ The next two numbers are adjacent input sites.
      __ The next 'dim' numbers are the coefficients of the separating hyperplane.
      __ The last number is its offset.
      __ Use 'Tv' to verify that the hyperplanes are perpendicular bisectors.
      __ It will list relevant statistics to stderr.
      Fo :
      __ print separating hyperplanes for outer, unbounded Voronoi regions.
      __ The first number is the number of separating hyperplanes.
      __ Each remaining line starts with 3+dim.
      __ The next two numbers are adjacent input sites on the convex hull.
      __ The next dim numbers are the coefficients of the separating hyperplane.
      __ The last number is its offset.
      __ Use 'Tv' to verify that the hyperplanes are perpendicular bisectors.
      __ It will list relevant statistics to stderr,
      Fv :
      __ list ridges of Voronoi vertices for pairs of input sites.
    */



    int exitcode = 0;
    try
    {
        exitcode = qh_new_qhull( dim, numpoints, &points[0],
                                 ismalloc,
                                 command,
                                 NULL, stderr );
    }
    catch ( ... )
    {
        std::cerr << "catch qhull exception." << std::endl;
        exitcode = 1;
    }

    if ( ! exitcode )
    {
        print_voronoi_output();
        qh NOerrexit = True;
    }

    // free long memory
    qh_freeqhull( ! qh_ALL );

    // free short memory and memory allocator
    int curlong, totlong;
    qh_memfreeshort( &curlong, &totlong );
    if ( curlong || totlong )
    {
        std::cerr << "qhull internal warning: did not free " << totlong
                  << " bytes of long memory (" << curlong << " pieces)"
                  << std::endl;
    }

    return 0;
}
