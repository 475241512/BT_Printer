   AREA _ASC16, DATA, READONLY  

    EXPORT  ASC16_Base
    EXPORT  ASC16_End  
    
    ALIGN 4
ASC16_Base
    INCBIN .\LIB_Source\ASC16x16_0_127.bin                 ; ASCÂë 0~127, 2K Byte
ASC16_End
    
    END