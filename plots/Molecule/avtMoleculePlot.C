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
//                              avtMoleculePlot.C                            //
// ************************************************************************* //

#include <avtMoleculePlot.h>

#include <avtColorTables.h>
#include <avtLevelsLegend.h>
#include <avtVariableLegend.h>
#include <avtLevelsMapper.h>
#include <avtLookupTable.h>
#include <MoleculeAttributes.h>
#include <avtFeatureEdgesFilter.h>
#include <avtUserDefinedMapper.h>
#include <avtMoleculeFilter.h>
#include <avtCallback.h>
#include <AtomicProperties.h>

#include <DebugStream.h>
#include <LineAttributes.h>
#include <InvalidColortableException.h>
#include <ImproperUseException.h>

#include <math.h>
#include <limits.h>
#include <float.h>
#include <stdio.h>

#include <string>
#include <vector>

using std::string;
using std::vector;

// ****************************************************************************
//  Method: avtMoleculePlot constructor
//
//  Programmer: Jeremy Meredith
//  Creation:   February  3, 2006
//
//  Modifications:
// ****************************************************************************

avtMoleculePlot::avtMoleculePlot()
{
    renderer = avtMoleculeRenderer::New();

    avtCustomRenderer_p cr;
    CopyTo(cr, renderer);
    mapper  = new avtUserDefinedMapper(cr);

    levelsLegend  = new avtLevelsLegend;
    levelsLegend->SetTitle("Molecule");

    variableLegend  = new avtVariableLegend;
    variableLegend->SetTitle("Molecule");

    levelsLUT = new avtLookupTable;
    variableLUT = new avtLookupTable;

    moleculeFilter = new avtMoleculeFilter();

    CreateLegendColorMaps();

    //
    // This is to allow the legend to reference counted so the behavior can
    // still access it after the plot is deleted.  The legend cannot be
    // reference counted all of the time since we need to know that it is a
    // LevelsLegend.
    //
    levelsLegendRefPtr = levelsLegend;
    variableLegendRefPtr = variableLegend;
}


// ****************************************************************************
//  Method: avtMoleculePlot destructor
//
//  Programmer: Jeremy Meredith
//  Creation:   February  3, 2006
//
//  Modifications:
//
//    Mark C. Miller, Thu Aug 10 11:36:57 PDT 2006
//    Fixed leak for levelsLUT, variableLUT
// ****************************************************************************

avtMoleculePlot::~avtMoleculePlot()
{
    if (mapper != NULL)
    {
        delete mapper;
        mapper = NULL;
    }

    if (moleculeFilter != NULL)
    {
        delete moleculeFilter;
        moleculeFilter = NULL;
    }

    if (levelsLUT != NULL)
    {
        delete levelsLUT;
        levelsLUT = NULL;
    }

    if (variableLUT != NULL)
    {
        delete variableLUT;
        variableLUT = NULL;
    }

    renderer = NULL;
    //
    // Do not delete the levelsLegend since it is being held by levLegendRefPtr.
    //
}


// ****************************************************************************
//  Method:  avtMoleculePlot::Create
//
//  Purpose:
//    Call the constructor.
//
//  Programmer:  Jeremy Meredith
//  Creation:    February 3, 2006
//
// ****************************************************************************

avtPlot*
avtMoleculePlot::Create()
{
    return new avtMoleculePlot;
}


// ****************************************************************************
//  Method: avtMoleculePlot::SetAtts
//
//  Purpose:
//      Sets the plot with the attributes.
//
//  Arguments:
//      atts    The attributes for this molecule plot.
//
//  Programmer: Jeremy Meredith
//  Creation:   February  3, 2006
//
//  Modifications:
//
//     Carson Brownlee, July 27, 2012
//     Fixing manta updates for molecule plot
//
// ****************************************************************************

