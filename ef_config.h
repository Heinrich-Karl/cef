#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define EF_CONFIG_CREATE_NUM 2
#define EF_CONFIG_ADD_GROW 2
#define EF_CONFIG_TEXT_HASH_MAX 64
#define EF_CONFIG_TEXT_PARSE_MAX "256"
#define EF_CONFIG_HASH_TABLE_CREATE_SIZE 2
#define EF_CONFIG_HASH_TABLE_UPDATE_GROW 2

#define ef_config_parse_text( A, B, C ) ef_config_text_parse( A, B, C )
#define ef_config_hash_text( A ) ef_config_text_hash( A )
#define ef_config_create_hash_table() ef_config_hash_table_create()
#define ef_config_read_hash_table( A, B, C ) ef_config_hash_table_read( A, B, C )
#define ef_config_update_hash_table( A, B, C ) ef_config_hash_table_update( A, B, C )
#define ef_config_delete_hash_table( A ) ef_config_hash_table_delete( A )

enum EF_CONFIG_RETURN{
	EF_CONFIG_HASH_TABLE_READ_ERROR_UNFOUND = -9,
	EF_CONFIG_HASH_TABLE_READ_ERROR_OUT_OF_BOUNDS,
	EF_CONFIG_HASH_TABLE_READ_ERROR_NULL_PTR,
	EF_CONFIG_TEXT_HASH_ERROR_NULL_PTR,
	EF_CONFIG_TEXT_PARSE_ERROR_ALLOC_FAIL,
	EF_CONFIG_TEXT_PARSE_ERROR_NULL_PTR,
	EF_CONFIG_TRIM_ERROR_NULL_PTR,
	EF_CONFIG_ADD_ERROR_ALLOC_FAIL,
	EF_CONFIG_ADD_ERROR_NULL_PTR,
	EF_CONFIG_READ_ERROR_NULL_PTR = 0,
	EF_CONFIG_TEXT_PARSE_SUCCESS = 1
};

typedef struct ef_config_t {
	int* p;
	int size;
	int capacity;
} ef_config_t;

typedef struct ef_config_hash_item_t {
	struct ef_config_hash_item_t* p;
	char* s;
	int v;
} ef_config_hash_item_t;

typedef struct ef_config_hash_table_t {
	ef_config_hash_item_t** p;
	int size;
} ef_config_hash_table_t;

ef_config_t* ef_config_create( void );
int ef_config_read( ef_config_t* cfg, int i );
void ef_config_update( ef_config_t* cfg, int i, int v );
void ef_config_delete( ef_config_t** cfgp );
int ef_config_add( ef_config_t* cfg, int v );
int ef_config_trim( ef_config_t* cfg );
int ef_config_text_parse( char* s, ef_config_t* cfg, ef_config_hash_table_t* hst );
int ef_config_text_hash( char* s );
ef_config_hash_table_t* ef_config_hash_table_create( void );
int ef_config_hash_table_read( ef_config_hash_table_t* hst, int hash, char* s );
void ef_config_hash_table_update( ef_config_hash_table_t* hst, char* s, int v );
void ef_config_hash_table_delete( ef_config_hash_table_t** hstp );

ef_config_t*
ef_config_create( void ){
	ef_config_t* cfg = (ef_config_t*) calloc( 1, sizeof( ef_config_t ) );
	if( cfg != NULL ){
		cfg->p = (int*) calloc( EF_CONFIG_CREATE_NUM, sizeof( int ) );
		if( cfg->p != NULL ){
			cfg->capacity = EF_CONFIG_CREATE_NUM;
			return cfg;
		}
	}
		return NULL;
}

int
ef_config_read( ef_config_t* cfg, int i ){
	if( cfg != NULL && cfg->p != NULL && i >= 0 && cfg->size > i )
		return cfg->p[i];
	return EF_CONFIG_READ_ERROR_NULL_PTR;
}

void
ef_config_update( ef_config_t* cfg, int i, int v ){
	if( cfg != NULL && cfg->p != NULL && i >= 0 && cfg->size > i )
		cfg->p[i] = v;
}

void
ef_config_delete( ef_config_t** cfgp ){
	if( (*cfgp) != NULL ){
		if( (*cfgp)->p != NULL )
			free( (*cfgp)->p );
		free( (*cfgp) );
		(*cfgp) = NULL;
	}
}

int
ef_config_add( ef_config_t* cfg, int v ){
	if( cfg != NULL && cfg->p != NULL && cfg->capacity > cfg->size ){
		cfg->size++;
		cfg->p[cfg->size - 1] = v;
		return cfg->size - 1;
	}else{
		int* q = realloc( cfg->p, sizeof( int ) * cfg->capacity * EF_CONFIG_ADD_GROW );
		if( q != NULL ){
			cfg->p = q;
			cfg->capacity *= EF_CONFIG_ADD_GROW;
			return ef_config_add( cfg, v );
		}
		return EF_CONFIG_ADD_ERROR_ALLOC_FAIL;
	}
	return EF_CONFIG_ADD_ERROR_NULL_PTR;
}

int
ef_config_trim( ef_config_t* cfg ){
	if( cfg != NULL && cfg->p != NULL && cfg->capacity > cfg->size ){
		int* q = realloc( cfg->p, sizeof( int ) * cfg->size );
		if( q != NULL ){
			cfg->capacity = cfg->size;
			cfg->p = q;
		}
		return cfg->capacity == cfg->size;
	}
	return EF_CONFIG_TRIM_ERROR_NULL_PTR;
}

