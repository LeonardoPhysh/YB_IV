
kb_tst:
	arm-linux-gcc -I/home/leonardo/Workspace/YB-IV/include --static -Wall keyboard_tst.c uart.c /home/leonardo/Workspace/YB-IV/src/misc/common.c -o kb_tst -lpthread -lm
	#gcc -I/home/leonardo/Workspace/YB-IV/Tax-Code/include --static -Wall keyboard_tst.c uart.c /home/leonardo/Workspace/Tax_System/Tax-Code/src/misc/common.c -o kb_tst -lpthread -lm

card_tst:
	arm-linux-gcc -I/home/leonardo/Workspace/YB-IV/include --static -Wall card_tst.c uart.c /home/leonardo/Workspace/YB-IV/src/misc/common.c -o card_tst -lm
	#gcc -I/home/leonardo/Workspace/YB-IV/Tax-Code/include --static -Wall card_tst.c uart.c /home/leonardo/Workspace/Tax_System/Tax-Code/src/misc/common.c -o card_tst -lm

new_lcm_tst:
	arm-linux-gcc --static -I../include ../src/misc/common.c lcm_api.c -o new_lcm_tst -lm

add_user:
	#arm-linux-gcc -I../include --static add_user.c ../src/misc/file_core.c -o add_user
	gcc -I../include --static add_user.c ../src/misc/file_core.c -o add_user

add_mach:
	arm-linux-gcc -I../include --static add_machine.c ../src/misc/common.c ../src/misc/file_core.c -o add_mach -lm

add_cfg:
	arm-linux-gcc -I../include --static add_syscfg.c ../src/misc/common.c ../src/misc/file_core.c -o add_cfg -lm


print_tst:
	arm-linux-gcc -I../include --static print.c ../src/misc/common.c -o print_tst -lm
