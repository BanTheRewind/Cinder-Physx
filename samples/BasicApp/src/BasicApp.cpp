#include "cinder/app/App.h"
#include "cinder/Camera.h"
#include "cinder/CameraUi.h"
#include "cinder/gl/gl.h"

#include "CinderPhysx.h"

class BasicApp : public ci::app::App 
{
public:
	BasicApp();

	void				draw() override;
	void				keyDown( ci::app::KeyEvent event ) override;
	void				update() override;
private:
	ci::CameraPersp		mCamera;
	ci::CameraUi		mCamUi;

	double				mElapsedSeconds;
};

using namespace ci;
using namespace ci::app;
using namespace std;

#include "cinder/Log.h"
#include "cinder/app/RendererGl.h"

BasicApp::BasicApp()
{
	mElapsedSeconds	= 0.0;
	mCamera = CameraPersp( getWindowWidth(), getWindowHeight(), 60.0f, 0.01f, 1000.0f );
	mCamera.lookAt( vec3( 0.0f, 0.0f, 5.0f ), vec3( 0.0f ) );
	mCamUi = CameraUi( &mCamera, getWindow() );

	gl::enableVerticalSync();
}

void BasicApp::draw()
{
	gl::clear();
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
		break;
	}
}

void BasicApp::update()
{
	double e = getElapsedSeconds();
	float d		= (float)( e - mElapsedSeconds );


	mElapsedSeconds = e;
}

CINDER_APP( BasicApp, RendererGl, []( App::Settings* settings )
{
	settings->disableFrameRate();
	settings->setWindowSize( 1280, 720 );
}; )
 