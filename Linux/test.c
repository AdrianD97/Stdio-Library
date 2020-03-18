#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <fcntl.h>

#include "so_stdio.h"
#include "utils.h"

#define INVALID_FILE_DESCRIPTOR	-1

void test_SO_FILE_structure()
{
	printf("test_SO_FILE_structure()\n");
	SO_FILE *f;

	f = (SO_FILE *)malloc(sizeof(SO_FILE));

	if (!f) {
		fprintf(stderr, "malloc() failed when alloc memory for a SO_FILE structure.\n");
		return;
	}

	f->file_descriptor = INVALID_FILE_DESCRIPTOR;
	f->index = 0;
	f->buff_size = 0;
	f->current_position = 0;
	f->last_op_type = NO_OPERATION;
	f->err_flag = NOT_ERR;
	strcpy(f->mode, "r+");
	strcpy(f->buffer, "Ana are mere multe.");

	assert(f->file_descriptor == INVALID_FILE_DESCRIPTOR);
	assert(f->index == 0);
	assert(f->buff_size == 0);
	assert(f->current_position == 0);
	assert(f->last_op_type == NO_OPERATION);
	assert(f->err_flag == NOT_ERR);
	assert(strcmp(f->mode, "r+") == 0);
	assert(strcmp(f->buffer, "Ana are mere multe.") == 0);

	free(f);
}

void test_so_open_and_so_close_functions()
{
	printf("test_so_open_and_so_close_functions()\n");
	SO_FILE *f;
	int res;

	res = so_fclose(NULL);
	assert(res < 0);

	f = so_fopen("./file.txt", "a+");
	assert(f != NULL);
	res = so_fclose(f);
	assert(res == 0);
}

void test_so_fgetc()
{
	printf("test_so_fgetc()\n");
	SO_FILE *f;
	int res;

	f = so_fopen("./file.txt", "a+");
	assert(f != NULL);

	for (int i = 0; i < 4096; ++i) {
		res = so_fgetc(f);
		printf("%c", res);
	}

	printf("\n\n\n");
	printf("%d\n", f->index);
	printf("%ld\n", f->current_position);
	printf("%d\n", f->err_flag);
	printf("%d\n", f->buff_size);
	printf("%s\n", f->mode);
	printf("\n");


	for (int i = 0; i < 4096; ++i) {
		res = so_fgetc(f);
		printf("%c", res);
	}

	printf("\n\n\n");
	printf("%d\n", f->index);
	printf("%ld\n", f->current_position);
	printf("%d\n", f->err_flag);
	printf("%d\n", f->buff_size);
	printf("%s\n", f->mode);
	printf("\n");

	for (int i = 0; i < 3840; ++i) {
		res = so_fgetc(f);
		printf("%c", res);
	}

	printf("\n\n\n");
	printf("%d\n", f->index);
	printf("%ld\n", f->current_position);
	printf("%d\n", f->err_flag);
	printf("%d\n", f->buff_size);
	printf("%s\n", f->mode);
	printf("\n");

	res = so_fgetc(f);
	printf("%d", res);

	printf("\n\n\n");
	printf("%d\n", f->index);
	printf("%ld\n", f->current_position);
	printf("%d\n", f->err_flag);
	printf("%d\n", f->buff_size);
	printf("%s\n", f->mode);
	printf("\n");

	res = so_fgetc(f);
	printf("%d", res);

	printf("\n\n\n");
	printf("%d\n", f->index);
	printf("%ld\n", f->current_position);
	printf("%d\n", f->err_flag);
	printf("%d\n", f->buff_size);
	printf("%s\n", f->mode);
	printf("\n");

	res = so_fclose(f);
	assert(res == 0);	
}

