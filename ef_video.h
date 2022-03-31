/* This file will render frames */
#include <stdlib.h>
#include <string.h>
#include <cglm/cglm.h>
#include <cglm/mat4.h>
#include <png.h>

#define EF_VIDEO_VIEW_NAME "view"
#define EF_VIDEO_MODEL_NAME "model"
#define EF_VIDEO_DIM_NAME "window_dim"

#define EF_VIDEO_CREATE_TEX_TABLE_NUM 8
#define EF_VIDEO_UPDATE_TEX_TABLE_GROW 2
#define EF_VIDEO_CREATE_LAYER_INDEX -1

#define ef_video_clear_color( A, B, C, D ) glClearColor( A, B, C, D )
#define ef_video_clear() glClear( GL_COLOR_BUFFER_BIT )

enum EF_VIDEO_RETURN {
	EF_VIDEO_READ_TEX_TABLE_ERROR = -6,
	EF_VIDEO_READ_LAYER_INDEX_ERROR_NULL_PTR,
	EF_VIDEO_UPDATE_TEX_TABLE_ERROR_NULL_PTR,
	EF_VIDEO_UPDATE_TEX_TABLE_ERROR_ALLOC_FAIL,
	EF_VIDEO_UPDATE_TEX_TABLE_PNG_ERROR,
	EF_VIDEO_UPDATE_TEX_TABLE_TEX_ERROR_ALLOC_FAIL
};

typedef struct ef_video_tex_table_t {
	GLuint* p;
	int size;
	int capacity;
} ef_video_tex_table_t;

typedef struct ef_video_animation_t {
	int frame;
	int base;
	int max;
	float time;
	float fps;
} ef_video_animation_t;

typedef struct ef_video_element_t {
	float x;
	float y;
	float rot;
	float sx;
	float sy;
	int texture;
	char* s;
	ef_video_animation_t* ap;
} ef_video_element_t;

typedef struct ef_video_layer_t {
	int width;
	int height;
	int index;
	GLuint texture;
	GLuint frame_buffer;
} ef_video_layer_t;

typedef struct ef_video_scene_t {
	GLint view_loc;
	GLint model_loc;
	GLint dim_loc;
	mat4 view;
	float view_x;
	float view_y;
	float view_rot;
	float view_scl;
	ef_video_tex_table_t* tt;
	int width;
	int height;
} ef_video_scene_t;

/* DRAW */
void ef_video_draw_element( ef_video_scene_t* scn, ef_video_element_t* e );

/* SET */

/* GET */

/* CREATE */
ef_video_tex_table_t* ef_video_create_tex_table( ef_video_scene_t* scn );
ef_video_element_t* ef_video_create_element( float x, float y, int texture, char* s, ef_video_animation_t* ap );
ef_video_scene_t* ef_video_create_scene( GLuint s_program, int width, int height );
ef_video_layer_t* ef_video_create_layer( float x, float y, int width, int height );

/* READ */
GLuint ef_video_read_tex_table( ef_video_scene_t* scn, int index );
int ef_video_read_layer_index( ef_video_layer_t* lay );

/* UPDATE */
int ef_video_update_tex_table( ef_video_scene_t* scn, char* img, int w, int h );
int ef_video_update_tex_table_png( ef_video_scene_t* scn, char* filename );
int ef_video_update_tex_table_tex( ef_video_scene_t* scn, GLuint texture );
void ef_video_update_scene_view_pos( ef_video_scene_t* scn, float x, float y );
void ef_video_update_scene_view_rot( ef_video_scene_t* scn, float rot );
void ef_video_update_scene_view_scl( ef_video_scene_t* scn, float scl );
void ef_video_update_element_pos( ef_video_element_t* e, float x, float y );
void ef_video_update_element_rot( ef_video_element_t* e, float rot );
void ef_video_update_element_scl( ef_video_element_t* e, float sx, float sy );

