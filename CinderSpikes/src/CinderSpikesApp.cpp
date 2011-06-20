#include "cinder/app/AppBasic.h"
#include "cinder/gl/gl.h"

#include "SpikeChannelController.h"
//#include "CinderStringRenderer.h"
#include "CocoaGLStringRenderer.h"
#include <zmq.hpp>
#include "spike_wave.pb.h"

using namespace ci;
using namespace ci::app;
using namespace std;
using namespace spike_visualization;

// must match audio unit
#define PRE_TRIGGER     33
#define POST_TRIGGER    33


// Hard-coded stuff (for now)
// should eventually go in some kind of conf file
#define N_CHANNELS  32
#define ROWS        4
#define COLS        4
const int hard_coded_channel_order[N_CHANNELS] = {7, 10,1, 14,5, 12,3, 11,
                                                  2, 16,22,15,4, 9, 18,28,
                                                  6, 13,21,27,8, 32,17,31,
                                                  24,26,20,30,23,25,19,29};


class CinderSpikesApp : public AppBasic {

  protected:
  
    vector<SpikeChannelControllerPtr> spike_controllers;
    
    // layout of the spike channels per "page"
    unsigned int rows, cols, n_pages;
    unsigned int n_channels;
    
    int *channel_order;
    
    // the current "page" of spike channels that we're on
    int current_page; 
    
    // animation related
    bool slide_animation_active;
    float animation_start_time, animation_duration;
    int from_page, to_page, animation_direction, animation_count;

    void connectSocketToSpikeChannel(SocketPtr, int);
    void computePlotDimensions(int r, int c, 
                           GLfloat *offset_x, GLfloat *offset_y, 
                           GLfloat *plot_width, GLfloat *plot_height);
    void animatePageSlide(int from, int to, int direction, float duration);

  public:
    CinderSpikesApp() : current_page(0),
                        n_channels(N_CHANNELS), // hard-code for now
                        rows(ROWS),
                        cols(COLS),
                        slide_animation_active(false),
                        channel_order((int*)hard_coded_channel_order)
    {         
    }
  
	void setup();
	void mouseDown( MouseEvent event );
    void mouseUp( MouseEvent event );	
    void mouseDrag( MouseEvent event );
    void mouseWheel( MouseEvent event );
    void keyDown(KeyEvent event);
	void update();
	void draw();
    void drawPage(int page_number, GLfloat page_offset_x=0.0, GLfloat page_offset_y=0.0);
    void prepareSettings( Settings *settings);
};



void CinderSpikesApp::prepareSettings( Settings *settings ){
    settings->setWindowSize( 200*cols, 160*rows );
    settings->setFrameRate( 60.0f );
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
    
    //boost::shared_ptr<GLStringRenderer> str_renderer(new CinderStringRenderer());
    boost::shared_ptr<GLStringRenderer> str_renderer(new CocoaGLStringRenderer());

    // lay down spike renderers for each channel with 
    // appropriate offsets
    int r = 0;
    int c = -1; 
    int p = 0;    
    for(int ch = 0; ch < n_channels; ch++){
        
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
        
        GLfloat offset_y, offset_x, plot_width, plot_height;
        computePlotDimensions(r,c, &offset_x, &offset_y, &plot_width, &plot_height);
        
        int chan = ch;
        if(channel_order != NULL){
            chan = channel_order[ch] - 1;
        }

        SpikeRendererPtr renderer( new SpikeRenderer(min_ampl,
                                                     max_ampl,
                                                     min_time,
                                                     max_time,
                                                     plot_width, 
                                                     plot_height,
                                                     offset_x,
                                                     offset_y,
                                                     str_renderer,
                                                     chan+1) );

        
        SpikeChannelControllerPtr controller( new SpikeChannelController(chan, renderer) );
                                                                          
        spike_controllers.push_back(controller);
        
        
    }
    
    n_pages = p + 1;

}


void CinderSpikesApp::update()
{
    //if(slide_animation_active) return;
    
    assert( spike_controllers.size() == n_channels);
    
    // for each spike channel
    for(int c = 0; c < n_channels; c++){
        spike_controllers[c]->update();
    }
    

}

void CinderSpikesApp::drawPage(int page_number, GLfloat page_offset_x, GLfloat page_offset_y){
    int start_channel = rows*cols*page_number;
    int end_channel = start_channel + rows*cols;
    if(end_channel > n_channels) end_channel = n_channels;
    
    float screen_width = getWindowWidth();
    
    for(int ch = start_channel; ch < end_channel; ch++){
        
        SpikeChannelControllerPtr controller = spike_controllers[ch];
        
        int c = (ch - start_channel) % cols;
        int r = (ch - start_channel) / cols;
        
        GLfloat offset_y, offset_x, plot_width, plot_height;
        computePlotDimensions(r,c, &offset_x, &offset_y, &plot_width, &plot_height);
        
        if( (page_offset_x + offset_x + plot_width) < 0 ){
            continue;
        }
        
        if( (page_offset_x + offset_x) > screen_width ){
            continue;
        }
        controller->setViewport( page_offset_x + offset_x, page_offset_y + offset_y, plot_width, plot_height);
        controller->render(false);
    }

}

void CinderSpikesApp::animatePageSlide(int _from, int _to, int direction, float duration=0.5)
{

    from_page = _from;
    to_page = _to;
    animation_duration = duration;
    animation_direction = direction;
    animation_start_time = getElapsedSeconds();
    slide_animation_active = true;
    animation_count = 0;
    
    std::cerr << "Animating from: " << from_page << " to " << to_page << std::endl;
}

void CinderSpikesApp::draw()
{
	// clear out the window with black
    gl::clear( Color( 0, 0, 0 ), true ); 
    
    if(slide_animation_active){
        animation_count++;
        
        // from_page, to_page, start_time, end_time,
        float elapsed = getElapsedSeconds() - animation_start_time;
        
        // check if animation is over
        if(elapsed >= animation_duration){
            slide_animation_active = false;
            current_page = to_page;
            drawPage(current_page);
            std::cerr << animation_count << std::endl;
            return;
        }
        
        float animation_fraction = elapsed / animation_duration;
        float width = getWindowWidth();
        
        if(animation_direction < 0){
            drawPage(from_page, -animation_fraction*width);
            drawPage(to_page, (1-animation_fraction)*width);
        } else {
            drawPage(from_page, (animation_fraction)*width);
            drawPage(to_page, (-1+animation_fraction)*width);
        }
        
    } else {
        drawPage(current_page);
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
    float x = event.getX();
    float y = event.getY();
    
    for(int ch = start_channel; ch < end_channel; ch++){
        SpikeChannelControllerPtr controller = spike_controllers[ch];
        controller->scrollWheel(delta,x,getWindowHeight() - y);
    }
}


void CinderSpikesApp::keyDown( KeyEvent event )
{
    
    char c = event.getChar();
    
    if( c == '>' ){
        int _from = current_page;
        int _to = _from + 1;
        if(_to >= n_pages){
            _to = 0;
        }
        
        animatePageSlide(_from, _to, 1);

    } else if( c == '<' ){
        int _from = current_page;
        int _to = _from - 1;
        
        if(_to < 0){
            _to = (n_pages - 1);
        }
        animatePageSlide(_from,_to, -1);
    }
    
}

CINDER_APP_BASIC( CinderSpikesApp, RendererGl )
