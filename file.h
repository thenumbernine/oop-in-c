#pragma once

#include <stdlib.h> //fopen, fclose, fread, fwrite, ftell, fseek

typedef FILE * FILEp_t;
MAKE_TYPEINFO(FILEp_t)

#define CLASS_file_fields (\
	(FILEp_t, fp, 1) /*TODO auto index and pass index into FOR_EACH*/ \
)
#define CLASS_file_methods (\
	(alloc, file_t *, ()),\
	(free, void, (file_t *)),\
	(destroy, void, (file_t *)),\
	(init, void, (file_t *, char const * filename, char const * mode)),\
	(close, void, (file_t *)),\
	(read, string_t *, (file_t const *, size_t size)),\
	(write, void, (file_t const *, string_t const *)),\
	(tell, size_t, (file_t *)),\
	(seek, void, (file_t *, size_t offset, int whence))\
)
CLASS(file)

void file_init(
	file_t * const f,
	char const * const filename,
	char const * const mode
) {
	f->fp = fopen(filename, mode);
	if (!f->fp) fail("failed to open filename \"%s\" for mode \"%s\"", filename, mode);
}

void file_close(
	file_t * const f
) {
	if (f->fp) fclose(f->fp);
	f->fp = NULL;
}

void file_destroy(
	file_t * const f
) {
	if (!f) return;
	file_close(f);
}

string_t * file_read(
	file_t const * const f,
	size_t size
) {
	if (!f->fp) fail("file is already closed");
	string_t * s = newobj(string,_size,size);
	size_t n = fread(s->ptr, 1, size, f->fp);
	if (n != size) fail("expected to read %lu bytes but could only read %lu", size, n);
	return s;
}

void file_write(
	file_t const * const f,
	string_t const * const s
) {
	if (!f->fp) fail("file is already closed");
	size_t n = fwrite(s->ptr, 1, s->len, f->fp);
	if (n != s->len) fail("expected to write %lu bytes but could only write %lu", s->len, n);
}

size_t file_tell(
	file_t * const f
) {
	if (!f->fp) fail("file is already closed");
	return ftell(f->fp);
}

void file_seek(
	file_t * const f,
	size_t offset,
	int whence
) {
	if (!f->fp) fail("file is already closed");
	if (fseek(f->fp, offset, whence) == -1) fail("failed to seek to %lu %d", offset, whence);
}

MAKE_DEFAULTS(file, ALLOC, FREE)
