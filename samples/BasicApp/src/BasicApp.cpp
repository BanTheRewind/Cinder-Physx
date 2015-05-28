#include "cinder/app/App.h"
#include "cinder/Camera.h"
#include "cinder/CameraUi.h"
#include "cinder/gl/gl.h"

#include "CinderPhysx.h"

#include "PxRigidDynamic.h"
#include "PxShape.h"

class BasicApp : public ci::app::App 
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

	ci::gl::BatchRef		mBatchStockColorWirePlane;

	physx::PxRigidStatic*	mActorPlane;
	PhysxRef				mPhysx;
};

using namespace ci;
using namespace ci::app;
using namespace physx;
using namespace std;

#include "cinder/Log.h"
#include "cinder/app/RendererGl.h"

BasicApp::BasicApp()
{
	mCamera	= CameraPersp( getWindowWidth(), getWindowHeight(), 60.0f, 0.01f, 100.0f );
	mCamera.lookAt( vec3( 0.0f, 0.0f, 5.0f ), vec3( 0.0f ) );
	mCamUi	= CameraUi( &mCamera, getWindow() );

	// Initialize Physx
	mPhysx = Physx::create();

	// Create a scene. Multiples scenes are allowed.
	mPhysx->createScene();

	// Set scene size
	mPhysx->getScene()->lockWrite();

	// Create a plane
	mActorPlane = PxCreatePlane(
		*mPhysx->getPhysics(),
		PxPlane( Physx::to( vec3( 0.0f, 0.0f, 1.0f ) ), 0.0f ),
		*mPhysx->getPhysics()->createMaterial( 0.5f, 0.5f, 0.1f )
		);

	// Scale plane
	mat4 m = Physx::from( mActorPlane->getGlobalPose() );
	CI_LOG_V( m );

	m = glm::translate( m, vec3( 1.2f, 0.3f, 0.5f ) );
	m = glm::rotate( m, 0.5f, vec3( 1.0f ) );
	//m = glm::scale();
	CI_LOG_V( m );
	mActorPlane->setGlobalPose( PxTransform( Physx::to( m ) ) );

	// Add the plane to the scene.
	mPhysx->addActor( mActorPlane, mPhysx->getScene() );

	gl::GlslProgRef stockColor	= gl::getStockShader(gl::ShaderDef().color() );
	gl::VboMeshRef wirePlane	= gl::VboMesh::create( geom::WirePlane().subdivisions( ivec2( 32, 32 ) ) );
	mBatchStockColorWirePlane	= gl::Batch::create( wirePlane, stockColor );

	resize();

	gl::enableVerticalSync();
}

void BasicApp::draw()
{
	gl::clear();
	const gl::ScopedMatrices scopedMatrices;
	gl::setMatrices( mCamera );

	{
		const gl::ScopedModelMatrix scopedModelMatrix;
		gl::multModelMatrix( Physx::from( mActorPlane->getGlobalPose() ) );
		mBatchStockColorWirePlane->draw();
	}
}

void BasicApp::keyDown( ci::app::KeyEvent event )
{
	switch ( event.getCode() ) {
	case KeyEvent::KEY_f:
		setFullScreen( !isFullScreen() );
		break;
	case KeyEvent::KEY_q:
		quit();
		break;
	case KeyEvent::KEY_SPACE:
		// TO DO add actor
		break;
	}
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
 