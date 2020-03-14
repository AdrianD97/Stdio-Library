build: libso_stdio.so

libso_stdio.so: so_stdio.o
	gcc -shared $^ -o libso_stdio.so

so_stdio.o: so_stdio.c
	gcc -fPIC -c $^ -o $@

clean:
	rm *.o libso_stdio.so