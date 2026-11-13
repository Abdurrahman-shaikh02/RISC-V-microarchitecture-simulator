// Copied from collegereport/demo/asm/03_hazards.s (the documented demo program).
// EXPECT r1 = 10
// EXPECT r2 = 20
// EXPECT r3 = 30
// EXPECT r4 = 60
// EXPECT r5 = 60
// EXPECT r6 = 120
// EXPECT mem32[0x40000] = 60

// ============================================================
// 03_hazards.s  -  data hazards (RAW) and a load-use stall
//
// Every instruction needs the result of the one before it.
// Run it twice and compare pipevis.txt and the cycle count:
//   forwarding ON  : results are bypassed to EX, only the
//                    load-use pair costs one bubble
//   forwarding OFF : each dependent instruction waits in ID
//                    until the producer has written back
//
// Expected:
//   r1 = 0000000a (10)
//   r2 = 00000014 (20)
//   r3 = 0000001e (30)
//   r4 = 0000003c (60)
//   r5 = 0000003c (60, loaded back from memory)
//   r6 = 00000078 (120)
// ============================================================

        lui     r10, 0x40          // r10 = 0x40000 (data area)

        addi    r1, r0, 10         // r1 = 10
        add     r2, r1, r1         // RAW on r1      -> 20
        add     r3, r2, r1         // RAW on r2, r1  -> 30
        add     r4, r3, r3         // RAW on r3      -> 60

        sw      r4, 0(r10)         // store needs r4 (store data hazard)
        lw      r5, 0(r10)         // r5 = 60
        add     r6, r5, r5         // LOAD-USE: needs r5 straight after the load -> 120

        addi    r17, r0, 0
        ecall
