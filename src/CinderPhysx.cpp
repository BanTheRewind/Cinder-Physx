#include "CinderPhysx.h"

#include "cinder/CinderAssert.h"
#include "cinder/Log.h"
#include "cinder/System.h"

using namespace ci;
using namespace physx;
using namespace physx::debugger;
using namespace physx::debugger::comm;
using namespace std;

PxFilterFlags FilterShader(
	PxFilterObjectAttributes attributes0, PxFilterData filterData0,
	PxFilterObjectAttributes attributes1, PxFilterData filterData1,
	PxPairFlags& pairFlags, const void* constantBlock, PxU32 constantBlockSize )
{
	pairFlags = PxPairFlag::eTRIGGER_DEFAULT | PxPairFlag::eCONTACT_DEFAULT | PxPairFlag::eNOTIFY_TOUCH_FOUND;
	return PxFilterFlag::eDEFAULT;
}

#if defined( CINDER_COCOA_TOUCH )
PhysxRef Physx::create()
{
	return create( PxTolerancesScale() );
}

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
#else
PhysxRef Physx::create( bool connectToPvd )
{
	return create( PxTolerancesScale(), connectToPvd );
}

PhysxRef Physx::create( const PxTolerancesScale& scale, bool connectToPvd )
{
	PxCookingParams params( scale );
	params.meshWeldTolerance	= 0.001f;
	params.meshPreprocessParams = PxMeshPreprocessingFlags( 
		PxMeshPreprocessingFlag::eWELD_VERTICES					| 
		PxMeshPreprocessingFlag::eREMOVE_UNREFERENCED_VERTICES	| 
		PxMeshPreprocessingFlag::eREMOVE_DUPLICATED_TRIANGLES );
	return PhysxRef( new Physx( scale, params, connectToPvd ) );
}

PhysxRef Physx::create( const PxTolerancesScale& scale, const PxCookingParams& params, bool connectToPvd )
{
	return PhysxRef( new Physx( scale, params, connectToPvd ) );
}
#endif

Physx::Physx( const PxTolerancesScale& scale, const PxCookingParams& params
#if !defined( CINDER_COCOA_TOUCH )
, bool connectToPvd
#endif
)
: mCooking( nullptr ), mCpuDispatcher( nullptr ), mFoundation( nullptr ),
mPhysics( nullptr ), mProfileZoneManager( nullptr )
#if !defined( CINDER_COCOA_TOUCH )
, mPvdConnection( nullptr )
#endif
#if PX_SUPPORT_GPU_PHYSX
, mCudaContextManager( nullptr )
#endif
{
	mFoundation = PxCreateFoundation( PX_PHYSICS_VERSION, mAllocator, getErrorCallback() );
	CI_ASSERT( mFoundation != nullptr );

#if !defined( CINDER_COCOA_TOUCH )
	if ( connectToPvd ) {
		mProfileZoneManager = &PxProfileZoneManager::createProfileZoneManager( mFoundation );
		CI_ASSERT( mProfileZoneManager != nullptr );
	}
#endif

#if defined( CINDER_COCOA_TOUCH )
	mPhysics = PxCreatePhysics( PX_PHYSICS_VERSION, *mFoundation, scale, false, mProfileZoneManager );
#else
	mPhysics = PxCreatePhysics( PX_PHYSICS_VERSION, *mFoundation, scale, connectToPvd, mProfileZoneManager );
#endif
	CI_ASSERT( mPhysics != nullptr );
	CI_ASSERT( PxInitExtensions( *mPhysics ) );

	mCooking = PxCreateCooking( PX_PHYSICS_VERSION, *mFoundation, params );
	CI_ASSERT( mCooking != nullptr );

	mCpuDispatcher = PxDefaultCpuDispatcherCreate( System::getNumCores() );
	CI_ASSERT( mCpuDispatcher != nullptr );

#if PX_SUPPORT_GPU_PHYSX
	PxCudaContextManagerDesc cudaContextManagerDesc;
	mCudaContextManager = PxCreateCudaContextManager( *mFoundation, cudaContextManagerDesc, mProfileZoneManager );
	if ( mCudaContextManager && !mCudaContextManager->contextIsValid() ) {
		mCudaContextManager->release();
		mCudaContextManager = nullptr;
	}
#endif

#if !defined( CINDER_COCOA_TOUCH )
	if ( connectToPvd && mPhysics->getPvdConnectionManager() ) {
		mPhysics->getPvdConnectionManager()->addHandler( *this );
	}
#endif
}

