// no halt: executes the zero filled text segment as bubbles until it runs out
// EXPECT exit = 1
// EXPECT output: [FATAL]: attempt to fetch instruction from non text segment
// CONFIGS: ideal ideal_nofwd
        addi    r1, r0, 1