/* DELETE */
void ef_video_delete_tex_table( ef_video_tex_table_t** ttp );
void ef_video_delete_element( ef_video_element_t** ep );
void ef_video_delete_scene( ef_video_scene_t** scnp );
void ef_video_delete_layer( ef_video_layer_t** layp );

/* MISC */
void ef_video_calc_mat( mat4 m, float x, float y, float rot, float sx, float sy );
//void ef_video_render_layer( ef_video_scene_t* scn, ef_video_layer_t* lay );
void ef_video_open_layer( ef_video_scene_t* scn, ef_video_layer_t* lay );
void ef_video_close_layer( ef_video_scene_t* scn, ef_video_layer_t* lay );

/* DRAW */

void
ef_video_draw_element( ef_video_scene_t* scn, ef_video_element_t* e ){
	if( scn != NULL && e != NULL ){
		mat4 model;
		if( e->ap != NULL ){
			ef_video_animation_t* ap = e->ap;
			float time = glfwGetTime();
			if( ( time - ap->time ) >= ( 1.0 / ap->fps ) ){
				ap->time = time;	
				if( ap->frame >= ap->max ){
					ap->frame = 0;
				}else
					ap->frame++;
				e->texture = ap->base + ap->frame;
			}
		}
		GLuint texture = ef_video_read_tex_table( scn, e->texture );
		ef_video_calc_mat( model, e->x, e->y, e->rot, e->sx, e->sy );
		glUniformMatrix4fv( scn->model_loc, 1, GL_FALSE, (float*) model );
		glBindTexture( GL_TEXTURE_2D, texture );
		glDrawElements( GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0 );
		glBindTexture( GL_TEXTURE_2D, 0 );
	}
}

/* SET */

/* GET */

/* CREATE */

ef_video_tex_table_t*
ef_video_create_tex_table( ef_video_scene_t* scn ){
	ef_video_tex_table_t* tt = (ef_video_tex_table_t*) malloc( sizeof( ef_video_tex_table_t ) );
	if( tt != NULL ){
		tt->size = 0;
		tt->capacity = 0;
		tt->p = calloc( EF_VIDEO_CREATE_TEX_TABLE_NUM, sizeof( int ) );
		if( tt->p != NULL )
			tt->capacity = EF_VIDEO_CREATE_TEX_TABLE_NUM;
	}
	scn->tt = tt;
	return tt;
}

ef_video_element_t*
ef_video_create_element( float x, float y, int texture, char* s, ef_video_animation_t* ap ){
	ef_video_element_t* e = (ef_video_element_t*) malloc( sizeof( ef_video_element_t ) );
	if( e != NULL ){
		e->x = x;
		e->y = y;
		e->rot = 0.0f;
		e->sx = 1.0f;
		e->sy = 1.0f;
		e->texture = texture;
		e->s = s;
		e->ap = ap;
		if( e->ap != NULL )
			e->ap->base = e->texture;
	}
	return e;
}

ef_video_scene_t*
ef_video_create_scene( GLuint s_program, int width, int height ){
	ef_video_scene_t* scn = (ef_video_scene_t*) malloc( sizeof( ef_video_scene_t ) );
	if( scn != NULL ){
		scn->view_loc = glGetUniformLocation( s_program, EF_VIDEO_VIEW_NAME );
		scn->model_loc = glGetUniformLocation( s_program, EF_VIDEO_MODEL_NAME );
		scn->dim_loc = glGetUniformLocation( s_program, EF_VIDEO_DIM_NAME );
		glm_mat4_identity( scn->view );
		glUniformMatrix4fv( scn->view_loc, 1, GL_FALSE, (float*) scn->view );
		glUniform2f( scn->dim_loc, width, height );
		scn->view_x = 0.0f;
		scn->view_y = 0.0f;
		scn->view_rot = 0.0f;
		scn->view_scl = 1.0f;
		scn->tt = NULL;
		scn->width = width;
		scn->height = height;
	}
	return scn;
}

