.entry mov
    .entry W
    .extern W
mov sdergwd,1
;COmment
mov: cmp LENGTH ,1
LOOP: prn LENGTH ,1
HEY: mov LENGTH ,1
lea 300 ,           @r4
LOOP: jmp L1
prn 5
bne 1
sub @r1, @r4
add -800 ,@r0
L1: inc 100
LOL:    .entry LOOP
jmp W
    END: stop , @r2
STR: .string "abcdef"
LENGTH: .data 10,-100,5000
K: .data 22
    .extern L3

