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

    public:

        virtual GLuint stringToTexture(std::string str, float font_size,float *width, float *height){

            Font f("Helvetica", font_size * 2.5);
            Color text_color(0.6, 0.6, 0.6);
            Color bg_color(0.0, 0.0, 0.0);
        
            TextLayout t = TextLayout();
            
            t.setBorder(3,3);
            t.clear(bg_color);   
            t.setColor(text_color);
            t.setFont(f);

            t.addLine(str);
                     
            
            
            
            
            Surface surf = t.render();
            Texture::Format fmt;
            fmt.setTargetRect();
            Texture tex(surf, fmt);
            tex.setDoNotDispose();
            
            *width = surf.getWidth();
            *height = surf.getHeight();
            
            return tex.getTextureId();
        }
};
