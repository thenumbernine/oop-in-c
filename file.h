#pragma once

STRUCT(file,
	(FILE*, fp, 0))

//TODO there's no real dif between FREE and DELETE I think ... both are suppoesd to be the ::delete() operator ... but the _delete is supposed to be the invocation ...
// same with _NEW and _ALLOC
// so how about use vtables and just put one of those entries in ,and have ea generic "new" and "delete" which calsl into vtables?
MAKE_DEFAULTS(file, ALLOC, FREE, DELETE)

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

str_t * file_read(
	file_t * const f,
	size_t size
) {
	str_t * s = str_new_size(size);
	size_t n = fread(s->ptr, 1, size, f->fp);
	if (n != size) fail("expected %lu bytes but only read %lu", size, n);
	return s;
}
