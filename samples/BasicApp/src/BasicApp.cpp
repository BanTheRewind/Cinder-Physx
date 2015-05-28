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

	void					draw() override;
	void					keyDown( ci::app::KeyEvent event ) override;
	void					resize() override;
	void					update() override;
private:
	ci::CameraPersp			mCamera;
	ci::CameraUi			mCamUi;

	ci::gl::BatchRef		mBatchStockColorWireCube;
	ci::gl::BatchRef		mBatchStockColorWirePlane;
	ci::gl::BatchRef		mBatchStockColorWireSphere;

	physx::PxMaterial*		mMaterial;
	PhysxRef				mPhysx;
	virtual void			onObjectOutOfBounds( physx::PxShape& shape, physx::PxActor& actor );
	virtual void			onObjectOutOfBounds( physx::PxAggregate& aggregate );
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
	mCamera.lookAt( vec3( 0.0f, 3.0f, 20.0f ), vec3( 0.0f, 2.0f, 0.0f ) );
	mCamUi	= CameraUi( &mCamera, getWindow() );

	// Initialize Physx
	mPhysx = Physx::create();

	// Create a material for all actors
	mMaterial = mPhysx->getPhysics()->createMaterial( 0.5f, 0.5f, 1.0f );

	// Create a scene. Multiples scenes are allowed.
	mPhysx->createScene();

	// Connects onObjectOutOfBounds to scene. This lets us manage 
	// objects that have gone out of bounds.
	mPhysx->getScene()->setBroadPhaseCallback( this );

	// Add the plane to the scene.
	mPhysx->addActor( PxCreatePlane(
		*mPhysx->getPhysics(),
		PxPlane( Physx::to( vec3( 0.0f, 1.0f, 0.0f ) ), 0.0f ),
		*mMaterial
		), mPhysx->getScene() );

	// Create shader and geometry batches
	gl::GlslProgRef stockColor	= gl::getStockShader(gl::ShaderDef().color() );
	gl::VboMeshRef wireCube		= gl::VboMesh::create( geom::WireCube() );
	gl::VboMeshRef wirePlane	= gl::VboMesh::create( geom::WirePlane()
													   .axes( vec3( 0.0f, 1.0f, 0.0f ), vec3( 0.0f, 0.0f, 1.0f ) )
													   .size( vec2( 100.0f ) )
													   .subdivisions( ivec2( 32 ) ) );
	gl::VboMeshRef wireSphere	= gl::VboMesh::create( geom::WireSphere()
													   .subdivisionsAxis( 16 )
													   .subdivisionsCircle( 16 )
													   .subdivisionsHeight( 16 ) );
	mBatchStockColorWireCube	= gl::Batch::create( wireCube,		stockColor );
	mBatchStockColorWirePlane	= gl::Batch::create( wirePlane,		stockColor );
	mBatchStockColorWireSphere	= gl::Batch::create( wireSphere,	stockColor );

	resize();

	gl::enableVerticalSync();
}

void BasicApp::draw()
{
	gl::clear();
	const gl::ScopedMatrices scopedMatrices;
	gl::setMatrices( mCamera );

	for ( const auto& iter : mPhysx->getActors() ) {
		const gl::ScopedModelMatrix scopedModelMatrix;
		PxRigidActor* actor = static_cast<PxRigidActor*>( iter.second );
		gl::multModelMatrix( Physx::from( actor->getGlobalPose() ) );
		
		PxShape* shape = nullptr;
		actor->getShapes( &shape, sizeof( PxShape ) * actor->getNbShapes() );

		for ( uint32_t i = 0; i < actor->getNbShapes(); ++i, ++shape ) {
			switch ( shape->getGeometryType() ) {
			case PxGeometryType::eBOX:
				gl::scale( Physx::from( actor->getWorldBounds() ).getSize() );
				mBatchStockColorWireCube->draw();
				break;
			case PxGeometryType::ePLANE:
				mBatchStockColorWirePlane->draw();
				break;
			case PxGeometryType::eSPHERE:
				gl::scale( Physx::from( actor->getWorldBounds() ).getSize() );
				mBatchStockColorWireSphere->draw();
				break;
			}
		}
	}
}

void BasicApp::keyDown( ci::app::KeyEvent event )
{
	switch ( event.getCode() ) {
	case KeyEvent::KEY_SPACE:
		{
			vec3 p( randVec3() * 5.0f );
			p.y		= glm::abs( p.y );
			float r = randFloat( 0.01f, 1.0f );

			PxRigidDynamic* actor = nullptr;
			if ( randBool() ) {
				actor = PxCreateDynamic( 
					*mPhysx->getPhysics(), 
					PxTransform( Physx::to( p ) ), 
					PxBoxGeometry( Physx::to( vec3( r ) ) ),
					*mMaterial, 
					r * 100.0f );
			} else {
				actor = PxCreateDynamic( 
					*mPhysx->getPhysics(), 
					PxTransform( Physx::to( p ) ), 
					PxSphereGeometry( r ), 
					*mMaterial, 
					r * 100.0f );
			}

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

void BasicApp::onObjectOutOfBounds( physx::PxShape& shape, physx::PxActor& actor)
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
 