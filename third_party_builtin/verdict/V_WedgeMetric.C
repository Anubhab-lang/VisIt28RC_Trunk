
/*
 *
 * WedgeMetric.cpp contains quality calculations for wedges
 *
 * Copyright (C) 2003 Sandia National Laboratories <cubit@sandia.gov>
 *
 * This file is part of VERDICT
 *
 * This copy of VERDICT is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * VERDICT is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */


#include "verdict.h"
#include "VerdictVector.h"
#include <memory.h> 

/*
  the wedge element


        5
        ^
       / \  
      / | \   
     / /2\ \       
   6/_______\4
    | /   \ |
    |/_____\|
   3         1
  
*/



/*!
  
  calculate the volume of a wedge

  this is done by dividing the wedge into 3 tets
  and summing the volume of each tet

*/

C_FUNC_DEF VERDICT_REAL v_wedge_volume( int num_nodes, VERDICT_REAL coordinates[][3] )
{

  double volume = 0;
  VerdictVector side1, side2, side3;

  if ( num_nodes == 6 )
  {

    // divide the wedge into 3 tets and calculate each volume

    side1.set( coordinates[1][0] - coordinates[0][0],
               coordinates[1][1] - coordinates[0][1],
               coordinates[1][2] - coordinates[0][2]);

    side2.set( coordinates[2][0] - coordinates[0][0],
               coordinates[2][1] - coordinates[0][1],
               coordinates[2][2] - coordinates[0][2]);


    side3.set( coordinates[3][0] - coordinates[0][0],
               coordinates[3][1] - coordinates[0][1],
               coordinates[3][2] - coordinates[0][2]);

    volume = side3 % (side1 * side2)  / 6;

    side1.set( coordinates[4][0] - coordinates[1][0],
               coordinates[4][1] - coordinates[1][1],
               coordinates[4][2] - coordinates[1][2]);

    side2.set( coordinates[5][0] - coordinates[1][0],
               coordinates[5][1] - coordinates[1][1],
               coordinates[5][2] - coordinates[1][2]);


    side3.set( coordinates[3][0] - coordinates[1][0],
               coordinates[3][1] - coordinates[1][1],
               coordinates[3][2] - coordinates[1][2]);

    volume += side3 % (side1 * side2)  / 6;

    side1.set( coordinates[5][0] - coordinates[1][0],
               coordinates[5][1] - coordinates[1][1],
               coordinates[5][2] - coordinates[1][2]);

    side2.set( coordinates[2][0] - coordinates[1][0],
               coordinates[2][1] - coordinates[1][1],
               coordinates[2][2] - coordinates[1][2]);


    side3.set( coordinates[3][0] - coordinates[1][0],
               coordinates[3][1] - coordinates[1][1],
               coordinates[3][2] - coordinates[1][2]);

    volume += side3 % (side1 * side2)  / 6;

  }

  return (VERDICT_REAL)volume;

}



C_FUNC_DEF void v_wedge_quality( int num_nodes, VERDICT_REAL coordinates[][3], 
    unsigned int metrics_request_flag, WedgeMetricVals *metric_vals )
{
  memset( metric_vals, 0, sizeof(WedgeMetricVals) );

  if(metrics_request_flag & V_WEDGE_VOLUME)
    metric_vals->volume = v_wedge_volume(num_nodes, coordinates);
}

