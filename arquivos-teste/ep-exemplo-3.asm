; verifica se o numero digitado, de dois decimais, eh divisivel por 3, se for retorna 01 senao retorna 00
@ /0000
        GD /000
        SB ROT04
        MM ROT00
JUMP00  LD ROT00
        SB ROT01
        JN JUMP01
        JZ JUMP02
        MM ROT00
        JP JUMP00
JUMP01  LD ROT02
        AD ROT04
        PD /100
        HM /0000
JUMP02  LD ROT03
        AD ROT04
        PD /100
        HM /0000

@ /0300
ROT00 K /0000; numerador
ROT01 K /0003; denominador
ROT02 K /0000; falso
ROT03 K /0001; verdadeiro
ROT04 K /3030; converte ascii
