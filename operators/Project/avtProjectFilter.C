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
//  File: avtProjectFilter.C
// ************************************************************************* //

#include <avtProjectFilter.h>

#include <vtkCell.h>
#include <vtkCellData.h>
#include <vtkPointData.h>
#include <vtkRectilinearGrid.h>
#include <vtkStructuredGrid.h>
#include <vtkUnstructuredGrid.h>
#include <vtkPolyData.h>

#include <avtDatasetExaminer.h>
#include <avtExtents.h>
#include <vtkVisItUtility.h>

#include <ImproperUseException.h>


// ****************************************************************************
//  Method: avtProjectFilter constructor
//
//  Programmer: Jeremy Meredith
//  Creation:   September  3, 2004
//
// ****************************************************************************

avtProjectFilter::avtProjectFilter()
{
}


// ****************************************************************************
//  Method: avtProjectFilter destructor
//
//  Programmer: Jeremy Meredith
//  Creation:   September  3, 2004
//
//  Modifications:
//
// ****************************************************************************

avtProjectFilter::~avtProjectFilter()
{
}


// ****************************************************************************
//  Method:  avtProjectFilter::Create
//
//  Programmer: Jeremy Meredith
//  Creation:   September  3, 2004
//
// ****************************************************************************

avtFilter *
avtProjectFilter::Create()
{
    return new avtProjectFilter();
}


// ****************************************************************************
//  Method:      avtProjectFilter::SetAtts
//
//  Purpose:
//      Sets the state of the filter based on the attribute object.
//
//  Arguments:
//      a        The attributes to use.
//
//  Programmer: Jeremy Meredith
//  Creation:   September  3, 2004
//
// ****************************************************************************

void
avtProjectFilter::SetAtts(const AttributeGroup *a)
{
    atts = *(const ProjectAttributes*)a;
}


// ****************************************************************************
//  Method: avtProjectFilter::Equivalent
//
//  Purpose:
//      Returns true if creating a new avtProjectFilter with the given
//      parameters would result in an equivalent avtProjectFilter.
//
//  Programmer: Jeremy Meredith
//  Creation:   September  3, 2004
//
// ****************************************************************************

bool
avtProjectFilter::Equivalent(const AttributeGroup *a)
{
    return (atts == *(ProjectAttributes*)a);
}


// ****************************************************************************
//  Method: avtProjectFilter::ExecuteData
//
//  Purpose:
//      Sends the specified input and output through the Project filter.
//
//  Arguments:
//      in_dr      The input data representation.
//
//  Returns:       The output data representation.
//
//  Programmer: Jeremy Meredith
//  Creation:   September  3, 2004
//
//  Modifications:
//    Jeremy Meredith, Fri Sep 10 16:15:55 PDT 2004
//    Added projection of vectors.
//
//    Hank Childs, Tue Aug  3 09:28:35 PDT 2010
//    Fix bug with project of cell-centered vectors.
//
//    Kathleen Biagas, Wed Aug  8 17:31:54 PDT 2012
//    Use templatized ProjectVectors.
//
//    Eric Brugger, Thu Jul 31 14:44:16 PDT 2014
//    Modified the class to work with avtDataRepresentation.
//
// ****************************************************************************

