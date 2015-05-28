#include "CinderPhysx.h"

#include "cinder/CinderAssert.h"
#include "cinder/Log.h"
#include "cinder/System.h"

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

PhysxRef Physx::create( const PxTolerancesScale& scale, const PxCookingParams& params )
{
	return PhysxRef( new Physx( scale, params ) );
}

Physx::Physx( const PxTolerancesScale& scale, const PxCookingParams& params )
: mCooking( nullptr ), mCpuDispatcher( nullptr ), mFoundation( nullptr ), 
mPhysics( nullptr ), 
#if PX_SUPPORT_GPU_PHYSX
mCudaContextManager( nullptr )
#endif
{
	mFoundation = PxCreateFoundation( PX_PHYSICS_VERSION, mAllocator, getErrorCallback() );
	CI_ASSERT( mFoundation != nullptr );

	mPhysics = PxCreatePhysics( PX_PHYSICS_VERSION, *mFoundation, scale, false, nullptr );
	CI_ASSERT( mPhysics != nullptr );
	CI_ASSERT( PxInitExtensions( *mPhysics ) );

	mCooking = PxCreateCooking( PX_PHYSICS_VERSION, *mFoundation, params );
	CI_ASSERT( mCooking != nullptr );

	mCpuDispatcher = PxDefaultCpuDispatcherCreate( System::getNumCores() );
	CI_ASSERT( mCpuDispatcher != nullptr );

#if PX_SUPPORT_GPU_PHYSX
	PxCudaContextManagerDesc cudaContextManagerDesc;
	mCudaContextManager = PxCreateCudaContextManager( *mFoundation, cudaContextManagerDesc, nullptr );
	if ( mCudaContextManager && !mCudaContextManager->contextIsValid() ) {
		mCudaContextManager->release();
		mCudaContextManager = nullptr;
	}
#endif
}