Physx::~Physx()
{
#if !defined( CINDER_COCOA_TOUCH )
	pvdDisconnect();
#endif
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
	if ( mCudaContextManager != nullptr ) {
		mCudaContextManager->release();
		mCudaContextManager = nullptr;
	}
#endif
	if ( mPhysics != nullptr ) {
		mPhysics->release();
		mPhysics = nullptr;
	}
	if ( mFoundation != nullptr ) {
		mFoundation->release();
		mFoundation = nullptr;
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

PxProfileZoneManager* Physx::getProfileZoneManager() const
{
	return mProfileZoneManager;
}

#if !defined( CINDER_COCOA_TOUCH )
PvdConnection* Physx::getPvdConnection() const
{
	return mPvdConnection;
}
#endif

void Physx::update( float deltaInSeconds )
{
	for ( uint32_t id : mDeletedActors ) {
		map<uint32_t, PxActor*>::iterator iter = mActors.find( id );
		if ( iter != mActors.end() ) {
			if ( iter->second != nullptr ) {
				iter->second->release();
				iter->second = nullptr;
			}
			mActors.erase( iter );
		}
	}
	mDeletedActors.clear();

	for ( auto& iter : mScenes ) {
		iter.second->simulate( deltaInSeconds );
		while ( !iter.second->fetchResults( true ) ) {
		}
	}
}

uint32_t Physx::addActor( PxActor* actor, uint32_t sceneId )
{
	return addActor( actor, getScene( sceneId ) );
}

uint32_t Physx::addActor( PxActor* actor, PxScene* scene )
{
	uint32_t id			= mActors.empty() ? 0 : mActors.rbegin()->first + 1;
	uintptr_t userData	= id;
	actor->userData		= (void*)userData;
	mActors[ id ]		= actor;
	scene->addActor( *actor );
	return id;
}

void Physx::clearActors()
{
	for ( const auto& iter : mActors ) {
		eraseActor( iter.first );
	}
}

void Physx::eraseActor( uint32_t id )
{
	mDeletedActors.push_back( id );
}

void Physx::eraseActor( PxActor& actor )
{
	uintptr_t id = (uintptr_t)actor.userData;
	eraseActor( (uint32_t)id );
}

void Physx::eraseActor( PxActor* actor )
{
	uintptr_t id = (uintptr_t)actor->userData;
	eraseActor( (uint32_t)id );
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

void Physx::clearScenes()
{
	vector<uint32_t> ids;
	for ( const auto& iter : mScenes ) {
		ids.push_back( iter.first );
	}
	for ( uint32_t id : ids ) {
		eraseScene( id );
	}
}

uint32_t Physx::createScene()
{
	CI_ASSERT( mPhysics != nullptr );
	CI_ASSERT( mCpuDispatcher != nullptr );
	PxSceneDesc desc( mPhysics->getTolerancesScale() );
	desc.broadPhaseType = PxBroadPhaseType::eMBP;
	desc.cpuDispatcher	= mCpuDispatcher;
	desc.filterShader	= FilterShader;

	desc.flags			|= PxSceneFlag::eENABLE_ACTIVETRANSFORMS;
	desc.gravity		= PxVec3( 0.0f, -9.81f, 0.0f );
	
#if PX_SUPPORT_GPU_PHYSX
	if ( mCudaContextManager != nullptr && mCudaContextManager->contextIsValid() ) {
		desc.gpuDispatcher = mCudaContextManager->getGpuDispatcher();
	}
#endif
	return createScene( desc );
}

uint32_t Physx::createScene( const PxSceneDesc& desc )
{
	CI_ASSERT( mPhysics != nullptr );
	PxScene* scene	= mPhysics->createScene( desc );
	PxBroadPhaseRegion broadPhaseRegion;
	broadPhaseRegion.bounds = to( AxisAlignedBox( vec3( -100.0f ), vec3( 100.0f ) ) );
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

PxConvexMesh* Physx::createConvexMesh( const vector<vec3>& positions, PxConvexFlags flags )
{
	if ( positions.empty() ) {
		return nullptr;
	}
	PxConvexMeshDesc desc;
	desc.points.count	= (PxU32)positions.size();
	desc.points.data	= (PxVec3*)&positions[ 0 ];
	desc.points.stride	= sizeof( PxVec3 );
	desc.flags			= flags;
	
	PxDefaultMemoryOutputStream buffer;
	if ( !mCooking->cookConvexMesh( desc, buffer ) ) {
		return nullptr;
	}
	
	PxDefaultMemoryInputData input( buffer.getData(), buffer.getSize() );
	return mPhysics->createConvexMesh( input );
}

PxTriangleMesh* Physx::createTriangleMesh( const vector<vec3>& positions, size_t numTriangles, vector<uint32_t> indices )
{
	if ( positions.empty() ) {
		return nullptr;
	}
	if ( indices.empty() ) {
		CI_ASSERT( positions.size() % 3 == 0 );
		uint32_t count = (uint32_t)positions.size();
		for ( uint32_t i = 0; i < count; ++i ) {
			indices.push_back( i );
		}
		numTriangles = indices.size() / 3;
	}
	
	PxTriangleMeshDesc desc;
	desc.points.count		= (PxU32)positions.size();
	desc.points.data		= (PxVec3*)&positions[ 0 ];
	desc.points.stride		= sizeof( PxVec3 );
	desc.triangles.count	= (PxU32)numTriangles;
	desc.triangles.data		= (PxU32*)&indices[ 0 ];
	desc.triangles.stride	= sizeof( PxU32 ) * 3;
	
	PxDefaultMemoryOutputStream writeBuffer;
	if ( !mCooking->cookTriangleMesh( desc, writeBuffer ) ) {
		return nullptr;
	}
	
	PxDefaultMemoryInputData readBuffer( writeBuffer.getData(), writeBuffer.getSize() );
	return mPhysics->createTriangleMesh( readBuffer );
}

#if !defined( CINDER_COCOA_TOUCH )
void Physx::pvdConnect( const string& host, int32_t port, 
						 int32_t timeout, PxVisualDebuggerConnectionFlags connectionFlags )
{
	pvdDisconnect();
	if ( mPhysics->getPvdConnectionManager() ) {
		mPvdConnection = PxVisualDebuggerExt::createConnection( 
			mPhysics->getPvdConnectionManager(), 
			host.c_str(), port, timeout, connectionFlags );
	}
}

void Physx::pvdDisconnect()
{
	if ( mPhysics->getPvdConnectionManager() ) {
		if ( mPvdConnection != nullptr ) {
			if ( mPvdConnection->isConnected() ) {
				mPvdConnection->disconnect();
			}
			mPvdConnection->release();
			mPvdConnection = nullptr;
		}
		if ( mPhysics->getPvdConnectionManager()->isConnected() ) {
			mPhysics->getPvdConnectionManager()->disconnect();
		}
	}
}

void Physx::onPvdSendClassDescriptions( PvdConnection& )
{
}

void Physx::onPvdConnected( PvdConnection& )
{
	mPhysics->getVisualDebugger()->setVisualizeConstraints( true );
	mPhysics->getVisualDebugger()->setVisualDebuggerFlag( PxVisualDebuggerFlag::eTRANSMIT_CONSTRAINTS,	true );
	mPhysics->getVisualDebugger()->setVisualDebuggerFlag( PxVisualDebuggerFlag::eTRANSMIT_CONTACTS,		true );
	mPhysics->getVisualDebugger()->setVisualDebuggerFlag( PxVisualDebuggerFlag::eTRANSMIT_SCENEQUERIES, true );
}

void Physx::onPvdDisconnected( PvdConnection& )
{
}
#endif

PxErrorCallback& Physx::getErrorCallback()
{
	static PxDefaultErrorCallback defaultErrorCallback;
	return defaultErrorCallback;
}
