//
//  Shader.fsh
//  launcher
//
//  Created by hahasasa on 2/29/16.
//  Copyright © 2016 hahasasa. All rights reserved.
//

varying lowp vec4 colorVarying;

void main()
{
    gl_FragColor = colorVarying;
}
