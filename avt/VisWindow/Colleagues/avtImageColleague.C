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
//                              avtImageColleague.C                          //
// ************************************************************************* //

#include <avtImageColleague.h>

#include <snprintf.h>

#include <vtkActor2D.h>
#include <vtkCoordinate.h>
#include <vtkImageData.h>
#include <vtkImageMapper.h>
#include <vtkImageReader2.h>
#include <vtkImageReader2Factory.h>
#include <vtkImageResample.h>
#include <vtkProperty2D.h>
#include <vtkRenderer.h>

#include <AnnotationObject.h>
#include <avtCallback.h>
#include <FileFunctions.h>
#include <VisWindow.h>
#include <VisWindowColleagueProxy.h>

#include <DebugStream.h>

#define RESAMPLE_IMAGE

// ****************************************************************************
// Method: avtImageColleague::avtImageColleague
//
// Purpose: 
//   Constructor for the avtImageColleague class.
//
// Arguments:
//   m : The vis window proxy.
//
// Programmer: John C. Anderson
// Creation:   Mon Jul 12 15:55:58 PDT 2004
//
// Modifications:
//   Brad Whitlock, Tue Jun 28 17:15:59 PST 2005
//   Moved code to CreateActorAndMapper.
//
// ****************************************************************************

avtImageColleague::avtImageColleague(VisWindowColleagueProxy &m):
    avtAnnotationColleague(m),
    actor(NULL),
    mapper(NULL),
    resample(NULL),
    iData(NULL),
    width(100),
    height(100),
    useOpacityColor(false),
    maintainAspectRatio(true),
    addedToRenderer(false)
{
    CreateActorAndMapper();
}

// ****************************************************************************
// Method: avtImageColleague::~avtImageColleague
//
// Purpose: 
//   Destructor for the avtImageColleague class.
//
// Programmer: John C. Anderson
// Creation:   Mon Jul 12 16:24:34 PDT 2004
//
// Modifications:
//   
// ****************************************************************************
avtImageColleague::~avtImageColleague()
{
    // A blank image will actually delete some stuff...
    UpdateImage("");

    actor->Delete();
    mapper->Delete();
}

// ****************************************************************************
// Method: avtImageColleague::CreateActorAndMapper
//
// Purpose: 
//   Creates the actor and the mapper.
//
// Programmer: Brad Whitlock
// Creation:   Tue Jun 28 17:15:47 PST 2005
//
// Modifications:
//   
// ****************************************************************************

void
avtImageColleague::CreateActorAndMapper()
{
    // Create the image mapper.
    mapper = vtkImageMapper::New();
    mapper->SetColorWindow(255.0);
    mapper->SetColorLevel(127.5);
    mapper->SetZSlice(0);
#ifdef TESTING_IMAGE_RESCALE_BY_MAPPER
    mapper->SetRenderToRectangle(1);
#endif

    //
    // Create and position the actor.
    //
    actor = vtkActor2D::New();
    actor->SetMapper(mapper);

    // Setup coordinate system to normalized viewport.
    vtkCoordinate *pos = actor->GetPositionCoordinate();
    pos->SetCoordinateSystemToNormalizedViewport();
    pos = actor->GetPosition2Coordinate();
    pos->SetCoordinateSystemToNormalizedViewport();
}

// ****************************************************************************
// Method: avtImageColleague::AddToRenderer
//
// Purpose: 
//   This method adds the text actor to the renderer.
//
// Programmer: Brad Whitlock
// Creation:   Thu Nov 6 15:52:19 PST 2003
//
// Modifications:
//   
// ****************************************************************************
void 
avtImageColleague::AddToRenderer()
{
    if(!addedToRenderer && ShouldBeAddedToRenderer())
    {
        mediator.GetForeground()->AddActor2D(actor);
        addedToRenderer = true;
    }
}

