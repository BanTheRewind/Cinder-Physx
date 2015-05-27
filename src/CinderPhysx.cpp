#include "CinderPhysx.h"

using namespace ci;
using namespace physx;
using namespace std;

PhysxRef Physx::create()
{
	return PhysxRef( new Physx() );
}

Physx::Physx()
: mBufferedActiveTransforms( nullptr ), mCooking( nullptr ), 
mCpuDispatcher( nullptr ), mCudaContextManager( nullptr ), mFoundation( nullptr ), 
mMaterial( nullptr ), mPhysics( nullptr ), mProfileZoneManager( nullptr ), 
mScene( nullptr )
{
}

Physx::~Physx()
{
}

uint32_t Physx::from( PxU32 v )
{

}

float Physx::from( PxReal v )
{

}

mat3 Physx::from( const PxMat33& m )
{

}

mat4 Physx::from( const PxMat44& m )
{

}

quat Physx::from( const PxQuat& m )
{

}

vec2 Physx::from( const PxVec2& v )
{

}

vec3 Physx::from( const PxVec3& v )
{

}

vec4 Physx::from( const PxVec4& v )
{

}

pair<quat, vec3> Physx::from( const PxTransform& t )
{

}

AxisAlignedBox Physx::from( const PxBounds3& b )
{

}


PxU32 Physx::to( uint32_t v )
{

}

PxReal Physx::to( float v )
{

}

PxMat33 Physx::to( const mat3& m )
{

}

PxMat44 Physx::to( const mat4& m )
{

}

PxQuat Physx::to( const quat& m )
{

}

PxVec2 Physx::to( const vec2& v )
{

}

PxVec3 Physx::to( const vec3& v )
{

}

PxVec4 Physx::to( const vec4& v )
{

}

PxTransform Physx::to( const quat& m, const vec3& v )
{

}

PxTransform Physx::to( const pair<quat, vec3>& p )
{

}

PxBounds3 Physx::to( const AxisAlignedBox& b )
{

}

PxDefaultAllocator Physx::getAllocator()
{
	return mAllocator;
}

PxActiveTransform* Physx::getBufferedActiveTransforms()
{
	return mBufferedActiveTransforms;
}

PxCooking* Physx::getCooking()
{
	return mCooking;
}

PxDefaultCpuDispatcher* Physx::getCpuDispatcher()
{
	return mCpuDispatcher;
}

PxCudaContextManager* Physx::getCudaContextManager()
{
	return mCudaContextManager;
}

PxFoundation* Physx::getFoundation()
{
	return mFoundation;
}

PxMaterial* Physx::getMaterial()
{
	return mMaterial;
}

PxPhysics* Physx::getPhysics()
{
	return mPhysics;
}

PxProfileZoneManager* Physx::getProfileZoneManager()
{
	return mProfileZoneManager;
}

PxScene* Physx::getScene()
{
	return mScene;
}

PxErrorCallback& Physx::getErrorCallback()
{
	static PxDefaultErrorCallback defaultErrorCallback;
	return defaultErrorCallback;
}