void
avtMoleculePlot::SetAtts(const AttributeGroup *a)
{
    renderer->SetAtts(a);
    needsRecalculation =
        atts.ChangesRequireRecalculation(*(const MoleculeAttributes*)a);
    if (avtCallback::UseManta())  //Temporary fix to force updates
      needsRecalculation = true;
    atts = *(const MoleculeAttributes*)a;

    moleculeFilter->SetAtts(&atts);

    SetLegend(atts.GetLegendFlag());

    SetLegendRange();
}

// ****************************************************************************
//  Method: avtMoleculePlot::GetDataExtents
//
//  Purpose:
//      Gets the data extents used by the plot.
//
//  Arguments:
//      extents The extents used by the plot.
//
//  Programmer: Jeremy Meredith
//  Creation:   February  3, 2006
//
// ****************************************************************************

void
avtMoleculePlot::GetDataExtents(vector<double> &extents)
{
    double min=0, max=0;

    if (dataExtents.size() == 2)
    {
        min = dataExtents[0];
        max = dataExtents[1];
    }

    extents.push_back(min);
    extents.push_back(max);
}

// ****************************************************************************
// Method: avtMoleculePlot::SetColorTable
//
// Purpose:
//   Sets the plot's color table if the color table is the same as that of
//   the plot or we are using the default color table for the plot.
//
// Arguments:
//   ctName : The name of the color table to use.
//
// Returns:    Returns true if the color table is updated.
//
// Programmer: Jeremy Meredith
// Creation:   March 23, 2006
//
// Modifications:
//
// ****************************************************************************

bool
avtMoleculePlot::SetColorTable(const char *ctName)
{
    bool match = false;
    avtColorTables *ct = avtColorTables::Instance();

    string ect = atts.GetElementColorTable();
    if (ect == "Default")
        ect = ct->GetDefaultDiscreteColorTable();

    string rct = atts.GetResidueTypeColorTable();
    if (rct == "Default")
        rct = ct->GetDefaultDiscreteColorTable();

    string dct = atts.GetResidueSequenceColorTable();
    if (dct == "Default")
        dct = ct->GetDefaultDiscreteColorTable();

    string cct = atts.GetContinuousColorTable();
    if (cct == "Default")
        cct = ct->GetDefaultContinuousColorTable();

    if (ect == ctName ||
        rct == ctName ||
        dct == ctName ||
        cct == ctName)
    {
        match = true;
        renderer->InvalidateColors();
        SetLegendRange();
    }

    return match;
}

// ****************************************************************************
//  Method: avtMoleculePlot::SetLegend
//
//  Purpose:
//      Turns the legend on or off.
//
//  Arguments:
//      legendOn     true if the legend should be turned on, false otherwise.
//
//  Programmer: Jeremy Meredith
//  Creation:   February  3, 2006
//
// ****************************************************************************

void
avtMoleculePlot::SetLegend(bool legendOn)
{
    if (legendOn)
    {
        levelsLegend->LegendOn();
        variableLegend->LegendOn();
    }
    else
    {
        levelsLegend->LegendOff();
        variableLegend->LegendOff();
    }
}


// ****************************************************************************
//  Method: avtMoleculePlot::GetMapper
//
//  Purpose:
//      Gets the levels mapper as its base class (avtMapper) for our base
//      class (avtPlot).
//
//  Returns:    The mapper for this plot.
//
//  Programmer: Jeremy Meredith
//  Creation:   February  3, 2006
//
// ****************************************************************************

avtMapper *
avtMoleculePlot::GetMapper(void)
{
    return mapper;
}


// ****************************************************************************
//  Method: avtMoleculePlot::ApplyOperators
//
//  Purpose:
//      Performs the implied operators for an molecule plot.
//
//  Arguments:
//      input   The input data object.
//
//  Returns:    The data object after the molecule filter is applied.
//
//  Programmer: Jeremy Meredith
//  Creation:   February  3, 2006
//
//  Modifications:
//
// ****************************************************************************

avtDataObject_p
avtMoleculePlot::ApplyOperators(avtDataObject_p input)
{
    moleculeFilter->SetInput(input);
    return moleculeFilter->GetOutput();
}

