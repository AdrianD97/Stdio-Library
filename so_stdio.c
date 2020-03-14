#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "so_stdio.h"

/* va retine pid-ul procesului copil
 * pentru a putea mai tarziu sa asteptam
 * ca acesta sa se termine (waitpid are nevoie
 * de pid-ul procesului pe care il asteptam
 * sa-si incheie executia)
 */
pid_t child_pid;

/* intoarce flagul/flagurile cu care
 * se doreste sa se deschida fiserul.
 */
int get_open_flags(const char *mode)
{
	if (strcmp(mode, RDONLY) == 0)
		return O_RDONLY;

	if (strcmp(mode, RD_WR) == 0)
		return O_RDWR;

	if (strcmp(mode, WDONLY) == 0)
		return O_CREAT | O_WRONLY | O_TRUNC;

	if (strcmp(mode, WD_RD) == 0)
		return O_CREAT | O_RDWR | O_TRUNC;

	if (strcmp(mode, APPEND) == 0)
		return O_CREAT | O_WRONLY | O_APPEND;

	if (strcmp(mode, APPEND_READ) == 0)
		return O_CREAT | O_RDWR | O_APPEND;

	return INVALID_MODE;
}

int so_fflush(SO_FILE *stream)
{
	int bytes_written;

	if (stream->last_op_type != WRITE_OPERATION)
		return SO_EOF;

	stream->index = 0;
	while (stream->buff_size > 0) {
		bytes_written = write(stream->file_descriptor,
			&stream->buffer[stream->index], stream->buff_size);
		if (bytes_written < 0) {
			stream->err_flag = ERR;
			return SO_EOF;
		}

		stream->index += bytes_written;
		stream->buff_size -= bytes_written;
	}

	stream->index = 0;

	return 0;
}

int so_fseek(SO_FILE *stream, long offset, int whence)
{
	int res;
	off_t pos;

	if (stream->last_op_type == READ_OPERATION) {
		stream->index = stream->buff_size = 0;
	} else if (stream->last_op_type == WRITE_OPERATION) {
		res = so_fflush(stream);
		if (res) {
			stream->err_flag = ERR;
			return SO_EOF;
		}
	}
	stream->last_op_type = NO_OPERATION;

	if (whence == SEEK_CUR) {
		offset += stream->current_position;
		whence = SEEK_SET;
	}

	if (whence == SEEK_END && offset == 0)
		stream->eof = EOF_;
	else
		stream->eof = NO_EOF_;

	pos = lseek(stream->file_descriptor, offset, whence);
	if (pos < 0) {
		stream->err_flag = ERR;
		return SO_EOF;
	}

	stream->current_position = (long)pos;

	return 0;
}

SO_FILE *so_fopen(const char *pathname, const char *mode)
{
	SO_FILE *file;
	int fd, flags;

	flags = get_open_flags(mode);

	if (flags == INVALID_MODE)
		return NULL;

	fd = open(pathname, flags);
	if (fd < 0)
		return NULL;

	file = (SO_FILE *)malloc(sizeof(SO_FILE));
	if (!file)
		return NULL;

	file->file_descriptor = fd;
	file->index = file->current_position = 0;
	file->buff_size = 0;
	file->last_op_type = NO_OPERATION;
	file->err_flag = NOT_ERR;
	file->eof = NO_EOF_;
	strcpy(file->mode, mode);
	strcpy(file->buffer, "");

	return file;
}

int so_fclose(SO_FILE *stream)
{
	int result, ret;

	if (!stream)
		return SO_EOF;

	result = 0;

	if (stream->last_op_type == WRITE_OPERATION)
		result = so_fflush(stream);

	ret = close(stream->file_descriptor);
	if (ret < 0)
		result = SO_EOF;

	free((void *)stream);

	return result;
}

