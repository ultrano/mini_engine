//
//  Shader.fsh
//  me_app
//
//  Created by hahasasa on 3/16/16.
//  Copyright © 2016 hahasasa. All rights reserved.
//

varying lowp vec4 colorVarying;

void main()
{
    gl_FragColor = colorVarying;
}
