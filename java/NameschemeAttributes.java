// ***************************************************************************
//
// Copyright (c) 2000 - 2014, Lawrence Livermore National Security, LLC
// Produced at the Lawrence Livermore National Laboratory
// LLNL-CODE-442911
// All rights reserved.
//
// This file is  part of VisIt. For  details, see https://visit.llnl.gov/.  The
// full copyright notice is contained in the file COPYRIGHT located at the root
// of the VisIt distribution or at http://www.llnl.gov/visit/copyright.html.
//
// Redistribution  and  use  in  source  and  binary  forms,  with  or  without
// modification, are permitted provided that the following conditions are met:
//
//  - Redistributions of  source code must  retain the above  copyright notice,
//    this list of conditions and the disclaimer below.
//  - Redistributions in binary form must reproduce the above copyright notice,
//    this  list of  conditions  and  the  disclaimer (as noted below)  in  the
//    documentation and/or other materials provided with the distribution.
//  - Neither the name of  the LLNS/LLNL nor the names of  its contributors may
//    be used to endorse or promote products derived from this software without
//    specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
// ARE  DISCLAIMED. IN  NO EVENT  SHALL LAWRENCE  LIVERMORE NATIONAL  SECURITY,
// LLC, THE  U.S.  DEPARTMENT OF  ENERGY  OR  CONTRIBUTORS BE  LIABLE  FOR  ANY
// DIRECT,  INDIRECT,   INCIDENTAL,   SPECIAL,   EXEMPLARY,  OR   CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
// SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
// CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
// LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
// OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
// DAMAGE.
//
// ***************************************************************************

package llnl.visit;

import java.util.Vector;
import java.lang.Integer;

// ****************************************************************************
// Class: NameschemeAttributes
//
// Purpose:
//    Information and methods to manage nameschemes
//
// Notes:      Autogenerated by xml2java.
//
// Programmer: xml2java
// Creation:   omitted
//
// Modifications:
//   
// ****************************************************************************

public class NameschemeAttributes extends AttributeSubject
{
    private static int NameschemeAttributes_numAdditionalAtts = 7;

    public NameschemeAttributes()
    {
        super(NameschemeAttributes_numAdditionalAtts);

        namescheme = new String("");
        externalArrayNames = new Vector();
        externalArrayOffsets = new Vector();
        externalArrayData = new Vector();
        allExplicitNames = new Vector();
        explicitIds = new Vector();
        explicitNames = new Vector();
    }

    public NameschemeAttributes(int nMoreFields)
    {
        super(NameschemeAttributes_numAdditionalAtts + nMoreFields);

        namescheme = new String("");
        externalArrayNames = new Vector();
        externalArrayOffsets = new Vector();
        externalArrayData = new Vector();
        allExplicitNames = new Vector();
        explicitIds = new Vector();
        explicitNames = new Vector();
    }

    public NameschemeAttributes(NameschemeAttributes obj)
    {
        super(NameschemeAttributes_numAdditionalAtts);

        int i;

        namescheme = new String(obj.namescheme);
        externalArrayNames = new Vector(obj.externalArrayNames.size());
        for(i = 0; i < obj.externalArrayNames.size(); ++i)
            externalArrayNames.addElement(new String((String)obj.externalArrayNames.elementAt(i)));

        externalArrayOffsets = new Vector();
        for(i = 0; i < obj.externalArrayOffsets.size(); ++i)
        {
            Integer iv = (Integer)obj.externalArrayOffsets.elementAt(i);
            externalArrayOffsets.addElement(new Integer(iv.intValue()));
        }
        externalArrayData = new Vector();
        for(i = 0; i < obj.externalArrayData.size(); ++i)
        {
            Integer iv = (Integer)obj.externalArrayData.elementAt(i);
            externalArrayData.addElement(new Integer(iv.intValue()));
        }
        allExplicitNames = new Vector(obj.allExplicitNames.size());
        for(i = 0; i < obj.allExplicitNames.size(); ++i)
            allExplicitNames.addElement(new String((String)obj.allExplicitNames.elementAt(i)));

        explicitIds = new Vector();
        for(i = 0; i < obj.explicitIds.size(); ++i)
        {
            Integer iv = (Integer)obj.explicitIds.elementAt(i);
            explicitIds.addElement(new Integer(iv.intValue()));
        }
        explicitNames = new Vector(obj.explicitNames.size());
        for(i = 0; i < obj.explicitNames.size(); ++i)
            explicitNames.addElement(new String((String)obj.explicitNames.elementAt(i)));


        SelectAll();
    }

