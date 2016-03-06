mov r1, #100
mov r2, #0

loop:
cmp r2, #8
str r2, [r1, r2, LSL#2]
add r2, r2, #1
bne loop

mov r1, #100
mov r3, #0
mov r2, #0
loop2:
cmp r2, #8
ldr r0, [r1, r2, LSL#2]
add r3, r3, r0
add r2, r2, #1
bne loop2

swi 0x11