ef_video_layer_t*
ef_video_create_layer( float x, float y, int width, int height ){
	ef_video_layer_t* lay = (ef_video_layer_t*) malloc( sizeof( ef_video_layer_t ) );
	if( lay != NULL ){
		lay->width = 0;
		lay->height = 0;
		lay->index = EF_VIDEO_CREATE_LAYER_INDEX;
		if( width > 0 && height > 0 ){
			lay->width = width;
			lay->height = height;
		}
		glGenTextures( 1, &(lay->texture) );
		glBindTexture( GL_TEXTURE_2D, lay->texture );
		glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, lay->width, lay->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
		glBindTexture( GL_TEXTURE_2D, 0 );
		glGenFramebuffers( 1, &(lay->frame_buffer) );
		glBindFramebuffer( GL_FRAMEBUFFER, lay->frame_buffer );
		glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, lay->texture, 0 );
		glBindFramebuffer( GL_FRAMEBUFFER, 0 );
	}
	return lay;
}

/* READ */

GLuint
ef_video_read_tex_table( ef_video_scene_t* scn, int index ){
	if( scn != NULL && index >= 0 && scn->tt->size > index ){
		return scn->tt->p[index];
	}
	return EF_VIDEO_READ_TEX_TABLE_ERROR;
}

int
ef_video_read_layer_index( ef_video_layer_t* lay ){
	if( lay != NULL ){
		return lay->index;
	}
	return EF_VIDEO_READ_LAYER_INDEX_ERROR_NULL_PTR;
}

/* UPDATE */

int
ef_video_update_tex_table( ef_video_scene_t* scn, char* img, int w, int h ){
	ef_video_tex_table_t* tt = scn->tt;
	if( tt != NULL && img != NULL ){
		while( tt->size >= tt->capacity ){
			GLuint* q = (GLuint*) realloc( tt->p, tt->capacity * EF_VIDEO_UPDATE_TEX_TABLE_GROW * sizeof( GLuint ) );
			if( q == NULL )
				return EF_VIDEO_UPDATE_TEX_TABLE_ERROR_ALLOC_FAIL;
			tt->p = q;
			tt->capacity *= EF_VIDEO_UPDATE_TEX_TABLE_GROW;
		}
		glGenTextures( 1, tt->p + tt->size );
		glBindTexture( GL_TEXTURE_2D, tt->p[tt->size] );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
		glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, img );
		glGenerateMipmap( GL_TEXTURE_2D );
		glBindTexture( GL_TEXTURE_2D, 0 );
		return tt->size++;
	}
	return EF_VIDEO_UPDATE_TEX_TABLE_ERROR_NULL_PTR;
}

