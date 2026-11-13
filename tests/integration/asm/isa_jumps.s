// jal / jalr: links, targets, call and return, backward jumps
// EXPECT r1 = 4
// EXPECT r4 = 42
// EXPECT r5 = 1
// EXPECT r6 = 0
// EXPECT r7 = 16
// EXPECT r8 = 24
// EXPECT r9 = 0
// EXPECT r10 = 7
// EXPECT r11 = 0
// EXPECT r12 = 3

        jal     r1, func            // 0: call, r1 = 4
        addi    r5, r0, 1           // 4: runs after the return
        jal     r0, skip            // 8
        addi    r6, r0, 99          // 12: skipped
skip:
        auipc   r7, 0               // 16: r7 = 16
        jalr    r8, 16(r7)          // 20: jump to 32, r8 = 24
        addi    r9, r0, 99          // 24: skipped
        addi    r9, r0, 99          // 28: skipped
target:
        addi    r10, r0, 7          // 32
        addi    r11, r0, 3
        addi    r12, r0, 0
back:
        addi    r11, r11, -1
        addi    r12, r12, 1
        beq     r11, r0, out
        jal     r0, back
out:
        addi    r17, r0, 0
        ecall

func:
        addi    r4, r0, 42
        jalr    r0, 0(r1)
