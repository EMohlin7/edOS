#include "printf.h"
#include <stdarg.h>
#include "display.h"

#define NUM_MAX_CHARS 20

//Flags
#define LEFT_JUST       1
#define SIGN            (1 << 1)
#define NO_SIGN         (1 << 2)
#define HASH            (1 << 3)
#define PAD_ZEROS       (1 << 4)
#define USE_PRECISION   (1 << 5) //Used to signal that the precision value should be used
#define DBL_TOKEN(x)    ((x << 8) + x)

typedef enum token{
    CHAR,
    BEGIN,
    FLAG,
    DOT,
    NUMBER,
    LENGTH,
    SPECIFIER,
    END
} Token;

typedef enum length{
    integer,
    character,
    shortInt,
    longInt
} Length;

typedef struct format{
    uint64_t width;
    uint64_t precision;
    Length length;
    uint8_t flags;
    char spec;
} Format;

static Token lookahead;
static uint64_t tokenValue;
static uint64_t index;
static const char* string;

static char lengths[] = {'h', 'l', 0};
static char flags[] = {'-', '+', ' ', '#', '0', 0};
static char specifiers[] = {'d', 'i', 'u', 'o', 'x', 'X', 'c', 's', 'p', 'n', 0};
static char numbers[] = "0123456789";


static char find(char c, char* arr){
    for(int i = 0; arr[i] != NULL; ++i){
        if(arr[i] == c)
            return arr[i];
    }
    return 0;
}

//Format: %[flags][width][.precision][length]specifier
static Token lex(){
    static int isFormat = 0;
    while (1)
    {
        tokenValue = string[index++];
        if(tokenValue == NULL)
            return END;

        if(isFormat == 0){
            if(tokenValue == '%'){
                if(string[index] == '%'){
                    ++index;
                    return CHAR;
                }
                isFormat = 1;
                return BEGIN;
            }
            return CHAR;
        }

        //In format state
        if(find(tokenValue, flags))
            return FLAG;

        if(tokenValue == '.')
            return DOT;

        if(find(tokenValue, numbers)){
            tokenValue -= 48; //ASCII to int
            while(find(string[index], numbers)){
                tokenValue = tokenValue*10 + string[index]-48;
                ++index;
            }
            return NUMBER;
        }

        if(find(tokenValue, lengths)){
            if(tokenValue == 'h' && string[index] == 'h'){
                ++index;
                tokenValue = DBL_TOKEN('h');
                return LENGTH;
            }
            else if(tokenValue == 'l' && string[index] == 'l'){
                ++index;
                tokenValue = DBL_TOKEN('l');
                return LENGTH;
            }

            return LENGTH;
        }

        if(find(tokenValue, specifiers)){
            isFormat = 0;
            return SPECIFIER;
        }
    }
}

static void match(Token token){
    if(lookahead == token){
        lookahead = lex();
    }else{
        //error
        while(1);
    }
}

static uint64_t text(uint64_t printed){

    if(lookahead == CHAR){
        char c = tokenValue;
        match(CHAR);
        print(c);
        printed = text(++printed);
    }
    else{
        //empty
    }

    return printed;
}

static uint8_t flag(uint8_t flags){
    if(lookahead == FLAG){
        switch(tokenValue){
            case '-':
                flags |= LEFT_JUST;
                break;
            case '+':
                flags |= SIGN;
                break;
            case ' ':
                flags |= NO_SIGN;
                break;
            case '#':
                flags |= HASH;
                break;
            case '0':
                flags |= PAD_ZEROS;
                break;
            default:
                break;
        }

        match(FLAG); flags = flag(flags);
    }else{
        //empty
    }
    return flags;
}

static uint64_t width(){
    uint64_t width = 0;
    if(lookahead == NUMBER){
        width = tokenValue;
        match(NUMBER);
    }
    return width;
}

static uint64_t precision(uint8_t* flags){
    uint64_t precision = 0;
    if(lookahead == DOT){
        match(DOT); 
        if(lookahead == NUMBER){
                precision = tokenValue;
            match(NUMBER);
        }
        
        *flags |= USE_PRECISION;
    }
    return precision;
}

static Length length(){
    Length length = integer;
    if(lookahead == LENGTH){
        switch (tokenValue)
        {
        case 'h':
            length = shortInt;
            break;
        case DBL_TOKEN('h'):
            length = character;
            break;
        case 'l':
            length = longInt;
            break;
        case DBL_TOKEN('l'):
            length = longInt;
            break;
        
        default:
            break;
        }
        match(LENGTH); 
    }
    return length;
}


static uint64_t preceedingChars(int64_t val, uint64_t numChars, const Format* f){
    uint64_t printed = 0;

    //TODO: This is not true.
    if(f->flags&USE_PRECISION && val == 0 && f->precision == 0) //Precision 0 should not print anything for the number zero
        return 0;

    

    if(f->flags&HASH){       
        if(f->spec == 'o'){
            print('0');
            ++printed;
        }
        if(f->spec == 'x'){
            print('0');
            print('x');
            printed += 2;
        }
        if(f->spec == 'X'){
            print('0');
            print('X');
            printed += 2;
        }
    }
    
    //Precision specifies minimum number of DIGITS
    if(f->flags&USE_PRECISION && f->precision > numChars){
        for(uint64_t i = 0; i < f->precision - numChars; ++i){
            print('0');
            ++printed;
        }
    }

    //Right justified
    if(f->width > printed+numChars && !(f->flags&LEFT_JUST)){
        //Width specifies minimum number of CHARACTERS
        char c = f->flags&PAD_ZEROS ? '0' : ' ';
        uint64_t temp = printed+numChars;
        for(uint64_t i = 0; i < f->width - temp; ++i){
            print(c);
            ++printed;
        }
    }

    return printed;
}