    public int Offset()
    {
        return super.Offset() + super.GetNumAdditionalAttributes();
    }

    public int GetNumAdditionalAttributes()
    {
        return NameschemeAttributes_numAdditionalAtts;
    }

    public boolean equals(NameschemeAttributes obj)
    {
        int i;

        // Compare the elements in the externalArrayNames vector.
        boolean externalArrayNames_equal = (obj.externalArrayNames.size() == externalArrayNames.size());
        for(i = 0; (i < externalArrayNames.size()) && externalArrayNames_equal; ++i)
        {
            // Make references to String from Object.
            String externalArrayNames1 = (String)externalArrayNames.elementAt(i);
            String externalArrayNames2 = (String)obj.externalArrayNames.elementAt(i);
            externalArrayNames_equal = externalArrayNames1.equals(externalArrayNames2);
        }
        // Compare the elements in the externalArrayOffsets vector.
        boolean externalArrayOffsets_equal = (obj.externalArrayOffsets.size() == externalArrayOffsets.size());
        for(i = 0; (i < externalArrayOffsets.size()) && externalArrayOffsets_equal; ++i)
        {
            // Make references to Integer from Object.
            Integer externalArrayOffsets1 = (Integer)externalArrayOffsets.elementAt(i);
            Integer externalArrayOffsets2 = (Integer)obj.externalArrayOffsets.elementAt(i);
            externalArrayOffsets_equal = externalArrayOffsets1.equals(externalArrayOffsets2);
        }
        // Compare the elements in the externalArrayData vector.
        boolean externalArrayData_equal = (obj.externalArrayData.size() == externalArrayData.size());
        for(i = 0; (i < externalArrayData.size()) && externalArrayData_equal; ++i)
        {
            // Make references to Integer from Object.
            Integer externalArrayData1 = (Integer)externalArrayData.elementAt(i);
            Integer externalArrayData2 = (Integer)obj.externalArrayData.elementAt(i);
            externalArrayData_equal = externalArrayData1.equals(externalArrayData2);
        }
        // Compare the elements in the allExplicitNames vector.
        boolean allExplicitNames_equal = (obj.allExplicitNames.size() == allExplicitNames.size());
        for(i = 0; (i < allExplicitNames.size()) && allExplicitNames_equal; ++i)
        {
            // Make references to String from Object.
            String allExplicitNames1 = (String)allExplicitNames.elementAt(i);
            String allExplicitNames2 = (String)obj.allExplicitNames.elementAt(i);
            allExplicitNames_equal = allExplicitNames1.equals(allExplicitNames2);
        }
        // Compare the elements in the explicitIds vector.
        boolean explicitIds_equal = (obj.explicitIds.size() == explicitIds.size());
        for(i = 0; (i < explicitIds.size()) && explicitIds_equal; ++i)
        {
            // Make references to Integer from Object.
            Integer explicitIds1 = (Integer)explicitIds.elementAt(i);
            Integer explicitIds2 = (Integer)obj.explicitIds.elementAt(i);
            explicitIds_equal = explicitIds1.equals(explicitIds2);
        }
        // Compare the elements in the explicitNames vector.
        boolean explicitNames_equal = (obj.explicitNames.size() == explicitNames.size());
        for(i = 0; (i < explicitNames.size()) && explicitNames_equal; ++i)
        {
            // Make references to String from Object.
            String explicitNames1 = (String)explicitNames.elementAt(i);
            String explicitNames2 = (String)obj.explicitNames.elementAt(i);
            explicitNames_equal = explicitNames1.equals(explicitNames2);
        }
        // Create the return value
        return ((namescheme.equals(obj.namescheme)) &&
                externalArrayNames_equal &&
                externalArrayOffsets_equal &&
                externalArrayData_equal &&
                allExplicitNames_equal &&
                explicitIds_equal &&
                explicitNames_equal);
    }

