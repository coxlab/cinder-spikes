#include "cinder/app/AppBasic.h"
#include "cinder/gl/gl.h"

using namespace ci;
using namespace ci::app;
using namespace std;

class CinderSpikesApp : public AppBasic {
  public:
	void setup();
	void mouseDown( MouseEvent event );	
	void update();
	void draw();
};

void CinderSpikesApp::setup()
{
}

void CinderSpikesApp::mouseDown( MouseEvent event )
{
}

void CinderSpikesApp::update()
{
}

void CinderSpikesApp::draw()
{
	// clear out the window with black
	gl::clear( Color( 0, 0, 0 ) ); 
}

CINDER_APP_BASIC( CinderSpikesApp, RendererGl )