// ****************************************************************************
//  Method: avtMoleculePlot::ApplyOperators
//
//  Purpose:
//      Performs the rendering transformation for an molecule plot.
//
//  Arguments:
//      input   The input data object.
//
//  Returns:    The data object after the feature edges is applied.
//
//  Programmer: Jeremy Meredith
//  Creation:   February  3, 2006
//
//  Modifications:
//
// ****************************************************************************

avtDataObject_p
avtMoleculePlot::ApplyRenderingTransformation(avtDataObject_p input)
{
    return input;
}


// ****************************************************************************
//  Method: avtMoleculePlot::CustomizeBehavior
//
//  Purpose:
//      Customizes the behavior of the output.
//
//  Programmer: Jeremy Meredith
//  Creation:   February  3, 2006
//
//  Modifications:
//    Jeremy Meredith, Mon Aug 28 18:25:52 EDT 2006
//    Account for model number directory prefix.
//
//    Jeremy Meredith, Thu Mar 12 17:32:34 EDT 2009
//    Allow any variable starting with "element" to be treated as
//    an atomic number -- this makes creating new element expressions
//    to override ones in the file possible.
//
// ****************************************************************************

void
avtMoleculePlot::CustomizeBehavior(void)
{
    SetLegendRange();

    if( varname != 0 )
    {
      string v(varname);
      if (v == "element" ||
          (v.length()>7 && v.substr(0,7) == "element") ||
          v == "resseq"  ||
          v == "restype" ||
          (v.length()>8 && v.substr(v.length()-8) == "/element") ||
          (v.length()>7 && v.substr(v.length()-7) == "/resseq")  ||
          (v.length()>8 && v.substr(v.length()-8) == "/restype"))
      {
          behavior->SetLegend(levelsLegendRefPtr);
      }
      else
      {
        behavior->SetLegend(variableLegendRefPtr);
      }
    }
    else
    {
      debug1 << "Can not access the legend because " <<
        "the variable name has not been set" << std::endl;

      EXCEPTION1(ImproperUseException,
                 "Can not access the legend because"
                 "the variable name has not been set");
    }
}


// ****************************************************************************
//  Method: avtMoleculePlot::CustomizeMapper
//
//  Purpose:
//    Use the info to set the isolevels in the legend.
//
//  Programmer: Jeremy Meredith
//  Creation:   February  3, 2006
//
//  Modifications:
//    Jeremy Meredith, Thu Apr 22 14:37:32 EDT 2010
//    Added a 2D mode to the renderer.
//
// ****************************************************************************

void
avtMoleculePlot::CustomizeMapper(avtDataObjectInformation &info)
{
    renderer->SetIs2D(info.GetAttributes().GetSpatialDimension() == 2);
}


// ****************************************************************************
//  Method: avtMoleculePlot::ReleaseData
//
//  Purpose:
//      Release the problem sized data associated with this plot.
//
//  Programmer: Jeremy Meredith
//  Creation:   February  3, 2006
//
// ****************************************************************************

void
avtMoleculePlot::ReleaseData(void)
{
    avtPointDataPlot::ReleaseData();
}



// ****************************************************************************
//  Method:  avtMoleculePlot::EnhanceSpecification
//
//  Purpose:
//    Add the necessary variables to the pipeline specification.
//    Force original zone numbers to always be on.
//
//  Arguments:
//    spec       the original pipeline spec
//
//  Programmer:  Jeremy Meredith
//  Creation:    March 23, 2006
//
//  Modifications:
//    Jeremy Meredith, Mon Aug 28 18:18:17 EDT 2006
//    Bonds are now line segments cells, and atoms are both points and
//    vertex cells.  This means we cannot look at cell data when looking
//    for atom arrays.  Also, account for model number directory prefix.
//
//    Jeremy Meredith, Thu Mar 12 17:32:34 EDT 2009
//    Allow any variable starting with "element" to be treated as
//    an atomic number -- this makes creating new element expressions
//    to override ones in the file possible.
//
//    Jeremy Meredith, Wed May 20 11:49:18 EDT 2009
//    Actually made change suggested by previous comment.
//
// ****************************************************************************