void test_so_fputc()
{
	printf("test_so_fputc()\n");
	SO_FILE *f;
	int res;

	f = so_fopen("./file.txt", "a+");
	assert(f != NULL);

	res = so_fputc((int)'A', f);
	printf("%d\n", f->index);
	printf("%ld\n", f->current_position);
	printf("%d\n", f->err_flag);
	printf("%d\n", f->buff_size);
	printf("%s\n", f->mode);
	printf("\n\n\n");

	res = so_fputc((int)'d', f);
	printf("%d\n", f->index);
	printf("%ld\n", f->current_position);
	printf("%d\n", f->err_flag);
	printf("%d\n", f->buff_size);
	printf("%s\n", f->mode);
	printf("\n\n\n");

	res = so_fputc((int)'r', f);
	printf("%d\n", f->index);
	printf("%ld\n", f->current_position);
	printf("%d\n", f->err_flag);
	printf("%d\n", f->buff_size);
	printf("%s\n", f->mode);
	printf("\n\n\n");

	res = so_fputc((int)'i', f);
	printf("%d\n", f->index);
	printf("%ld\n", f->current_position);
	printf("%d\n", f->err_flag);
	printf("%d\n", f->buff_size);
	printf("%s\n", f->mode);
	printf("\n\n\n");

	res = so_fputc((int)'a', f);
	printf("%d\n", f->index);
	printf("%ld\n", f->current_position);
	printf("%d\n", f->err_flag);
	printf("%d\n", f->buff_size);
	printf("%s\n", f->mode);
	printf("\n\n\n");

	res = so_fputc((int)'n', f);
	printf("%d\n", f->index);
	printf("%ld\n", f->current_position);
	printf("%d\n", f->err_flag);
	printf("%d\n", f->buff_size);
	printf("%s\n", f->mode);
	printf("\n\n\n");

	for (int i = 0; i < 4090; ++i) {
		res = so_fputc((int)'z', f);
	}

	printf("%d\n", f->index);
	printf("%ld\n", f->current_position);
	printf("%d\n", f->err_flag);
	printf("%d\n", f->buff_size);
	printf("%s\n", f->mode);
	printf("\n\n\n");

	res = so_fputc((int)' ', f);
	res = so_fputc((int)'?', f);
	res = so_fputc((int)'\n', f);
	res = so_fputc(970, f);
	res = so_fputc(9700, f);
	printf("%d\n", f->index);
	printf("%ld\n", f->current_position);
	printf("%d\n", f->err_flag);
	printf("%d\n", f->buff_size);
	printf("%s\n", f->mode);
	printf("\n\n\n");

	res = so_fclose(f);
	assert(res == 0);
}

void test_so_fgetc_and_so_fputc()
{
	printf("test_so_fgetc_and_so_fputc()\n");
	SO_FILE *f;
	int res;
	
	f = so_fopen("./file.txt", "a+");
	assert(f != NULL);

	// res = so_fgetc(f);
	// printf("|%c|\n", (char)res);

	// res = so_fgetc(f);
	// printf("|%c|\n", (char)res);

	// res = so_fgetc(f);
	// printf("|%c|\n", (char)res);

	// res = so_fgetc(f);
	// printf("|%c|\n", (char)res);

	// res = so_fgetc(f);
	// printf("|%c|\n", (char)res);

	// printf("%d\n", f->index);
	// printf("%ld\n", f->current_position);
	// printf("%d\n", f->err_flag);
	// printf("%d\n", f->buff_size);
	// printf("%s\n", f->mode);
	// printf("\n\n\n");

	res = so_fputc((int)'N', f);
	res = so_fputc((int)'M', f);
	res = so_fputc((int)'L', f);
	res = so_fputc((int)'U', f);
	printf("%d\n", f->index);
	printf("%ld\n", f->current_position);
	printf("%d\n", f->err_flag);
	printf("%d\n", f->buff_size);
	printf("%s\n", f->mode);
	printf("\n\n\n");

	for (int i = 0; i < 5; ++i) {
		res = so_fgetc(f);
		printf("|%d|\n", res);
	}

	printf("%d\n", f->index);
	printf("%ld\n", f->current_position);
	printf("%d\n", f->err_flag);
	printf("%d\n", f->buff_size);
	printf("%s\n", f->mode);
	printf("\n\n\n");

	// res = so_fputc((int)'W', f);
	// res = so_fputc((int)'X', f);
	// res = so_fputc((int)'Y', f);
	// res = so_fputc((int)'P', f);

	// printf("%d\n", f->index);
	// printf("%ld\n", f->current_position);
	// printf("%d\n", f->err_flag);
	// printf("%d\n", f->buff_size);
	// printf("%s\n", f->mode);
	// printf("\n\n\n");

	res = so_fclose(f);
	assert(res == 0);
}

