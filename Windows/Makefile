CC = cl
LINK = link
CFLAGS = /c /MD

build: so_stdio.dll

so_stdio.dll: so_stdio.obj
	$(LINK) /nologo /dll /out:so_stdio.dll /implib:so_stdio.lib so_stdio.obj
	
so_stdio.obj: so_stdio.c
	$(CC) $(CFLAGS) so_stdio.c /Foso_stdio.obj

clean:
	del *.obj