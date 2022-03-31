.POSIX:

PROJ = cef

SRC = ${PROJ}.c
OBJ = ${SRC:.c=.o}

INCS = -I/usr/include -I/usr/include/opus -I${HOME}/proj/include
LIBS = ${HOME}/proj/lib/ef_audio.o -lm -lGL -lglfw -lGLEW -lpng -ldl -lpthread -lopus -lopusfile

INCLUDE = ${HOME}/proj/include
LIB = ${HOME}/proj/lib

CFLAGS	= ${INCS}
LDFLAGS	= ${LIBS}

CC = cc

all: options ef_shader.h ${PROJ}

options:
	@echo openal_raw build options:
	@echo "CFLAGS 	= ${CFLAGS}"
	@echo "LDFLAGS	= ${LDFLAGS}"
	@echo "CC	= ${CC}"

.c.o:
	${CC} -c ${CFLAGS} $<

${OBJ}: 

${PROJ}: ${OBJ}
	${CC} -o $@ ${OBJ} ${LDFLAGS}

vertex.glsl:

fragment.glsl:

ef_shader.h: vertex.glsl fragment.glsl
	@echo "test"
	echo "char ef_shader_vertex_source[] =" > ef_shader.h
	cat vertex.glsl | sed "s/^/\"/; s/$$/\\\n\"/" >> ef_shader.h
	echo ";" >> ef_shader.h
	echo "char ef_shader_fragment_source[] =" >> ef_shader.h
	cat fragment.glsl | sed "s/^/\"/; s/$$/\\\n\"/" >> ef_shader.h
	echo ";" >> ef_shader.h

cef.h:

install: cef.h ef_shader.h
	cp -f ${OBJ} ${LIB}
	cp -f ${PROJ}.h ${INCLUDE}
	cp -f ef_config.h ${INCLUDE}
	cp -f ef_init.h ${INCLUDE}
	cp -f ef_video.h ${INCLUDE}
	cp -f ef_shader.h ${INCLUDE}

clean:
	rm -f ${PROJ} ${OBJ} ef_shader.h

.PHONY: all options install clean