avtContract_p
avtMoleculePlot::EnhanceSpecification(avtContract_p spec)
{
    avtDataRequest_p ds = spec->GetDataRequest();
    const char *primaryVariable = ds->GetVariable();

    vector<string> added_vars;

    //
    // Determine if we need the "element" array
    //
    if (atts.GetScaleRadiusBy() == MoleculeAttributes::Covalent ||
        atts.GetScaleRadiusBy() == MoleculeAttributes::Atomic)
    {
        string pv(primaryVariable);
        if (pv != "element" &&
            (pv.length()<=7 || pv.substr(0,7) != "element") &&
            (pv.length()<=8 || pv.substr(pv.length()-8) != "/element"))
        {
            added_vars.push_back("element");
        }
    }

    //
    // Determine if we need another array for the radius variable
    //
    if (atts.GetScaleRadiusBy() == MoleculeAttributes::Variable &&
        atts.GetRadiusVariable() != "default")
    {
        if (string(primaryVariable) != atts.GetRadiusVariable())
        {
            added_vars.push_back(atts.GetRadiusVariable());
        }
    }

    //
    // The pipeline specification should really be const -- it is used
    // elsewhere, so we can't modify it and return it.  Make a copy and
    // in the new copy, add a secondary variable.
    //
    avtDataRequest_p nds = new avtDataRequest(primaryVariable,
                                                         ds->GetTimestep(),
                                                         ds->GetRestriction());
    for (size_t i=0; i<added_vars.size(); i++)
    {
        nds->AddSecondaryVariable(added_vars[i].c_str());
    }

    //
    // We *always* need node numbers.  I think we need zone numbers, too!
    //
    nds->TurnZoneNumbersOn();
    nds->TurnNodeNumbersOn();

    //
    // Create the new pipeline spec from the data spec, and return
    //
    avtContract_p rv = new avtContract(spec, nds);

    return rv;
}


// ****************************************************************************
//  Method:  avtMoleculePlot::GetLegend
//
//  Purpose:
//    Return the active legend points.
//
//  Arguments:
//    none
//
//  Programmer:  Jeremy Meredith
//  Creation:    March 23, 2006
//
//  Modifications:
//    Jeremy Meredith, Mon Aug 28 18:25:52 EDT 2006
//    Account for model number directory prefix.
//
//    Jeremy Meredith, Thu Mar 12 17:32:34 EDT 2009
//    Allow any variable starting with "element" to be treated as
//    an atomic number -- this makes creating new element expressions
//    to override ones in the file possible.
//
// ****************************************************************************
avtLegend_p
avtMoleculePlot::GetLegend(void)
{
    if( varname != 0 )
    {
      string v(varname);

      if (v == "element" ||
          (v.length()>7 && v.substr(0,7) == "element") ||
          v == "resseq"  ||
          v == "restype" ||
          (v.length()>8 && v.substr(v.length()-8) == "/element") ||
          (v.length()>7 && v.substr(v.length()-7) == "/resseq")  ||
          (v.length()>8 && v.substr(v.length()-8) == "/restype"))
      {
          return levelsLegendRefPtr;
      }
      else
      {
          return variableLegendRefPtr;
      }
    }
    else
    {
      debug1 << "Can not access the legend because " <<
        "the variable name has not been set" << std::endl;

      // EXCEPTION1(ImproperUseException,
      //                 "Can not access the legend because "
      //                 "the variable name has not been set");

      return NULL;
    }
}

