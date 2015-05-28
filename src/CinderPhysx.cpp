#include "CinderPhysx.h"

#include "cinder/CinderAssert.h"
#include "cinder/Log.h"

using namespace ci;
using namespace physx;
using namespace std;

PhysxRef Physx::create( const PxTolerancesScale& scale )
{
	PxCookingParams params( scale );
	params.meshWeldTolerance	= 0.001f;
	params.meshPreprocessParams = PxMeshPreprocessingFlags( 
		PxMeshPreprocessingFlag::eWELD_VERTICES					| 
		PxMeshPreprocessingFlag::eREMOVE_UNREFERENCED_VERTICES	| 
		PxMeshPreprocessingFlag::eREMOVE_DUPLICATED_TRIANGLES );
	return PhysxRef( new Physx( scale, params ) );
}

// TODO use an options class which has these args and thread count as members
PhysxRef Physx::create( const PxTolerancesScale& scale, const PxCookingParams& params )
{
	return PhysxRef( new Physx( scale, params ) );
}

Physx::Physx( const PxTolerancesScale& scale, const PxCookingParams& params )
: mBufferedActiveTransforms( nullptr ), mCooking( nullptr ), 
mCpuDispatcher( nullptr ), mCudaContextManager( nullptr ), 
mFoundation( nullptr ), mPhysics( nullptr ), mScene( nullptr )
{
	mFoundation = PxCreateFoundation( PX_PHYSICS_VERSION, mAllocator, getErrorCallback() );
	CI_ASSERT( mFoundation != nullptr );

#if PX_SUPPORT_GPU_PHYSX
	PxCudaContextManagerDesc cudaContextManagerDesc;
	mCudaContextManager = PxCreateCudaContextManager( *mFoundation, cudaContextManagerDesc, nullptr );
	if ( mCudaContextManager && !mCudaContextManager->contextIsValid() ) {
		mCudaContextManager->release();
		mCudaContextManager = nullptr;
	}
#endif

	mPhysics = PxCreatePhysics( PX_PHYSICS_VERSION, *mFoundation, scale, false, nullptr );
	CI_ASSERT( mPhysics != nullptr );

	CI_ASSERT( PxInitExtensions( *mPhysics ) );

	mCooking = PxCreateCooking( PX_PHYSICS_VERSION, *mFoundation, params );
	CI_ASSERT( mCooking != nullptr );

	mPhysics->registerDeletionListener( *this, PxDeletionEventFlag::eUSER_RELEASE );

	mCpuDispatcher = PxDefaultCpuDispatcherCreate( 1 ); // TODO allow multiple threads
	CI_ASSERT( mCpuDispatcher != nullptr );
}

Physx::~Physx()
{
}

void Physx::onRelease( const PxBase* observed, void* userData, PxDeletionEventFlag::Enum deletionEvent )
{
	PX_UNUSED( userData );
	PX_UNUSED( deletionEvent );

	if ( observed->is<PxRigidActor>() ) {
		const PxRigidActor* actor = static_cast<const PxRigidActor*>( observed );

		//std::vector<PxRigidActor*>::iterator iter = std::find( mActors.begin(), mActors.end(), actor );
		//if ( iter != mActors.end() ) {
		//	mActors.erase( iter );
		//}
	}
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

void Physx::createScene()
{
	CI_ASSERT( mCpuDispatcher	!= nullptr );
	CI_ASSERT( mPhysics			!= nullptr );
	PxSceneDesc desc( mPhysics->getTolerancesScale() );
	desc.cpuDispatcher		= mCpuDispatcher;
	desc.filterShader		= PxDefaultSimulationFilterShader;
	desc.flags				|= PxSceneFlag::eENABLE_ACTIVETRANSFORMS | PxSceneFlag::eREQUIRE_RW_LOCK;
	desc.gravity			= PxVec3( 0.0f, -9.81f, 0.0f );
	if ( mCudaContextManager != nullptr && mCudaContextManager->contextIsValid() ) {
		desc.gpuDispatcher	= mCudaContextManager->getGpuDispatcher();
	}
	createScene( desc );
}

void Physx::createScene( const PxSceneDesc& desc )
{
	mScene = mPhysics->createScene( desc );
	CI_ASSERT( mScene != nullptr );
}

PxDefaultAllocator Physx::getAllocator() const
{
	return mAllocator;
}

PxActiveTransform* Physx::getBufferedActiveTransforms() const
{
	return mBufferedActiveTransforms;
}

PxCooking* Physx::getCooking() const
{
	return mCooking;
}

PxDefaultCpuDispatcher* Physx::getCpuDispatcher() const
{
	return mCpuDispatcher;
}

PxCudaContextManager* Physx::getCudaContextManager() const
{
	return mCudaContextManager;
}

PxFoundation* Physx::getFoundation() const
{
	return mFoundation;
}

PxPhysics* Physx::getPhysics() const
{
	return mPhysics;
}

PxScene* Physx::getScene() const
{
	return mScene;
}

PxErrorCallback& Physx::getErrorCallback()
{
	static PxDefaultErrorCallback defaultErrorCallback;
	return defaultErrorCallback;
}