int
ef_video_update_tex_table_png( ef_video_scene_t* scn, char* filename ){
	if( scn == NULL || filename == NULL )
		return EF_VIDEO_UPDATE_TEX_TABLE_PNG_ERROR;
	FILE* file = fopen( filename, "rb" );
	if( file != NULL ){
		const int sig_size = 8;
		char sig[sig_size];
		if( !( fread( sig, sizeof( char ), sig_size, file ) == sig_size && !png_sig_cmp( sig, 0, sig_size ) ) ){
			fclose( file );
			return EF_VIDEO_UPDATE_TEX_TABLE_PNG_ERROR;
		}
		png_structp png_ptr = png_create_read_struct( PNG_LIBPNG_VER_STRING, NULL, NULL, NULL );
		if( png_ptr == NULL ){
			fclose( file );
			return EF_VIDEO_UPDATE_TEX_TABLE_PNG_ERROR;
		}
		png_infop info_ptr = png_create_info_struct( png_ptr );
		if( info_ptr == NULL ){
			fclose( file );
			png_destroy_read_struct( &png_ptr, NULL, NULL );
			return EF_VIDEO_UPDATE_TEX_TABLE_PNG_ERROR;
		}
		png_init_io( png_ptr, file );
		png_set_sig_bytes( png_ptr, sig_size );
		png_read_info( png_ptr, info_ptr );
		int width = png_get_image_width( png_ptr, info_ptr );
		int height = png_get_image_height( png_ptr, info_ptr );
		int color_type = png_get_color_type( png_ptr, info_ptr );
		int bit_depth = png_get_bit_depth( png_ptr, info_ptr );
		if( color_type == PNG_COLOR_TYPE_PALETTE ){
			png_set_palette_to_rgb( png_ptr );
			png_read_update_info( png_ptr, info_ptr );
			color_type = png_get_color_type( png_ptr, info_ptr );
		}
		if( color_type == PNG_COLOR_TYPE_GRAY || color_type == PNG_COLOR_TYPE_GRAY_ALPHA ){
			png_set_gray_to_rgb( png_ptr );
			png_read_update_info( png_ptr, info_ptr );
			color_type = png_get_color_type( png_ptr, info_ptr );
		}
		if( color_type == PNG_COLOR_TYPE_RGB ){
			png_set_add_alpha( png_ptr, 255, PNG_FILLER_AFTER );
			png_read_update_info( png_ptr, info_ptr );
			color_type = png_get_color_type( png_ptr, info_ptr );
		}
		if( bit_depth == 16 ){
			png_set_scale_16( png_ptr );
			png_read_update_info( png_ptr, info_ptr );
			bit_depth = png_get_bit_depth( png_ptr, info_ptr );
		}
		png_bytepp row_pointers = (png_bytepp) malloc( sizeof( png_bytep ) * height );
		if( row_pointers == NULL ){
			png_destroy_info_struct( png_ptr, &info_ptr );
			png_destroy_read_struct( &png_ptr, NULL, NULL );	
			return EF_VIDEO_UPDATE_TEX_TABLE_PNG_ERROR;
		}
		for( int i = 0; i < height; i++ ){
			row_pointers[i] = (png_bytep) malloc( png_get_rowbytes( png_ptr, info_ptr ) );
			if( row_pointers[i] == NULL ){
				for( int j = 0; j < i; j++ )
					free( row_pointers[i] );
				free( row_pointers );
				png_destroy_info_struct( png_ptr, &info_ptr );
				png_destroy_read_struct( &png_ptr, NULL, NULL );
				return EF_VIDEO_UPDATE_TEX_TABLE_PNG_ERROR;
			}
		}
		png_read_image( png_ptr, row_pointers );
		fclose( file );
		int rtrn = EF_VIDEO_UPDATE_TEX_TABLE_PNG_ERROR;
		char* image = (char*) malloc( 4 * sizeof( char ) * width * height );
		if( image != NULL ){
			for( int i = 0; i < height; i++ ){
				memcpy( image + 4 * width * i , row_pointers[i], width * 4 );
				free( row_pointers[i] );
			}
			rtrn = ef_video_update_tex_table( scn, image, width, height );
			free( image );
		}else{
			for( int i = 0; i < height; i++ )
				free( row_pointers[i] );
		}
		free( row_pointers );
		png_destroy_info_struct( png_ptr, &info_ptr );
		png_destroy_read_struct( &png_ptr, NULL, NULL );
		return rtrn;
	}
	return EF_VIDEO_UPDATE_TEX_TABLE_PNG_ERROR;
}

int
ef_video_update_tex_table_tex( ef_video_scene_t* scn, GLuint texture ){
	if( scn != NULL && scn->tt != NULL ){
		ef_video_tex_table_t* tt = scn->tt;
		for( int i = 0; i < tt->size; i++ ){
			if( texture == tt->p[i] )
				return i;
		}
		while( tt->size >= tt->capacity ){
			GLuint* q = (GLuint*) realloc( tt->p, tt->capacity * EF_VIDEO_UPDATE_TEX_TABLE_GROW * sizeof( GLuint ) );
			if( q == NULL )
				return EF_VIDEO_UPDATE_TEX_TABLE_TEX_ERROR_ALLOC_FAIL;
			tt->p = q;
			tt->capacity *= EF_VIDEO_UPDATE_TEX_TABLE_GROW;
		}
		tt->p[tt->size] = texture;
		return tt->size++;
	}
}

