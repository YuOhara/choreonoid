/**
   \file
   \author Shin'ichiro Nakaoka
*/

#include "Body.h"
#include "BodyCustomizerInterface.h"
#include <cnoid/EigenUtil>
#include <cnoid/ValueTree>

using namespace std;
using namespace boost;
using namespace cnoid;

namespace {

const bool PUT_DEBUG_MESSAGE = true;

bool pluginsInDefaultDirectoriesLoaded = false;

#ifndef uint
typedef unsigned int uint;
#endif

struct BodyHandleEntity {
    Body* body;
};

typedef std::map<std::string, Link*> NameToLinkMap;
typedef std::map<std::string, Device*> DeviceNameMap;
typedef std::map<std::string, ReferencedPtr> CacheMap;
}


namespace cnoid {

class BodyImpl
{
public:
    NameToLinkMap nameToLinkMap;
    DeviceNameMap deviceNameMap;
    CacheMap cacheMap;
    MappingPtr info;
    Vector3 centerOfMass;
    double mass;
    std::string name;
    std::string modelName;

    // Members for the customizer
    BodyCustomizerHandle customizerHandle;
    BodyCustomizerInterface* customizerInterface;
    BodyHandleEntity bodyHandleEntity;
    BodyHandle bodyHandle;

    bool installCustomizer(BodyCustomizerInterface* customizerInterface);
};
}


Body::Body()
{
    initialize();
    rootLink_ = createLink();
    numActualJoints = 0;

    impl->centerOfMass.setZero();
    impl->mass = 0.0;
    impl->info = new Mapping();
}


void Body::initialize()
{
    impl = new BodyImpl;
    
    rootLink_ = 0;
    
    impl->customizerHandle = 0;
    impl->customizerInterface = 0;
    impl->bodyHandleEntity.body = this;
    impl->bodyHandle = &impl->bodyHandleEntity;
}


Body::Body(const Body& org)
{
    copy(org);
}


void Body::copy(const Body& org)
{
    initialize();

    impl->centerOfMass = org.impl->centerOfMass;
    impl->mass = org.impl->mass;
    impl->name = org.impl->name;
    impl->modelName = org.impl->modelName;
    impl->info = org.impl->info;

    setRootLink(cloneLinkTree(org.rootLink()));

    // deep copy of the devices
    const DeviceList<>& orgDevices = org.devices();
    for(size_t i=0; i < orgDevices.size(); ++i){
        const Device& orgDevice = *orgDevices[i];
        Device* device = orgDevice.clone();
        device->setLink(link(orgDevice.link()->index()));
        addDevice(device);
    }

    // deep copy of the extraJoints
    for(size_t i=0; i < org.extraJoints_.size(); ++i){
        const ExtraJoint& orgExtraJoint = org.extraJoints_[i];
        ExtraJoint extraJoint(orgExtraJoint);
        for(int j=0; j < 2; ++j){
            extraJoint.link[j] = link(orgExtraJoint.link[j]->index());
        }
        extraJoints_.push_back(extraJoint);
    }

    if(org.impl->customizerInterface){
        installCustomizer(org.impl->customizerInterface);
    }
}


Link* Body::cloneLinkTree(const Link* orgLink)
{
    Link* link = createLink(orgLink);
    for(Link* orgChild = orgLink->child(); orgChild; orgChild = orgChild->sibling()){
        link->appendChild(cloneLinkTree(orgChild));
    }
    return link;
}


Body* Body::clone() const
{
    return new Body(*this);
}


Link* Body::createLink(const Link* org) const
{
    return org ? new Link(*org) : new Link();
}


void Body::cloneShapes(SgCloneMap& cloneMap)
{
    const int n = linkTraverse_.numLinks();
    for(int i=0; i < n; ++i){
        Link* link = linkTraverse_[i];
        SgNode* visualShape = link->visualShape();
        if(visualShape){
            link->setVisualShape(visualShape->cloneNode(cloneMap));
        }
        SgNode* collisionShape = link->collisionShape();
        if(collisionShape){
            if(collisionShape == visualShape){
                link->setCollisionShape(link->visualShape());
            } else {
                link->setCollisionShape(collisionShape->cloneNode(cloneMap));
            }
        }
    }
}


Body::~Body()
{
    if(impl->customizerHandle){
        impl->customizerInterface->destroy(impl->customizerHandle);
    }
    delete rootLink_;
    delete impl;
}


void Body::setRootLink(Link* link)
{
    if(rootLink_){
        delete rootLink_;
    }
    rootLink_ = link;

    if(rootLink_){
        updateLinkTree();
    }
}


Link* Body::createEmptyJoint(int jointId)
{
    Link* empty = createLink();
    empty->setJointId(jointId);
    return empty;
}


