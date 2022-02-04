// -*-c++-*-

/*
 *Copyright:

 Copyright (C) Tomoharu NAKASHIMA

 This code is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 3, or (at your option)
 any later version.

 This code is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this code; see the file COPYING.  If not, write to
 the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.

 *EndCopyright:
 */

/////////////////////////////////////////////////////////////////////

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "sirm.h"

#include <cmath>
#include <iostream>
#include <cstdlib>
#include <fstream>

namespace rcsc {

/*-------------------------------------------------------------------*/
/*!

 */
SIRM::SIRM()
    : M_module_name(),
      M_num_partitions( 5 ),
      M_max_domain( 1.0 ),
      M_min_domain( 0.0 ),
      M_max_range( 1.0 ),
      M_min_range( 0.0 ),
      M_weight( 0.25 ), // M_weight( 0.5 ),
      M_alpha( 0.1 ),
      M_beta( 0.1 ),
      M_gamma( 0.01 ),
      M_eta( 0.01 )
{
    generateFuzzyRules();
}

void
SIRM::setModuleName( const std::string & module_name )
{
    M_module_name = module_name;
}

/*-------------------------------------------------------------------*/
/*!

 */
void
SIRM::generateFuzzyRules()
{
    M_a.clear();
    M_b.clear();
    M_c.clear();
    M_membership.clear();

    M_a.resize( M_num_partitions, 0.5 );
    M_b.resize( M_num_partitions, 0.5 );
    M_c.resize( M_num_partitions, 0.0 );
    M_membership.resize( M_num_partitions );

    if ( M_num_partitions <= 0 )
    {
        std::cerr << "Invalid value for num_partitions ( "
                  << M_num_partitions << " ). quit." << std::endl;
        return;
    }
    else if ( M_num_partitions == 1 )
    {
        M_a.push_back( ( M_max_domain - M_min_domain ) / 2.0 );
        M_b.push_back( ( M_max_domain - M_min_domain ) / 4.0 );
        M_c.push_back( ( M_max_range - M_min_range ) / 2.0 );

        return;
    }

    double interval = ( M_max_domain - M_min_domain ) / static_cast< double >( M_num_partitions - 1 );
    for ( int i = 0; i < M_num_partitions; ++i )
    {
        M_a[i] = M_min_domain + interval * static_cast< double >( i );
        M_b[i] = interval / 2.0;
        M_c[i] = 0.0;
        // M_c[i] = ( M_max_range - M_min_range ) / 2.0;
    }
}

/*-------------------------------------------------------------------*/
/*!

 */
void
SIRM::setNumPartitions( const int num_partitions )
{
    M_num_partitions = num_partitions;

    generateFuzzyRules();
}

/*-------------------------------------------------------------------*/
/*!

 */
void
SIRM::setDomain( const double min_domain,
                 const double max_domain )
{
    M_min_domain = min_domain;
    M_max_domain = max_domain;

    generateFuzzyRules();
}

/*-------------------------------------------------------------------*/
/*!

 */
void
SIRM::setRange( const double min_range,
                const double max_range )
{
    M_min_range = min_range;
    M_max_range = max_range;

    generateFuzzyRules();
}

/*-------------------------------------------------------------------*/
/*!

 */
void
SIRM::setAlpha( const double alpha )
{
    M_alpha = alpha;
}

/*-------------------------------------------------------------------*/
/*!

 */
void
SIRM::setBeta( const double beta )
{
    M_beta = beta;
}

/*-------------------------------------------------------------------*/
/*!

 */
void
SIRM::setGamma( const double gamma )
{
    M_gamma = gamma;
}

/*-------------------------------------------------------------------*/
/*!

 */
void
SIRM::setEta( const double eta )
{
    M_eta = eta;
}

/*-------------------------------------------------------------------*/
/*!

 */
double
SIRM::calculateOutput( const double x )
{
    const int max_partitions = M_num_partitions;

    M_input = x;

    double numerator = 0.0;
    M_denominator = 0.0;
    for ( int i = 0; i < max_partitions; ++i )
    {
        M_membership[i] = std::exp( - ( x - M_a[i] ) * ( x - M_a[i] ) / M_b[i] );

        numerator += M_membership[i] * M_c[i];
        M_denominator += M_membership[i];
    }

    M_output = numerator / M_denominator;

    return M_output;
}

/*-------------------------------------------------------------------*/
/*!

 */
void
SIRM::trainSIRM( const double target,
                 const double actual )
{
    double prev_weight = M_weight;

    std::vector< double > prev_a;
    std::vector< double > prev_b;
    std::vector< double > prev_c;
    for ( int i = 0; i < M_num_partitions; ++i )
    {
        prev_a.push_back( M_a[i] );
        prev_b.push_back( M_b[i] );
        prev_c.push_back( M_c[i] );
    }

    // weight
    {
        double delta_output = M_alpha
            * ( target - actual )
            * M_output;
        M_weight += delta_output;
    }

    // the other parameters
    for ( int i = 0; i < M_num_partitions; ++i )
    {
        // c (output)
        double delta_c = M_beta
            * prev_weight * ( target - actual )
            * ( M_membership[i] / M_denominator );
        M_c[i] += delta_c;

        // a (mean)
        double delta_a = M_gamma
            * prev_weight * ( target - actual )
            * ( prev_c[i] - M_output )
            * ( M_membership[i] / M_denominator )
            * ( 2.0 * ( M_input - prev_a[i] ) / prev_b[i] );
        M_a[i] += delta_a;

        // b (variance)
        double delta_b = M_eta
            * prev_weight * ( target - actual )
            * ( prev_c[i] - M_output )
            * ( M_membership[i] / M_denominator )
            * ( ( M_input - prev_a[i] ) / prev_b[i] )
            * ( ( M_input - prev_a[i] ) / prev_b[i] );
        M_b[i] += delta_b;
    }
}

/*-------------------------------------------------------------------*/
/*!

 */
bool
SIRM::saveParameters( const std::string & dirpath )
{
    if ( M_module_name.empty() )
    {
        std::cerr << __FILE__ << ' ' << __LINE__
                  << ": (SIRM::saveParameters) Empty module name!" << std::endl;
        return false;
    }

    std::string filepath = dirpath;
    if ( ! filepath.empty()
         && *filepath.rbegin() != '/' )
    {
        filepath += '/';
    }
    filepath += M_module_name;
    filepath += ".sirm";

    std::ofstream ofs( filepath.c_str() );
    if ( ! ofs )
    {
        std::cerr << __FILE__ << ' ' << __LINE__
                  << ": Cannot open the parameter file [" << filepath << "]" << std::endl;
        return false;
    }

    ofs << M_num_partitions << std::endl;
    for ( int i = 0; i < M_num_partitions; ++i )
    {
        ofs << M_a[i] << ' ' << M_b[i] << ' ' << M_c[i] << '\n';
    }

    ofs.flush();
    ofs.close();

    return true;
}

/*-------------------------------------------------------------------*/
/*!

 */
bool
SIRM::loadParameters( const std::string & dirpath )
{
    if ( M_module_name.empty() )
    {
        std::cerr << __FILE__ << ' ' << __LINE__ << ": (SIRM::loadParameters) "
                  << "Empty module name!" << std::endl;
        return false;
    }

    std::string filepath = dirpath;
    if ( ! filepath.empty()
         && *filepath.rbegin() != '/' )
    {
        filepath += '/';
    }
    filepath += M_module_name;
    filepath += ".sirm";

    std::ifstream ifs( filepath.c_str() );
    if ( ! ifs )
    {
        std::cerr << __FILE__ << ' ' << __LINE__ << ": (SIRM::loadParameters) "
                  << "Cannot open the parameter file [" << filepath << "]" << std::endl;
        generateFuzzyRules();
        return false;
    }

    {
        std::string line_buf;
        if ( ! std::getline( ifs, line_buf ) )
        {
            generateFuzzyRules();
            return false;
        }

        int num = 0;
        if ( std::sscanf( line_buf.c_str(), " %d ", &num ) != 1 )
        {
            std::cerr << __FILE__ << ' ' << __LINE__ << ": (SIRM::loadParameters) "
                      << "Illegal partition size [" << line_buf << "]"
                      << " file=[" << filepath << "]" << std::endl;
            generateFuzzyRules();
            return false;
        }
        M_num_partitions = num;
    }

    int n_read = 0;
    for ( int i = 0; i < M_num_partitions; ++i )
    {
        std::string line_buf;
        if ( ! std::getline( ifs, line_buf ) )
        {
            break;
        }

        if ( std::sscanf( line_buf.c_str(), " %lf %lf %lf ",
                          &M_a[i], &M_b[i], &M_c[i] ) != 3 )
        {
            std::cerr << __FILE__ << ' ' << __LINE__ << ": (SIRM::loadParameters) "
                      << "Illegal line " << i + 2
                      << " [" << line_buf << "]"
                      << " file=[" << filepath << "]" << std::endl;
            break;
        }
        ++n_read;
    }

    ifs.close();

    if ( n_read != M_num_partitions )
    {
        std::cerr << __FILE__ << ' ' << __LINE__ << ": (SIRM::loadParameters) "
                  << "Illegal parameter size. generate default parameters. " << std::endl;
        generateFuzzyRules();
    }

    return true;
}

/*-------------------------------------------------------------------*/
/*!

 */
void
SIRM::showParameters()
{
    std::cout << M_num_partitions << '\n';
    for ( int i = 0; i < M_num_partitions; ++i )
    {
        std::cout << M_a[i] << " " << M_b[i] << " " << M_c[i] << '\n';
    }
    std::cout << std::flush;
}

/*-------------------------------------------------------------------*/
/*!

 */
double
SIRM::weight()
{
    return M_weight;
}

/*-------------------------------------------------------------------*/
/*!

 */
void
SIRM::setWeight( const double weight )
{
    M_weight = weight;
}

}
