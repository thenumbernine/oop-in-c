#pragma once

STRUCT(file,
	(FILE*, fp, 0))

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
	file_t const * const f,
	size_t size
) {
	if (!f->fp) fail("file is already closed");
	str_t * s = str_new_size(size);
	size_t n = fread(s->ptr, 1, size, f->fp);
	if (n != size) fail("expected to read %lu bytes but could only read %lu", size, n);
	return s;
}

void file_write(
	file_t const * const f,
	str_t const * const s
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

//TODO there's no real dif between FREE and DELETE I think ... both are suppoesd to be the ::delete() operator ... but the _delete is supposed to be the invocation ...
// same with _NEW and _ALLOC
// so how about use vtables and just put one of those entries in ,and have ea generic "new" and "delete" which calsl into vtables?
MAKE_DEFAULTS(file, ALLOC, FREE, DELETE)