// ****************************************************************************
//  Method:
//
//  Purpose:
//    Could this be any harder?  The variable legend is nice and
//    straightforward, but the levels one will do some strange things
//    unless you get it just right.  Anyway, this is where we figure
//    out how big to make the legend, get its colors, and map the
//    "labels" in the legend to its index in the color map.
//
//  Arguments:
//    none
//
//  Programmer:  Jeremy Meredith
//  Creation:    March 23, 2006
//
//  Modifications:
//
//    Hank Childs, Thu May 18 11:16:20 PDT 2006
//    Fix UMRs.
//
//    Jeremy Meredith, Mon Aug 28 18:27:19 EDT 2006
//    Account for model number directory prefix.
//
//    Jeremy Meredith, Thu Mar 12 17:32:34 EDT 2009
//    Allow any variable starting with "element" to be treated as
//    an atomic number -- this makes creating new element expressions
//    to override ones in the file possible.
//
//    Jeremy Meredith, Wed May 20 11:49:18 EDT 2009
//    MAX_ELEMENT_NUMBER now means the actual max element number, not the
//    total number of known elements in visit.  Added a fake "0" element
//    which means "unknown", and hydrogen now starts at 1.
//
// ****************************************************************************

void
avtMoleculePlot::SetLegendRange()
{
    double min = 0., max = 1.;
    mapper->GetCurrentDataRange(min,max);

    // Set attributes for continuous legend
    variableLegend->SetVarRange(min,max);
    if (atts.GetMinFlag())
        min = atts.GetScalarMin();
    if (atts.GetMaxFlag())
        max = atts.GetScalarMax();
    variableLegend->SetRange(min,max);
    renderer->SetRange(min,max);

    if (atts.GetContinuousColorTable() == "Default")
        variableLUT->SetColorTable(NULL, false);
    else
        variableLUT->SetColorTable(atts.GetContinuousColorTable().c_str(), true);

    variableLegend->SetLookupTable(variableLUT->GetLookupTable());

    // Set attributes for levels legend
    vector <string> usedLabels;
    behavior->GetInfo().GetAttributes().GetLabels(usedLabels);
    size_t nlabels = usedLabels.size();

    if (nlabels == 0)
    {
        levelsLegend->SetColorBarVisibility(0);
        levelsLegend->SetMessage("No subsets present");
    }
    else if( varname != 0 )
    {
        levelsLegend->SetColorBarVisibility(1);
        levelsLegend->SetMessage(NULL);

        avtColorTables *ct = avtColorTables::Instance();
        string varName = string(varname);
        string colortablename = "";
        int    numcolors = 0;

        if (varName == "element" ||
            (varName.length()>7 && varName.substr(0,7) == "element") ||
            (varName.length()>8 && varName.substr(varName.length()-8) == "/element"))
        {
            colortablename = atts.GetElementColorTable();
            if (colortablename == "Default")
                colortablename = string(ct->GetDefaultDiscreteColorTable());

            numcolors = MAX_ELEMENT_NUMBER+1;

            levelsLegend->SetLabelColorMap(elementColorMap);
        }
        else if (varName == "resseq" ||
                 (varName.length()>7 && varName.substr(varName.length()-7) == "/resseq"))
        {
            colortablename = atts.GetResidueSequenceColorTable();
            if (colortablename == "Default")
                colortablename = string(ct->GetDefaultDiscreteColorTable());

            // Populate the residue map so that it can be used by the
            // renderer for drawing resseq.
            numcolors = int(max);
            residueColorMap.clear();
            for (size_t i=0; i<nlabels; i++)
            {
                int val;
                sscanf(usedLabels[i].c_str(), "%d", &val);
                --val;
                residueColorMap.insert(LevelColorMap::value_type(
                usedLabels[i], val));
            }
            levelsLegend->SetLabelColorMap(residueColorMap);

            // Make the renderer use the levelsLUT for colors.
            renderer->SetLevelsLUT(levelsLUT);
        }
        else if (varName == "restype" ||
                 (varName.length()>8 && varName.substr(varName.length()-8) == "/restype"))
        {
            colortablename = atts.GetResidueTypeColorTable();
            if (colortablename == "Default")
                colortablename = string(ct->GetDefaultDiscreteColorTable());

            // Make sure that all of the labels that we're using are learned
            // so we get the right items.
            vector<string> uLabels;
            for(size_t k = 0; k < nlabels; k += 3)
            {
                if(usedLabels[k] == "on")
                    uLabels.push_back(usedLabels[k+1]);

                if(ResiduenameToNumber(usedLabels[k+1].c_str()) < 0)
                {
                    debug4 << "Learning new residue: "
                           << usedLabels[k+1].c_str() << ", "
                           << usedLabels[k+2].c_str() << endl;

                    AddResiduename(usedLabels[k+1].c_str(),
                                   usedLabels[k+2].c_str());
                }
            }

            usedLabels = uLabels;
            nlabels = usedLabels.size();

            // Redo the colormap in case there are new restypes that we've
            // learned about.
            CreateLegendColorMaps();
            numcolors = NumberOfKnownResidues();

            levelsLegend->SetLabelColorMap(residueColorMap);
            renderer->SetLevelsLUT(0);
        }
        else
        {
            colortablename = atts.GetContinuousColorTable();
            if (colortablename == "Default")
                colortablename = string(ct->GetDefaultContinuousColorTable());

            numcolors = (int)nlabels;//256;

            levelsLegend->SetLabelColorMap(blankColorMap);
        }

        if (!ct->ColorTableExists(colortablename.c_str()))
        {
            EXCEPTION1(InvalidColortableException, colortablename);
        }

        unsigned char *colors = new unsigned char[3*numcolors];
        unsigned char *cptr = colors;
        if (ct->IsDiscrete(colortablename.c_str()))
        {
            // The CT is discrete, get its color color control points.
            for (int i = 0; i < numcolors; ++i)
            {
                unsigned char rgb[3] = {0,0,0};
                ct->GetControlPointColor(colortablename.c_str(), i, rgb);

                *cptr++ = rgb[0];
                *cptr++ = rgb[1];
                *cptr++ = rgb[2];
            }
        }
        else
        {
            // The CT is continuous, sample the CT so we have a unique color
            // for each element.
            unsigned char *rgb = ct->GetSampledColors(colortablename.c_str(), numcolors);
            if (rgb)
            {
                for (int i = 0; i < numcolors; ++i)
                {
                    int j = i * 3;
                    *cptr++ = rgb[j];
                    *cptr++ = rgb[j+1];
                    *cptr++ = rgb[j+2];
                }
                delete [] rgb;
            }
        }
        levelsLUT->SetLUTColors(colors, numcolors);

        levelsLegend->SetLookupTable(levelsLUT->GetLookupTable());

        levelsLegend->SetLevels(usedLabels);
        levelsLegend->SetVarRange(min,max);
        levelsLegend->SetColorBarVisibility(1);
        levelsLegend->SetMessage(NULL);
    }
    else
    {
      debug1 << "Can not access the legend because " <<
        "the variable name has not been set" << std::endl;

      EXCEPTION1(ImproperUseException,
                 "Can not access the legend because"
                 "the variable name has not been set");
    }
}


