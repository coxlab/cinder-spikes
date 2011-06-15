#include "cinder/app/AppBasic.h"
#include "cinder/gl/gl.h"

#include "spikevis/SpikeRenderer.h"
#include <zmq/zmq.hpp>

using namespace ci;
using namespace ci::app;
using namespace std;
using namespace spike_visulization;

typedef shared_ptr<SpikeRenderer> SpikeRendererPtr;
typedef shared_ptr<zmq::socket_t> SocketPtr;

class CinderSpikesApp : public AppBasic {

  protected:
  
    vector<SpikeRendererPtr> spike_renderers;
    vector<SocketPtr> spike_sockets;
    
    // layout of the spike channels per "page"
    unsigned int rows, cols;
    
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
}

void CinderSpikesApp::mouseDown( MouseEvent event )
{
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
            
            shared_ptr<GLSpikeWave> gl_wave(new GLSpikeWave(PRE_TRIGGER+POST_TRIGGER, -PRE_TRIGGER/44100., 1.0/44100., data_buffer));
            
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
        renderer->draw();
    }
    
}


CINDER_APP_BASIC( CinderSpikesApp, RendererGl )
