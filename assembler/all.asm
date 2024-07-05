int 1
irt
hlt
hcf
pxr R0, 1
pxw 1, R0
cal 3
jmp 3
jcu LEG 1
jcs LEG 1
prd R0, 1
prd R0, R0
pwr 1, R0
pwr R0, R0
xrd R0, IP
xrd R0, CF
xrd R0, LR
xrd R0, SP
xrd R0, OF
xrd R0, FL
xrd R0, F1
xrd R0, F2
xwr IP, R0
xwr UI, R0
xwr LR, R0
xwr SP, R0
xwr OF, R0
xwr FL, R0
xwr F1, R0
xwr F2, R0
xwr IP, 3
mrd R0, 3
mrd R0, R3
mwr 3, R0
mro R0, 3
mro R0, R3
mwo 3, R0
mul R0, 3
mul R0, R3
cmp R0, 3
cmp R0, R3
div R0, 3
div R0, R3
tst R0, 3
tst R0, R3
mov R0, 3
mov R0, R3
pop R0
psh R0
cal R0
jmp R0
ret
ret SOCZ
mcu LEG R0, R0
mcs LEG R0, R0
add R0, 3
add R0, R3
sub R0, 3
sub R0, R3
not R0, 3
not R0, R3
and R0, 3
and R0, R3
orr R0, 3
orr R0, R3
xor R0, 3
xor R0, R3
shl R0, 3
shl R0, R3
shr R0, 3
shr R0, R3
nop
