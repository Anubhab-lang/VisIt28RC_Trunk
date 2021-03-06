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
//                           avtRevolvedSurfaceArea.C                        //
// ************************************************************************* //

#include <avtRevolvedSurfaceArea.h>

#include <float.h>
#include <math.h>

#include <vtkCell.h>
#include <vtkCellData.h>
#include <vtkDataArray.h>
#include <vtkDataSet.h>
#include <vtkDataSetRemoveGhostCells.h>
#include <vtkGeometryFilter.h>
#include <vtkIntArray.h>
#include <vtkPoints.h>
#include <vtkPolyData.h>
#include <vtkStreamingDemandDrivenPipeline.h>
#include <vtkVisItFeatureEdges.h>

#include <avtCallback.h>

#include <DebugStream.h>
#include <InvalidDimensionsException.h>

static double AreaOfCone(double, double, double);


// ****************************************************************************
//  Method: avtRevolvedSurfaceArea constructor
//
//  Programmer: Hank Childs
//  Creation:   September 8, 2002
//
//  Modifications:
//    Kathleen Bonnell, Tue Feb  7 14:14:33 PST 2006
//    Added revolveAboutX. 
//
// ****************************************************************************

avtRevolvedSurfaceArea::avtRevolvedSurfaceArea()
{
    haveIssuedWarning = false;
    revolveAboutX = true;
}


// ****************************************************************************
//  Method: avtRevolvedSurfaceArea::PreExecute
//
//  Purpose:
//      Sets up a data member that ensures that we don't issue multiple
//      warnings.
//
//  Programmer: Hank Childs
//  Creation:   September 8, 2002
//
//  Modifications:
//    Kathleen Bonnell, Tue Feb  7 14:14:33 PST 2006
//    Added revolveAboutX. 
//
// ****************************************************************************

void
avtRevolvedSurfaceArea::PreExecute(void)
{
    avtSingleInputExpressionFilter::PreExecute();

    avtDataAttributes &atts = GetInput()->GetInfo().GetAttributes();
    if (atts.GetSpatialDimension() != 2)
    {
        EXCEPTION2(InvalidDimensionsException, "Revolved surface area",
                                               "2-dimensional");
    }
    haveIssuedWarning = false;
    if (atts.GetMeshCoordType() == AVT_ZR)
        revolveAboutX = false;
    else 
        revolveAboutX = true;
}


// ****************************************************************************
//  Method: avtRevolvedSurfaceArea::DeriveVariable
//
//  Purpose:
//      Determines the volume each cell would take if it were revolved around
//      the axis y=0.
//
//  Programmer: Hank Childs
//  Creation:   September 8, 2002
//
//  Modifications:
//    Jeremy Meredith, Thu Jul 24 13:14:32 PDT 2003
//    Added code to get the boundary edges before calculating the
//    revolved surface area.
//
//    Hank Childs, Tue Mar 30 11:00:35 PST 2004
//    The variable created must have the same number of tuples as the input
//    dataset.
//
//    Hank Childs, Mon Aug 30 17:09:57 PDT 2004
//    Remove call to SetGhostLevel for vtkDataSetRemoveGhostCells filter.
//
//    Kathleen Bonnell, Tue Feb  7 14:14:33 PST 2006
//    Changed GetLineArea call to GetCellArea. 
//
//    Kathleen Bonnell, Tue May 16 09:41:46 PDT 2006 
//    Removed call to SetSource(NULL), with new vtk pipeline, it also removes
//    necessary information from the dataset. 
//
//    Mark C. Miller, Wed Aug 22 09:43:01 PDT 2012
//    Fixed leaks of VTK objects from early return.
//
//    Kathleen Biagas, Fri Jan 25 16:30:50 PST 2013
//    Call Update on filter not data object.
//   
// ****************************************************************************

