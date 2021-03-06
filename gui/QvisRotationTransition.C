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
#include <QvisRotationTransition.h>
#include <QvisColorButton.h>
#include <QButtonGroup>
#include <QFrame>
#include <QLabel>
#include <QLayout>
#include <QRadioButton>
#include <QSpinBox>

// ****************************************************************************
// Method: QvisRotationTransition::QvisRotationTransition
//
// Purpose: 
//   Constructor
//
// Arguments:
//   pix    : The pixmap to use in the widget.
//   parent : The widhet's parent.
//   name   : The widget's name.
//
// Programmer: Brad Whitlock
// Creation:   Wed Nov 8 10:12:57 PDT 2006
//
// Modifications:
//   Brad Whitlock, Tue Apr  8 15:26:49 PDT 2008
//   Support for internationalization.
//
//   Brad Whitlock, Tue Oct  7 09:37:43 PDT 2008
//   Qt 4.
//
// ****************************************************************************

QvisRotationTransition::QvisRotationTransition(const QPixmap &pix,
    QWidget *parent) : QGroupBox(parent)
{
    QVBoxLayout *innerLayout = new QVBoxLayout(this);
    innerLayout->setMargin(10);
    innerLayout->addSpacing(15);

    // Add the pixmap here.
    QLabel *pixLabel = new QLabel(this);
    pixLabel->setPixmap(pix);
    innerLayout->addWidget(pixLabel);
    innerLayout->addSpacing(15);

    QHBoxLayout *hLayout = new QHBoxLayout(0);
    innerLayout->addLayout(hLayout);
    QGridLayout *gLayout = new QGridLayout(0);
    hLayout->addLayout(gLayout);
    gLayout->setSpacing(10);
    hLayout->addStretch(10);

    // Angle controls.
    startAngle = new QSpinBox(this);
    startAngle->setMinimum(-360 * 100);
    startAngle->setMaximum(360 * 100);
    startAngle->setValue(0);
    QString deg; deg.sprintf("%c", 176);
    QString startLabel(tr("Start angle") + deg);
    gLayout->addWidget(
        new QLabel(startLabel, this), 0, 0);
    gLayout->addWidget(startAngle, 0, 1);
    
    endAngle = new QSpinBox(this);
    endAngle->setMinimum(-360 * 100);
    endAngle->setMaximum(360 * 100);
    endAngle->setValue(360);
    QString endLabel(tr("End angle") +deg);
    gLayout->addWidget(
        new QLabel(endLabel, this), 1, 0);
    gLayout->addWidget(endAngle, 1, 1);

    // Number of steps controls.
    nSteps = new QSpinBox(this);
    nSteps->setMinimum(1);
    nSteps->setValue(30);
    gLayout->addWidget(
        new QLabel(tr("Number of steps"), this), 2, 0);
    gLayout->addWidget(nSteps, 2, 1);

    innerLayout->addStretch(10);
}

// ****************************************************************************
// Method: QvisRotationTransition::~QvisRotationTransition
//
// Purpose: 
//   Destructor
//
// Programmer: Brad Whitlock
// Creation:   Tue Nov 14 13:26:39 PST 2006
//
// Modifications:
//   
// ****************************************************************************

QvisRotationTransition::~QvisRotationTransition()
{
}

//
// Set/Get methods.
//

void
QvisRotationTransition::setStartAngle(float sa)
{
    startAngle->setValue(int(sa));
}

void
QvisRotationTransition::setEndAngle(float ea)
{
    endAngle->setValue(int(ea));
}

void
QvisRotationTransition::getAngles(float &sa, float &ea) const
{
    sa = float(startAngle->value());
    ea = float(endAngle->value());
}

void
QvisRotationTransition::setNSteps(int val)
{
    nSteps->setValue(val);
}

int
QvisRotationTransition::getNSteps() const
{
    return nSteps->value();
}

