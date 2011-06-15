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


class CinderStringRenderer {

    public:

        virtual GLuint stringToTexture(std::string str, float font_size,float *width, float *height){

            Font f("Helvetica", font_size);
            Color c(1.0, 1.0, 1.0);
        
            TextLayout t = TextLayout();
            t.addLine(str);            
            t.setFont(f);
            t.setColor(c);
            
            Surface surf = t.render();
            Texture tex(surf);
            tex.setDoNotDispose();
            
            return tex.getId();
        }
};
