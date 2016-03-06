mov r1, #100
mov r0, #0
mov r4, #1
mov r2, #0

loop:
cmp r2, #5
str r0, [r1, r2, LSL#2]
mov r5, r4
add r4, r4, r0
mov r0, r5
add r2, r2, #1
bne loop

mov r1, #100
mov r2, #0
loop2:
cmp r2, #5
ldr r0, [r1, r2, LSL#2]
add r2, r2, #1
bne loop2 

swi 0x11