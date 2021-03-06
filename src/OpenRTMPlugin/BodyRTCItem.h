/**
   @author Shizuko Hattori
   @author Shin'ichiro Nakaoka
*/

#ifndef CNOID_OPENRTM_PLUGIN_BODY_RTC_ITEM_H
#define CNOID_OPENRTM_PLUGIN_BODY_RTC_ITEM_H

#include "VirtualRobotRTC.h"
#include "RTCItem.h"
#include <cnoid/ControllerItem>
#include <cnoid/BasicSensorSimulationHelper>
#include <cnoid/Body>
#ifdef ENABLE_SIMULATION_PROFILING
#include <cnoid/TimeMeasure>
#endif
#include "exportdecl.h"

namespace cnoid {

class MessageView;

class CNOID_EXPORT BodyRTCItem : public ControllerItem
{
public:
    static void initialize(ExtensionManager* ext);
        
    BodyRTCItem();
    BodyRTCItem(const BodyRTCItem& org);
    virtual ~BodyRTCItem();
        
    virtual bool start(Target* target);
    virtual double timeStep() const;
    virtual void input();
    virtual bool control();
    virtual void output();
    virtual void stop();

    const BodyPtr& body() const { return simulationBody; };
    const DeviceList<ForceSensor>& forceSensors() const { return forceSensors_; }
    const DeviceList<RateGyroSensor>& gyroSensors() const { return gyroSensors_; }
    const DeviceList<AccelSensor>& accelSensors() const { return accelSensors_; }

    double controlTime() const { return controlTime_; }
       
    enum ConfigMode {
        CONF_FILE_MODE = 0,
        CONF_ALL_MODE,
        N_CONFIG_MODES
    };

    void setControllerModule(const std::string& name);
    void setConfigFile(const std::string& filename);
    void setConfigMode(int mode);
    void setPeriodicRate(double freq);
    void setAutoConnectionMode(bool on); 

#ifdef ENABLE_SIMULATION_PROFILING
    virtual void getProfilingNames(std::vector<std::string>& profilingNames);
    virtual void getProfilingTimes(std::vector<double>& profilingTimes);
#endif

protected:
    virtual void onPositionChanged();
    virtual void onDisconnectedFromRoot();
    virtual ItemPtr doDuplicate() const;
    virtual void doPutProperties(PutPropertyFunction& putProperty);
    virtual bool store(Archive& archive);
    virtual bool restore(const Archive& archive);
        
private:
    BodyPtr simulationBody;
    DeviceList<ForceSensor> forceSensors_;
    DeviceList<RateGyroSensor> gyroSensors_;
    DeviceList<AccelSensor> accelSensors_;
    double timeStep_;

    // The world time step is used if the following values are 0
    double executionCycleProperty;
    double executionCycle;
    double executionCycleCounter;
        
    const Target* controllerTarget;
    double controlTime_;
    std::ostream& os;

    std::string bodyName;
    RTC::CorbaNaming* naming;
    BridgeConf* bridgeConf;
    VirtualRobotRTC* virtualRobotRTC;
    OpenRTM::ExtTrigExecutionContextService_var virtualRobotEC;
        
    Selection configMode;
    bool autoConnect;
    RTComponent* rtcomp;

    typedef std::map<std::string, RTC::PortService_var> PortMap;

    struct RtcInfo
    {
        RTC::RTObject_var rtcRef;
        PortMap portMap;
        OpenRTM::ExtTrigExecutionContextService_var execContext;
        double timeRate;
        double timeRateCounter;
    };
    typedef boost::shared_ptr<RtcInfo> RtcInfoPtr;

    typedef std::map<std::string, RtcInfoPtr> RtcInfoMap;
    RtcInfoMap rtcInfoMap;
    typedef std::vector<RtcInfoPtr> RtcInfoVector;
    RtcInfoVector rtcInfoVector;

    std::string moduleName;
    std::string moduleFileName;
    std::string confFileName;
    std::string instanceName;
    int oldMode;
    MessageView* mv;

    void createRTC(BodyPtr body);
    void setdefaultPort(BodyPtr body);
    void activateComponents();
    void deactivateComponents();
    void detectRtcs();
    void setupRtcConnections();
    RtcInfoPtr addRtcVectorWithConnection(RTC::RTObject_var new_rtcRef);
    void makePortMap(RtcInfoPtr& rtcInfo);
    int connectPorts(RTC::PortService_var outPort, RTC::PortService_var inPort);

    void setInstanceName(const std::string& name);
    void deleteModule(bool waitToBeDeleted);

#ifdef ENABLE_SIMULATION_PROFILING
    double bodyRTCTime;
    double controllerTime;
    TimeMeasure timer;
#endif

};
        
typedef ref_ptr<BodyRTCItem> BodyRTCItemPtr;

}

#endif
