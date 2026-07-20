1. Introduction (Overview)
        |
        |--------------------------> Quick tutorial
        |--------------------------> Requirements :
                                                riscv64 tool chain

2. Features (ALL features)

3. Project Architecture 

4. Tutorial (FULL)

5. Configuration
        All configurable parameters

6. CPU Architecture
        the block diagram
        explain each component, give appropriate links too
        explain how the stalls work, how the flushes work!
        briefly mention that the control signals are saved in the pipeline registers because....
        more details on each stage's control signals in 7.pipeline

7. Pipeline
        use the same cpu datapath diagram...(each subsection gets a part of the overall diagram)
        subsections are if, id, ex, ma, wb...
        explain how the stalls work, how the flushes work!
        explain the corresponding control signals in each stage..
        at the end  walk through execution of branch, sw, add

8. Hazard Handling
        All three hazards 
        data:forwarding
        control:branchpred
        structural:the hazard where the instruction and the data caches simultaneously want to evict the same block in l2 or below...

9. Branch Prediction
        How it is handled... (the api) 
        i) id flush 
        ii) ex_flush

10. Cache Hierarchy
        Explain how the cache system is seperate from the rest of the machine.
        The cache system only simulates cache hierarchy and calculates the total number of cycles required
        A bug in the cache system will only affect the statistical accuracy and not the computational accuracy of the simulator
        Then structs...
        How are they used configurably...

11. Memory System
        dram... little endian...

12. Assembler
        Two pass...
        symbol table - list
        supports all rv32i formats...
        give reference to the pdf

13. Statistics
        All supported stats:
        Measured stats
        Derived stats
        what they all mean...

14. Testing

15. Limitations

16. Future Work
        predictor, cache policy, assembler redesign, implement exceptions and interrupts, 
        (future prospects... others might wanna work on)