Physx::~Physx()
{
	for ( auto& iter : mActors ) {
		iter.second->release();
	}
	mActors.clear();

	for ( auto& iter : mScenes ) {
		iter.second->release();
	}
	mScenes.clear();

	if ( mCooking != nullptr ) {
		mCooking->release();
		mCooking = nullptr;
	}
	if ( mCpuDispatcher != nullptr ) {
		mCpuDispatcher->release();
		mCpuDispatcher = nullptr;
	}
#if PX_SUPPORT_GPU_PHYSX
	if ( mCpuDispatcher != nullptr ) {
		mCudaContextManager->release();
		mCudaContextManager = nullptr;
	}
#endif
	if ( mFoundation != nullptr ) {
		mFoundation->release();
		mFoundation = nullptr;
	}
	if ( mPhysics != nullptr ) {
		mPhysics->release();
		mPhysics = nullptr;
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

mat4 Physx::from( const PxTransform& t )
{
	return from( t.q, t.p );
}

mat4 Physx::from( const PxQuat& q, const PxVec3& v )
{
	return from( PxMat33( q ), v );
}

mat4 Physx::from( const PxMat33& m, const PxVec3& v )
{
	return mat4(
		m.column0.x, m.column0.y, m.column0.z, 0.0f, 
		m.column1.x, m.column1.y, m.column1.z, 0.0f, 
		m.column2.x, m.column2.y, m.column2.z, 0.0f, 
		v.x, v.y, v.z, 1.0f
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

PxBounds3 Physx::to( const AxisAlignedBox& b )
{
	return PxBounds3( to( b.getMin() ), to( b.getMax() ) );
}

PxDefaultAllocator Physx::getAllocator() const
{
	return mAllocator;
}

PxCooking* Physx::getCooking() const
{
	return mCooking;
}

PxDefaultCpuDispatcher* Physx::getCpuDispatcher() const
{
	return mCpuDispatcher;
}

#if PX_SUPPORT_GPU_PHYSX
PxCudaContextManager* Physx::getCudaContextManager() const
{
	return mCudaContextManager;
}
#endif

PxFoundation* Physx::getFoundation() const
{
	return mFoundation;
}

PxPhysics* Physx::getPhysics() const
{
	return mPhysics;
}

void Physx::update( float deltaInSeconds )
{
	for ( auto& iter : mScenes ) {
		iter.second->simulate( deltaInSeconds );
		while ( !iter.second->fetchResults( true ) ) {
		}
	}
}

uint32_t Physx::addActor( PxActor* actor, uint32_t sceneId )
{
	return addActor(actor, getScene( sceneId ) );
}

uint32_t Physx::addActor( PxActor* actor, PxScene* scene )
{
	uint32_t id = mActors.empty() ? 0 : mActors.rbegin()->first + 1;
	mActors[ id ] = actor;
	scene->lockWrite();
	scene->addActor( *actor );
	return id;
}

void Physx::eraseActor( uint32_t id )
{
	map<uint32_t, PxActor*>::iterator iter = mActors.find( id );
	if ( iter != mActors.end() ) {
		if ( iter->second != nullptr ) {
			iter->second->release();
			iter->second = nullptr;
		}
		mActors.erase( iter );
	}
}

void Physx::eraseActor( PxActor* actor )
{
	for ( map<uint32_t, PxActor*>::iterator iter = mActors.begin(); iter != mActors.end(); ) {
		if ( iter->second == actor ) {
			if ( actor != nullptr ) {
				actor->release();
				actor = nullptr;
			}
			iter = mActors.erase( iter );
			break;
		} else {
			++iter;
		}
	}
}

PxActor* Physx::getActor( uint32_t id ) const
{
	if ( mActors.find( id ) != mActors.end() ) {
		return mActors.at( id );
	}
	return nullptr;
}

const map<uint32_t, PxActor*>& Physx::getActors() const
{
	return mActors;
}

uint32_t Physx::createScene()
{
	CI_ASSERT( mPhysics != nullptr );
	CI_ASSERT( mCpuDispatcher != nullptr );
	PxSceneDesc desc( mPhysics->getTolerancesScale() );
	desc.broadPhaseType = PxBroadPhaseType::eMBP;
	desc.cpuDispatcher	= mCpuDispatcher;
	desc.filterShader	= PxDefaultSimulationFilterShader;
	desc.flags			|= PxSceneFlag::eENABLE_ACTIVETRANSFORMS | PxSceneFlag::eREQUIRE_RW_LOCK;
	desc.gravity		= PxVec3( 0.0f, -9.81f, 0.0f );
	if ( mCudaContextManager != nullptr && mCudaContextManager->contextIsValid() ) {
		desc.gpuDispatcher = mCudaContextManager->getGpuDispatcher();
	}
	return createScene( desc );
}

uint32_t Physx::createScene( const PxSceneDesc& desc )
{
	CI_ASSERT( mPhysics != nullptr );
	PxScene* scene	= mPhysics->createScene( desc );
	PxBroadPhaseRegion broadPhaseRegion;
	broadPhaseRegion.bounds = to( AxisAlignedBox( vec3( -100.0f ), vec3( 100.0f ) ) );
	scene->lockWrite();
	scene->addBroadPhaseRegion( broadPhaseRegion );
	CI_ASSERT( scene != nullptr );
	uint32_t id		= mScenes.empty() ? 0 : mScenes.rbegin()->first + 1;
	mScenes[ id ]	= scene;
	return id;
}

void Physx::eraseScene( uint32_t id )
{
	map<uint32_t, PxScene*>::iterator iter = mScenes.find( id );
	if ( iter != mScenes.end() ) {
		if ( iter->second != nullptr ) {
			iter->second->release();
			iter->second = nullptr;
		}
		mScenes.erase( iter );
	}
}

void Physx::eraseScene( PxScene* scene )
{
	for ( map<uint32_t, PxScene*>::iterator iter = mScenes.begin(); iter != mScenes.end(); ) {
		if ( iter->second == scene ) {
			if ( scene != nullptr ) {
				scene->release();
				scene = nullptr;
			}
			iter = mScenes.erase( iter );
			break;
		} else {
			++iter;
		}
	}
}

PxScene* Physx::getScene( uint32_t id ) const
{
	if ( mScenes.find( id ) != mScenes.end() ) {
		return mScenes.at( id );
	}
	return nullptr;
}

const map<uint32_t, PxScene*>& Physx::getScenes() const
{
	return mScenes;
}

PxErrorCallback& Physx::getErrorCallback()
{
	static PxDefaultErrorCallback defaultErrorCallback;
	return defaultErrorCallback;
}