void test_so_fseek()
{
	printf("test_so_fseek()\n");
	SO_FILE *f;
	int res;
	
	f = so_fopen("./file.txt", "r+");
	assert(f != NULL);

	res = so_fgetc(f);
	printf("|%c|\n", (char)res);

	res = so_fgetc(f);
	printf("|%c|\n", (char)res);

	res = so_fgetc(f);
	printf("|%c|\n", (char)res);

	res = so_fgetc(f);
	printf("|%c|\n", (char)res);

	res = so_fgetc(f);
	printf("|%c|\n", (char)res);

	printf("%d\n", f->index);
	printf("%ld\n", f->current_position);
	printf("%d\n", f->err_flag);
	printf("%d\n", f->buff_size);
	printf("%s\n", f->mode);
	printf("\n\n\n");

	res = so_fseek(f, (long)(-2), SEEK_CUR);
	res = so_fgetc(f);
	printf("|%c|\n", (char)res);

	res = so_fgetc(f);
	printf("|%c|\n", (char)res);

	printf("%d\n", f->index);
	printf("%ld\n", f->current_position);
	printf("%d\n", f->err_flag);
	printf("%d\n", f->buff_size);
	printf("%s\n", f->mode);
	printf("\n\n\n");

	res = so_fputc((int)'J', f);
	res = so_fputc((int)'K', f);
	res = so_fputc((int)'L', f);

	printf("%d\n", f->index);
	printf("%ld\n", f->current_position);
	printf("%d\n", f->err_flag);
	printf("%d\n", f->buff_size);
	printf("%s\n", f->mode);
	printf("\n\n\n");

	res = so_fseek(f, (long)(-3), SEEK_CUR);
	res = so_fgetc(f);
	printf("|%c|\n", (char)res);

	res = so_fgetc(f);
	printf("|%c|\n", (char)res);

	res = so_fgetc(f);
	printf("|%c|\n", (char)res);

	printf("%d\n", f->index);
	printf("%ld\n", f->current_position);
	printf("%d\n", f->err_flag);
	printf("%d\n", f->buff_size);
	printf("%s\n", f->mode);
	printf("\n\n\n");

	res = so_fseek(f, (long)(-5), SEEK_END);
	res = so_fputc((int)'J', f);
	res = so_fputc((int)'K', f);
	printf("%d\n", f->index);
	printf("%ld\n", f->current_position);
	printf("%d\n", f->err_flag);
	printf("%d\n", f->buff_size);
	printf("%s\n", f->mode);
	printf("\n\n\n");

	res = so_fseek(f, (long)(15), SEEK_SET);
	res = so_fputc((int)'J', f);
	res = so_fputc((int)'K', f);
	printf("%d\n", f->index);
	printf("%ld\n", f->current_position);
	printf("%d\n", f->err_flag);
	printf("%d\n", f->buff_size);
	printf("%s\n", f->mode);
	printf("\n\n\n");

	res = so_fseek(f, (long)(-2), SEEK_CUR);
	res = so_fgetc(f);
	printf("|%c|\n", (char)res);

	res = so_fgetc(f);
	printf("|%c|\n", (char)res);

	printf("%d\n", f->index);
	printf("%ld\n", f->current_position);
	printf("%d\n", f->err_flag);
	printf("%d\n", f->buff_size);
	printf("%s\n", f->mode);
	printf("\n\n\n");

	res = so_fseek(f, (long)0, SEEK_END);
	res = so_fgetc(f);
	printf("|%d|\n", (char)res);

	res = so_fclose(f);
	assert(res == 0);
}

