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

	void								draw() override;
	void								keyDown( ci::app::KeyEvent event ) override;
	void								resize() override;
	void								update() override;
private:
	ci::CameraPersp						mCamera;
	ci::CameraUi						mCamUi;

	double								mElapsedSeconds;

	std::vector<physx::PxRigidActor*>	mActors;
	std::vector<physx::PxActor*>		mDeletedActors;
};

using namespace ci;
using namespace ci::app;
using namespace std;

#include "cinder/Log.h"
#include "cinder/app/RendererGl.h"

BasicApp::BasicApp()
{
	mElapsedSeconds	= 0.0;

	mCamera	= CameraPersp( getWindowWidth(), getWindowHeight(), 60.0f, 0.01f, 1000.0f );
	mCamera.lookAt( vec3( 0.0f, 0.0f, 5.0f ), vec3( 0.0f ) );
	mCamUi	= CameraUi( &mCamera, getWindow() );

	resize();

	gl::enableVerticalSync();
}

void BasicApp::draw()
{
	gl::clear();
	const gl::ScopedMatrices scopedMatrices;
	gl::setMatrices( mCamera );
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
	double e	= getElapsedSeconds();
	float d		= (float)( e - mElapsedSeconds );

	mElapsedSeconds = e;
}

CINDER_APP( BasicApp, RendererGl, []( App::Settings* settings )
{
	settings->disableFrameRate();
	settings->setWindowSize( 1280, 720 );
} )
 