//
//  CinderStringRenderer.h
//  CinderSpikes
//
//  Created by David Cox on 6/15/11.
//  Copyright 2011 The Rowland Institute at Harvard. All rights reserved.
//

#include "cinder/gl/gl.h"
#include "cinder/Text.h"
#include "cinder/gl/Texture.h"
#include "GLStringRenderer.h"

using namespace ci;
using namespace ci::gl;


class CinderStringRenderer : public GLStringRenderer {

    protected:
    
        //TextLayout t;
        Color text_color;
        Color bg_color;
        Font f;
        Texture::Format fmt;
    
    public:
    
        CinderStringRenderer():
            text_color(0.6,0.6,0.6),
            bg_color(0.0, 0.0, 0.0),
            f("Helvetica", 6 * 2.5){
            fmt.setTargetRect();
            
        }
    

        virtual GLuint stringToTexture(std::string str, float font_size,float *width, float *height){

            TextLayout t;
            
            t.setBorder(3,3);
            t.clear(bg_color);   
            t.setColor(text_color);
            t.setFont(f);

            t.addLine(str);
             
            Surface surf = t.render();
//            
            Texture tex(surf, fmt);
            tex.setDoNotDispose();
            
            *width = surf.getWidth();
            *height = surf.getHeight();
            
            return tex.getTextureId();
        }
};
