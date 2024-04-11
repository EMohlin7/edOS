#include <stdarg.h>
#include "display.h"

#define NUM_MAX_CHARS 20

//Flags
#define LEFT_JUST   1
#define SIGN        (1 << 1)
#define NO_SIGN     (1 << 2)
#define HASH        (1 << 3)
#define PAD_ZEROS   (1 << 4)

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
    int64_t precision;
    Length length;
    uint8_t flags;
    char spec;
} Format;

Token lookahead;
uint64_t tokenValue;
int index;
const char* string;


char lengths[] = {'h', 'l', 0};
char flags[] = {'-', '+', ' ', '#', '0', 0};
char specifiers[] = {'d', 'i', 'u', 'o', 'x', 'X', 'c', 's', 'p', 'n', 0};
char numbers[] = "0123456789";


static char find(char c, char* arr){
    for(int i = 0; arr[i] != NULL; ++i){
        if(arr[i] == c)
            return arr[i];
    }
    return 0;
}

//Format: %[flags][width][.precision][length]specifier
Token lex(){
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
                tokenValue = *(uint16_t*)"hh";
                return LENGTH;
            }
            else if(tokenValue == 'l' && string[index] == 'l'){
                ++index;
                tokenValue == *(uint16_t*)"ll";
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

void match(Token token){
    if(lookahead == token){
        lookahead = lex();
    }else{
        //error
        while(1);
    }
}

uint64_t text(uint64_t printed){

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

uint8_t flag(uint8_t flags){
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

uint64_t width(){
    uint64_t width = 0;
    if(lookahead == NUMBER){
        width = tokenValue;
        match(NUMBER);
    }
    return width;
}

int64_t precision(){
    int64_t precision = -1;
    if(lookahead == DOT){
        match(DOT); 
        if(lookahead == NUMBER){
            if(tokenValue > INT64_MAX)
                precision = INT64_MAX;
            else    
                precision = tokenValue;
            match(NUMBER);
        }
        else{
            precision = 0;
        }
    }
    return precision;
}

Length length(){
    Length length = integer;
    if(lookahead == LENGTH){
        switch (tokenValue)
        {
        case 'h':
            /* code */
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

    if(val < 0){
        val = -val;
        if(!(f->flags&NO_SIGN)){
            print('-');
        }
        else{
            print(' ');
        }
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

    //Right justified
    if(f->width > numChars && !(f->flags&LEFT_JUST)){
        char c = f->flags&PAD_ZEROS ? '0' : ' ';
        for(uint64_t i = 0; i < f->width - numChars; ++i){
            print(c);
            ++printed;
        }
    }

    return printed;
}


static uint64_t printInteger(const void* orgVal, const Format* f){
    int64_t val;
    uint64_t printed = 0;


    if(f->length == integer){
        val = *(int32_t*)orgVal;
    }
    else if(f->length == character){
        val = *(int8_t*)orgVal;
    }
    else if(f->length == shortInt){
        val = *(int16_t*)orgVal;
    }
    else{
        val = *(int64_t*)orgVal;
    }
    if(f->precision == 0 && val == 0)
        goto skipPrint;


    if(val < 0)
        val = -val;

    char number[NUM_MAX_CHARS];
    int i = NUM_MAX_CHARS-1;
    uint64_t numChars = 0;
    int64_t temp = val;
    do
    {
        number[i] = (temp % 10)+48;
        temp /= 10;
        ++numChars;
    } while (i-- > 0 && temp > 0);

    if(f->precision > 0){
        if(numChars < INT64_MAX){
            for(; (int64_t)numChars < f->precision; ++numChars){
                number[i--] = '0'; 
            }
        }
    }
    i += 1;

    printed = preceedingChars(val, numChars, f);
    for( ; number[i] != NULL && i < NUM_MAX_CHARS; ++i){
        print(number[i]);
        printed++;
    }

    //Left justified
    if(f->flags&LEFT_JUST){
        if(f->width > numChars){
            char c = f->flags&PAD_ZEROS ? '0' : ' ';
            for(uint64_t i = 0; i < f->width - numChars; ++i){
                print(c);
                ++printed;
            }
        }
    }

skipPrint:
    return printed;
}

uint64_t format(uint64_t printed){
    Format f;
    if(lookahead == BEGIN){
        match(BEGIN); f.flags=flag(0); f.width=width(); f.precision=precision(); f.length=length(); 
        char spec = tokenValue;
        f.spec = spec;
        match(SPECIFIER);
        int val = 235;
        switch (spec)
        {
        case 'd':
            printed += printInteger(&val, &f);
            break;
        case 'i':
            printed += printInteger(&val, &f);
            break;
        default:
            break;
        }
    }

    return printed;
}

uint64_t list(uint64_t printed){
    if(lookahead == CHAR || lookahead == BEGIN){
        printed = text(printed); printed = format(printed); printed = list(printed);
    }
    return printed;
}


uint64_t parse(const char* text){
    index = 0;
    string = text;
    lookahead = lex();
    uint64_t printed = list(0); match(END);
    return printed;
}



uint64_t printf(const char* format){
    
    return parse(format);    
}