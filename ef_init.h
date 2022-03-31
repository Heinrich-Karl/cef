#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <GL/gl.h>
#include <stdio.h>

#define EF_INIT_INFO_LOG_SIZE 512

/* Planning
 * to add mutable geometry shaders
 * */

GLFWwindow* ef_init_window( const char* title, int width, int height );
void ef_init_window_key_callback( GLFWwindow* window, void (*key_callback)(GLFWwindow*,int,int,int,int) );
void ef_init_cursor_pos_callback( GLFWwindow* window, void (*cursor_callback)(GLFWwindow*,double,double) );
void ef_init_mouse_button_callback( GLFWwindow* window, void (*mouse_callback)(GLFWwindow*,int,int,int) );
void ef_init_scroll_callback( GLFWwindow* window, void (*scroll_callback)(GLFWwindow*,double,double) );
GLuint ef_init_shader( char* filename, GLenum type );
GLuint ef_init_shader_source( char* v_shader_source, char* f_shader_source );
GLuint ef_init_vertices( GLuint* v_array, GLuint* v_buffer, GLuint* e_buffer );
void ef_init_end( GLuint* v_array, GLuint* v_buffer, GLuint* e_buffer );

void ef_init_window_close_callback( GLFWwindow* window );

GLFWwindow*
ef_init_window( const char* title, int width, int height ){
	glfwInit();
	glfwWindowHint( GLFW_CONTEXT_VERSION_MAJOR, 4 );
	glfwWindowHint( GLFW_CONTEXT_VERSION_MINOR, 1 );
	glfwWindowHint( GLFW_RESIZABLE, GL_FALSE );
	GLFWwindow* window = glfwCreateWindow( width, height, title, NULL, NULL );
	glfwMakeContextCurrent( window );
	glewExperimental = GL_TRUE;
	glewInit();
	glViewport( 0, 0, width, height );
	glfwSetWindowCloseCallback( window, ef_init_window_close_callback );
	return window;
}

void
ef_init_window_key_callback( GLFWwindow* window, void (*key_callback)(GLFWwindow*,int,int,int,int) ){
	if( window != NULL && key_callback != NULL )
		glfwSetKeyCallback( window, key_callback );

}

void
ef_init_cursor_pos_callback( GLFWwindow* window, void (*cursor_callback)(GLFWwindow*,double,double) ){
	if( window != NULL && cursor_callback != NULL )
		glfwSetCursorPosCallback( window, cursor_callback );
}

void
ef_init_mouse_button_callback( GLFWwindow* window, void (*mouse_callback)(GLFWwindow*,int,int,int) ){
	if( window != NULL && mouse_callback != NULL )
		glfwSetMouseButtonCallback( window, mouse_callback );
}

void
ef_init_scroll_callback( GLFWwindow* window, void (*scroll_callback)(GLFWwindow*,double,double) ){
	if( window != NULL && scroll_callback != NULL )
		glfwSetScrollCallback( window, scroll_callback );
}

GLuint
ef_init_shader( char* filename, GLenum type ){
	GLuint shader_program = glCreateProgram();
	FILE* file = fopen( filename, "rb" );
	if( file != NULL ){
		fseek( file, 0, SEEK_END );
		long size = ftell( file );
		fseek( file, 0, SEEK_SET );
		char* buffer = (char*) malloc( size * sizeof( char ) );
		fread( buffer, sizeof( char ), size, file );
		fclose( file );
		glProgramBinary( shader_program, type, buffer, size );
		glUseProgram( shader_program );
	}
	return shader_program;
}