avtDataRepresentation *
avtProjectFilter::ExecuteData(avtDataRepresentation *in_dr)
{
    //
    // Get the VTK data set.
    //
    vtkDataSet *in_ds = in_dr->GetDataVTK();

    int  datatype = in_ds->GetDataObjectType();
    vtkPointSet *out_ds = NULL;
    switch (datatype)
    {
      case VTK_RECTILINEAR_GRID:
        out_ds = ProjectRectilinearGrid((vtkRectilinearGrid*)in_ds);
        break;

      case VTK_STRUCTURED_GRID:
      case VTK_UNSTRUCTURED_GRID:
      case VTK_POLY_DATA:
        out_ds = ProjectPointSet((vtkPointSet*)in_ds);
        break;

      default:
        EXCEPTION0(ImproperUseException);
    }

    //
    // We have said normals are inappropriate, since this will be a
    // 2D data set.  We have not touched the vectors, however --
    // we have only projected the mesh itself so far.
    //
    vtkDataArray *vectors;
    vectors = out_ds->GetPointData()->GetVectors();
    if (vectors)
    {
        vtkDataArray *arr = vectors->NewInstance();
        arr->SetNumberOfComponents(3);
        arr->Allocate(3*vectors->GetNumberOfTuples());
        if (arr->GetDataType() == VTK_FLOAT)
            ProjectVectors<float>(in_ds, out_ds, vectors, arr, false);
        else if (arr->GetDataType() == VTK_DOUBLE)
            ProjectVectors<double>(in_ds, out_ds, vectors, arr, false);
        arr->SetName(vectors->GetName());
        out_ds->GetPointData()->RemoveArray(vectors->GetName());
        out_ds->GetPointData()->SetVectors(arr);
        arr->Delete();
    }
    vectors = out_ds->GetCellData()->GetVectors();
    if (vectors)
    {
        vtkDataArray *arr = vectors->NewInstance();
        arr->SetNumberOfComponents(3);
        arr->Allocate(3*vectors->GetNumberOfTuples());
        if (arr->GetDataType() == VTK_FLOAT)
            ProjectVectors<float>(in_ds, out_ds, vectors, arr, true);
        else if (arr->GetDataType() == VTK_DOUBLE)
            ProjectVectors<double>(in_ds, out_ds, vectors, arr, true);
        arr->SetName(vectors->GetName());
        out_ds->GetCellData()->RemoveArray(vectors->GetName());
        out_ds->GetCellData()->SetVectors(arr);
        arr->Delete();
    }

    avtDataRepresentation *out_dr = new avtDataRepresentation(out_ds,
        in_dr->GetDomain(), in_dr->GetLabel());

    out_ds->Delete();

    return out_dr;
}


// ****************************************************************************
//  Method: avtProjectFilter::PostExecute
//
//  Purpose:
//      Finds the extents once we have been projected and set those.
//
//  Programmer: Hank Childs
//  Creation:   January 20, 2005
//
//   Modifications:
//
//    Hank Childs, Thu Aug 26 13:47:30 PDT 2010
//    Change extents names.
//
// ****************************************************************************

void
avtProjectFilter::PostExecute(void)
{
    avtPluginDataTreeIterator::PostExecute();

    avtDataAttributes& outAtts = GetOutput()->GetInfo().GetAttributes();

    // get the outputs's spatial extents
    double se[6];
    avtDataset_p output = GetTypedOutput();
    avtDatasetExaminer::GetSpatialExtents(output, se);

    // over-write spatial extents
    outAtts.GetOriginalSpatialExtents()->Clear();
    outAtts.GetThisProcsOriginalSpatialExtents()->Set(se);
}


// ****************************************************************************
//  Method:  avtProjectFilter::ProjectPoint
//
//  Purpose:
//    Project a single point in-place.
//
//  Arguments:
//    x,y,z      the point to project
//
//  Programmer:  Jeremy Meredith
//  Creation:    September  3, 2004
//
// ****************************************************************************
void
avtProjectFilter::ProjectPoint(float &x,float &y,float &z)
{
    switch (atts.GetProjectionType())
    {
      case ProjectAttributes::ZYCartesian:
        x = z;
        y = y;
        z = 0;
        break;

      case ProjectAttributes::XZCartesian:
        x = x;
        y = z;
        z = 0;
        break;

      case ProjectAttributes::XYCartesian:
        x = x;
        y = y;
        z = 0;
        break;

      case ProjectAttributes::XRCylindrical:
        {
            float r = sqrt(y*y + z*z);
            x = x;
            y = r;
            z = 0;
        }
        break;
      case ProjectAttributes::YRCylindrical:
        {
            float r = sqrt(x*x + z*z);
            x = y;
            y = r;
            z = 0;
        }
        break;

      case ProjectAttributes::ZRCylindrical:
        {
            float r = sqrt(x*x + y*y);
            x = z;
            y = r;
            z = 0;
        }
        break;
    }
}

// ****************************************************************************
//  Method:  avtProjectFilter::ProjectPoint
//
//  Purpose:
//    Project a single point in-place.  Double precision copy version.
//
//  Arguments:
//    x,y,z      the point to project
//
//  Programmer:  Jeremy Meredith
//  Creation:    April  1, 2010
//
// ****************************************************************************
void
avtProjectFilter::ProjectPoint(double &x,double &y,double &z)
{
    switch (atts.GetProjectionType())
    {
      case ProjectAttributes::ZYCartesian:
        x = z;
        y = y;
        z = 0;
        break;

      case ProjectAttributes::XZCartesian:
        x = x;
        y = z;
        z = 0;
        break;

      case ProjectAttributes::XYCartesian:
        x = x;
        y = y;
        z = 0;
        break;

      case ProjectAttributes::XRCylindrical:
        {
            double r = sqrt(y*y + z*z);
            x = x;
            y = r;
            z = 0;
        }
        break;
      case ProjectAttributes::YRCylindrical:
        {
            double r = sqrt(x*x + z*z);
            x = y;
            y = r;
            z = 0;
        }
        break;

      case ProjectAttributes::ZRCylindrical:
        {
            double r = sqrt(x*x + y*y);
            x = z;
            y = r;
            z = 0;
        }
        break;
    }
}