vtkDataArray *
avtRevolvedSurfaceArea::DeriveVariable(vtkDataSet *in_ds, int currentDomainsIndex)
{
    //
    // Create a copy of the input with each zone's id number.  This will be 
    // used to match up the line segments with the zones they came from later.
    //
    vtkDataSet *tmp_ds = in_ds->NewInstance();
    tmp_ds->ShallowCopy(in_ds);
    vtkIdType n_orig_cells = tmp_ds->GetNumberOfCells();
    vtkIntArray *iarray = vtkIntArray::New();
    iarray->SetName("_rsa_ncells");
    iarray->SetNumberOfTuples(n_orig_cells);
    for (vtkIdType i = 0 ; i < n_orig_cells ; i++)
        iarray->SetValue(i, i);
    tmp_ds->GetCellData()->AddArray(iarray);
    iarray->Delete();

    //
    // Create the boundary edges.
    //
    vtkGeometryFilter *geomFilter = vtkGeometryFilter::New();
    vtkVisItFeatureEdges *boundaryFilter = vtkVisItFeatureEdges::New();
    vtkPolyData *allLines = NULL;

    avtDataAttributes &atts = GetInput()->GetInfo().GetAttributes();
    if (atts.GetTopologicalDimension() == 2)
    {
        geomFilter->SetInputData(tmp_ds);
        boundaryFilter->BoundaryEdgesOn();
        boundaryFilter->FeatureEdgesOff();
        boundaryFilter->NonManifoldEdgesOff();
        boundaryFilter->ManifoldEdgesOff();
        boundaryFilter->ColoringOff();
    
        boundaryFilter->SetInputConnection(geomFilter->GetOutputPort());
        vtkStreamingDemandDrivenPipeline::SetUpdateGhostLevel(boundaryFilter->GetInformation(), 2);
        boundaryFilter->Update();

        allLines = boundaryFilter->GetOutput();
    }
    else if (tmp_ds->GetDataObjectType() == VTK_POLY_DATA)
    {
        allLines = (vtkPolyData *) tmp_ds;
    }
    else
    {
        geomFilter->SetInputData(tmp_ds);
        allLines = geomFilter->GetOutput();
    }

    //
    // Remove ghost zones.
    //
    vtkDataSetRemoveGhostCells *gzFilter = vtkDataSetRemoveGhostCells::New();
    gzFilter->SetInputData(allLines);
    gzFilter->Update();
    vtkDataSet *ds_1d_nogz = gzFilter->GetOutput();

    // We need line segment polydata, and should have it by now.
    if (ds_1d_nogz->GetDataObjectType() != VTK_POLY_DATA)
    {
        tmp_ds->Delete();
        geomFilter->Delete();
        gzFilter->Delete();
        boundaryFilter->Delete();
        debug1 << "ERROR:Did not get polydata from ghost zone filter output\n";
        return NULL;
    }
    vtkPolyData *pd_1d_nogz = (vtkPolyData*)ds_1d_nogz;

    //
    // Only some of the zones are actually external to the domain and
    // contribute to the resolved surface area.  These zones are exactly
    // the zones in pd_1d_nogz.  But we need to create an output that is
    // sized for the input mesh.  So construct an array with all 0's (the
    // zones that aren't on the exterior contribute 0 surface area) and then
    // add in the contributions from the zones on the exterior.
    //
    vtkDataArray *arr = CreateArrayFromMesh(in_ds);
    arr->SetNumberOfTuples(n_orig_cells);
    for (vtkIdType i = 0 ; i < n_orig_cells ; i++)
        arr->SetTuple1(i, 0.);

    vtkIdType ncells = pd_1d_nogz->GetNumberOfCells();
    vtkIntArray *orig_cells = (vtkIntArray *)
                            pd_1d_nogz->GetCellData()->GetArray("_rsa_ncells");
    for (vtkIdType i = 0 ; i < ncells ; i++)
    {
        vtkCell *cell = pd_1d_nogz->GetCell(i);
        double area = GetCellArea(cell);
        int orig_cell = orig_cells->GetValue(i);
        double orig_area = arr->GetTuple1(orig_cell);
        double new_area = area + orig_area;
        arr->SetTuple1(orig_cell, new_area);
    }

    //
    // Clean up.
    //
    tmp_ds->Delete();
    geomFilter->Delete();
    gzFilter->Delete();
    boundaryFilter->Delete();

    return arr;
}


// ****************************************************************************
//  Method: avtRevolvedSurfaceArea::GetCellArea
//
//  Purpose:
//      Tests that cell is a line, and if so calls GetLineArea. 

//  Arguments:
//      cell   The input cell.
//      y      The y-coordinates of the input line.
//
//  Returns:    The surface area of the revolved cell, 0 if the cell
//              is not a line. 
//
//  Programmer: Kathleen Bonnell 
//  Creation:   February 7, 2006 
//
// ****************************************************************************
 
