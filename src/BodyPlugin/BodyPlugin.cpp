/*! @file
  @author Shin'ichiro Nakaoka
*/

#include "BodyItem.h"
#include "BodyMotionItem.h"
#include "WorldItem.h"
#include "WorldLogFileItem.h"
#include "SensorVisualizerItem.h"
#include "BodyTrackingCameraItem.h"
#include "SimulatorItem.h"
#include "AISTSimulatorItem.h"
#include "GLVisionSimulatorItem.h"
#include "ZMPSeqItem.h"
#include "MultiDeviceStateSeqItem.h"
//#include "FilterDialogs.h"
#include "KinematicFaultChecker.h"
#include "BodyBar.h"
#include "LeggedBodyBar.h"
#include "LinkSelectionView.h"
#include "BodyLinkView.h"
#include "JointSliderView.h"
#include "JointStateView.h"
#include "BodyStateView.h"
#include "JointGraphView.h"
#include "LinkGraphView.h"
#include "KinematicsBar.h"
#include "SimulationBar.h"
#include "BodyMotionEngine.h"
#include "EditableSceneBody.h"
#include "HrpsysFileIO.h"
#include "CollisionSeqEngine.h"
#include "CollisionSeqItem.h"
#include <cnoid/ExecutablePath>
#include <cnoid/Plugin>
#include <cnoid/ItemManager>
#include <boost/bind.hpp>
#include "gettext.h"

using namespace cnoid;

namespace {
  
class BodyPlugin : public Plugin
{
public:
    BodyPlugin() : Plugin("Body") {
        setActivationPriority(0);
    }

    virtual bool initialize(){

        Body::addCustomizerDirectory(
            executableTopDirectory() + "/" + CNOID_PLUGIN_SUBDIR + "/customizer");

        BodyItem::initializeClass(this);
        BodyMotionItem::initializeClass(this);
        WorldItem::initializeClass(this);
        WorldLogFileItem::initializeClass(this);
        SimulatorItem::initializeClass(this);
        AISTSimulatorItem::initializeClass(this);
        GLVisionSimulatorItem::initializeClass(this);
        SensorVisualizerItem::initializeClass(this);
        BodyTrackingCameraItem::initializeClass(this);

        BodyMotionEngine::initialize(this);
        CollisionSeqEngine::initialize(this);
        //initializeFilterDialogs(*this);
        KinematicFaultChecker::initialize(this);

        // This should be after the initialization of BodyMotionEngine
        ZMPSeqItem::initializeClass(this); 
        MultiDeviceStateSeqItem::initializeClass(this);

        EditableSceneBody::initializeClass(this);

        SimulationBar::initialize(this);
        addToolBar(BodyBar::instance());
        addToolBar(LeggedBodyBar::instance());
        addToolBar(KinematicsBar::instance());

        LinkSelectionView::initializeClass(this);
        BodyLinkView::initializeClass(this);
        JointSliderView::initializeClass(this);
        JointStateView::initializeClass(this);
        BodyStateView::initializeClass(this);
        JointGraphView::initializeClass(this);
        LinkGraphView::initializeClass(this);

        CollisionSeqItem::initislizeClass(this);

        initializeHrpsysFileIO(this);

        return true;
    }

    virtual const char* description() {

        static std::string text =
            str(fmt(_("Body Plugin Version %1%\n")) % CNOID_FULL_VERSION_STRING) +
            "\n" +
            _("This plugin has been developed by Shin'ichiro Nakaoka and Choreonoid Development Team, AIST, "
              "and is distributed as a part of the Choreonoid package.\n"
              "\n") +
            LGPLtext() +
            "\n" +
            _("The Collision deteciton module used in this plugin is implemented using "
              "the OPCODE library (http://www.codercorner.com/Opcode.htm).\n");
        
        return text.c_str();
        
    }
};
}

CNOID_IMPLEMENT_PLUGIN_ENTRY(BodyPlugin);
