#include "cinder/app/App.h"
#include "cinder/Camera.h"
#include "cinder/CameraUi.h"
#include "cinder/gl/gl.h"

#include "Model.h"
#include "CinderPhysx.h"

class InstancedApp : public ci::app::App, public physx::PxBroadPhaseCallback
{
public:
	InstancedApp();

	void				draw() override;
#if defined( CINDER_COCOA_TOUCH )
	void				touchesBegan( ci::app::TouchEvent event ) override;
	void				touchesEnded( ci::app::TouchEvent event ) override;
#else
	void				keyDown( ci::app::KeyEvent event ) override;
#endif
	void				resize() override;
	void				update() override;
private:
	ci::CameraPersp		mCamera;
	ci::CameraUi		mCamUi;

	ci::gl::VboRef		mVboInstancedSpheres;

	ci::gl::BatchRef	mBatchStockColorPlane;
	ci::gl::BatchRef	mBatchInstancedSphere;

	void				addActor();
	physx::PxMaterial*	mMaterial;
	PhysxRef			mPhysx;
	virtual void		onObjectOutOfBounds( physx::PxShape& shape, physx::PxActor& actor ) override;
	virtual void		onObjectOutOfBounds( physx::PxAggregate& aggregate ) override;
#if defined( CINDER_COCOA_TOUCH )
	bool				mTouching;
#endif
};

using namespace ci;
using namespace ci::app;
using namespace physx;
using namespace std;

#include "cinder/app/RendererGl.h"
#include "cinder/Log.h"
#include "cinder/Rand.h"