// ****************************************************************************
//  Method:  avtProjectFilter::ProjectRectilinearGrid
//
//  Purpose:
//    Converts a rectilinear grid to a curvilinear grid while projecting
//
//  Arguments:
//    in_ds      the rectilinear grid to project
//
//  Programmer:  Jeremy Meredith
//  Creation:    September  6, 2004
//
//  Modifications:
//    Jeremy Meredith, Fri Sep 10 16:16:12 PDT 2004
//    Always convert to a curvilinear grid.  The extra code wasn't even
//    worth it because I expect no one will ever want to do it.
//    
//    Kathleen Biagas, Wed Aug  8 15:37:10 PDT 2012
//    Preserve coordinate data type.
//
//    Eric Brugger, Thu Jul 31 14:44:16 PDT 2014
//    Modified the class to work with avtDataRepresentation.
//
// ****************************************************************************
vtkPointSet *
avtProjectFilter::ProjectRectilinearGrid(vtkRectilinearGrid *in_ds)
{
    int  dims[3];
    in_ds->GetDimensions(dims);

    int  numPts = dims[0]*dims[1]*dims[2];

    vtkPoints *pts = vtkVisItUtility::NewPoints(in_ds);
    pts->SetNumberOfPoints(numPts);

    vtkDataArray *x = in_ds->GetXCoordinates();
    vtkDataArray *y = in_ds->GetYCoordinates();
    vtkDataArray *z = in_ds->GetZCoordinates();

    int index = 0;
    for (int k = 0 ; k < dims[2] ; k++)
    {
        for (int j = 0 ; j < dims[1] ; j++)
        {
            for (int i = 0 ; i < dims[0] ; i++)
            {
                double pt[3];
                pt[0] = x->GetComponent(i,0);
                pt[1] = y->GetComponent(j,0);
                pt[2] = z->GetComponent(k,0);

                ProjectPoint(pt[0],pt[1],pt[2]);

                pts->SetPoint(index++, pt);
            }
        }
    }

    vtkStructuredGrid *out = vtkStructuredGrid::New();
    out->SetDimensions(dims);
    out->SetPoints(pts);
    pts->Delete();
    out->GetCellData()->ShallowCopy(in_ds->GetCellData());
    out->GetPointData()->ShallowCopy(in_ds->GetPointData());

    return out;
}


// ****************************************************************************
//  Method:  avtProjectFilter::ProjectPointSet
//
//  Purpose:
//    Projects a descendent of vtkPointSet.  This includes
//    curvilinear, unstructured, point meshes, and poly data.
//
//  Arguments:
//    in_ds      the data set to project
//
//  Programmer:  Jeremy Meredith
//  Creation:    September  6, 2004
//
//  Modifications:
//
//    Hank Childs, Fri Oct  5 08:55:01 PDT 2007
//    In the special case where we have a curvilinear mesh with dimensions
//    1xYxZ or Xx1xZ, then change the projected mesh to be XxYx1.
//
//    Kathleen Biagas, Wed Aug  8 17:33:35 PDT 2012
//    Preserve coordinate type.
//
//    Eric Brugger, Thu Jul 31 14:44:16 PDT 2014
//    Modified the class to work with avtDataRepresentation.
//
// ****************************************************************************

