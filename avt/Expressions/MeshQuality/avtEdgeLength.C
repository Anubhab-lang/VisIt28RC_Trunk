/*****************************************************************************
*
* Copyright (c) 2000 - 2014, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-442911
* All rights reserved.
*
* This file is  part of VisIt. For  details, see https://visit.llnl.gov/.  The
* full copyright notice is contained in the file COPYRIGHT located at the root
* of the VisIt distribution or at http://www.llnl.gov/visit/copyright.html.
*
* Redistribution  and  use  in  source  and  binary  forms,  with  or  without
* modification, are permitted provided that the following conditions are met:
*
*  - Redistributions of  source code must  retain the above  copyright notice,
*    this list of conditions and the disclaimer below.
*  - Redistributions in binary form must reproduce the above copyright notice,
*    this  list of  conditions  and  the  disclaimer (as noted below)  in  the
*    documentation and/or other materials provided with the distribution.
*  - Neither the name of  the LLNS/LLNL nor the names of  its contributors may
*    be used to endorse or promote products derived from this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED. IN  NO EVENT  SHALL LAWRENCE  LIVERMORE NATIONAL  SECURITY,
* LLC, THE  U.S.  DEPARTMENT OF  ENERGY  OR  CONTRIBUTORS BE  LIABLE  FOR  ANY
* DIRECT,  INDIRECT,   INCIDENTAL,   SPECIAL,   EXEMPLARY,  OR   CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/

// ************************************************************************* //
//                               avtEdgeLength.C                             //
// ************************************************************************* //

#include <avtEdgeLength.h>

#include <float.h>

#include <vtkCell.h>
#include <vtkDataSet.h>
#include <vtkPoints.h>

#include <avtCallback.h>

#include <InvalidDimensionsException.h>


// ****************************************************************************
//  Method: avtEdgeLength constructor
//
//  Programmer: Hank Childs
//  Creation:   September 22, 2005
//
// ****************************************************************************

avtEdgeLength::avtEdgeLength()
{
    takeMin = true;
}


// ****************************************************************************
//  Method: avtEdgeLength::DeriveVariable
//
//  Purpose:
//      Find the minimum or maximum edge length.
//
//  Programmer: Hank Childs
//  Creation:   September 22, 2005
//
// ****************************************************************************

vtkDataArray *
avtEdgeLength::DeriveVariable(vtkDataSet *in_ds, int currentDomainsIndex)
{
    vtkDataArray *arr = CreateArrayFromMesh(in_ds);
    vtkIdType ncells = in_ds->GetNumberOfCells();
    arr->SetNumberOfTuples(ncells);

    for (vtkIdType i = 0 ; i < ncells ; i++)
    {
        vtkCell *cell = in_ds->GetCell(i);
        double vol = GetEdgeLength(cell);
        arr->SetTuple1(i, vol);
    }

    return arr;
}


// ****************************************************************************
//  Method: avtEdgeLength::GetEdgeLength
//
//  Purpose:
//      Evaluate the length of each edge and determine the minimum or
//      maximum.
//
//  Arguments:
//      cell    The input zone.
//
//  Returns:    The side volume of the zone.
//
//  Programmer: Hank Childs
//  Creation:   September 22, 2005
//
//  Modifications:
//
//    Hank Childs, Fri Jun  9 14:34:50 PDT 2006
//    Removed unused variable.
//
//    Hank Childs, Fri Nov  3 16:05:12 PST 2006
//    Add support for actual edges.  Also have vertices return 0.
//
// ****************************************************************************
 
double
avtEdgeLength::GetEdgeLength(vtkCell *cell)
{
    int celltype = cell->GetCellType();
    if (celltype == VTK_VERTEX)
        return 0.;

    if (celltype == VTK_LINE)
    {
        double pt1[3], pt2[3], a[3];

        vtkPoints *pts = cell->GetPoints();
        pts->GetPoint(0, pt1);
        pts->GetPoint(1, pt2);
        a[0] = pt2[0] - pt1[0]; 
        a[1] = pt2[1] - pt1[1]; 
        a[2] = pt2[2] - pt1[2]; 

        double length = sqrt(a[0]*a[0] + a[1]*a[1] + a[2]*a[2]);
        return length;
    }

    //
    // Calculate the value of each edge and then return the minimum or maximum.
    //
    int nEdges = cell->GetNumberOfEdges();
    double rv = +FLT_MAX;
    if (!takeMin)
        rv = -FLT_MAX;
    for (int i = 0 ; i < nEdges ; i++)
    {
        vtkCell *edge = cell->GetEdge(i);
        vtkPoints *pts = edge->GetPoints();

        double pt1[3], pt2[3], a[3];

        pts->GetPoint(0, pt1);
        pts->GetPoint(1, pt2);

        a[0] = pt2[0] - pt1[0]; 
        a[1] = pt2[1] - pt1[1]; 
        a[2] = pt2[2] - pt1[2]; 

        double length = sqrt(a[0]*a[0] + a[1]*a[1] + a[2]*a[2]);
        if (takeMin)
            rv = (rv < length ? rv : length);
        else
            rv = (rv > length ? rv : length);
    }

    return rv;
}


