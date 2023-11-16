user:
	gcc main.c -o TP_User_SEA

toy:
	gcc -no-pie toy_c_program/toy_c_program.c -o toy_c_program/toy_c_program

clean:
	rm TP_User_SEA
	rm toy_c_program/toy_c_program
	
full:
	make user
	make toy