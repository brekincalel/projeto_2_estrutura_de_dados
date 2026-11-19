CC ?= cc
AR ?= ar
CFLAGS ?= -std=c11 -Wall -Wextra -Wpedantic -O2
CPPFLAGS += -Ilib -D_POSIX_C_SOURCE=200809L -D_FILE_OFFSET_BITS=64
LDLIBS += -lm

BUILD_DIR = build
LIBRARY = libindice.a

SOURCES = \
	src/abb.c \
	src/avl.c \
	src/chave.c \
	src/csv.c \
	src/indexador.c \
	src/item.c \
	src/reader.c \
	src/reader_empresas.c \
	src/reader_nasa.c \
	src/rubro_negra.c
OBJECTS = $(SOURCES:src/%.c=$(BUILD_DIR)/%.o)
HEADERS = $(wildcard lib/*.h) src/reader_privado.h src/persistencia_privado.h

.PHONY: all test clean

# Ainda não há main.c; por enquanto, o build produz a biblioteca dos módulos.
all: $(LIBRARY)

$(LIBRARY): $(OBJECTS)
	$(AR) rcs $@ $^

$(BUILD_DIR)/%.o: src/%.c
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

$(OBJECTS): $(HEADERS)

$(BUILD_DIR)/test_arvores: tests/test_arvores.c $(HEADERS) $(LIBRARY)
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) $< -L. -lindice $(LDLIBS) -o $@

test: $(BUILD_DIR)/test_arvores
	./$(BUILD_DIR)/test_arvores

clean:
	rm -rf $(BUILD_DIR) $(LIBRARY) trabalho
