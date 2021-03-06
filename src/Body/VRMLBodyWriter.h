
/*! @file
*/

#ifndef CNOID_BODY_VRMLBODY_WRITER_INCLUDED
#define CNOID_BODY_VRMLBODY_WRITER_INCLUDED

#include <cnoid/VRMLWriter>
#include "VRMLBody.h"
#include <map>
#include <string>
#include <iostream>
#include <boost/filesystem.hpp>
#include "exportdecl.h"

namespace cnoid {

class VRMLBodyWriter;

class CNOID_EXPORT VRMLBodyWriter : public VRMLWriter
{
public:
    VRMLBodyWriter(std::ostream& out);

protected:
    void registerNodeMethodMap();

private:
    void writeHumanoidNode(VRMLNodePtr node);
    void writeJointNode(VRMLNodePtr node);
    void writeSegmentNode(VRMLNodePtr node);
};

};


#endif