void Body::updateLinkTree()
{
    isStaticModel_ = true;
    
    impl->nameToLinkMap.clear();
    linkTraverse_.find(rootLink());

    const int numLinks = linkTraverse_.numLinks();
    jointIdToLinkArray.clear();
    jointIdToLinkArray.reserve(numLinks);
    numActualJoints = 0;
    Link** virtualJoints = (Link**)alloca(numLinks * sizeof(Link*));
    int numVirtualJoints = 0;
    double m = 0.0;
    
    for(int i=0; i < numLinks; ++i){
        Link* link = linkTraverse_[i];
        link->setIndex(i);
        impl->nameToLinkMap[link->name()] = link;

        const int id = link->jointId();
        if(id >= 0){
            if(id >= jointIdToLinkArray.size()){
                jointIdToLinkArray.resize(id + 1, 0);
            }
            if(!jointIdToLinkArray[id]){
                jointIdToLinkArray[id] = link;
                ++numActualJoints;
            }
        }
        if(link->jointType() != Link::FIXED_JOINT){
            isStaticModel_ = false;
            if(id < 0){
                virtualJoints[numVirtualJoints++] = link;
            }
        }
        m += link->mass();
    }

    for(size_t i=0; i < jointIdToLinkArray.size(); ++i){
        if(!jointIdToLinkArray[i]){
            jointIdToLinkArray[i] = createEmptyJoint(i);
            ++numActualJoints;
        }
    }

    if(numVirtualJoints > 0){
        const int n = jointIdToLinkArray.size();
        jointIdToLinkArray.resize(n + numVirtualJoints);
        for(int i=0; i < numVirtualJoints; ++i){
            jointIdToLinkArray[n + i] = virtualJoints[i];
        }
    }

    impl->mass = m;
}


void Body::resetDefaultPosition(const Position& T)
{
    rootLink_->setOffsetPosition(T);
}


const std::string& Body::name() const
{
    return impl->name;
}


void Body::setName(const std::string& name)
{
    impl->name = name;
}


const std::string& Body::modelName() const
{
    return impl->modelName;
}


void Body::setModelName(const std::string& name)
{
    impl->modelName = name;
}


const Mapping* Body::info() const
{
    return impl->info.get();
}


Mapping* Body::info()
{
    return impl->info.get();
}


void Body::resetInfo(Mapping* info)
{
    impl->info = info;
    //doResetInfo(*info);
}


void Body::addDevice(Device* device)
{
    device->setIndex(devices_.size());
    devices_.push_back(device);
    if(!device->name().empty()){
        impl->deviceNameMap[device->name()] = device;
    }
}


void Body::clearDevices()
{
    devices_.clear();
    impl->deviceNameMap.clear();
}


Device* Body::findDeviceSub(const std::string& name) const
{
    DeviceNameMap::const_iterator p = impl->deviceNameMap.find(name);
    if(p != impl->deviceNameMap.end()){
        return p->second;
    }
    return 0;
}


Referenced* Body::findCacheSub(const std::string& name)
{
    CacheMap::iterator p = impl->cacheMap.find(name);
    if(p != impl->cacheMap.end()){
        return p->second.get();
    }
    return 0;
}


const Referenced* Body::findCacheSub(const std::string& name) const
{
    CacheMap::iterator p = impl->cacheMap.find(name);
    if(p != impl->cacheMap.end()){
        return p->second.get();
    }
    return 0;
}


void Body::insertCache(const std::string& name, Referenced* cache)
{
    impl->cacheMap[name] = cache;
}


bool Body::getCaches(PolymorphicReferencedArrayBase<>& out_caches, std::vector<std::string>& out_names) const
{
    out_caches.clear_elements();
    out_names.clear();
    for(CacheMap::const_iterator p = impl->cacheMap.begin(); p != impl->cacheMap.end(); ++p){
        if(out_caches.try_push_back(p->second)){
            out_names.push_back(p->first);
        }
    }
    return !out_names.empty();
}


void Body::removeCache(const std::string& name)
{
    impl->cacheMap.erase(name);
}


Link* Body::link(const std::string& name) const
{
    NameToLinkMap::const_iterator p = impl->nameToLinkMap.find(name);
    return (p != impl->nameToLinkMap.end()) ? p->second : 0;
}


double Body::mass() const
{
    return impl->mass;
}


const Vector3& Body::centerOfMass() const
{
    return impl->centerOfMass;
}


void Body::initializeState()
{
    rootLink_->T() = rootLink_->Tb();

    rootLink_->v().setZero();
    rootLink_->w().setZero();
    rootLink_->dv().setZero();
    rootLink_->dw().setZero();
    
    const int n = linkTraverse_.numLinks();
    for(int i=0; i < n; ++i){
        Link* link = linkTraverse_[i];
        link->u() = 0.0;
        link->q() = 0.0;
        link->dq() = 0.0;
        link->ddq() = 0.0;
    }
 
    calcForwardKinematics(true, true);
    clearExternalForces();
    initializeDeviceStates();
}


void Body::clearExternalForces()
{
    int n = linkTraverse_.numLinks();
    for(int i=0; i < n; ++i){
        Link* link = linkTraverse_[i];
        link->F_ext().setZero();
    }
}