vtkPointSet *
avtProjectFilter::ProjectPointSet(vtkPointSet *in_ds)
{
    vtkPointSet *out_ds = in_ds->NewInstance();
    out_ds->ShallowCopy(in_ds);

    vtkPoints *old_pts = in_ds->GetPoints();
    int npoints = old_pts->GetNumberOfPoints();

    // Make a new point array
    vtkPoints *new_pts = vtkPoints::New(old_pts->GetDataType());
    new_pts->DeepCopy(old_pts);
    if (old_pts->GetDataType() == VTK_FLOAT)
    {
        float *points = (float*)new_pts->GetVoidPointer(0); 
        for (int i = 0 ; i < npoints ; i++)
        {
            ProjectPoint(points[i*3+0],
                         points[i*3+1],
                         points[i*3+2]);
        }
    }
    else if (old_pts->GetDataType() == VTK_DOUBLE)
    {
        double *points = (double*)new_pts->GetVoidPointer(0);
        for (int i = 0 ; i < npoints ; i++)
        {
            ProjectPoint(points[i*3+0],
                         points[i*3+1],
                         points[i*3+2]);
        }
    }
    out_ds->SetPoints(new_pts);
    new_pts->Delete();

    //
    // If we are projecting a funny 2D curvilinear grid, then make it
    // be a normal 2D curvilinear grid.
    //
    if (out_ds->GetDataObjectType() == VTK_STRUCTURED_GRID)
    {
        vtkStructuredGrid *sgrid = (vtkStructuredGrid *) out_ds;
        int dims[3];
        sgrid->GetDimensions(dims);
        if (dims[0] == 1 && dims[1] != 1 && dims[2] != 1)
        {
            dims[0] = dims[1];
            dims[1] = dims[2];
            dims[2] = 1;
            sgrid->SetDimensions(dims);
        }
        if (dims[0] != 1 && dims[1] == 1 && dims[2] != 1)
        {
            dims[1] = dims[2];
            dims[2] = 1;
            sgrid->SetDimensions(dims);
        }
    }

    return out_ds;
}

// ****************************************************************************
//  Method:  avtProjectFilter::ProjectVectors
//
//  Purpose:
//    Project some vectors!
//
//  Arguments:
//    old_ds          the original dataset
//    new_ds          the transformed one
//    in              the vectors to project
//    out             the place to store the new vectors
//    cell_centered   true if these vectors are cell data
//
//  Programmer:  Jeremy Meredith
//  Creation:    September 10, 2004
//
//  Modifications:
//    Jeremy Meredith, Thu Apr  1 14:43:48 EDT 2010
//    Took into account various ways one might want to transform a vector.
//
//    Kathleen Bonnell, Thu Dec  9 14:49:04 PST 2010
//    Normalize the vector before projection in the AsDirection case.
//
//    Kathleen Biagas, Wed Aug  8 17:36:03 PDT 2012
//    Templatized to handle different data types.
//
// ****************************************************************************

template <class T> void
avtProjectFilter::ProjectVectors(vtkDataSet *old_ds, 
                                 vtkDataSet *new_ds,
                                 vtkDataArray *in,
                                 vtkDataArray *out,
                                 bool cell_centered)
{
    const double instantEps    = 1.e-5;
    const double instantEpsInv = 1.e+5;

    int nvectors  = in->GetNumberOfTuples();
    T *inptr  = (T*)in->GetVoidPointer(0);
    T *outptr = (T*)out->GetVoidPointer(0);

    for (int i=0; i<nvectors; i++)
    {
        double oldpt[3];
        double newpt[3];

        if (cell_centered)
        {
            old_ds->GetPoint(old_ds->GetCell(i)->GetPointId(0), oldpt);
            new_ds->GetPoint(new_ds->GetCell(i)->GetPointId(0), newpt);
        }
        else
        {
            old_ds->GetPoint(i, oldpt);
            new_ds->GetPoint(i, newpt);
        }

        double u = inptr[i*3+0];
        double v = inptr[i*3+1];
        double w = inptr[i*3+2];

        switch (atts.GetVectorTransformMethod())
        {
          case ProjectAttributes::None:
            outptr[i*3+0] = u;
            outptr[i*3+1] = v;
            outptr[i*3+2] = w;
            break;
            
          case ProjectAttributes::AsPoint:
            ProjectPoint(u,v,w);
            outptr[i*3+0] = u;
            outptr[i*3+1] = v;
            outptr[i*3+2] = w;
            break;
            
          case ProjectAttributes::AsDisplacement:
            {
            double x = oldpt[0] + u;
            double y = oldpt[1] + v;
            double z = oldpt[2] + w;

            ProjectPoint(x,y,z);

            outptr[i*3+0] = x - newpt[0];
            outptr[i*3+1] = y - newpt[1];
            outptr[i*3+2] = z - newpt[2];
            }
            break;

          case ProjectAttributes::AsDirection:
            {
            double mag = sqrt(u*u+v*v+w*w);
            double nu = 0., nv = 0., nw = 0.;
            if (mag != 0.)
            {
                nu = u/mag;
                nv = v/mag;
                nw = w/mag;
            }
            double x = oldpt[0] + nu*instantEps;
            double y = oldpt[1] + nv*instantEps;
            double z = oldpt[2] + nw*instantEps;

            ProjectPoint(x,y,z);

            outptr[i*3+0] = (x - newpt[0])*instantEpsInv*mag;
            outptr[i*3+1] = (y - newpt[1])*instantEpsInv*mag;
            outptr[i*3+2] = (z - newpt[2])*instantEpsInv*mag;
            }
            break;
        }
    }
}

