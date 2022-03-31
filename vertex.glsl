#version 410 core
layout (location = 0) in vec3 position;
layout (location = 1) in vec2 tex_coord;
out vec2 tex;
uniform mat4 view;
uniform mat4 model;
uniform vec2 window_dim = vec2( 1280.0, 720.0 );
void main(){
	gl_Position = 
		view * mat4(
			( window_dim.y / window_dim.x ), 0.0, 0.0, 0.0,
			0.0, 1.0, 0.0, 0.0,
			0.0, 0.0, 1.0, 0.0,
			0.0, 0.0, 0.0, 1.0
		) * model * vec4( position, 1.0 )
	;
	tex = vec2( tex_coord.x, 1.0 - tex_coord.y );
}
