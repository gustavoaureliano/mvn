; calcula um numero (NUM) elevado a (POT)
@ /0000
        LD ROT01
        MM ROT03
JUMP00  LD ROT03
        SB ROT02
        MM ROT03
        JZ JUMP01
        LD ROT00
        ML ROT00
        MM ROT04
        JP JUMP00
JUMP01  HM /0000

@ /0300
ROT00   K /0003
ROT01   K /0002
ROT02   K /0001
ROT03   K /0000
ROT04   K /0000
