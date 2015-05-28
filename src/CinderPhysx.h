#pragma once

#include "cinder/AxisAlignedBox.h"
#include "cinder/Matrix.h"
#include "cinder/Quaternion.h"
#include "PxPhysics.h"
#include "PxPhysicsAPI.h"
#include "extensions/PxExtensionsAPI.h"
#include <map>
#include <memory>

typedef std::shared_ptr<class Physx>		PhysxRef;

class Physx
{
public:
	static PhysxRef									create( const physx::PxTolerancesScale& scale = physx::PxTolerancesScale() );
	static PhysxRef									create( const physx::PxTolerancesScale& scale, 
														   const physx::PxCookingParams& params );
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

	void											update( float deltaInSeconds = 1.0f / 60.0f );

	uint32_t										addActor( physx::PxActor* actor, uint32_t sceneId );
	uint32_t										addActor( physx::PxActor* actor, physx::PxScene* scene );
	void											eraseActor( uint32_t id );
	void											eraseActor( physx::PxActor* actor );
	physx::PxActor*									getActor( uint32_t id = 0 ) const;
	const std::map<uint32_t, physx::PxActor*>&		getActors() const;

	uint32_t										createScene();
	uint32_t										createScene( const physx::PxSceneDesc& desc );
	void											eraseScene( uint32_t id );
	void											eraseScene( physx::PxScene* scene );
	physx::PxScene*									getScene( uint32_t id = 0 ) const;
	const std::map<uint32_t, physx::PxScene*>&		getScenes() const;
protected:
	Physx( const physx::PxTolerancesScale& scale, const physx::PxCookingParams& params );

	physx::PxErrorCallback&							getErrorCallback();
	std::map<uint32_t, physx::PxActor*>				mActors;
	physx::PxDefaultAllocator						mAllocator;
	physx::PxCooking*								mCooking;
	physx::PxDefaultCpuDispatcher*					mCpuDispatcher;
#if PX_SUPPORT_GPU_PHYSX
	physx::PxCudaContextManager*					mCudaContextManager;
#endif
	physx::PxFoundation*							mFoundation;
	physx::PxPhysics*								mPhysics;
	std::map<uint32_t, physx::PxScene*>				mScenes;
};