int so_fgetc(SO_FILE *stream)
{
	int mode;
	int bytes_read, result;
	int size = BUFF_SIZE;

	/*
	 *	verific daca se incearca citirea dintr-un fisier
	 *	care a fost deschis doar pentru scriere(w sau a),
	 *	iar in caz afirmativ semnalez o eroare
	 */
	mode = strcmp(stream->mode, WDONLY) & strcmp(stream->mode, APPEND);
	if (!mode) {
		stream->err_flag = ERR;
		return SO_EOF;
	}

	if (stream->last_op_type == WRITE_OPERATION) {
		result = so_fflush(stream);
		if (result) {
			stream->err_flag = ERR;
			return SO_EOF;
		}
	}

	/* bufferul nu este gol */
	if (stream->buff_size > 0) {
		--stream->buff_size;
		++stream->current_position;
		stream->last_op_type = READ_OPERATION;
		return (int)(unsigned char)stream->buffer[stream->index++];
	}

	/* bufferul este gol */
	stream->index = 0;

	bytes_read = read(stream->file_descriptor,
		&stream->buffer[stream->index], size);
	if (bytes_read < 0) {
		stream->err_flag = ERR;
		return SO_EOF;
	}

	/* nu mai am date in fisierul propriu-zis, deci eroare */
	if (bytes_read == 0) {
		stream->eof = EOF_;
		stream->err_flag = ERR;
		return SO_EOF;
	}

	stream->buff_size += bytes_read;
	++stream->current_position;
	--stream->buff_size;
	stream->last_op_type = READ_OPERATION;

	return (int)(unsigned char)stream->buffer[stream->index++];
}

int so_fputc(int c, SO_FILE *stream)
{
	/*
	 *	verific daca se incearca scrierea intr-un fisier
	 *	care a fost deschis doar pentru citire(r),
	 *	iar in caz afirmativ semnalez o eroare.
	 */
	off_t pos;
	int result, whence;
	long offset;
	int seek = 0;

	if (strcmp(stream->mode, RDONLY) == 0) {
		stream->err_flag = ERR;
		return SO_EOF;
	}

	/*
	 *	daca fisierul a fost deschis in mod append atunci cursorul va
	 *	fi pozitionat la sfarsitul fisierului
	 */
	if (strcmp(stream->mode, APPEND) == 0) {
		pos = lseek(stream->file_descriptor, 0, SEEK_END);
		if (pos < 0) {
			stream->err_flag = ERR;
			return SO_EOF;
		}

		stream->current_position = pos;
	}

	/*
	 *	verific daca ultima operatie a fost read,
	 *  caz in care aplic o operatie de so_fseek
	 */
	if (stream->last_op_type == READ_OPERATION) {
		offset = stream->current_position;
		whence = SEEK_SET;

		/*
		 *	daca fisierul a fost deschis in modul append + read,
		 *	atunci scrierea (daca este realizata dupa operatia
		 * de citire) trebuie facuta la sfarsitul fisierului.
		 */
		if (strcmp(stream->mode, APPEND_READ) == 0) {
			offset = 0;
			whence = SEEK_END;
		}

		seek = 1;
	}

	/*
	 *	daca fisierul a fost deschis in modul append + read,
	 *	atunci scrierea(daca este realizata inaintea operatiei de read)
	 *	trebuie facuta la sfarsitul fisierului.
	 */
	if (strcmp(stream->mode, APPEND_READ) == 0
		&& stream->last_op_type == NO_OPERATION) {
		offset = 0;
		whence = SEEK_END;
		seek = 1;
	}

	if (seek) {
		pos = so_fseek(stream, offset, whence);
		if (pos < 0) {
			stream->err_flag = ERR;
			return SO_EOF;
		}
	}

	/* bufferul nu este plin */
	if (stream->index < BUFF_SIZE) {
		++stream->current_position;
		stream->buffer[stream->index++] = (unsigned char)c;
		++stream->buff_size;
		stream->last_op_type = WRITE_OPERATION;

		return c;
	}

	/* bufferul este plin */
	result = so_fflush(stream);
	if (result) {
		stream->err_flag = ERR;
		return SO_EOF;
	}

	++stream->buff_size;
	++stream->current_position;
	stream->buffer[stream->index++] = (unsigned char)c;

	stream->last_op_type = WRITE_OPERATION;

	return c;
}