void test_so_ftell()
{
	printf("test_so_ftell()\n");
	SO_FILE *f, *ff;
	int res;
	
	f = so_fopen("./file.txt", "r+");
	assert(f != NULL);

	res = so_fgetc(f);
	printf("|%c|\n", (char)res);

	res = so_fgetc(f);
	printf("|%c|\n", (char)res);

	res = so_fgetc(f);
	printf("|%c|\n", (char)res);

	res = so_fgetc(f);
	printf("|%c|\n", (char)res);

	res = so_fgetc(f);
	printf("|%c|\n", (char)res);

	printf("%ld\n", so_ftell(f));

	res = so_fseek(f, (long)(-2), SEEK_CUR);
	printf("%ld\n", so_ftell(f));
	res = so_fgetc(f);
	printf("|%c|\n", (char)res);

	res = so_fgetc(f);
	printf("|%c|\n", (char)res);
	printf("%ld\n", so_ftell(f));

	res = so_fputc((int)'J', f);
	res = so_fputc((int)'K', f);
	res = so_fputc((int)'L', f);

	printf("%ld\n", so_ftell(f));

	res = so_fseek(f, (long)0, SEEK_END);
	printf("%ld\n", so_ftell(f));


	res = so_fgetc(f);
	printf("%d\n", res);
	printf("%d\n", f->err_flag);

	printf("%ld\n", so_ftell(f));

	printf("fd = %d\n", so_fileno(f));

	ff = so_fopen("file.txt", "a+");
	assert(ff != NULL);

	printf("fd = %d\n", so_fileno(ff));

	res = so_fclose(ff);
	assert(res == 0);

	res = so_fclose(f);
	assert(res == 0);
}

void test_so_feof()
{
	printf("test_so_feof()\n");
	SO_FILE *f;
	int res;
	
	f = so_fopen("./file.txt", "r+");
	assert(f != NULL);

	for (int i = 0; i < 14; ++i) {
		res = so_fgetc(f);
		printf("|%c|\n", (char)res);

		// res = so_feof(f);
		// printf("res = %d\n", res);
	}

	res = so_fgetc(f);
	printf("Here |%c|\n", (char)res);

	res = so_feof(f);
	printf("res = %d\n", res);

	printf("current_position = %ld\n", f->current_position);

	// res = so_fseek(f, (long)0, SEEK_END);

	// printf("%d\n", so_feof(f));

	// res = so_fseek(f, (long)(-4), SEEK_END);

	// printf("%d\n", so_feof(f));

	// res = so_fseek(f, (long)(0), SEEK_SET);
	// printf("%d\n", so_feof(f));

	// res = so_fgetc(f);
	// printf("|%c|\n", (char)res);

	// res = so_fgetc(f);
	// printf("|%c|\n", (char)res);

	// res = so_fgetc(f);
	// printf("|%c|\n", (char)res);

	// res = so_fgetc(f);
	// printf("|%c|\n", (char)res);

	// res = so_fgetc(f);
	// printf("|%c|\n", (char)res);

	// printf("%ld\n", so_ftell(f));

	// printf("%d\n", so_feof(f));

	// res = so_fseek(f, (long)(-4), SEEK_END);
	// res = so_fgetc(f);
	// printf("|%c|\n", (char)res);

	// res = so_fgetc(f);
	// printf("|%c|\n", (char)res);

	// res = so_fgetc(f);
	// printf("|%c|\n", (char)res);

	// res = so_fgetc(f);
	// printf("|%c|\n", (char)res);

	// printf("%d\n", so_feof(f));

	// res = so_fseek(f, (long)(-3), SEEK_END);
	// printf("%ld\n", so_ftell(f));

	// res = so_fgetc(f);
	// printf("|%c|\n", (char)res);

	// res = so_fgetc(f);
	// printf("|%c|\n", (char)res);

	// printf("%ld\n", so_ftell(f));

	// printf("%d\n", so_feof(f));

	// res = so_fgetc(f);
	// printf("|%c|\n", (char)res);

	// printf("%ld\n", so_ftell(f));

	// printf("%d\n", so_feof(f));

	// printf("%d\n", so_ferror(f));

	// res = so_fgetc(f);
	// printf("|%d|\n", (char)res);

	// printf("%d\n", so_ferror(f));

	res = so_fclose(f);
	assert(res == 0);
}

