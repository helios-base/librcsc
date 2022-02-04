
#include "qhull/src/qhull_a.h"

#include <random>
#include <cstdio>
#include <csetjmp>
#include <iostream>


namespace {
const double noise_2d[4][2] = { { +1.0e-8, +1.0e-8, },
                                { +1.0e-8, -1.0e-8, },
                                { -1.0e-8, -1.0e-8, },
                                { +1.0e-8, -1.0e-8, } };
int noise_index = 0;
}


void
print_summary()
{
    std::cout << qh num_vertices << " vertices and "
              << qh num_facets << " facets with normals:" << std::endl;

    facetT * facet;
    FORALLfacets
    {
        for ( int k = 0; k < qh hull_dim; ++k )
        {
            std::printf( "%6.2g ", facet->normal[k] );
        }
        std::printf( "\n" );
    }
}

#if 0
void
find_delaunay( int dim )
{
    coordT point[100];

    for ( int k = 0; k < dim; ++k )
    {
        point[k] = 0.5;
    }

    qh_setdelaunay( dim + 1, 1, point );

    realT bestdist;
    boolT isoutside;
    facetT * facet;
    facet = qh_findbestfacet( point, qh_ALL, &bestdist, &isoutside );

    if ( facet->tricoplanar )
    {
        std::fprintf( stderr,
                      "find_delaunay: not implemented for triangulated, non-simplicial Delaunay regions (tricoplanar facet, f%d).\n",
                      facet->id );
        qh_errexit( qh_ERRqhull, facet, NULL );
    }

    vertexT *vertex, **vertexp;
    FOREACHvertex_( facet->vertices )
    {
        for ( int k = 0; k < dim; ++k )
        {
            printf ( "%5.2f ", vertex->point[k] );
        }
        printf ( "\n" );
    }
}
#endif

void
print_triangles()
{
    facetT * facet;
    vertexT *vertex, **vertexp;

    FORALLfacets
    {
        if ( ! facet->upperdelaunay )
        {
            std::cout << qh_setsize( facet->vertices ) << ": ";
            FOREACHvertex_( facet->vertices )
            {
                std::cout << qh_pointid( vertex->point ) << " (";
                for ( int d = 0; d < 2; ++d )
                {
                    std::cout << vertex->point[d] << ' ';
                }
                std::cout << ") ";
            }
            std::cout << std::endl;
        }
    }
}


int
main()
{
    std::cout << "test qhull delaunay triangulation" << std::endl;

    const int dim = 2;
    const int numpoints = 4;
    coordT points[dim*numpoints];
    char flags[250];

#if 0
    {
        std::mt19937 engine( 100 );
        stdt::uniform_real_distribution<> dst( -100.0, 100.0 );

        for ( int i = 0; i < numpoints; ++i )
        {
            for ( int j = 0; j < dim; ++j )
            {
                points[i*dim + j] = dst( engine );
            }
        }
    }
#else
    {
        int i = 0;

        points[i++] = -10.0 + noise_2d[noise_index][0];
        points[i++] = -10.0 + noise_2d[noise_index][1];
        noise_index = ( noise_index + 1 ) % 4;

        points[i++] = -10.0 + noise_2d[noise_index][0];
        points[i++] = +10.0 + noise_2d[noise_index][1];
        noise_index = ( noise_index + 1 ) % 4;

        points[i++] = +10.0 + noise_2d[noise_index][0];
        points[i++] = +10.0 + noise_2d[noise_index][1];
        noise_index = ( noise_index + 1 ) % 4;

        points[i++] = +10.0 + noise_2d[noise_index][0];
        points[i++] = -10.0 + noise_2d[noise_index][1];
        noise_index = ( noise_index + 1 ) % 4;
    }
#endif

    for ( int i = 0; i < numpoints; ++i )
    {
        for ( int j = 0; j < dim; ++j )
        {
            std::cout << points[i*dim + j] << ' ';
        }
        std::cout << std::endl;
    }

#if 0
    coordT * rows[numpoints];
    for ( int i = 0; i < numpoints; ++i )
    {
        rows[i] = points + i*dim;
    }

    qh_printmatrix( stdout, "input", rows, numpoints, dim );
#endif

    snprintf( flags, 250, "qhull s d Fv" );
    // p  : output voronoi vertices
    // Fi : output hyperplanes, for inner bounded regions
    // Fo : output hyperplanes, for outer unbounded regions


    //snprintf( flags, 250, "qhull s d Tc" );
    // QJ: joggle the input and it full dimensional
    //   snprintf( flags, 250, "qhull s d Tcv QJ" );
    // QJ: joggle the input and it full dimensional
    //   snprintf( flags, 250, "qhull s d Tcv QJ" );
    // Qz: add a point "at infinity" (i.e., above the paraboloid)

    int exitcode = 0;
    try
    {
        exitcode = qh_new_qhull( dim, numpoints, points,
                                 False, // ismalloc == false
                                 flags,
                                 stdout, stderr );
    }
    catch ( ... )
    {
        std::cerr << "catch qhull exception." << std::endl;
    }


    if  ( ! exitcode ) // if no error
    {
        /* 'qh facet_list' contains the convex hull */
        /*
          If you want a Voronoi diagram ('v') and do not request output (i.e., outfile=NULL),
          call qh_setvoronoi_all() after qh_new_qhull().
        */
        print_summary();
        //         FORALLfacets {
        //             /* ... your code ... */
        //         }
        std::cout << "\nfind " << dim << "-d Delaunay triangle closest to [0.5, 0.5, ...]" << std::endl;
        exitcode = setjmp( qh errexit );
        if ( ! exitcode )
        {
            /*
              Trap Qhull errors in findDelaunay().  Without the setjmp(), Qhull
              will exit() after reporting an error
            */
            qh NOerrexit = False;
            //find_delaunay( dim );
            print_triangles();
        }
        qh NOerrexit = True;
    }

    // free long memory
    qh_freeqhull( ! qh_ALL );

    // free short memory and memory allocator
    int curlong, totlong;
    qh_memfreeshort( &curlong, &totlong );
    if ( curlong || totlong )
    {
        std::fprintf( stderr,
                      "qhull internal warning (user_eg, #1): did not free %d bytes of long memory (%d pieces)\n",
                      totlong, curlong );
    }

    return 0;
}
