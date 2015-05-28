#include "cinder/app/App.h"
#include "cinder/Camera.h"
#include "cinder/CameraUi.h"
#include "cinder/gl/gl.h"

#include "CinderPhysx.h"

#include "PxRigidDynamic.h"
#include "PxShape.h"

class BasicApp : public ci::app::App, public physx::PxBroadPhaseCallback
{
public:
	BasicApp();

	void				draw() override;
	void				keyDown( ci::app::KeyEvent event ) override;
	void				resize() override;
	void				update() override;
private:
	ci::CameraPersp		mCamera;
	ci::CameraUi		mCamUi;

	ci::gl::BatchRef	mBatchStockColorCapsule;
	ci::gl::BatchRef	mBatchStockColorCube;
	ci::gl::BatchRef	mBatchStockColorPlane;
	ci::gl::BatchRef	mBatchStockColorSphere;

	physx::PxMaterial*	mMaterial;
	PhysxRef			mPhysx;
	virtual void		onObjectOutOfBounds( physx::PxShape& shape, physx::PxActor& actor );
	virtual void		onObjectOutOfBounds( physx::PxAggregate& aggregate );
};

using namespace ci;
using namespace ci::app;
using namespace physx;
using namespace std;

#include "cinder/app/RendererGl.h"
#include "cinder/Log.h"
#include "cinder/Rand.h"

BasicApp::BasicApp()
{
	mCamera	= CameraPersp( getWindowWidth(), getWindowHeight(), 60.0f, 0.01f, 1000.0f );
	mCamera.lookAt( vec3( 0.0f, 0.0f, 30.0f ), vec3( 0.0f, 0.0f, 0.0f ) );
	mCamUi	= CameraUi( &mCamera, getWindow() );

	// Initialize Physx
	mPhysx = Physx::create();

	// Create a material for all actors
	mMaterial = mPhysx->getPhysics()->createMaterial( 0.5f, 0.5f, 0.5f );

	// Create a scene. Multiples scenes are allowed.
	mPhysx->createScene();

	// Connects onObjectOutOfBounds to scene. This lets us manage 
	// objects that have gone out of bounds.
	mPhysx->getScene()->setBroadPhaseCallback( this );

	// Add the plane to the scene
	mPhysx->addActor( PxCreatePlane(
		*mPhysx->getPhysics(),
		PxPlane( Physx::to( vec3( 0.0f, 1.0f, 0.0f ) ), 5.0f ),
		*mMaterial
		), mPhysx->getScene() );
	
	// Rotate plane so everything slides away
	PxRigidStatic* floor	= static_cast<PxRigidStatic*>( mPhysx->getActor() );
	mat4 m					= Physx::from( floor->getGlobalPose() );
	m						= glm::rotate( m, 0.5f, vec3( 0.0f, -1.0f, 1.0f ) );
	floor->setGlobalPose( PxTransform( Physx::to( m ) ) );

	// Create shader and geometry batches
	gl::GlslProgRef stockColor	= gl::getStockShader( gl::ShaderDef().color() );
	gl::VboMeshRef capsule		= gl::VboMesh::create( geom::Capsule()
													   .subdivisionsAxis( 16 )
													   .subdivisionsHeight( 16 ) );
	gl::VboMeshRef cube			= gl::VboMesh::create( geom::Cube().colors() );
	gl::VboMeshRef plane		= gl::VboMesh::create( geom::Plane()
													   .axes( vec3( 0.0f, 1.0f, 0.0f ), vec3( 0.0f, 0.0f, 1.0f ) )
													   .size( vec2( 200.0f ) )
													   .subdivisions( ivec2( 32 ) ) );
	gl::VboMeshRef sphere		= gl::VboMesh::create( geom::Sphere()
													   .colors()
													   .subdivisions( 32 ) );
	mBatchStockColorCapsule		= gl::Batch::create( capsule,	stockColor );
	mBatchStockColorCube		= gl::Batch::create( cube,		stockColor );
	mBatchStockColorPlane		= gl::Batch::create( plane,		stockColor );
	mBatchStockColorSphere		= gl::Batch::create( sphere,	stockColor );

	resize();

	gl::enableDepthRead();
	gl::enableDepthWrite();
	gl::enableVerticalSync();
	gl::polygonMode( GL_FRONT_AND_BACK, GL_LINE );
}

