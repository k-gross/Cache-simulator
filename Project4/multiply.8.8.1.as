        lw      0       2       mcand    load reg1 with mcand
        lw      0       3       mplier   load reg2 with mplier
        lw      0       4       fiftn
        lw      0       5       one
        lw      0       6       neg2
start   beq     0       4       done
        nor     3       6       7       checks if current bit is 1, if resulting value in reg7 is 0 then it is
        beq     0       7       good    if reg7 is 0, then cur bit is 1, need to add to running total
        beq     0       0       skip    if not then skip over adding
good    add     1       2       1       adds running total
skip    add     2       2       2       shift over mcand after we're done
        add     5       4       4       increment loop counter by 1
        add     6       6       6       shift the mask over 1 then add 1 to it
        add     5       6       6
        beq     0       0       start   go back to the beginning of the loop
        noop
done    halt                            end of program
fiftn  .fill   -15
neg1    .fill   -1
stAddr  .fill   start                   will contain the address of start (2)'
neg2    .fill   -2                      will contain -2 which we need for masking because of nor
one     .fill   1
mcand   .fill   6203
mplier  .fill   1429