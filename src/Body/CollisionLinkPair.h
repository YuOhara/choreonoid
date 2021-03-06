/*!
  @file
  @author Shin'ichiro Nakaoka
*/

#ifndef CNOID_BODY_COLLISION_LINK_PAIR_H_INCLUDED
#define CNOID_BODY_COLLISION_LINK_PAIR_H_INCLUDED

#include "Body.h"
#include <cnoid/CollisionDetector>

namespace cnoid {
    
struct CollisionLinkPair
{
    CollisionLinkPair() {
        link[0] = 0;
        link[1] = 0;
    }

    CollisionLinkPair(BodyPtr body1, Link* link1, BodyPtr body2, Link* link2, const CollisionPair& collisionPair){
        body[0] = body1;
        body[1] = body2;
        link[0] = link1;
        link[1] = link2;
        collisions = collisionPair.collisions;
    }

    bool isSelfCollision() const {
        return (body[0] == body[1]);
    }
        
    BodyPtr body[2];
    Link* link[2];
    std::vector<Collision> collisions;
};
    
typedef boost::shared_ptr<CollisionLinkPair> CollisionLinkPairPtr;
}

#endif
