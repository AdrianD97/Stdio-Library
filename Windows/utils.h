#ifndef UTILS_H
#define UTILS_H

#define NO_OPERATION	(-1)
#define READ_OPERATION	0
#define WRITE_OPERATION	1

#define ERR	1
#define NOT_ERR	0

#define SIZE	3
#define BUFF_SIZE	4096

#define INVALID_MODE	(-1)
#define INVALID_ACTION	(-1)

#define RDONLY	"r"
#define RD_WR	"r+"
#define WDONLY	"w"
#define WD_RD	"w+"
#define APPEND	"a"
#define APPEND_READ	"a+"

#define EOF_	1
#define NO_EOF_	0

#define CMD_SIZE 500

#define INVALID_HANDLE	-1

struct _so_file {
	/*
	 *	handle-ul prin care este
	 *	identificata instanta de fisier deschisa.
	 */
	HANDLE handle;

	/*
	 * indexul caracterului curent, disponibil din buffer(pentru citire)
	 * pozitia disponibila pentru a putea adauga un caracter in buffer
	 */
	int index;

	/* pozitia curenta din fisier */
	DWORD current_position;

	/* tipul ultimei operatii realizate pe fisier (read or write)*/
	int last_op_type;

	/*
	 *	indica faptul daca a aparut sau nu o eroare
	 *	cand s-a realizat o operatie pe instanta de fisier
	 */
	int err_flag;

	/* dimensiunea curenta a bufferului(cati bytes mai am in buffer) */
	DWORD buff_size;

	/* indica faptul daca s-a ajuns la sfarsitul fisierului */
	int eof;

	/*
	 * va retine informatii despre procesul copil
	 * creat (doar pentru functia so_popen)
	 * pentru a putea mai tarziu sa asteptam ca acesta
	 * sa se termine(WaitForSingleObject are nevoie de
	 * handle-ul procesului pe care il asteptam sa-si
	 * incheie executia)
	 */
	PROCESS_INFORMATION proc_child_info;

	/* modul in care a fost deschis fisierul */
	char mode[SIZE];

	/*
	 *	bufferul asociat structurii pe care se realizeaza
	 *	operatiile de citire, respectiv de scriere
	 */
	char buffer[BUFF_SIZE];
};

#endif /* UTILS_H */