int
ef_config_text_parse( char* s, ef_config_t* cfg, ef_config_hash_table_t* hst ){
	if( cfg != NULL && s != NULL ){
		char delim[] = "\n";
		char* tok = strtok( s, delim );
		while( tok != NULL ){
			char format[] = "%"EF_CONFIG_TEXT_PARSE_MAX"s %d\n";
			char t[atoi( EF_CONFIG_TEXT_PARSE_MAX )];
			int v;
			sscanf( tok, format, t, &v );
			int hash = ef_config_text_hash( t );
			if( hash == EF_CONFIG_TEXT_HASH_ERROR_NULL_PTR )
				continue;
			int i = ef_config_hash_table_read( hst, hash, t );
			if( i == EF_CONFIG_HASH_TABLE_READ_ERROR_OUT_OF_BOUNDS )
				return EF_CONFIG_HASH_TABLE_READ_ERROR_OUT_OF_BOUNDS;
			for( ; i >= cfg->size; ){
				if( ef_config_add( cfg, 0 ) == EF_CONFIG_ADD_ERROR_ALLOC_FAIL )
					return EF_CONFIG_TEXT_PARSE_ERROR_ALLOC_FAIL;
			}
			ef_config_update( cfg, i, v );
			tok = strtok( NULL, delim );
			
		}
		return EF_CONFIG_TEXT_PARSE_SUCCESS;
	}
	return EF_CONFIG_TEXT_PARSE_ERROR_NULL_PTR;
}

int
ef_config_text_hash( char* s ){
	if( s != NULL ){
		int h = 0;
		for( int i = 0; s[i] != '\0'; i++ )
			h += abs( ( (int) s[i] ) << i );
		return h % EF_CONFIG_TEXT_HASH_MAX;
	}
	return EF_CONFIG_TEXT_HASH_ERROR_NULL_PTR;
}

ef_config_hash_table_t*
ef_config_hash_table_create( void ){
	ef_config_hash_table_t* hst = (ef_config_hash_table_t*) malloc( sizeof( ef_config_hash_table_t ) );
	if( hst != NULL )
		hst->p = (ef_config_hash_item_t**) malloc( EF_CONFIG_HASH_TABLE_CREATE_SIZE * sizeof( ef_config_hash_item_t* ) );
	else
		return NULL;
	if( hst->p != NULL ){
		for( int i = 0; i < EF_CONFIG_HASH_TABLE_CREATE_SIZE; i++ )
			hst->p[i] = NULL;
		hst->size = EF_CONFIG_HASH_TABLE_CREATE_SIZE;
		return hst;
	}
	return NULL;
}

int
ef_config_hash_table_read( ef_config_hash_table_t* hst, int hash, char* s ){
	if( hst != NULL && hst->p != NULL && hst->p[hash] != NULL && s != NULL ){
		if( hash >= hst->size )
			return EF_CONFIG_HASH_TABLE_READ_ERROR_OUT_OF_BOUNDS;
		for( ef_config_hash_item_t* hi = hst->p[hash];; ){
			if( hi->s != NULL && strcmp( s, hi->s ) == 0 )
				return hi->v;
			else if( hi->p == NULL )
				return EF_CONFIG_HASH_TABLE_READ_ERROR_UNFOUND;
			else
				hi = hi->p;
		}
	}
	return EF_CONFIG_HASH_TABLE_READ_ERROR_NULL_PTR;
}

void
ef_config_hash_table_update( ef_config_hash_table_t* hst, char* s, int v ){
	if( hst != NULL && hst->p != NULL && s != NULL ){
		int hash = ef_config_text_hash( s );
		while( hash >= hst->size ){
			ef_config_hash_item_t** p = (ef_config_hash_item_t**) realloc( hst->p, EF_CONFIG_HASH_TABLE_UPDATE_GROW * hst->size * sizeof( ef_config_hash_item_t* ) );
			if( p == NULL )
				return;
			hst->p = p;
			for( int i = hst->size; i < hst->size * EF_CONFIG_HASH_TABLE_UPDATE_GROW; i++ )
				hst->p[i] = NULL;
			hst->size *= EF_CONFIG_HASH_TABLE_UPDATE_GROW;
		}
		ef_config_hash_item_t* ha = (ef_config_hash_item_t*) malloc( sizeof( ef_config_hash_item_t ) );
		if( ha == NULL )
			return;
		ha->s = (char*) malloc( ( strlen( s ) + 1 ) * sizeof( char ) );
		if( ha->s == NULL ){
			free( ha );
			return;
		}
		strcpy( ha->s, s );
		ha->v = v;
		ha->p = NULL;
		if( hst->p[hash] != NULL )
			for( ef_config_hash_item_t* hi = hst->p[hash];; ){
				if( hi->p == NULL ){
					hi->p = ha;
					return;
				}
				hi = hi->p;
			}
		hst->p[hash] = ha;
	}
}

void
ef_config_hash_table_delete( ef_config_hash_table_t** hstp ){
	if( hstp != NULL ){
		ef_config_hash_table_t* hst = *hstp;
		if( hst != NULL ){
			if( hst->p != NULL ){
				for( int i = 0; i < hst->size; i++ ){
					for( ef_config_hash_item_t* hi = hst->p[i]; hi != NULL; ){
						if( hi->s != NULL )
							free( hi->s );
						hi = hi->p;
					}
					for( ef_config_hash_item_t* hi = hst->p[i]; hi != NULL; ){
						if( hst->p[i]->p == NULL )
							break;
						ef_config_hash_item_t* hp = hi;
						hi = hi->p;
						if( hi == NULL ){
							hi = hst->p[i];
							printf( "Turns out %d of %s is reachable!\n", __LINE__, __FILE__ );
							continue;
						}
						if( hi->p == NULL ){
							free( hi );
							hp->p = NULL;
							hi = hst->p[i];
							continue;
						}
					}
					free( hst->p[i] );
				}
				free( hst->p );
			}
			free( hst );
			*hstp = NULL;
		}
	}
}