static uint64_t printInt(uint64_t val, const Format* f, uint8_t base){
    uint64_t printed = 0;

    char number[NUM_MAX_CHARS];
    int i = NUM_MAX_CHARS-1;
    uint64_t numChars = 0;
    uint64_t temp = val;
    do
    {
        uint64_t num = (temp % base);
        number[i] = num+ (num < 10 ? 48 : f->spec=='X'? 55 : 87);  //Convert to ascii, including numbers bigger than 9 (for hexadecimal)
        temp /= base;
        ++numChars;
    } while (i-- > 0 && temp > 0);
    i += 1;

    printed = preceedingChars(val, numChars, f);
    for( ; number[i] != NULL && i < NUM_MAX_CHARS; ++i){
        print(number[i]);
        printed++;
    }

    //Left justified
    //Width specifies minimum number of CHARACTERS
    if(f->flags&LEFT_JUST && f->width > printed){
        char c = f->flags&PAD_ZEROS ? '0' : ' ';
        uint64_t temp = printed;
        for(uint64_t i = 0; i < f->width - temp; ++i){
            print(c);
            ++printed;
        }
    }

    return printed;
}

static uint64_t printUnsignedInt(va_list argptr, const Format* f, uint8_t base){
    uint64_t val;
    uint64_t printed = 0;


    if(f->length == longInt){
        val = va_arg(argptr, unsigned long int);
    }
    else{
        //Everything smaller than int is promoted to int when passed through '...'
        val = va_arg(argptr, unsigned int);
    }

    if(f->flags&SIGN){
        print('+');
        ++printed;
    }
    else if(f->flags&NO_SIGN){
        print(' ');
        ++printed;
    }

    return printInt(val, f, base);
}

static uint64_t printInteger(va_list argptr, const Format* f){
    int64_t val = 0;
    uint64_t printed = 0;

    if(f->length == longInt){
        val = va_arg(argptr, long int);
    }
    else{
        //Everything smaller than int is promoted to int when passed through '...'
        val = va_arg(argptr, int);
    }
   
    if(val < 0){
        val = -val;
        print('-');
        
        ++printed;        
    }
    else if(f->flags&SIGN){
        print('+');
        ++printed;
    }
    else if(f->flags&NO_SIGN){
        print(' ');
        ++printed;
    }

   
    return printInt(val, f, 10);
}

static uint64_t printString(const char* string, const Format* f){
    uint64_t printed = 0;
    
    //Right justified padding
    if(f->width > 0 && !(f->flags&LEFT_JUST)){
        uint64_t size = 0;
        for(; string[size] != NULL; ++size)
            ;

        if(f->width > size){
            char c = f->flags&PAD_ZEROS ? '0' : ' ';
            for(uint64_t i = 0; i < f->width-size; ++i){
                print(c);
                ++printed;
            }
        }
    }
    
    //Print string
    for(int i = 0; string[i] != NULL; ++i){
        print(string[i]);
        ++printed;
    }

    //Right justified padding
    if(f->flags&LEFT_JUST && f->width > printed){
        char c = f->flags&PAD_ZEROS ? '0' : ' ';
        for(uint64_t i = 0; i < f->width-printed; ++i){
            print(c);
            ++printed;
        }
    }
    return printed;
}

static uint64_t printCharacter(int value, const Format* f){
    char c[2] = {(char)value, 0};
    return printString(c, f);
}

static uint64_t format(uint64_t printed, va_list argptr){
    Format f;
    if(lookahead == BEGIN){
        match(BEGIN); f.flags=flag(0); f.width=width(); f.precision=precision(&(f.flags)); f.length=length(); 
        char spec = tokenValue;
        f.spec = spec;
        match(SPECIFIER);
        

        #define SIGNED() printed += printInteger(argptr, &f)
        #define USIGNED(x) printed += printUnsignedInt(argptr, &f, x)
        switch (spec)
        {
        case 'd':
            SIGNED();
            break;
        case 'i':
            SIGNED();
            break;
        case 'u':
            USIGNED(10);
            break;
        case 'x':
            USIGNED(16);
            break;
        case 'X':
            USIGNED(16);
            break;
        case 'o':
            USIGNED(8);
            break;
        case 'p':
            f.spec = 'x';
            f.length = longInt;
            USIGNED(16);
            break;
        case 's': ;
            char* s = va_arg(argptr, char*);
            printed += printString(s, &f);
            break;
        case 'c': ;
            int c = va_arg(argptr, int);
            printed += printCharacter(c, &f);
            break;
        default:
            break;
        }
    }

    return printed;
}

static uint64_t list(uint64_t printed, va_list argptr){
    if(lookahead == CHAR || lookahead == BEGIN){
        printed = text(printed); printed = format(printed, argptr); printed = list(printed, argptr);
    }
    return printed;
}


static uint64_t parse(const char* text, va_list argptr){
    index = 0;
    string = text;
    lookahead = lex();
    uint64_t printed = list(0, argptr); match(END);
    return printed;
}



uint64_t printf(const char* format, ...){
    va_list argptr;
    va_start(argptr, format);
    uint64_t printed = parse(format, argptr);    
    va_end(argptr);
    return printed;
}