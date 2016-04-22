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

#ifndef RCSC_ANN_SIRM_H
#define RCSC_ANN_SIRM_H

#include <vector>
#include <string>

namespace rcsc {

class SIRM {
private:
    std::string M_module_name;

    int M_num_partitions;

    double M_max_domain;
    double M_min_domain;
    double M_max_range;
    double M_min_range;
    double M_weight;

    std::vector< double > M_a; //! mean of the antecedent fuzzy set
    std::vector< double > M_b; //! variance of the antecedent fuzzy set
    std::vector< double > M_c; //! consequent output

    // used for training the SIRM
    double M_alpha;
    double M_beta;
    double M_gamma;
    double M_eta;
    std::vector< double > M_membership;
    double M_denominator;
    double M_output;
    double M_input;

    //! generate initial fuzzy rules
    void generateFuzzyRules();

public:

    SIRM();

    void setModuleName( const std::string & module_name );

    //! calculate the output of the rule module for an input value
    double calculateOutput( const double x );

    //! parameter tuning based on gradient decent
    void trainSIRM( const double target,
                    const double actual );

    //! save parameters
    bool saveParameters( const std::string & dirpath );

    //! load parameters
    bool loadParameters( const std::string & dirpath );

    //! show parameters on the screen
    void showParameters();

    void setAlpha( const double alpha );
    void setBeta( const double beta );
    void setGamma( const double gamma );
    void setEta( const double eta );
    void setDomain( const double min_domain,
                    const double max_domain );
    void setRange( const double min_range,
                   const double max_range );
    void setNumPartitions( const int num_partitions );
    void setWeight( const double weight );

    double weight();
};

}

#endif