long so_ftell(SO_FILE *stream)
{
	if (stream->err_flag == ERR)
		return (long)SO_EOF;

	return stream->current_position;
}

int so_fileno(SO_FILE *stream)
{
	return stream->file_descriptor;
}

int so_feof(SO_FILE *stream)
{
	return stream->eof;
}

int so_ferror(SO_FILE *stream)
{
	return stream->err_flag;
}

size_t so_fread(void *ptr, size_t size, size_t nmemb, SO_FILE *stream)
{
	size_t items_read = 0;
	int res, mode;

	mode = strcmp(stream->mode, WDONLY) & strcmp(stream->mode, APPEND);
	if (!mode) {
		stream->err_flag = ERR;
		return 0;
	}

	if (so_feof(stream))
		return 0;

	for (size_t i = 0; i < nmemb; ++i) {
		for (size_t j = 0; j < size; ++j) {
			res = so_fgetc(stream);

			if (res < 0) {
				if (j > 0) {
					res = so_fseek(stream,
						(long)(-j), SEEK_CUR);
					if (res < 0)
						return 0;
				}

				return items_read;
			}

			((char *)ptr)[i * size + j] = (char)res;
		}

		++items_read;
	}

	return items_read;
}

size_t so_fwrite(const void *ptr, size_t size, size_t nmemb, SO_FILE *stream)
{
	size_t items_written = 0;
	int res;

	if (strcmp(stream->mode, RDONLY) == 0) {
		stream->err_flag = ERR;
		return 0;
	}

	for (size_t i = 0; i < nmemb; ++i) {
		for (size_t j = 0; j < size; ++j) {
			res = so_fputc((int)(unsigned char)
				(((char *)ptr)[i * size + j]), stream);

			if (res < 0) {
				res = so_fseek(stream,
					(long)((-i) * size + j), SEEK_CUR);

				return 0;
			}
		}

		++items_written;
	}

	return items_written;
}

SO_FILE *so_popen(const char *command, const char *type)
{
	pid_t pid;
	int res;
	int fds[2];
	int fd;
	SO_FILE *file;
	const char * const argv[] = {"sh", "-c", command, NULL};

	res = pipe(fds);
	if (res < 0)
		return NULL;

	pid = fork();

	if (pid < 0) {
		close(fds[0]);
		close(fds[1]);
		return NULL;
	}

	if (pid == 0) {
		if (strcmp(type, RDONLY) == 0) {
			close(fds[0]);
			dup2(fds[1], STDOUT_FILENO);
			close(fds[1]);
		} else {
			close(fds[1]);
			dup2(fds[0], STDIN_FILENO);
			close(fds[0]);
		}

		res = execvp("sh", (char *const *)argv);

		exit(res);
	}

	if (strcmp(type, RDONLY) == 0) {
		close(fds[1]);
		fd = fds[0];
	} else {
		close(fds[0]);
		fd = fds[1];
	}

	child_pid = pid;

	file = (SO_FILE *)malloc(sizeof(SO_FILE));
	if (!file) {
		close(fd);
		return NULL;
	}

	file->file_descriptor = fd;
	file->index = file->current_position = 0;
	file->buff_size = 0;
	file->last_op_type = NO_OPERATION;
	file->err_flag = NOT_ERR;
	file->eof = NO_EOF_;
	strcpy(file->mode, type);
	strcpy(file->buffer, "");

	return file;
}

int so_pclose(SO_FILE *stream)
{
	int res, status;

	if (!child_pid)
		return SO_EOF;

	res = so_fclose(stream);
	if (res < 0)
		return SO_EOF;

	res = waitpid(child_pid, &status, 0);

	if (res < 0)
		return SO_EOF;

	return status;
}
