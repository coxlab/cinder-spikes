#include "cinder/app/AppBasic.h"
#include "cinder/gl/gl.h"

#include "SpikeRenderer.h"
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

typedef boost::shared_ptr<SpikeRenderer> SpikeRendererPtr;
typedef boost::shared_ptr<zmq::socket_t> SocketPtr;
typedef boost::shared_ptr<GLSpikeWave> GLSpikeWavePtr;

class CinderSpikesApp : public AppBasic {

  protected:
  
    vector<SpikeRendererPtr> spike_renderers;
    vector<SocketPtr> spike_sockets;
    
    // layout of the spike channels per "page"
    unsigned int rows, cols, n_pages;
    
    // the current "page" of spike channels that we're on
    int current_page; 


  public:
	void setup();
	void mouseDown( MouseEvent event );	
	void update();
	void draw();
};

void CinderSpikesApp::setup()
{

    int n_channels = 32; // hard-code for now
    int rows = 4;
    int cols = 4;

    GLfloat plot_width = 100.0; // for now
    GLfloat plot_height = 100.0;
    double min_ampl = -0.099;
    double max_ampl = 0.099;
    double min_time = -0.00125;
    double max_time = 0.00125;
    
    boost::shared_ptr<GLStringRenderer> str_renderer(new CinderStringRenderer());
    

    // lay down spike renderers for each channel with 
    // appropriate offsets
    int r, c, p;
    
    for(int c = 0; c < n_channels; c++){
        
        GLfloat offset_y = r * plot_width;
        GLfloat offset_x = c * plot_height;
        

        
        SpikeRendererPtr renderer( new SpikeRenderer(min_ampl,
                                                     max_ampl,
                                                     min_time,
                                                     max_time,
                                                     plot_width, 
                                                     plot_height,
                                                     offset_x,
                                                     offset_y,
                                                     str_renderer) );

        spike_renderers.push_back(renderer);
        
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

void CinderSpikesApp::mouseDown( MouseEvent event )
{

    // hit test against all of the renderers
    vector<SpikeRendererPtr>::iterator i;
    for(i = spike_renderers.begin(); i != spike_renderers.end(); i++){
        SpikeWaveSelectionAction action;
        (*i)->hitTest(0.0, 0.0, &action); // TODO
    }
}

void CinderSpikesApp::update()
{

    int n_channels = spike_sockets.size();
    assert( spike_renderers.size() == n_channels);
    
    // for each spike channel
    for(int c = 0; c < n_channels; c++){
    
        SocketPtr socket = spike_sockets[c];
        SpikeRendererPtr renderer = spike_renderers[c];
    
        zmq::message_t msg;
        
        // Receive a message 
        bool recvd = socket->recv(&msg, ZMQ_NOBLOCK);
        
        while(recvd){

            string data((const char *)msg.data(), msg.size());
            SpikeWaveBuffer wave;
            wave.ParseFromString(data);
            
            Float32 data_buffer[PRE_TRIGGER+POST_TRIGGER];
            
            for(int i = 0; i < wave.wave_sample_size(); i++){
            
                data_buffer[i] = wave.wave_sample(i);
            }
            
            GLSpikeWavePtr gl_wave(new GLSpikeWave(PRE_TRIGGER+POST_TRIGGER, -PRE_TRIGGER/44100., 1.0/44100., data_buffer));
            
            renderer->pushSpikeWave(gl_wave);
            
            recvd = socket->recv(&msg, ZMQ_NOBLOCK);
        }
    }

}

void CinderSpikesApp::draw()
{
	// clear out the window with black
	gl::clear( Color( 0, 0, 0 ) ); 
    
    int start_channel = rows*cols*current_page;
    int end_channel = start_channel + rows*cols;
    
    for(int c = start_channel; c < end_channel; c++){
    
        SpikeRendererPtr renderer = spike_renderers[c];
        renderer->render();
    }
    
}


CINDER_APP_BASIC( CinderSpikesApp, RendererGl )