void BasicApp::draw()
{
	gl::clear();
	const gl::ScopedMatrices scopedMatrices;
	gl::setMatrices( mCamera );

	for ( const auto& iter : mPhysx->getActors() ) {

		// Cast to rigid actor
		if ( iter.second->getType() == PxActorType::eRIGID_DYNAMIC || 
			 iter.second->getType() == PxActorType::eRIGID_STATIC ) {
			PxRigidActor* actor = static_cast<PxRigidActor*>( iter.second );

			// Apply actor's transform
			const gl::ScopedModelMatrix scopedModelMatrix;
			gl::multModelMatrix( Physx::from( actor->getGlobalPose() ) );
		
			// Get the actor's shapes
			PxShape* shape = nullptr;
			actor->getShapes( &shape, sizeof( PxShape ) * actor->getNbShapes() );
			for ( uint32_t i = 0; i < actor->getNbShapes(); ++i, ++shape ) {

				// Scale non-plane shapes to their bounds (plane is infinite)
				if ( shape->getGeometryType() != PxGeometryType::ePLANE ) {
					gl::scale( Physx::from( actor->getWorldBounds() ).getSize() );
				}

				// Use the appropriate batch to draw the geometry type
				switch ( shape->getGeometryType() ) {
				case PxGeometryType::eBOX:
					mBatchStockColorCube->draw();
					break;
				case PxGeometryType::eCAPSULE:
					mBatchStockColorCapsule->draw();
					break;
				case PxGeometryType::ePLANE:
					mBatchStockColorPlane->draw();
					break;
				case PxGeometryType::eSPHERE:
					mBatchStockColorSphere->draw();
					break;
				}
			}
		}
	}
}

void BasicApp::keyDown( ci::app::KeyEvent event )
{
	switch ( event.getCode() ) {
	case KeyEvent::KEY_SPACE:
		{
			// Choose random position and size
			vec3 p( randVec3() * 5.0f );
			p.y		= glm::abs( p.y );
			float r = randFloat( 0.01f, 1.0f );

			// Create a randomly shaped actor
			PxRigidDynamic* actor = nullptr;
			switch ( randInt( 0, 3 ) ) {
			case 0:
				actor = PxCreateDynamic( 
					*mPhysx->getPhysics(), 
					PxTransform( Physx::to( p ) ), 
					PxBoxGeometry( Physx::to( vec3( r ) ) ),
					*mMaterial, 
					r * 100.0f );
				break;
			case 1:
				actor = PxCreateDynamic( 
					*mPhysx->getPhysics(), 
					PxTransform( Physx::to( p ) ), 
					PxSphereGeometry( r ), 
					*mMaterial, 
					r * 100.0f );
				break;
			case 2:
				actor = PxCreateDynamic( 
					*mPhysx->getPhysics(), 
					PxTransform( Physx::to( p ) ), 
					PxCapsuleGeometry( r, r * 0.5f ), 
					*mMaterial, 
					r * 100.0f );
				break;
			}

			// Apply some motion and add it to the scene
			actor->setLinearVelocity( Physx::to( randVec3() ) );
			mPhysx->addActor( actor, mPhysx->getScene() );
		}
		break;
	case KeyEvent::KEY_f:
		setFullScreen( !isFullScreen() );
		break;
	case KeyEvent::KEY_q:
		quit();
		break;
	}
}

void BasicApp::onObjectOutOfBounds( physx::PxShape& shape, physx::PxActor& actor )
{
	mPhysx->eraseActor( actor );
}

void BasicApp::onObjectOutOfBounds( PxAggregate& aggregate )
{
	aggregate.release();
}

void BasicApp::resize()
{
	mCamera.setAspectRatio( getWindowAspectRatio() );
}

void BasicApp::update()
{
	mPhysx->update();
}

CINDER_APP( BasicApp, RendererGl( RendererGl::Options().msaa( 16 ) ), 
			[]( App::Settings* settings )
{
	settings->disableFrameRate();
	settings->setWindowSize( 1280, 720 );
} )
 