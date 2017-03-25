a: \
compile1
	./diskinfo $(ARG1)

b: \
compile2
	./disklist $(ARG1)

c: \
compile3
	./diskget $(ARG1) $(ARG2)

d: \
compile4
	./diskput $(ARG1) $(ARG2)

clean: 
	rm ANS1.PDF
	rm REMINDER.TXT
	rm ICEBERGS.TEX

cp: 
	cp disk.IMA disk2.IMA

compile1: \
diskinfo.c
	gcc  diskinfo.c -lm -o diskinfo -lhistory -lreadline

compile2: \
disklist.c	
	gcc disklist.c -o disklist -lhistory -lreadline

compile3: \
diskget.c
	gcc diskget.c -o diskget -lhistory -lreadline

compile4: \
diskput.c
	gcc diskput.c -lm -o diskput -lhistory -lreadline
