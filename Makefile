all: hostA_C.out hostB_S.out

hostA_C.out: hostA_C.c
	gcc hostA_C.c -o hostA_C.out -g

hostB_S.out: hostB_S.c
	gcc hostB_S.c -o hostB_S.out -g

clean: 
	rm -f *.out