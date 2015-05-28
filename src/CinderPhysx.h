#pragma once

#include "cinder/AxisAlignedBox.h"
#include "cinder/Matrix.h"
#include "cinder/Quaternion.h"
#include "PxPhysics.h"
#include "PxPhysicsAPI.h"
#include "extensions/PxExtensionsAPI.h"
#include <memory>

typedef std::shared_ptr<class Physx>		PhysxRef;

class Physx : public physx::PxDeletionListener
{
public:
	static PhysxRef							create( const physx::PxTolerancesScale& scale = physx::PxTolerancesScale() );
	static PhysxRef							create( const physx::PxTolerancesScale& scale, 
												   const physx::PxCookingParams& params );
	~Physx();

	static ci::mat3							from( const physx::PxMat33& m );
	static ci::mat4							from( const physx::PxMat44& m );
	static ci::quat							from( const physx::PxQuat& q );
	static ci::vec2							from( const physx::PxVec2& v );
	static ci::vec3							from( const physx::PxVec3& v );
	static ci::vec4							from( const physx::PxVec4& v );
	static std::pair<ci::quat, ci::vec3>	from( const physx::PxTransform& t );
	static ci::AxisAlignedBox				from( const physx::PxBounds3& b );

	static physx::PxMat33					to( const ci::mat3& m );
	static physx::PxMat44					to( const ci::mat4& m );
	static physx::PxQuat					to( const ci::quat& m );
	static physx::PxVec2					to( const ci::vec2& v );
	static physx::PxVec3					to( const ci::vec3& v );
	static physx::PxVec4					to( const ci::vec4& v );
	static physx::PxTransform				to( const ci::quat& q, const ci::vec3& v );
	static physx::PxTransform				to( const std::pair<ci::quat, ci::vec3>& p );
	static physx::PxBounds3					to( const ci::AxisAlignedBox& b );

	void									createScene();
	void									createScene( const physx::PxSceneDesc& desc );

	physx::PxDefaultAllocator				getAllocator() const;
	physx::PxActiveTransform*				getBufferedActiveTransforms() const;
	physx::PxCooking*						getCooking() const;
	physx::PxDefaultCpuDispatcher*			getCpuDispatcher() const;
	physx::PxCudaContextManager*			getCudaContextManager() const;
	physx::PxFoundation*					getFoundation() const;
	physx::PxPhysics*						getPhysics() const;
	physx::PxScene*							getScene() const;
protected:
	Physx( const physx::PxTolerancesScale& scale, const physx::PxCookingParams& params );

	virtual void							onRelease( const physx::PxBase* observed, void* userData, 
													  physx::PxDeletionEventFlag::Enum deletionEvent );

	physx::PxErrorCallback&					getErrorCallback();
	physx::PxDefaultAllocator				mAllocator;
	physx::PxActiveTransform*				mBufferedActiveTransforms;
	physx::PxCooking*						mCooking;
	physx::PxDefaultCpuDispatcher*			mCpuDispatcher;
	physx::PxCudaContextManager*			mCudaContextManager;
	physx::PxFoundation*					mFoundation;
	physx::PxPhysics*						mPhysics;
	physx::PxScene*							mScene;
};