// ****************************************************************************
// Method: avtImageColleague::RemoveFromRenderer
//
// Purpose: 
//   This method removes the text actor from the renderer.
//
// Programmer: Brad Whitlock
// Creation:   Thu Nov 6 15:52:38 PST 2003
//
// Modifications:
//   
// ****************************************************************************
void
avtImageColleague::RemoveFromRenderer()
{
    if(addedToRenderer)
    {
        mediator.GetForeground()->RemoveActor2D(actor);
        addedToRenderer = false;
    }
}

// ****************************************************************************
// Method: avtImageColleague::Hide
//
// Purpose: 
//   This method toggles the visible flag and either adds or removes the text
//   actor to/from the renderer.
//
// Programmer: Brad Whitlock
// Creation:   Thu Nov 6 15:52:57 PST 2003
//
// Modifications:
//   
// ****************************************************************************
void
avtImageColleague::Hide()
{
    SetVisible(!GetVisible());

    if(addedToRenderer)
        RemoveFromRenderer();
    else
        AddToRenderer();
}

// ****************************************************************************
// Method: avtImageColleague::ShouldBeAddedToRenderer
//
// Purpose: 
//   This method returns whether or not the text actor should be added to the
//   renderer.
//
// Programmer: Brad Whitlock
// Creation:   Thu Nov 6 15:53:36 PST 2003
//
// Modifications:
//   
// ****************************************************************************
bool
avtImageColleague::ShouldBeAddedToRenderer() const
{
    return GetVisible() && mediator.HasPlots();
}

