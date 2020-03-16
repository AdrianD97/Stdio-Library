#include <string.h>
#include <windows.h>

#include "so_stdio.h"
#include "utils.h"

/*
 * va retine informatii despre procesul copil creat
 * pentru a putea mai tarziu sa asteptam ca acesta
 * sa se termine(WaitForSingleObject are nevoie de
 * handle-ul procesului pe care il asteptam sa-si
 * incheie executia)
 */
PROCESS_INFORMATION proc_child_info;

/*
 * intoarce modul in care
 * se doreste sa se deschida fisierul.
 */
DWORD get_access_mode(const char *mode)
{
	int acc_mode;

	if (strcmp(mode, RDONLY) == 0)
		return GENERIC_READ;

	acc_mode = strcmp(mode, WDONLY) & strcmp(mode, APPEND);
	if (!acc_mode)
		return GENERIC_WRITE;

	acc_mode = strcmp(mode, RD_WR) & strcmp(mode, WD_RD)
		& strcmp(mode, APPEND_READ);
	if (!acc_mode)
		return GENERIC_READ | GENERIC_WRITE;

	return INVALID_MODE;
}

/*
 * intoarce actiunea care se doreste a fi facuta pe
 * fisier daca acesta exista sau nu exista
 */
DWORD get_action(const char *mode)
{
	int action;

	action = strcmp(mode, RDONLY) & strcmp(mode, RD_WR);
	if (!action)
		return OPEN_EXISTING;

	action = strcmp(mode, WDONLY) & strcmp(mode, WD_RD);
	if (!action)
		return CREATE_ALWAYS;

	action = strcmp(mode, APPEND) & strcmp(mode, APPEND_READ);
	if (!action)
		return OPEN_ALWAYS;

	return INVALID_ACTION;
}

int so_fflush(SO_FILE *stream)
{
	DWORD bytes_written;
	BOOL ret;

	if (stream->last_op_type != WRITE_OPERATION)
		return SO_EOF;

	stream->index = 0;
	while (stream->buff_size > 0) {
		ret = WriteFile(
			stream->handle,
		    &stream->buffer[stream->index],
		    stream->buff_size,
		    &bytes_written,
		    NULL
		);

		if (ret == FALSE) {
			stream->err_flag = ERR;
			return SO_EOF;
		}

		stream->index += (int)bytes_written;
		stream->buff_size -= bytes_written;
	}

	stream->index = 0;

	return 0;
}

int so_fseek(SO_FILE *stream, long offset, int whence)
{
	int res;
	DWORD pos;

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

	pos = SetFilePointer(stream->handle, offset, NULL, (DWORD)whence);
	if (pos == INVALID_SET_FILE_POINTER) {
		stream->err_flag = ERR;
		return SO_EOF;
	}

	stream->current_position = pos;

	return 0;
}

SO_FILE *so_fopen(const char *pathname, const char *mode)
{
	SO_FILE *file;
	DWORD acc_mode, action;
	HANDLE handle;

	acc_mode = get_access_mode(mode);
	if (acc_mode == INVALID_MODE)
		return NULL;

	action = get_action(mode);
	if (acc_mode == INVALID_ACTION)
		return NULL;

	handle = CreateFile(
		 (LPCSTR)pathname,
		 acc_mode,
		 FILE_SHARE_READ | FILE_SHARE_WRITE,
		 NULL,
		 action,
		 FILE_ATTRIBUTE_NORMAL,
		 NULL
	);

	if (handle == INVALID_HANDLE_VALUE)
		return NULL;

	file = (SO_FILE *)malloc(sizeof(SO_FILE));
	if (!file)
		return NULL;

	file->handle = handle;
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
	int result;
	BOOL ret;

	if (!stream)
		return SO_EOF;

	result = 0;

	if (stream->last_op_type == WRITE_OPERATION)
		result = so_fflush(stream);

	ret = CloseHandle(stream->handle);
	if (ret == FALSE)
		result = SO_EOF;

	free((void *)stream);

	return result;
}

