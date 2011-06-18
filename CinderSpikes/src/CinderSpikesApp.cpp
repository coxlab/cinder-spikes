#include "cinder/app/AppBasic.h"
#include "cinder/gl/gl.h"

#include "SpikeChannelController.h"
#include "CinderStringRenderer.h"
#include <zmq.hpp>
#include "spike_wave.pb.h"

using namespace ci;
using namespace ci::app;
using namespace std;
using namespace spike_visualization;

// must match audio unit
#define PRE_TRIGGER     33
#define POST_TRIGGER    33



class CinderSpikesApp : public AppBasic {

  protected:
  
    vector<SpikeChannelControllerPtr> spike_controllers;
    
    // layout of the spike channels per "page"
    unsigned int rows, cols, n_pages;
    unsigned int n_channels;
    
    // the current "page" of spike channels that we're on
    int current_page; 

    void connectSocketToSpikeChannel(SocketPtr, int);
    void computePlotDimensions(int r, int c, 
                           GLfloat *offset_x, GLfloat *offset_y, 
                           GLfloat *plot_width, GLfloat *plot_height);

  public:
    CinderSpikesApp() : current_page(0),
                        n_channels(2), // hard-code for now
                        rows(2),
                        cols(2)
    { }
  
	void setup();
	void mouseDown( MouseEvent event );
    void mouseUp( MouseEvent event );	
    void mouseDrag( MouseEvent event );
    void mouseWheel( MouseEvent event );
	void update();
	void draw();
    void prepareSettings( Settings *settings);
};



void CinderSpikesApp::prepareSettings( Settings *settings ){
    settings->setWindowSize( 200*cols, 160*rows );
    settings->setFrameRate( 30.0f );
}
      
void CinderSpikesApp::computePlotDimensions(int r, int c, 
                           GLfloat *offset_x, GLfloat *offset_y, 
                           GLfloat *plot_width, GLfloat *plot_height){

    float width = getWindowWidth() / cols;
    float height = getWindowHeight() / rows;
    *offset_y = (rows - r - 1) * height;
    *offset_x = c * width;
    *plot_width = width;
    *plot_height = height;
}

void CinderSpikesApp::setup()
{

    

    double min_ampl = -0.099;
    double max_ampl = 0.099;
    double min_time = -0.00125;
    double max_time = 0.00125;
    
    boost::shared_ptr<GLStringRenderer> str_renderer(new CinderStringRenderer());
    

    // lay down spike renderers for each channel with 
    // appropriate offsets
    int r, c, p;
    
    for(int ch = 0; ch < n_channels; ch++){
        
        GLfloat offset_y, offset_x, plot_width, plot_height;
        computePlotDimensions(r,c, &offset_x, &offset_y, &plot_width, &plot_height);
        

        SpikeRendererPtr renderer( new SpikeRenderer(min_ampl,
                                                     max_ampl,
                                                     min_time,
                                                     max_time,
                                                     plot_width, 
                                                     plot_height,
                                                     offset_x,
                                                     offset_y,
                                                     str_renderer) );

        
        SpikeChannelControllerPtr controller( new SpikeChannelController(ch, renderer) );
                                                                          
        spike_controllers.push_back(controller);
        
        // row and col update
        c++;
        if( c >= cols ){
            c = 0;
            r++;
            
            if( r >= rows ){
                r = 0;
                p++;
            }
        }
    }
    
    n_pages = p + 1;

}


void CinderSpikesApp::update()
{

    assert( spike_controllers.size() == n_channels);
    
    // for each spike channel
    for(int c = 0; c < n_channels; c++){
        spike_controllers[c]->update();
    }
    

}

void CinderSpikesApp::draw()
{
	// clear out the window with black
    //gl::setMatricesWindow( getWindowSize() );
    gl::clear( Color( 0, 0, 0 ) ); 
    
    int start_channel = rows*cols*current_page;
    int end_channel = start_channel + rows*cols;
    if(end_channel > n_channels) end_channel = n_channels;
    
    for(int ch = start_channel; ch < end_channel; ch++){
        
        SpikeChannelControllerPtr controller = spike_controllers[ch];
        
        int c = (ch - start_channel) % cols;
        int r = (ch - start_channel) / cols;
        
        GLfloat offset_y, offset_x, plot_width, plot_height;
        computePlotDimensions(r,c, &offset_x, &offset_y, &plot_width, &plot_height);
        
        
        controller->setViewport( offset_x, offset_y, plot_width, plot_height);
        controller->render();
    }
    
}

void CinderSpikesApp::mouseDown( MouseEvent event )
{

    int start_channel = rows*cols*current_page;
    int end_channel = start_channel + rows*cols;
    if(end_channel > n_channels) end_channel = n_channels;
    
    float x = (float)event.getX();
    float y = getWindowHeight() - (float)event.getY();
    
    for(int ch = start_channel; ch < end_channel; ch++){
        SpikeChannelControllerPtr controller = spike_controllers[ch];
        controller->mouseDown(x, y);
    }
}


void CinderSpikesApp::mouseUp( MouseEvent event )
{

    int start_channel = rows*cols*current_page;
    int end_channel = start_channel + rows*cols;
    if(end_channel > n_channels) end_channel = n_channels;
    
    float x = (float)event.getX();
    float y = getWindowHeight() - (float)event.getY();
    
    for(int ch = start_channel; ch < end_channel; ch++){
        SpikeChannelControllerPtr controller = spike_controllers[ch];
        controller->mouseUp(x, y);
    }
}

void CinderSpikesApp::mouseDrag( MouseEvent event )
{

    int start_channel = rows*cols*current_page;
    int end_channel = start_channel + rows*cols;
    if(end_channel > n_channels) end_channel = n_channels;
    
    float x = (float)event.getX();
    float y = getWindowHeight() - (float)event.getY();
    
    for(int ch = start_channel; ch < end_channel; ch++){
        SpikeChannelControllerPtr controller = spike_controllers[ch];
        controller->mouseDragged(x, y);
    }
}

void CinderSpikesApp::mouseWheel( MouseEvent event )
{
    
    int start_channel = rows*cols*current_page;
    int end_channel = start_channel + rows*cols;
    if(end_channel > n_channels) end_channel = n_channels;
    
    float delta = event.getWheelIncrement();
    
    for(int ch = start_channel; ch < end_channel; ch++){
        SpikeChannelControllerPtr controller = spike_controllers[ch];
        controller->scrollWheel(delta);
    }
}

CINDER_APP_BASIC( CinderSpikesApp, RendererGl )