double
avtRevolvedSurfaceArea::GetCellArea(vtkCell *cell)
{
    int cellType = cell->GetCellType();
    if (cellType != VTK_LINE)
    {
        if (!haveIssuedWarning)
        {
           char msg[1024];
           sprintf(msg, "The revolved surface area is only supported for "
                        " line segments.  %d is an invalid cell type.",
                         cellType);
           avtCallback::IssueWarning(msg);
        }
        haveIssuedWarning = true;
        return 0.;
    }

    vtkPoints *pts = cell->GetPoints();
    double p0[3];
    double p1[3];
    pts->GetPoint(0, p0);
    pts->GetPoint(1, p1);
    double  x[2], y[2];
    x[0] = p0[0];
    x[1] = p1[0];
    y[0] = p0[1];
    y[1] = p1[1];
    double area;
    if (revolveAboutX)
        area = GetLineArea(x, y);
    else
        area = GetLineArea(y, x);
    return area;
}

// ****************************************************************************
//  Method: avtRevolvedSurfaceArea::GetLineArea
//
//  Purpose:
//      Revolve the zone around the line y = 0.  This is done by making
//      two cones -- one that intersects the line y = 0 and one that is
//      the requested line segment, but extended to the line y = 0.  Then
//      we can difference them and get the surface area for just this cell.
//
//  Arguments:
//      x      The x-coordinates of the input line.
//      y      The y-coordinates of the input line.
//
//  Returns:    The surface area of the revolved line segment.
//
//  Programmer: Hank Childs
//  Creation:   March 18, 2003
//
//  Modifications:
//    Kathleen Bonnell, Tue Feb  7 14:14:33 PST 2006
//    Changed args from vtkCell to x and y coords of the line.
//    Moved cell-is-line test to GetCellArea method.
//
// ****************************************************************************
 
double
avtRevolvedSurfaceArea::GetLineArea(double x[2], double y[2])
{
    //
    // It's easier to think about this if we know x[0] < x[1]
    //
    if (x[1] < x[0])
    {
        double tmp;
        tmp = x[0]; x[0] = x[1]; x[1] = tmp;
        tmp = y[0]; y[0] = y[1]; y[1] = tmp;
    }

#if defined(_WIN32) && !defined(M_PI)
#define M_PI 3.141592653589793
#endif

    //
    // Make sure we are not dealing with a vertical line.
    //
    if (x[0] == x[1])
    {
        // Degenerate case -- this is a flat doughnut.
        if (y[1] < y[0])
        {
            double tmp;
            tmp = y[0]; y[0] = y[1]; y[1] = tmp;
        }
        double outer_circle = M_PI*y[1]*y[1];
        double inner_circle = M_PI*y[0]*y[0];
        return outer_circle - inner_circle;
    }

    double slope = (y[1] - y[0]) / (x[1] - x[0]);

    //
    // Make sure we are not dealing with a horizontal line.
    //
    if (slope == 0.)
    {
        // Degenerate case -- this is a cylinder.
        double height = x[1] - x[0];
        double radius = y[0];
        return 2*M_PI*radius*height;
    }

    //
    // Calculate the distance to the line y=0.
    //
    bool Pt0IsCloser = (fabs(y[0]) < fabs(y[1]));

    double rv = 0.;
    if (Pt0IsCloser)
    {
        double axis_intersection = (slope*x[1] - y[1]) / slope;
        double area_of_small_cone =
                    AreaOfCone(axis_intersection, x[0], y[0]);
        double area_of_big_cone =
                    AreaOfCone(axis_intersection, x[1], y[1]);
        rv = area_of_big_cone - area_of_small_cone;
    }
    else
    {
        double axis_intersection = (slope*x[1] - y[1]) / slope;
        double area_of_small_cone =
                    AreaOfCone(axis_intersection, x[1], y[1]);
        double area_of_big_cone =
                    AreaOfCone(axis_intersection, x[0], y[0]);
        rv = area_of_big_cone - area_of_small_cone;
    }

    return rv;
}


// ****************************************************************************
//  Function: AreaOfCone
// 
//  Purpose:
//      Returns the surface area obtained when revolving a line segment
//      around the y-axis.
// 
//  Arguments:
//      x0      The x-coordinate of the first point.  The y-coordinate for this
//              point is assumed to be 0.;
//      x1      The x-coordinate of the second point.
//      y1      The y-coordinate of the second point.
// 
//  Returns:    The surface area of the revolved line segment.
// 
//  Programmer: Hank Childs
//  Creation:   March 18, 2003
// 
// ****************************************************************************
 
static double
AreaOfCone(double x0, double x1, double y1)
{
    //
    // The formula for the surface area of a cone is:
    // A = pi*r*p, where p = sqrt(r*r + h*h)
    //
    // A useful website for this is: 
    //  http://www.rdrop.com/~half/Creations/Puzzles/ConeArea/
    //
    double r = y1 - 0.;
    double h = (x1 - x0);
    double p = sqrt(r*r + h*h);
    return M_PI*r*p;
}


