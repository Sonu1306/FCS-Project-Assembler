.data
arr:
    .word 10, 20, 30
    .asciz "hello"

.text
main:
    add x1, x2, x3
    addi x5, x1, 10
    beq x5, x2, skip
    lw x6, 0(x7)
    jal x0, done
skip:
    sub x8, x1, x2
    sw x8, 4(x9)
done:
    nop
