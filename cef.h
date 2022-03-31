#include "ef_config.h"
#include "ef_init.h"
#include "ef_video.h"
//#include <ef_audio.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "ef_shader.h"

#define EF_INIT_CONFIG_FILENAME "config.cfg"
#define EF_INIT_CONFIG "width 1280\nheight 720"

#define ef_window_t GLFWwindow
#define ef_set_key_callback( A ) ef_init_window_key_callback( ef_system->window, A )
#define ef_set_cursor_callback( A ) ef_init_cursor_pos_callback( ef_system->window, A )
#define ef_set_mouse_callback( A ) ef_init_mouse_button_callback( ef_system->window, A )
#define ef_set_scroll_callback( A ) ef_init_scroll_callback( ef_system->window, A )
#define ef_poll_events() glfwPollEvents()
#define ef_wait_events() glfwWaitEvents()
#define ef_wait_events_timeout( A ) glfwWaitEventsTimeout( A )
#define ef_swap() glfwSwapBuffers( ef_system->window )
#define ef_should_close() glfwWindowShouldClose( ef_system->window )
#define ef_get_time() glfwGetTime()

enum EF_CFG {
	EF_CFG_WIDTH,
	EF_CFG_HEIGHT,
	EF_CFG_SIZE
};

typedef struct ef_system_t {
	GLFWwindow* window;
	ef_config_t* cfg;
	ef_config_hash_table_t* hst; /* Some of these could be precomp */
	GLuint v_array;
	GLuint v_buffer;
	GLuint e_buffer;
	float delta_time;
	float prev_time;
} ef_system_t;

ef_system_t* ef_system = NULL;

void ef_init( char* title );
void ef_end( void );

/* CREATE */
ef_system_t* ef_create_system( void );

/* UPDATE */
void ef_update_system_window( ef_system_t* sys, GLFWwindow* win );
void ef_update_delta_time( void );

/* DELETE */
void ef_delete_system( ef_system_t** sysp );

void
ef_init( char* title ){
	ef_system_t* sys = ef_create_system();
	if( sys != NULL ){
		char* cfg_s;
		FILE* file = fopen( EF_INIT_CONFIG_FILENAME, "rb" );
		if( file != NULL ){
			fseek( file, 0, SEEK_END );
			long size = ftell( file );
			fseek( file, 0, SEEK_SET );
			cfg_s = (char*) malloc( size * sizeof( char ) );
			fread( cfg_s, sizeof( char ), size, file );
			fclose( file );
		}else{
			cfg_s = (char*) malloc( sizeof( EF_INIT_CONFIG ) );
			strcpy( cfg_s, EF_INIT_CONFIG );
		}
		ef_config_hash_table_update( sys->hst, "width", EF_CFG_WIDTH );
		ef_config_hash_table_update( sys->hst, "height", EF_CFG_HEIGHT );
		ef_config_text_parse( cfg_s, sys->cfg, sys->hst );
		ef_update_system_window( sys, ef_init_window( title, ef_config_read( sys->cfg, EF_CFG_WIDTH ), ef_config_read( sys->cfg, EF_CFG_HEIGHT ) ) );
		free( cfg_s );
		ef_init_vertices( &(sys->v_array), &(sys->v_buffer), &(sys->e_buffer) );
		sys->delta_time = 1.0f;
		sys->prev_time = ef_get_time();
	}
	ef_system = sys;
}

void
ef_end( void ){
	if( ef_system != NULL ){
		if( ef_system->window != NULL ){
			glfwDestroyWindow( ef_system->window );
			ef_system->window = NULL;
		}
		ef_init_end( &(ef_system->v_array), &(ef_system->v_buffer), &(ef_system->e_buffer) );
		ef_delete_system( &ef_system );
	}
}

ef_system_t*
ef_create_system( void ){
	ef_system_t* sys = (ef_system_t*) malloc( sizeof( ef_system_t ) );
	if( sys != NULL ){
		sys->window = NULL;
		sys->cfg = ef_config_create();
		sys->hst = ef_config_hash_table_create();
	}
	return sys;
}

void
ef_update_system_window( ef_system_t* sys, GLFWwindow* win ){
	if( sys != NULL ){
		sys->window = win;
	}
}

void
ef_update_delta_time( void ){
	ef_system->delta_time = ef_get_time() - ef_system->prev_time;
	ef_system->prev_time = ef_get_time();
}

void
ef_delete_system( ef_system_t** sysp ){
	if( (*sysp) == NULL )
		return;
	ef_config_delete( &((*sysp)->cfg) );
	ef_config_hash_table_delete( &((*sysp)->hst) );
	free( (*sysp) );
	(*sysp) = NULL;
}
