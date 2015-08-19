#pragma once

#include "cinder/AxisAlignedBox.h"
#include "cinder/Matrix.h"
#include "cinder/Quaternion.h"
#include "PxPhysics.h"
#include "PxPhysicsAPI.h"
#include "extensions/PxExtensionsAPI.h"
#include <map>
#include <memory>
#include <vector>

physx::PxFilterFlags FilterShader(
	physx::PxFilterObjectAttributes, physx::PxFilterData,
	physx::PxFilterObjectAttributes, physx::PxFilterData,
	physx::PxPairFlags&, const void*, physx::PxU32 );

typedef std::shared_ptr<class Physx> PhysxRef;

class Physx
#if !defined( CINDER_COCOA_TOUCH )
: public physx::debugger::comm::PvdConnectionHandler
#endif
{
public:
#if defined( CINDER_COCOA_TOUCH )
	static PhysxRef									create();
	static PhysxRef									create( const physx::PxTolerancesScale& scale );
	static PhysxRef									create( const physx::PxTolerancesScale& scale,
														   const physx::PxCookingParams& params );
#else
	static PhysxRef									create( bool connectToPvd = true );
	static PhysxRef									create( const physx::PxTolerancesScale& scale, bool connectToPvd = true );
	static PhysxRef									create( const physx::PxTolerancesScale& scale, 
														   const physx::PxCookingParams& params, bool connectToPvd = true );
#endif
	~Physx();

	static ci::mat3									from( const physx::PxMat33& m );
	static ci::mat4									from( const physx::PxMat44& m );
	static ci::mat4									from( const physx::PxTransform& t );
	static ci::mat4									from( const physx::PxQuat& q, const physx::PxVec3& v );
	static ci::mat4									from( const physx::PxMat33& m, const physx::PxVec3& v );
	static ci::quat									from( const physx::PxQuat& q );
	static ci::vec2									from( const physx::PxVec2& v );
	static ci::vec3									from( const physx::PxVec3& v );
	static ci::vec4									from( const physx::PxVec4& v );
	static ci::AxisAlignedBox						from( const physx::PxBounds3& b );

	static physx::PxMat33							to( const ci::mat3& m );
	static physx::PxMat44							to( const ci::mat4& m );
	static physx::PxQuat							to( const ci::quat& m );
	static physx::PxVec2							to( const ci::vec2& v );
	static physx::PxVec3							to( const ci::vec3& v );
	static physx::PxVec4							to( const ci::vec4& v );
	static physx::PxTransform						to( const ci::quat& q, const ci::vec3& v );
	static physx::PxBounds3							to( const ci::AxisAlignedBox& b );

	physx::PxDefaultAllocator						getAllocator() const;
	physx::PxActiveTransform*						getBufferedActiveTransforms() const;
	physx::PxCooking*								getCooking() const;
	physx::PxDefaultCpuDispatcher*					getCpuDispatcher() const;
#if PX_SUPPORT_GPU_PHYSX
	physx::PxCudaContextManager*					getCudaContextManager() const;
#endif
	physx::PxFoundation*							getFoundation() const;
	physx::PxPhysics*								getPhysics() const;
	physx::PxProfileZoneManager*					getProfileZoneManager() const;
#if !defined( CINDER_COCOA_TOUCH )
	physx::debugger::comm::PvdConnection*			getPvdConnection() const;
#endif

	void											update( float deltaInSeconds = 1.0f / 60.0f );

	uint32_t										addActor( physx::PxActor* actor, uint32_t sceneId );
	uint32_t										addActor( physx::PxActor* actor, physx::PxScene* scene );
	void											clearActors();
	void											eraseActor( uint32_t id );
	void											eraseActor( physx::PxActor& actor );
	void											eraseActor( physx::PxActor* actor );
	physx::PxActor*									getActor( uint32_t id = 0 ) const;
	const std::map<uint32_t, physx::PxActor*>&		getActors() const;

	void											clearScenes();
	uint32_t										createScene();
	uint32_t										createScene( const physx::PxSceneDesc& desc );
	void											eraseScene( uint32_t id );
	void											eraseScene( physx::PxScene* scene );
	physx::PxScene*									getScene( uint32_t id = 0 ) const;
	const std::map<uint32_t, physx::PxScene*>&		getScenes() const;

#if !defined( CINDER_COCOA_TOUCH )
	void											pvdConnect( const std::string& host = "127.0.0.1", int32_t port = 5425, 
																int32_t timeout = 1000, 
																physx::debugger::PxVisualDebuggerConnectionFlags connectionFlags = 
																physx::debugger::PxVisualDebuggerExt::getAllConnectionFlags() );
	void											pvdDisconnect();
#endif
	
	physx::PxConvexMesh*							createConvexMesh( const std::vector<ci::vec3>& positions,
																	 physx::PxConvexFlags flags = physx::PxConvexFlag::eCOMPUTE_CONVEX );
	physx::PxTriangleMesh*							createTriangleMesh( const std::vector<ci::vec3>& positions,
																	   size_t numTriangles = 0, 
																	   std::vector<uint32_t> indices = std::vector<uint32_t>() );
protected:
#if defined( CINDER_COCOA_TOUCH )
	Physx( const physx::PxTolerancesScale& scale, const physx::PxCookingParams& params );
#else
	Physx( const physx::PxTolerancesScale& scale, const physx::PxCookingParams& params, bool connectToPvd );

	virtual void									onPvdSendClassDescriptions( physx::debugger::comm::PvdConnection& );
	virtual void									onPvdConnected( physx::debugger::comm::PvdConnection& );
	virtual void									onPvdDisconnected( physx::debugger::comm::PvdConnection& );
#endif

	physx::PxErrorCallback&							getErrorCallback();
	std::map<uint32_t, physx::PxActor*>				mActors;
	physx::PxDefaultAllocator						mAllocator;
	physx::PxCooking*								mCooking;
	physx::PxDefaultCpuDispatcher*					mCpuDispatcher;
#if PX_SUPPORT_GPU_PHYSX
	physx::PxCudaContextManager*					mCudaContextManager;
#endif
	std::vector<uint32_t>							mDeletedActors;
	physx::PxFoundation*							mFoundation;
	physx::PxPhysics*								mPhysics;
	physx::PxProfileZoneManager*					mProfileZoneManager;
#if !defined( CINDER_COCOA_TOUCH )
	physx::debugger::comm::PvdConnection*			mPvdConnection;
#endif
	std::map<uint32_t, physx::PxScene*>				mScenes;
};