// ****************************************************************************
// Method: avtImageColleague::SetOptions
//
// Purpose: 
//   This method sets the text actor's properties from the values in the
//   annotation object.
//
// Programmer: John C. Anderson
// Creation:   Fri Sep 03 09:00:25 PDT 2004
//
// Modifications:
//    Brad Whitlock, Tue Aug 31 11:56:56 PDT 2004
//    I added code to set the object's visibility so objects that are not
//    visible remain invisible when restoring a session file.
//
//    Dave Bremer, Fri Mar  9 16:09:36 PST 2007
//    Slight change to error reporting message, for files that aren't readable
//
// ****************************************************************************
void
avtImageColleague::SetOptions(const AnnotationObject &annot)
{
    // Get the current options.
    AnnotationObject currentOptions;
    GetOptions(currentOptions);

    maintainAspectRatio = annot.GetFontShadow();

    bool updateOpacity = false;
    bool haveImage = true;

    const stringVector &text = annot.GetText();
    if(text.size() > 0 &&
       (text[0] != currentImage ||
        useOpacityColor != (annot.GetIntAttribute1()>0) ||
        (useOpacityColor && (opacityColor != annot.GetColor1()))))
    {
        useOpacityColor = annot.GetIntAttribute1();
        if(useOpacityColor)
            opacityColor = annot.GetColor1();

        updateOpacity = true;
        haveImage = UpdateImage(text[0]);
    }

    if(iData && useOpacityColor && updateOpacity)
    {
        // Make a copy of the data, but with an RGBA channel.
        if(iData->GetNumberOfScalarComponents() < 4)
        {
               vtkImageData *tmpdata = vtkImageData::New();
               tmpdata->SetNumberOfScalarComponents(4, tmpdata->GetInformation());
               tmpdata->SetExtent(iData->GetExtent());

               for(int i = 0; i < iData->GetDimensions()[0]; ++i)
                   for(int j = 0; j < iData->GetDimensions()[1]; ++j)
                   {
                       for(int c = 0;
                           c < iData->GetNumberOfScalarComponents();
                           ++c)
                       {
                           tmpdata->SetScalarComponentFromDouble(
                             i, j, 0, c,
                             iData->GetScalarComponentAsDouble(i, j, 0, c));
                       }
                       tmpdata->SetScalarComponentFromDouble(i, j, 0, 3, 255.);
                   }
#ifdef RESAMPLE_IMAGE
               resample->SetInputData(tmpdata);
#endif
               iData->Delete();
               iData = tmpdata;
        }

        // Go through and set the opacity to 0 for any matching colored pixel.
#ifdef CAN_SET_OPACITY
        double a = double(annot.GetColor1().Alpha());
#endif
        for(int i = 0; i < iData->GetDimensions()[0]; ++i)
            for(int j = 0; j < iData->GetDimensions()[1]; ++j)
            {
#ifdef CAN_SET_OPACITY
                iData->SetScalarComponentFromDouble(i, j, 0, 3, a);
#endif
                if(iData->GetScalarComponentAsDouble(i, j, 0, 0) == opacityColor.Red() &&
                   iData->GetScalarComponentAsDouble(i, j, 0, 1) == opacityColor.Green() &&
                   iData->GetScalarComponentAsDouble(i, j, 0, 2) == opacityColor.Blue())
                {
                    iData->SetScalarComponentFromDouble(i, j, 0, 3, 0);
                }
            }
#ifdef RESAMPLE_IMAGE
        resample->Update();
#endif
    }

    if((iData && resample) &&
       (width != (int) annot.GetPosition2()[0] ||
        height != (int) annot.GetPosition2()[1])
       )
    {
        if(!maintainAspectRatio)
        {
            width = (int) annot.GetPosition2()[0];
            height = (int) annot.GetPosition2()[1];    
        }
        else if(width != (int) annot.GetPosition2()[0])
        {
            width = (int) annot.GetPosition2()[0];
            height = width;
        }
        else if(height != (int) annot.GetPosition2()[1])
        {
            height = (int) annot.GetPosition2()[1];
            width = height;
        }

#ifdef RESAMPLE_IMAGE
        resample->SetAxisMagnificationFactor(0, width / 100.0F);
        resample->SetAxisMagnificationFactor(1, height / 100.0F);
        resample->SetAxisMagnificationFactor(2, 1);

        resample->Update();
#endif
    }

    actor->SetPosition(annot.GetPosition()[0],
                       annot.GetPosition()[1]);

#ifdef TESTING_IMAGE_RESCALE_BY_MAPPER
    actor->SetPosition2(0.2,
                        0.2);
#endif
    
#if 0 // This doesn't work right now.
    actor->GetProperty()->SetOpacity(annot.GetColor1().Alpha() / 255.0F);
#endif

    //
    // Set the object's visibility.
    //
    if(iData == 0 && addedToRenderer)
    {
        debug1 << "Removing the image from the renderer because it could not be read." << endl;
        mediator.GetForeground()->RemoveActor2D(actor);
        addedToRenderer = false;

        // Delete the actor and mapper so the image mapper does not freak.
        actor->Delete();
        mapper->Delete();

        // Create a new actor and mapper.
        CreateActorAndMapper();

        if(!haveImage)
        {
            char msg[1024];
            SNPRINTF(msg, 1024, "Could not read image file: %s.", text[0].c_str());
            avtCallback::IssueWarning(msg);
        }
    }
    else if(currentOptions.GetVisible() != annot.GetVisible())
    {
        SetVisible(annot.GetVisible());
        if(annot.GetVisible())
            AddToRenderer();
        else
            RemoveFromRenderer();
    }
}

// ****************************************************************************
// Method: avtImageColleague::UpdateImage
//
// Purpose: Updates the image if the filename can be read, does nothing
//          if the image filename cannot be used.
//   
//
// Arguments:
//
// Returns:
//
// Note:
//
// Programmer: John C. Anderson
// Creation:   Fri Jul 16 08:04:41 PDT 2004
//
// Modifications:
//   Brad Whitlock, Tue Jun 28 17:12:24 PST 2005
//   Added code to delete the image reader.
//
//   Dave Bremer, Fri Mar  9 16:09:36 PST 2007
//   Added more error checking on image reads.
//
//   Hank Childs, Fri Jan 23 14:58:19 PST 2009
//   Add an error message if the file can't be read.  Add support for tilde's
//   for home directories.
//
// ****************************************************************************