    // Property setting methods
    public void SetNamescheme(String namescheme_)
    {
        namescheme = namescheme_;
        Select(0);
    }

    public void SetExternalArrayNames(Vector externalArrayNames_)
    {
        externalArrayNames = externalArrayNames_;
        Select(1);
    }

    public void SetExternalArrayOffsets(Vector externalArrayOffsets_)
    {
        externalArrayOffsets = externalArrayOffsets_;
        Select(2);
    }

    public void SetExternalArrayData(Vector externalArrayData_)
    {
        externalArrayData = externalArrayData_;
        Select(3);
    }

    public void SetAllExplicitNames(Vector allExplicitNames_)
    {
        allExplicitNames = allExplicitNames_;
        Select(4);
    }

    public void SetExplicitIds(Vector explicitIds_)
    {
        explicitIds = explicitIds_;
        Select(5);
    }

    public void SetExplicitNames(Vector explicitNames_)
    {
        explicitNames = explicitNames_;
        Select(6);
    }

    // Property getting methods
    public String GetNamescheme() { return namescheme; }
    public Vector GetExternalArrayNames() { return externalArrayNames; }
    public Vector GetExternalArrayOffsets() { return externalArrayOffsets; }
    public Vector GetExternalArrayData() { return externalArrayData; }
    public Vector GetAllExplicitNames() { return allExplicitNames; }
    public Vector GetExplicitIds() { return explicitIds; }
    public Vector GetExplicitNames() { return explicitNames; }

    // Write and read methods.
    public void WriteAtts(CommunicationBuffer buf)
    {
        if(WriteSelect(0, buf))
            buf.WriteString(namescheme);
        if(WriteSelect(1, buf))
            buf.WriteStringVector(externalArrayNames);
        if(WriteSelect(2, buf))
            buf.WriteIntVector(externalArrayOffsets);
        if(WriteSelect(3, buf))
            buf.WriteIntVector(externalArrayData);
        if(WriteSelect(4, buf))
            buf.WriteStringVector(allExplicitNames);
        if(WriteSelect(5, buf))
            buf.WriteIntVector(explicitIds);
        if(WriteSelect(6, buf))
            buf.WriteStringVector(explicitNames);
    }

    public void ReadAtts(int index, CommunicationBuffer buf)
    {
        switch(index)
        {
        case 0:
            SetNamescheme(buf.ReadString());
            break;
        case 1:
            SetExternalArrayNames(buf.ReadStringVector());
            break;
        case 2:
            SetExternalArrayOffsets(buf.ReadIntVector());
            break;
        case 3:
            SetExternalArrayData(buf.ReadIntVector());
            break;
        case 4:
            SetAllExplicitNames(buf.ReadStringVector());
            break;
        case 5:
            SetExplicitIds(buf.ReadIntVector());
            break;
        case 6:
            SetExplicitNames(buf.ReadStringVector());
            break;
        }
    }

    public String toString(String indent)
    {
        String str = new String();
        str = str + stringToString("namescheme", namescheme, indent) + "\n";
        str = str + stringVectorToString("externalArrayNames", externalArrayNames, indent) + "\n";
        str = str + intVectorToString("externalArrayOffsets", externalArrayOffsets, indent) + "\n";
        str = str + intVectorToString("externalArrayData", externalArrayData, indent) + "\n";
        str = str + stringVectorToString("allExplicitNames", allExplicitNames, indent) + "\n";
        str = str + intVectorToString("explicitIds", explicitIds, indent) + "\n";
        str = str + stringVectorToString("explicitNames", explicitNames, indent) + "\n";
        return str;
    }


    // Attributes
    private String namescheme;
    private Vector externalArrayNames; // vector of String objects
    private Vector externalArrayOffsets; // vector of Integer objects
    private Vector externalArrayData; // vector of Integer objects
    private Vector allExplicitNames; // vector of String objects
    private Vector explicitIds; // vector of Integer objects
    private Vector explicitNames; // vector of String objects
}