void test_so_fread()
{
	printf("test_so_fread()\n");
	SO_FILE *f;
	int res;

	char buff[200];
	
	f = so_fopen("./file.txt", "r+");
	assert(f != NULL);

	res = so_fread((void *)buff, 4, 2, f);
	printf("%d\n", res);

	buff[8] = '\0';
	printf("buff = |%s|\n", buff);

	res = so_fread((void *)buff, 4, 3, f);
	printf("%d\n", res);
	buff[12] = '\0';
	printf("buff = |%s|\n", buff);

	res = so_fseek(f, (long)(-7), SEEK_END);
	printf("%ld\n", so_ftell(f));

	res = so_fread((void *)buff, 4, 2, f);
	printf("%d\n", res);
	buff[4 * res] = '\0';
	printf("buff = |%s|\n", buff);

	res = so_fgetc(f);
	printf("%c\n", res);

	res = so_fgetc(f);
	printf("%c\n", res);

	res = so_fgetc(f);
	printf("%c\n", res);

	res = so_fseek(f, (long)(-3), SEEK_END);
	printf("%ld\n", so_ftell(f));

	res = so_fread((void *)buff, 4, 1, f);
	printf("%d\n", res);
	buff[4 * res] = '\0';
	printf("buff = |%s|\n", buff);

	printf("%ld\n", so_ftell(f));

	res = so_fclose(f);
	assert(res == 0);
}

void test_so_fwrite()
{
	printf("test_so_fwrite()\n");
	SO_FILE *f;
	int res;

	char buff[200];
	
	strcpy(buff, "Adrian");

	f = so_fopen("./file.txt", "r+");
	assert(f != NULL);

	res = so_fwrite((const char*)buff, 3, 2, f);
	printf("%d\n", res);

	res = so_fseek(f, (long)(-3), SEEK_END);
	printf("%ld\n", so_ftell(f));

	res = so_fwrite((const char*)buff, 3, 2, f);
	printf("%d\n", res);

	printf("%ld\n", so_ftell(f));

	res = so_fseek(f, (long)7, SEEK_END);
	printf("%ld\n", so_ftell(f));

	res = so_fwrite((const char*)buff, 3, 2, f);
	printf("%d\n", res);

	res = so_fclose(f);
	assert(res == 0);
}


void test_a_mode()
{
	printf("test_a_mode()\n");
	SO_FILE *f;
	int res;

	char buff[200];
	
	strcpy(buff, " Adrian\n");

	f = so_fopen("./file.txt", "a");
	assert(f != NULL);

	printf("%ld\n", so_ftell(f));

	res = so_fwrite((const char*)buff, 4, 2, f);
	printf("%d\n", res);

	printf("%ld\n", so_ftell(f));

	res = so_fwrite((const char*)buff, 4, 2, f);
	printf("%d\n", res);

	printf("%ld\n", so_ftell(f));

	res = so_fseek(f, (long)8, SEEK_SET);
	printf("%d\n", res);
	printf("%ld\n", so_ftell(f));

	res = so_fwrite((const char*)buff, 4, 2, f);
	printf("%d\n", res);

	printf("%ld\n", so_ftell(f));

	res = so_fclose(f);
	assert(res == 0);
}

void test_so_popen()
{
	printf("test_so_popen()\n");
	SO_FILE *f;

	f = so_popen("ls -la", "r");
	assert(f != NULL);

	for (int i = 0; i < 90; ++i)
		printf("%c\n", so_fgetc(f));

	printf("so_pclose = %d\n", so_pclose(f));
}

int main(int argc, char const *argv[])
{

	// test_SO_FILE_structure();

	// test_so_open_and_so_close_functions();

	// test_so_fgetc();

	// test_so_fputc();

	// test_so_fgetc_and_so_fputc();

	// test_so_fseek();

	// test_so_ftell();

	// test_so_feof();

	// test_so_fread();

	// test_so_fwrite();

	// test_a_mode();

	test_so_popen();

	return 0;
}