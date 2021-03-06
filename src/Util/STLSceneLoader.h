#ifndef CNOID_UTIL_STL_SCENE_LOADER_H
#define CNOID_UTIL_STL_SCENE_LOADER_H

#include "AbstractSceneLoader.h"
#include "exportdecl.h"

namespace cnoid {

class STLSceneLoaderImpl;

class CNOID_EXPORT STLSceneLoader : public AbstractSceneLoader
{
public:
    virtual const char* format() const;
    virtual SgNode* load(const std::string& fileName);

private:
    STLSceneLoaderImpl* impl;
};

};

#endif
