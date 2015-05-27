#pragma once

#include "cinder/AxisAlignedBox.h"
#include "cinder/Matrix.h"
#include "cinder/Quaternion.h"
#include "PxPhysics.h"
#include "PxPhysicsAPI.h"
#include "extensions/PxExtensionsAPI.h"
#include <memory>

class Physx
{
public:
	~Physx();

	static uint32_t							from( physx::PxU32 v );
	static float							from( physx::PxReal v );
	static ci::mat3							from( const physx::PxMat33& m );
	static ci::mat4							from( const physx::PxMat44& m );
	static ci::quat							from( const physx::PxQuat& m );
	static ci::vec2							from( const physx::PxVec2& v );
	static ci::vec3							from( const physx::PxVec3& v );
	static ci::vec4							from( const physx::PxVec4& v );
	static std::pair<ci::quat, ci::vec3>	from( const physx::PxTransform& t );
	static ci::AxisAlignedBox				from( const physx::PxBounds3& b );

	static physx::PxU32						to( uint32_t v );
	static physx::PxReal					to( float v );
	static physx::PxMat33					to( const ci::mat3& m );
	static physx::PxMat44					to( const ci::mat4& m );
	static physx::PxQuat					to( const ci::quat& m );
	static physx::PxVec2					to( const ci::vec2& v );
	static physx::PxVec3					to( const ci::vec3& v );
	static physx::PxVec4					to( const ci::vec4& v );
	static physx::PxTransform				to( const ci::quat& m, const ci::vec3& v );
	static physx::PxTransform				to( const std::pair<ci::quat, ci::vec3>& p );
	static physx::PxBounds3					to( const ci::AxisAlignedBox& b );

	static physx::PxDefaultAllocator		getAllocator();
	static physx::PxActiveTransform*		getBufferedActiveTransforms();
	static physx::PxCooking*				getCooking();
	static physx::PxDefaultCpuDispatcher*	getCpuDispatcher();
	static physx::PxCudaContextManager*		getCudaContextManager();
	static physx::PxFoundation*				getFoundation();
	static physx::PxMaterial*				getMaterial();
	static physx::PxPhysics*				getPhysics();
	static physx::PxProfileZoneManager*		getProfileZoneManager();
	static physx::PxScene*					getScene();
protected:
	static std::shared_ptr<Physx>			get();
	Physx();

	physx::PxErrorCallback&					getErrorCallback();
	physx::PxDefaultAllocator				mAllocator;
	physx::PxActiveTransform*				mBufferedActiveTransforms;
	physx::PxCooking*						mCooking;
	physx::PxDefaultCpuDispatcher*			mCpuDispatcher;
	physx::PxCudaContextManager*			mCudaContextManager;
	physx::PxFoundation*					mFoundation;
	physx::PxMaterial*						mMaterial;
	physx::PxPhysics*						mPhysics;
	physx::PxProfileZoneManager*			mProfileZoneManager;
	physx::PxScene*							mScene;
};