int so_fgetc(SO_FILE *stream)
{
	int mode;
	DWORD bytes_read;
	int result;
	BOOL ret;
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
	ret = ReadFile(
		stream->handle,
	    &stream->buffer[stream->index],
		size,
	    &bytes_read,
	    NULL
	);

	if (ret == FALSE) {
		stream->eof = EOF_;
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
	DWORD pos;
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
		pos = SetFilePointer(stream->handle, 0, NULL, (DWORD)SEEK_END);
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

HANDLE so_fileno(SO_FILE *stream)
{
	return stream->handle;
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
	size_t i, j;

	mode = strcmp(stream->mode, WDONLY) & strcmp(stream->mode, APPEND);
	if (!mode) {
		stream->err_flag = ERR;
		return 0;
	}

	if (so_feof(stream))
		return 0;

	for (i = 0; i < nmemb; ++i) {
		for (j = 0; j < size; ++j) {
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
	size_t i, j;

	if (strcmp(stream->mode, RDONLY) == 0) {
		stream->err_flag = ERR;
		return 0;
	}

	for (i = 0; i < nmemb; ++i) {
		for (j = 0; j < size; ++j) {
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

/*
 * redirecteaza intrarea standard sau iesirea standard a procesului copil
 * in functie de valoarea lui opt.
 */
void redirect_handle(STARTUPINFO *psi, HANDLE h_file, int opt)
{
	if (h_file == INVALID_HANDLE_VALUE)
		return;

	psi->hStdInput = GetStdHandle(STD_INPUT_HANDLE);
	psi->hStdOutput = GetStdHandle(STD_OUTPUT_HANDLE);
	psi->hStdError = GetStdHandle(STD_ERROR_HANDLE);

	psi->dwFlags |= STARTF_USESTDHANDLES;

	if (opt == STD_INPUT_HANDLE)
		psi->hStdInput = h_file;
	else if (opt == STD_OUTPUT_HANDLE)
		psi->hStdOutput = h_file;
}

SO_FILE *so_popen(const char *command, const char *type)
{
	SECURITY_ATTRIBUTES sa;
	HANDLE h_read, h_write;
	BOOL ret;
	SO_FILE *file;
	STARTUPINFO si;
	PROCESS_INFORMATION pi;
	HANDLE handle_p, handle_c;

	char cmd[CMD_SIZE];

	strcpy(cmd, "cmd /C ");
	strcat(cmd, command);

	ZeroMemory(&sa, sizeof(sa));
	sa.nLength = sizeof(SECURITY_ATTRIBUTES);
	sa.bInheritHandle = TRUE;

	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);
	ZeroMemory(&pi, sizeof(pi));

	ret = CreatePipe(
		&h_read,
		&h_write,
		&sa,
		0
	);

	if (ret == FALSE)
		return NULL;

	if (strcmp(type, RDONLY) == 0) {
		handle_c = h_write;
		handle_p = h_read;
		redirect_handle(&si, h_write, STD_OUTPUT_HANDLE);
	} else if (strcmp(type, WDONLY) == 0) {
		handle_c = h_read;
		handle_p = h_write;
		redirect_handle(&si, h_read, STD_INPUT_HANDLE);
	}

	/* nu permitem copilului sa mostenesaca celalat capat al pipe-ului de care nu are nevoie */
	SetHandleInformation(handle_p, HANDLE_FLAG_INHERIT, 0);

	ret = CreateProcess(NULL, (LPSTR)cmd, NULL, NULL,
		TRUE, 0, NULL, NULL, &si, &pi);

	if (ret == FALSE) {
		CloseHandle(h_read);
		CloseHandle(h_write);
		return NULL;
	}

	CloseHandle(handle_c);
	proc_child_info = pi;

	file = (SO_FILE *)malloc(sizeof(SO_FILE));
	if (!file) {
		CloseHandle(handle_p);
		return NULL;
	}

	file->handle = handle_p;
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
	int res;
	DWORD ret;
	BOOL status;

	if (!proc_child_info.hProcess || !proc_child_info.hThread)
		return SO_EOF;

	res = so_fclose(stream);
	if (res < 0)
		return SO_EOF;

	ret = WaitForSingleObject(proc_child_info.hProcess, INFINITE);

	if (ret == WAIT_FAILED)
		return SO_EOF;

	status = GetExitCodeProcess(proc_child_info.hProcess, &ret);

	CloseHandle(proc_child_info.hProcess);
	CloseHandle(proc_child_info.hThread);

	if (ret == STILL_ACTIVE)
		return SO_EOF;

	return (int)ret;
}
