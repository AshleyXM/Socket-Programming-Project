all: serverM serverA serverB client

serverM:
	gcc serverM.c -o serverM

serverA:
	gcc serverA.c -o serverA

serverB:
	gcc serverB.c -o serverB

client:
	gcc client.c -o client

clean:
	rm -f serverM serverA serverB client