void Body::initializeDeviceStates()
{
    for(size_t i=0; i < devices_.size(); ++i){
        devices_[i]->clearState();
    }
}


const Vector3& Body::calcCenterOfMass()
{
    double m = 0.0;
    Vector3 mc = Vector3::Zero();
    int n = linkTraverse_.numLinks();
    
    for(int i=0; i < n; i++){
        Link* link = linkTraverse_[i];
        link->wc().noalias() = link->R() * link->c() + link->p();
        mc.noalias() += link->m() * link->wc();
        m += link->m();
    }

    impl->centerOfMass = mc / m;
    impl->mass = m;

    return impl->centerOfMass;
}


/**
   assuming Link::v,w is already computed by calcForwardKinematics(true);
   assuming Link::wc is already computed by calcCenterOfMass();
*/
void Body::calcTotalMomentum(Vector3& out_P, Vector3& out_L)
{
    out_P.setZero();
    out_L.setZero();

    Vector3 dwc;    // Center of mass speed in world frame
    Vector3 P;	    // Linear momentum of the link
    Vector3 L;	    // Angular momentum with respect to the world frame origin 
    Vector3 Llocal; // Angular momentum with respect to the center of mass of the link

    int n = linkTraverse_.numLinks();
    
    for(int i=0; i < n; i++){
        Link* link = linkTraverse_[i];
        dwc = link->v() + link->w().cross(link->R() * link->c());
        P   = link->m() * dwc;

        //L   = cross(link->wc, P) + link->R * link->I * trans(link->R) * link->w; 
        Llocal.noalias() = link->I() * link->R().transpose() * link->w();
        L     .noalias() = link->wc().cross(P) + link->R() * Llocal; 

        out_P += P;
        out_L += L;
    }
}


BodyCustomizerHandle Body::customizerHandle() const
{
    return impl->customizerHandle;
}


BodyCustomizerInterface* Body::customizerInterface() const
{
    return impl->customizerInterface;
}


bool Body::hasVirtualJointForces() const
{
    return (impl->customizerInterface && impl->customizerInterface->setVirtualJointForces);
}


void Body::setVirtualJointForces()
{
    if(impl->customizerInterface && impl->customizerInterface->setVirtualJointForces){
        impl->customizerInterface->setVirtualJointForces(impl->customizerHandle);
    }
}


/**
   The function installs the pre-loaded customizer corresponding to the model name.
*/
bool Body::installCustomizer()
{
    if(!pluginsInDefaultDirectoriesLoaded){
        loadBodyCustomizers(bodyInterface());
        pluginsInDefaultDirectoriesLoaded = true;
    }
		
    BodyCustomizerInterface* interface = findBodyCustomizer(impl->modelName);

    return interface ? installCustomizer(interface) : false;
}


bool Body::installCustomizer(BodyCustomizerInterface* customizerInterface)
{
    return impl->installCustomizer(customizerInterface);
}


bool BodyImpl::installCustomizer(BodyCustomizerInterface* customizerInterface)
{
    if(this->customizerInterface){
        if(customizerHandle){
            this->customizerInterface->destroy(customizerHandle);
            customizerHandle = 0;
        }
        this->customizerInterface = 0;
    }
	
    if(customizerInterface){
        customizerHandle = customizerInterface->create(bodyHandle, modelName.c_str());
        if(customizerHandle){
            this->customizerInterface = customizerInterface;
        }
    }

    return (customizerHandle != 0);
}


static inline Link* extractLink(BodyHandle bodyHandle, int linkIndex)
{
    return static_cast<BodyHandleEntity*>(bodyHandle)->body->link(linkIndex);
}


static int getLinkIndexFromName(BodyHandle bodyHandle, const char* linkName)
{
    Body* body = static_cast<BodyHandleEntity*>(bodyHandle)->body;
    Link* link = body->link(linkName);
    return (link ? link->index() : -1);
}


static const char* getLinkName(BodyHandle bodyHandle, int linkIndex)
{
    return extractLink(bodyHandle, linkIndex)->name().c_str();
}


static double* getJointValuePtr(BodyHandle bodyHandle, int linkIndex)
{
    return &(extractLink(bodyHandle,linkIndex)->q());
}


static double* getJointVelocityPtr(BodyHandle bodyHandle, int linkIndex)
{
    return &(extractLink(bodyHandle, linkIndex)->dq());
}


static double* getJointTorqueForcePtr(BodyHandle bodyHandle, int linkIndex)
{
    return &(extractLink(bodyHandle, linkIndex)->u());
}


BodyInterface* Body::bodyInterface()
{
    static BodyInterface interface = {
        BODY_INTERFACE_VERSION,
        getLinkIndexFromName,
        getLinkName,
        getJointValuePtr,
        getJointVelocityPtr,
        getJointTorqueForcePtr,
    };

    return &interface;
}