GLuint
ef_init_shader_source( char* v_shader_source, char* f_shader_source ){
	const int INFO_LOG_SIZE = 512;
	GLuint s_program;
	GLuint v_shader;
	GLuint f_shader;
	GLint success;
	v_shader = glCreateShader( GL_VERTEX_SHADER );
	f_shader = glCreateShader( GL_FRAGMENT_SHADER );
	glShaderSource( v_shader, 1, (const GLchar* const*) &v_shader_source, NULL );
	glShaderSource( f_shader, 1, (const GLchar* const*) &f_shader_source, NULL );
	glCompileShader( v_shader );
	glGetShaderiv( v_shader, GL_COMPILE_STATUS, &success );
	if( !success ){
		GLchar info_log[INFO_LOG_SIZE];
		glGetShaderInfoLog( v_shader, INFO_LOG_SIZE, NULL, info_log );
		fprintf( stderr, "%s, %d error: %s\n", __FILE__, __LINE__, info_log );
	}
	glCompileShader( f_shader );
	glGetShaderiv( f_shader, GL_COMPILE_STATUS, &success );
	if( !success ){
		GLchar info_log[INFO_LOG_SIZE];
		glGetShaderInfoLog( f_shader, INFO_LOG_SIZE, NULL, info_log );
		fprintf( stderr, "%s, %d error: %s\n", __FILE__, __LINE__, info_log );
	}
	s_program = glCreateProgram();
	glAttachShader( s_program, v_shader );
	glAttachShader( s_program, f_shader );
	glLinkProgram( s_program );
	glGetProgramiv( s_program, GL_LINK_STATUS, &success );
	if( !success ){
		GLchar info_log[INFO_LOG_SIZE];
		glGetProgramInfoLog( s_program, INFO_LOG_SIZE, NULL, info_log );
		fprintf( stderr, "%s, %d error: %s\n", __FILE__, __LINE__, info_log );
	}
	glUseProgram( s_program );
	glDeleteShader( v_shader );
	glDeleteShader( f_shader );
	return s_program;
}

GLuint ef_init_vertices( GLuint* v_array, GLuint* v_buffer, GLuint* e_buffer ){
	GLfloat vert[] = {
		-0.5f,	0.5f,	0.0f,	0.0f,	1.0f,
		0.5f,	0.5f,	0.0f,	1.0f,	1.0f,
		0.5f,	-0.5f,	0.0f,	1.0f,	0.0f,
		-0.5f,	-0.5f,	0.0f,	0.0f,	0.0f
	};
	GLuint indi[] = {
		0,	1,	2,
		0,	3,	2
	};
	GLuint vao;
	GLuint vbo;
	GLuint ebo;
	glGenVertexArrays( 1, &vao );
	glGenBuffers( 1, &vbo );
	glGenBuffers( 1, &ebo );
	glBindVertexArray( vao );
	glBindBuffer( GL_ARRAY_BUFFER, vbo );
	glBufferData( GL_ARRAY_BUFFER, sizeof( vert ), vert, GL_STATIC_DRAW );
	glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, sizeof( GLfloat ) * 5, (GLvoid*) 0);
	glEnableVertexAttribArray( 0 );
	glVertexAttribPointer( 1, 2, GL_FLOAT, GL_FALSE, sizeof( GLfloat ) * 5, (GLvoid*) ( 3 * sizeof( GLfloat ) ) );
	glEnableVertexAttribArray( 1 );
	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, ebo );
	glBufferData( GL_ELEMENT_ARRAY_BUFFER, sizeof( indi ), indi, GL_STATIC_DRAW );
	if( v_array != NULL )
		*v_array = vao;
	if( v_buffer != NULL )
		*v_buffer = vbo;
	if( e_buffer != NULL )
		*e_buffer = ebo;
	return vao;
}

void ef_init_end( GLuint* v_array, GLuint* v_buffer, GLuint* e_buffer ){
	glBindVertexArray( 0 );
	if( v_array != NULL )
		glDeleteVertexArrays( 1, v_array );
	if( v_buffer != NULL )
		glDeleteBuffers( 1, v_buffer );
	if( e_buffer != NULL )
		glDeleteBuffers( 1, e_buffer );
	glfwTerminate();
}

void
ef_init_window_close_callback( GLFWwindow* window ){
	glfwSetWindowShouldClose( window, GLFW_TRUE );
}
