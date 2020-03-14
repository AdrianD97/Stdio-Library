#ifndef SO_STDIO_H
#define SO_STDIO_H

#if defined(__linux__)
#define FUNC_DECL_PREFIX
#elif defined(_WIN32)
#include <Windows.h>

#ifdef DLL_EXPORTS
#define FUNC_DECL_PREFIX __declspec(dllexport)
#else
#define FUNC_DECL_PREFIX __declspec(dllimport)
#endif

#else
#error "Unknown platform"
#endif

#include <stdlib.h>

#define SEEK_SET	0	/* Seek from beginning of file.  */
#define SEEK_CUR	1	/* Seek from current position.  */
#define SEEK_END	2	/* Seek from end of file.  */

#define SO_EOF (-1)

#define NO_OPERATION	(-1)
#define READ_OPERATION	0
#define WRITE_OPERATION	1

#define INVALID_FILE_DESCRIPTOR	(-1)

#define ERR	1
#define NOT_ERR	0

#define SIZE	3
#define BUFF_SIZE	4096

#define INVALID_MODE	(-1)

#define RDONLY	"r"
#define RD_WR	"r+"
#define WDONLY	"w"
#define WD_RD	"w+"
#define APPEND	"a"
#define APPEND_READ	"a+"

#define EOF_	1
#define NO_EOF_	0

struct _so_file {
	/*
	 *	file descriptorul prin care este
	 *	identificata instanta de fisier deschisa.
	 */
	int file_descriptor;

	/*
	 * indexul caracterului curent, disponibil din buffer(pentru citire)
	 * pozitia disponibila pentru a putea adauga un caracter in buffer
	 */
	int index;

	/* pozitia curenta din fisier */
	long current_position;

	/* tipul ultimei operatii realizate pe fisier (read or write)*/
	int last_op_type;

	/*
	 *	indica faptul daca a aparut sau nu o eroare
	 *	cand s-a realizat o operatie pe instanta de fisier
	 */
	int err_flag;

	/* dimensiunea curenta a bufferului(cati bytes mai am in buffer) */
	int buff_size;

	/* indica faptul daca s-a ajuns la sfarsitul fisierului */
	int eof;

	/* modul in care a fost deschis fisierul */
	char mode[SIZE];

	/*
	 *	bufferul asociat structurii pe care se realizeaza
	 *	operatiile de citire, respectiv de scriere
	 */
	char buffer[BUFF_SIZE];
};

typedef struct _so_file SO_FILE;

FUNC_DECL_PREFIX SO_FILE *so_fopen(const char *pathname, const char *mode);
FUNC_DECL_PREFIX int so_fclose(SO_FILE *stream);

#if defined(__linux__)
FUNC_DECL_PREFIX int so_fileno(SO_FILE *stream);
#elif defined(_WIN32)
FUNC_DECL_PREFIX HANDLE so_fileno(SO_FILE *stream);
#else
#error "Unknown platform"
#endif


FUNC_DECL_PREFIX int so_fflush(SO_FILE *stream);

FUNC_DECL_PREFIX int so_fseek(SO_FILE *stream, long offset, int whence);
FUNC_DECL_PREFIX long so_ftell(SO_FILE *stream);

FUNC_DECL_PREFIX
size_t so_fread(void *ptr, size_t size, size_t nmemb, SO_FILE *stream);

FUNC_DECL_PREFIX
size_t so_fwrite(const void *ptr, size_t size, size_t nmemb, SO_FILE *stream);

FUNC_DECL_PREFIX int so_fgetc(SO_FILE *stream);
FUNC_DECL_PREFIX int so_fputc(int c, SO_FILE *stream);

FUNC_DECL_PREFIX int so_feof(SO_FILE *stream);
FUNC_DECL_PREFIX int so_ferror(SO_FILE *stream);

FUNC_DECL_PREFIX SO_FILE *so_popen(const char *command, const char *type);
FUNC_DECL_PREFIX int so_pclose(SO_FILE *stream);

#endif /* SO_STDIO_H */
