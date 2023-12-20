tracant:
	gcc main.c -o TP_User_SEA

trace:
	gcc -no-pie -static toy_c_program.c -o toy_c_program

clean:
	rm TP_User_SEA
	rm toy_c_program

run_tracant:
	./TP_User_SEA puissance addition

run_trace:
	./toy_c_program	

full:
	make tracant
	make trace