// ****************************************************************************
//  Method: avtMoleculePlot::CreateLegendColorMaps
//
//  Purpose:
//    Create the mapping between label names and colors.  This must be
//    done for atomic number and residue name, because we need index
//    number 6 (well, 5 in a zero-origin array) to *always* be colored
//    the color that carbon is supposed to be via index 6 (5) in the
//    discrete color table.  This map only needs to be created once.
//
//  Arguments:
//    none
//
//  Programmer:  Jeremy Meredith
//  Creation:    March 23, 2006
//
//  Modifications:
//    Jeremy Meredith, Wed May 20 11:49:18 EDT 2009
//    MAX_ELEMENT_NUMBER now means the actual max element number, not the
//    total number of known elements in visit.  Added a fake "0" element
//    which means "unknown", and hydrogen now starts at 1.
//
// ****************************************************************************
void
avtMoleculePlot::CreateLegendColorMaps()
{
    residueColorMap.clear();
    for (int i=0; i<NumberOfKnownResidues(); i++)
    {
        residueColorMap.insert(LevelColorMap::value_type(
            NumberToResiduename(i), i));
    }

    elementColorMap.clear();
    for (int i=0; i<=MAX_ELEMENT_NUMBER; i++)
    {
        elementColorMap.insert(LevelColorMap::value_type(element_names[i], i));
    }
}