// ****************************************************************************
//  Method:  avtProjectFilter::UpdateDataObjectInfo
//
//  Purpose:
//    Set up the atttributes and validity for the output of the filter.
//
//  Arguments:
//    none
//
//  Programmer:  Jeremy Meredith
//  Creation:    September  6, 2004
//
//  Modifications:
//
//    Hank Childs, Tue Feb  1 15:37:37 PST 2005
//    Allow normals, since if they are not appropriate, it will be detected.
//
//    Hank Childs, Fri Jan 13 09:58:47 PST 2006
//    Invalidate spatial meta-data.
//
//    Kathleen Biagas, Thu Aug 16 14:56:29 MST 2012
//    Indicate points were transformed by this operator.
//
//    Brad Whitlock, Mon Apr  7 15:55:02 PDT 2014
//    Add filter metadata used in export.
//    Work partially supported by DOE Grant SC0007548.
//
// ****************************************************************************

void
avtProjectFilter::UpdateDataObjectInfo(void)
{
    avtDataAttributes &inAtts      = GetInput()->GetInfo().GetAttributes();
    avtDataAttributes &outAtts     = GetOutput()->GetInfo().GetAttributes();
   
    if (inAtts.GetSpatialDimension() == 3)
        outAtts.SetSpatialDimension(2);

    GetOutput()->GetInfo().GetValidity().InvalidateSpatialMetaData();

    switch (atts.GetProjectionType())
    {
      case ProjectAttributes::ZYCartesian:
        outAtts.SetXLabel(inAtts.GetZLabel());
        outAtts.SetYLabel(inAtts.GetYLabel());
        outAtts.SetZLabel("Projected X Axis");
        break;

      case ProjectAttributes::XZCartesian:
        outAtts.SetXLabel(inAtts.GetXLabel());
        outAtts.SetYLabel(inAtts.GetZLabel());
        outAtts.SetZLabel("Projected Y Axis");
        break;

      case ProjectAttributes::XYCartesian:
        outAtts.SetXLabel(inAtts.GetXLabel());
        outAtts.SetYLabel(inAtts.GetYLabel());
        outAtts.SetZLabel("Projected Z Axis");
        break;

      case ProjectAttributes::XRCylindrical:
        outAtts.SetXLabel(inAtts.GetXLabel());
        outAtts.SetYLabel("Radial Axis");
        outAtts.SetZLabel("Projected Theta Axis");
        break;
      case ProjectAttributes::YRCylindrical:
        outAtts.SetXLabel(inAtts.GetYLabel());
        outAtts.SetYLabel("Radial Axis");
        outAtts.SetZLabel("Projected Theta Axis");
        break;
      case ProjectAttributes::ZRCylindrical:
        outAtts.SetXLabel(inAtts.GetZLabel());
        outAtts.SetYLabel("Radial Axis");
        outAtts.SetZLabel("Projected Theta Axis");
        break;
    }

    GetOutput()->GetInfo().GetValidity().SetPointsWereTransformed(true);
    outAtts.SetCanUseInvTransform(false);
    outAtts.SetCanUseTransform(false);
    outAtts.AddFilterMetaData("Project");
}

// ****************************************************************************
//  Method: avtProjectFilter::ModifyContract
//
//  Purpose:
//    Turn on zone/node numbers if needed for pick (for example).
//
//  Programmer: Jeremy Meredith
//  Creation:   September  9, 2004
//
//  Note:  I copied this implementation from Displace for now since
//         it seemed the most alike to this requirements for getting
//         pick to work.
//
//  Modifications:
//
// ****************************************************************************

avtContract_p
avtProjectFilter::ModifyContract(avtContract_p spec)
{
    avtContract_p rv = new avtContract(spec);
    if (rv->GetDataRequest()->MayRequireZones())
    {
        rv->GetDataRequest()->TurnZoneNumbersOn();
    }
    if (rv->GetDataRequest()->MayRequireNodes())
    {
        rv->GetDataRequest()->TurnNodeNumbersOn();
    }
    return rv;
}

