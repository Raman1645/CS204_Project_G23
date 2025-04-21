.data
n: .asciiz "abcdef"
b : .word 100 200
c: .half 110 200
d: .dword 0x100 0xAB
a: .byte 45
arr : .word 1 2 3 4 5 6 7 8 9
.text
add x1 x1 x2
and x2 x2 x22
or x3 x3 x23
sll x4 x4 x1
slt x5 x5 x24
sra x6 x6 x25
srl x7 x7 x8
sub x8 x8 x9
xor x10 x10 x11
mul x11 x11 x12
div x12 x12 x14
rem x13 x13 x14
addi x15 x15 -10
andi x16 x16 3
ori x17 x17 12
lb x18 2(x19)
ld x20 4(x21)
lh x22 6(x23)
lw x24 8(x25)
label9:
jalr x26 x27 10
sb x28 14(x29)
sw x30 -16(x31)
sd x12 18(x13)
sh x14 20(x15)
beq x6 x7 label1
label1:
bne x8 x9 label2
label2:
bge x4 x5 label3
label3:
blt x22 x23 label4
label4:
auipc x24 0x800
lui x25 0x1234
jal x25 label9