bool
avtImageColleague::UpdateImage(std::string filename)
{
    bool retval = true;

    if (filename.size() > 0 && filename[0] == '~')
        filename = ExpandUserPath(filename);

    // Get a reader for filename if possible.
    vtkImageReader2 *r =
        vtkImageReader2Factory::CreateImageReader2(filename.c_str());

    // If we got a valid reader:
    if(r && r->CanReadFile(filename.c_str()) == 3)
    {
        // Blank the current image first.
        retval = UpdateImage("");

        // Set the currentImage.
        currentImage = filename;

        // Read the image.
        r->SetFileName(filename.c_str());
        r->Update();
        iData = r->GetOutput();

        // Sometimes VTK reads fail silently.  Check image dims to
        // make sure this did not happen.  Will return 1,1,1 on some
        // failures.
        int dims[3];
        iData->GetDimensions(dims);
        if (dims[0] <= 1 && dims[1] <= 1)
        {
            iData = NULL;
        }
        
        if(iData != 0)
        {
            iData->Register(NULL);
            
            // Set the height and width.
            width = height = 100;

            // Resample the image to the proper size.
#ifdef RESAMPLE_IMAGE
            resample = vtkImageResample::New();
            resample->SetInputData(iData);

            mapper->SetInputConnection(resample->GetOutputPort());
#else
            mapper->SetInputData(iData);
#endif
        }
        else
        {
            mapper->SetInputData(NULL);
            retval = false;
        }

        // Delete the image reader.
        r->Delete();
    }
    else
    {
        currentImage = "";
        if (filename != "")
        {
            std::string msg = "Could not open file " + filename 
                            + " to create image annotation object";
            avtCallback::IssueWarning(msg.c_str());
        }

        if(mapper) { mapper->SetInputData(NULL); }
#ifdef RESAMPLE_IMAGE
        if(resample) { resample->Delete(); resample = NULL; }
#endif
        if(iData) { iData->Delete(); iData = NULL; }
    }

    return retval;
}

// ****************************************************************************
// Method: avtImageColleague::GetOptions
//
// Purpose: 
//   This method stores the text label's attributes in an object that can
//   be passed back to the client.
//
// Arguments:
//   annot : The AnnotationObject to populate.
//
// Programmer: John C. Anderson
// Creation:   Fri Sep 03 08:59:53 PDT 2004
//
// Modifications:
//   
// ****************************************************************************
void
avtImageColleague::GetOptions(AnnotationObject &annot)
{
    annot.SetObjectType(AnnotationObject::Image);
    annot.SetVisible(GetVisible());
    annot.SetActive(GetActive());

    annot.SetFontShadow(maintainAspectRatio);

    stringVector sv;
    sv.push_back(currentImage);
    annot.SetText(sv);

    annot.SetPosition(actor->GetPosition());

    double pos[] = {width, height, 0};
    annot.SetPosition2(pos);

    annot.SetIntAttribute1(useOpacityColor ? 1 : 0);
    annot.SetColor1(opacityColor);
}

// ****************************************************************************
// Method: avtImageColleague::HasPlots
//
// Purpose: 
//   This method is called when the vis window gets some plots. We use this
//   signal to add the text actor to the renderer.
//
// Programmer: Brad Whitlock
// Creation:   Thu Nov 6 15:56:06 PST 2003
//
// Modifications:
//   
// ****************************************************************************
void
avtImageColleague::HasPlots(void)
{
    AddToRenderer();
}

// ****************************************************************************
// Method: avtImageColleague::NoPlots
//
// Purpose: 
//   This method is called when the vis window has no plots. We use this signal
//   to remove the text actor from the renderer.
//
// Programmer: Brad Whitlock
// Creation:   Thu Nov 6 15:56:42 PST 2003
//
// Modifications:
//   
// ****************************************************************************
void
avtImageColleague::NoPlots(void)
{
    RemoveFromRenderer();
}
