/* This is a test of cef */
#include "cef.h"
#include <stdio.h>

void cef_key_callback( ef_window_t* window, int key, int scancode, int action, int mods ){}

void cef_cursor_callback( ef_window_t* window, double x, double y ){
	printf( "x: %f, y: %f\r", x, y );
}

void cef_mouse_callback( ef_window_t* window, int button, int action, int mods ){
	printf( "mouse\n" );
}

int main(){
	printf( "%s version %s @ %s\n", __FILE__, __DATE__, __TIME__ );

	ef_init( __FILE__ );
	//GLuint s_program = ef_init_shader( "default_shader.bin", 34655 );
	GLuint s_program = ef_init_shader_source( ef_shader_vertex_source, ef_shader_fragment_source );
	glfwSetInputMode( ef_system->window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN );
	ef_set_key_callback( cef_key_callback );
	ef_set_cursor_callback( cef_cursor_callback );
	ef_set_mouse_callback( cef_mouse_callback );

	/*//Makes fullscreen
	const GLFWvidmode* mode = glfwGetVideoMode( glfwGetPrimaryMonitor() );
	glfwSetWindowMonitor( window, glfwGetPrimaryMonitor(), 0, 0, mode->width, mode->height, mode->refreshRate );
	*/

	ef_video_scene_t* scn = ef_video_create_scene( s_program, ef_config_read( ef_system->cfg, EF_CFG_WIDTH ), ef_config_read( ef_system->cfg, EF_CFG_HEIGHT ) );
	ef_video_update_scene_view_scl( scn, 0.25f );
	ef_video_create_tex_table( scn );
	ef_video_update_tex_table_png( scn, "bg.png" );

	ef_video_element_t* background = ef_video_create_element( 0.0f, 0.0f, 0, NULL, NULL );
	ef_video_update_element_scl( background, 16.0f * 0.9f, 9.0f * 0.9f );

	char image[] = {
		255, 0, 0, 255,//Frame 0
		0, 255, 0, 255,
		0, 0, 255, 255,
		0, 0, 0, 0,
		255, 0, 0, 255,//Frame 1
		255, 0, 0, 255,
		0, 0, 255, 255,
		0, 0, 0, 0,
		255, 0, 0, 255,//Frame 2
		255, 0, 0, 255,
		0, 0, 255, 255,
		255, 0, 0, 255,
		255, 0, 0, 255,//Frame 3
		255, 0, 0, 255,
		255, 0, 0, 255,
		255, 0, 0, 255
	};
	ef_video_update_tex_table( scn, image, 2, 2 );
	ef_video_update_tex_table( scn, image + 16 * 1, 2, 2 );
	ef_video_update_tex_table( scn, image + 16 * 2, 2, 2 );
	ef_video_update_tex_table( scn, image + 16 * 3, 2, 2 );
	ef_video_animation_t ani;
	ani.frame = 0;
	ani.max = 3;
	ani.time = glfwGetTime();
	ani.fps = 6.0;
	ef_video_element_t* e = ef_video_create_element( 0.0f, 0.0f, 1, NULL, &ani );

	ef_video_update_tex_table_png( scn, "red.png" );
	ef_video_element_t* fb_element = ef_video_create_element( 0.0f, 0.0f, 5, NULL, NULL );
	ef_video_layer_t* lay = ef_video_create_layer( 0.0f, 0.0f, 500, 500 );
	ef_video_open_layer( scn, lay );
	ef_video_clear_color( 0.0f, 0.0f, 0.0f, 0.0f );
	ef_video_clear();
	ef_video_update_scene_view_scl( scn, 1.0f );
	ef_video_draw_element( scn, fb_element );
	ef_video_update_scene_view_scl( scn, 0.25f );
	ef_video_draw_element( scn, e );
	ef_video_close_layer( scn, lay );
	ef_video_element_t* result_e = ef_video_create_element( 1.0f, 1.0f, lay->index, NULL, NULL );
	result_e->sx = 5.0f;
	result_e->sy = 5.0f;

	ef_video_clear_color( 0.6f, 0.4f, 0.2f, 1.0f );
	do{
		ef_video_clear();
		//ef_poll_events();
		ef_wait_events_timeout( 0.0001 );// Low cpu util ☺
		ef_video_draw_element( scn, background );
		ef_video_draw_element( scn, e );
		result_e->rot -= 0.01f;
		result_e->x += 0.01f;
		ef_video_draw_element( scn, result_e );
		ef_swap();
	}while( !ef_should_close() );

	ef_video_delete_layer( &lay );

	ef_video_delete_element( &e );
	ef_video_delete_element( &background );
	ef_video_delete_scene( &scn );

	ef_end();

	return 0;
}