void
ef_video_update_scene_view_pos( ef_video_scene_t* scn, float x, float y ){
	scn->view_x = x;
	scn->view_y = y;
	ef_video_calc_mat( scn->view, scn->view_x, scn->view_y, scn->view_rot, scn->view_scl, scn->view_scl );
	glUniformMatrix4fv( scn->view_loc, 1, GL_FALSE, (float*) scn->view );
}

void ef_video_update_scene_view_rot( ef_video_scene_t* scn, float rot ){
	scn->view_rot = rot;
	ef_video_calc_mat( scn->view, scn->view_x, scn->view_y, scn->view_rot, scn->view_scl, scn->view_scl );
	glUniformMatrix4fv( scn->view_loc, 1, GL_FALSE, (float*) scn->view );
}

void
ef_video_update_scene_view_scl( ef_video_scene_t* scn, float scl ){
	scn->view_scl = scl;
	ef_video_calc_mat( scn->view, scn->view_x, scn->view_y, scn->view_rot, scn->view_scl, scn->view_scl );
	glUniformMatrix4fv( scn->view_loc, 1, GL_FALSE, (float*) scn->view );
}

void
ef_video_update_element_pos( ef_video_element_t* e, float x, float y ){
	e->x = x;
	e->y = y;
}

void
ef_video_update_element_rot( ef_video_element_t* e, float rot ){
	e->rot = rot;
}

void
ef_video_update_element_scl( ef_video_element_t* e, float sx, float sy ){
	e->sx = sx;
	e->sy = sy;
}

/* DELETE */

void
ef_video_delete_tex_table( ef_video_tex_table_t** ttp ){
	if( (*ttp) == NULL )
		return;
	if( (*ttp)->p != NULL ){
		glDeleteTextures( (*ttp)->size, (*ttp)->p );
		free( (*ttp)->p );
	}
	free( (*ttp) );
	(*ttp) = NULL;
}

void
ef_video_delete_element( ef_video_element_t** ep ){
	if( (*ep) == NULL )
		return;
	free( (*ep) );
	(*ep) = NULL;
}

void
ef_video_delete_scene( ef_video_scene_t** scnp ){
	if( (*scnp) == NULL )
		return;
	ef_video_delete_tex_table( &((*scnp)->tt) );
	free( (*scnp) );
	(*scnp) = NULL;
}

void
ef_video_delete_layer( ef_video_layer_t** layp ){
	if( (*layp) == NULL )
		return;
	glDeleteFramebuffers( 1, &((*layp)->frame_buffer) );
	glDeleteTextures( 1, &((*layp)->texture) );
	free( (*layp) );
	(*layp) = NULL;
}

/* MISC */

void
ef_video_calc_mat( mat4 m, float x, float y, float rot, float sx, float sy ){
	glm_mat4_identity( m );
	float v[] = {x, y, 0.0f};
	float sv[] = {sx, sy, 1.0f};
	glm_translate( m, v );
	glm_rotate_z( m, rot, m );
	glm_scale( m, sv );
}

void
ef_video_open_layer( ef_video_scene_t* scn, ef_video_layer_t* lay ){
	if( scn != NULL && lay != NULL ){
		glViewport( 0, 0, lay->width, lay->height );
		glUniform2f( scn->dim_loc, lay->width, lay->height );
		glBindFramebuffer( GL_FRAMEBUFFER, lay->frame_buffer );
	}
}

void
ef_video_close_layer( ef_video_scene_t* scn, ef_video_layer_t* lay ){
	if( scn != NULL && lay != NULL ){
		glBindTexture( GL_TEXTURE_2D, lay->texture );
		glGenerateMipmap( GL_TEXTURE_2D );
		glBindTexture( GL_TEXTURE_2D, 0 );
		lay->index = ef_video_update_tex_table_tex( scn, lay->texture );
		glBindFramebuffer( GL_FRAMEBUFFER, 0 );
		glUniform2f( scn->dim_loc, scn->width, scn->height );
		glViewport( 0, 0, scn->width, scn->height );
	}
}
