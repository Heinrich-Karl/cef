#version 410 core
in vec2 tex;
out vec4 color;
uniform sampler2D samp;
vec4 tmp_color;
void main(){
	tmp_color = texture( samp, tex );
	if( tmp_color.a == 0.0 )
		discard;
	color = tmp_color;
}
