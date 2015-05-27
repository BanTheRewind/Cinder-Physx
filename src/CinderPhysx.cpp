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

mat3 Physx::from( const PxMat33& m )
{
	return mat3(
		m.column0.x, m.column0.y, m.column0.z, 
		m.column1.x, m.column1.y, m.column1.z, 
		m.column2.x, m.column2.y, m.column2.z
		);
}

mat4 Physx::from( const PxMat44& m )
{
	return mat4(
		m.column0.x, m.column0.y, m.column0.z, m.column0.w, 
		m.column1.x, m.column1.y, m.column1.z, m.column1.w,
		m.column2.x, m.column2.y, m.column2.z, m.column2.w,
		m.column3.x, m.column3.y, m.column3.z, m.column3.w
		);
}

quat Physx::from( const PxQuat& q )
{
	return quat( q.w, q.x, q.y, q.z );
}

vec2 Physx::from( const PxVec2& v )
{
	return vec2( v.x, v.y );
}

vec3 Physx::from( const PxVec3& v )
{
	return vec3( v.x, v.y, v.z );
}

vec4 Physx::from( const PxVec4& v )
{
	return vec4( v.x, v.y, v.z, v.w );
}

pair<quat, vec3> Physx::from( const PxTransform& t )
{
	return make_pair( from( t.q ), from( t.p ) );
}

AxisAlignedBox Physx::from( const PxBounds3& b )
{
	return AxisAlignedBox( from( b.minimum ), from( b.maximum ) );
}

PxMat33 Physx::to( const mat3& m )
{
	return PxMat33( 
		to( vec3( m[ 0 ] ) ), 
		to( vec3( m[ 1 ] ) ), 
		to( vec3( m[ 2 ] ) ) 
		);
}

PxMat44 Physx::to( const mat4& m )
{
	return PxMat44(
		to( vec4( m[ 0 ] ) ),
		to( vec4( m[ 1 ] ) ),
		to( vec4( m[ 2 ] ) ), 
		to( vec4( m[ 3 ] ) )
		);
}

PxQuat Physx::to( const quat& q )
{
	return PxQuat( q.x, q.y, q.z, q.w );
}

PxVec2 Physx::to( const vec2& v )
{
	return PxVec2( v.x, v.y );
}

PxVec3 Physx::to( const vec3& v )
{
	return PxVec3( v.x, v.y, v.z );
}

PxVec4 Physx::to( const vec4& v )
{
	return PxVec4( v.x, v.y, v.z, v.w );
}

PxTransform Physx::to( const quat& q, const vec3& v )
{
	return PxTransform( to( v ), to( q ) );
}

PxTransform Physx::to( const pair<quat, vec3>& p )
{
	return PxTransform( to( p.second ), to( p.first ) );
}

PxBounds3 Physx::to( const AxisAlignedBox& b )
{
	return PxBounds3( to( b.getMin() ), to( b.getMax() ) );
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