InstancedApp::InstancedApp()
{
#if defined( CINDER_COCOA_TOUCH )
	mTouching = false;
#endif
	
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
	
	// Add some spheres
#if defined( CINDER_COCOA_TOUCH )
	for ( size_t i = 0; i < 500; ++i ) {
#else
	for ( size_t i = 0; i < 2000; ++i ) {
#endif
		addActor();
	}

	// Shortcut for shader loading and error handling
	auto loadGlslProg = [ &]( const gl::GlslProg::Format& format ) -> gl::GlslProgRef
	{
		string names = format.getVertexPath().string() + " + " +
			format.getFragmentPath().string();
		gl::GlslProgRef glslProg;
		try {
			glslProg = gl::GlslProg::create( format );
		} catch ( const Exception& ex ) {
			CI_LOG_EXCEPTION( names, ex );
			quit();
		}
		return glslProg;
	};

	// Load shader files
	DataSourceRef frag = loadAsset( "frag.glsl" );
	DataSourceRef vert = loadAsset( "vert.glsl" );

	// Create GLSL programs
#if defined( CINDER_GL_ES_3 )
	int32_t version = 300;
#else
	int32_t version = 330;
#endif
	gl::GlslProgRef glslProg			= loadGlslProg( gl::GlslProg::Format()
													   .version( version )
													   .vertex( vert ).fragment( frag ) );
	gl::GlslProgRef glslProgInstanced	= loadGlslProg( gl::GlslProg::Format()
													  .version( version )
													  .vertex( vert ).fragment( frag )
													  .define( "INSTANCED_MODEL" ) );

	// Create geometry
	gl::VboMeshRef plane	= gl::VboMesh::create( geom::Plane()
												  .axes( vec3( 0.0f, 1.0f, 0.0f ), vec3( 0.0f, 0.0f, 1.0f ) )
												  .normal( vec3( 1.0f, 0.0f, 0.0f ) )
												  .size( vec2( 200.0f ) )
												  .subdivisions( ivec2( 32 ) ) );
	gl::VboMeshRef sphere	= gl::VboMesh::create( geom::Sphere()
												  .colors()
												  .radius( 0.5f )
												  .subdivisions( 32 ) );

	// Initialize instancing data
	geom::BufferLayout bufferLayout;
	size_t stride = sizeof( Model );
	bufferLayout.append( geom::Attrib::CUSTOM_0, 16, stride, 0, 1 );
	bufferLayout.append( geom::Attrib::CUSTOM_1, 9, stride, sizeof( mat4 ), 1 );
	mVboInstancedSpheres = gl::Vbo::create( GL_ARRAY_BUFFER, 0, nullptr, GL_DYNAMIC_DRAW );
	sphere->appendVbo( bufferLayout, mVboInstancedSpheres );

	// Create batches
	mBatchInstancedSphere = gl::Batch::create( sphere, glslProgInstanced, {
		{ geom::Attrib::CUSTOM_0, "vInstanceModelMatrix" },
		{ geom::Attrib::CUSTOM_1, "vInstanceNormalMatrix" }
	} );
	mBatchStockColorPlane = gl::Batch::create( plane, glslProg );

#if !defined( CINDER_COCOA_TOUCH )
	// Connect to Physx Visual Debugger
	mPhysx->pvdConnect();
#endif

	resize();

	gl::enableDepthRead();
	gl::enableDepthWrite();
	gl::enableVerticalSync();
}

void InstancedApp::addActor()
{
	// Choose random position and size
	vec3 p( randVec3() * 5.0f );
	p.y = glm::abs( p.y );
	float r = randFloat( 0.01f, 1.0f );

	// Create a randomly shaped actor
	PxRigidDynamic* actor = PxCreateDynamic(
		*mPhysx->getPhysics(),
		PxTransform( Physx::to( p ) ),
		PxSphereGeometry( r ),
		*mMaterial,
		r * 100.0f );

	// Apply some motion and add it to the scene
	actor->setLinearVelocity( Physx::to( randVec3() ) );
	mPhysx->addActor( actor, mPhysx->getScene() );
}

void InstancedApp::draw()
{
	gl::clear();
	const gl::ScopedMatrices scopedMatrices;
	gl::setMatrices( mCamera );

	// Draw floor plane
	{
		const gl::ScopedModelMatrix scopedModelMatrix;
		gl::multModelMatrix( Physx::from( 
			static_cast<PxRigidStatic*>( mPhysx->getActor( 0 ) )->getGlobalPose() 
			) );
		mBatchStockColorPlane->draw();
	}

	// Draw instanced spheres
	mBatchInstancedSphere->drawInstanced( (GLsizei)mPhysx->getActors().size() - 1 );
}

#if defined( CINDER_COCOA_TOUCH )

void InstancedApp::touchesBegan( TouchEvent event )
{
	mTouching = true;
}

void InstancedApp::touchesEnded( TouchEvent event )
{
	mTouching = false;
}

#else

void InstancedApp::keyDown( ci::app::KeyEvent event )
{
	switch ( event.getCode() ) {
	case KeyEvent::KEY_SPACE:
		{
			int32_t count = randInt( 10 );
			for ( size_t i = 0; i < count; ++i ) {
				addActor();
			}
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

#endif

void InstancedApp::onObjectOutOfBounds( physx::PxShape& shape, physx::PxActor& actor )
{
	mPhysx->eraseActor( actor );
	addActor();
}

void InstancedApp::onObjectOutOfBounds( PxAggregate& aggregate )
{
	aggregate.release();
}

void InstancedApp::resize()
{
	mCamera.setAspectRatio( getWindowAspectRatio() );
}

void InstancedApp::update()
{
	mPhysx->update();

#if defined( CINDER_COCOA_TOUCH )
	if ( mTouching ) {
		addActor();
	}
#endif
	
	vector<Model> spheres;
	for ( const auto& iter : mPhysx->getActors() ) {
		if ( iter.second->getType() == PxActorType::eRIGID_DYNAMIC ) {
			PxRigidDynamic* actor = static_cast<PxRigidDynamic*>( iter.second );
			mat4 m = glm::scale( Physx::from( actor->getGlobalPose() ), Physx::from( actor->getWorldBounds() ).getSize() );
			spheres.push_back( Model()
				.modelMatrix( m )
				.normalMatrix( glm::inverseTranspose( mat3( mCamera.getViewMatrix() * m ) ) ) );
		}
	}
	mVboInstancedSpheres->bufferData( sizeof( Model ) * spheres.size(), spheres.data(), GL_DYNAMIC_DRAW );
}


#if defined( CINDER_COCOA_TOUCH )
CINDER_APP( InstancedApp, RendererGl( RendererGl::Options().msaa( 16 ) ),
	[]( App::Settings* settings )
{
	settings->disableFrameRate();
	settings->setMultiTouchEnabled( true );
} )
#else
CINDER_APP( InstancedApp, RendererGl( RendererGl::Options().msaa( 16 ) ),
	[]( App::Settings* settings )
{
	settings->disableFrameRate();
	settings->setWindowSize( 1280, 720 );
} )
#endif
 