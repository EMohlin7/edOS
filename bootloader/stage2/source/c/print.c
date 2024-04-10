#include <stdint.h>
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

Token lookahead;
uint64_t tokenValue;
int index;
uint64_t flagData, widthData, precisionData;
Length lenghtData;
const char* string;


char lengths[] = {'h', 'l', 0};
char flags[] = {'-', '+', ' ', '#', '0', 0};
char specifiers[] = {'d', 'i', 'u', 'o', 'x', 'X', 'f', 'F', 'e', 'E', 'g', 'G', 'a', 'A', 'c', 's', 'p', 'n', 0};
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
        printChar(c, 0x0F);
        printed = text(++printed);
    }
    else{
        //empty
    }

    return printed;
}

void flag(){
    if(lookahead == FLAG){
        switch(tokenValue){
            case '-':
                flagData |= LEFT_JUST;
                break;
            case '+':
                flagData |= SIGN;
                break;
            case ' ':
                flagData |= NO_SIGN;
                break;
            case '#':
                flagData |= HASH;
                break;
            case '0':
                flagData |= PAD_ZEROS;
                break;
            default:
                break;
        }

        match(FLAG); flag();
    }else{
        //empty
    }
}

void width(){
    if(lookahead == NUMBER){
        widthData = tokenValue;
        match(NUMBER);
    }
}

void precision(){
    if(lookahead == DOT){
        match(DOT); precisionData = tokenValue; match(NUMBER);
    }
}

void length(){
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
}

static uint64_t printInteger(int val){
    uint64_t printed = 0;
    if(val < 0){
        printChar('-', WHITE_BLACK_CHAR);
        val = -val;
        ++printed;
    }
    char number[NUM_MAX_CHARS];
    int i = NUM_MAX_CHARS-1;
    do
    {
        number[i] = (val % 10)+48;
        val /= 10;
    } while (i-- > 0 && val > 0);
    
    for( ; number[i] != NULL && i < NUM_MAX_CHARS; ++i){
        printChar(number[i], WHITE_BLACK_CHAR);
        printed++;
    }

    return printed;
}

uint64_t format(uint64_t printed){
    if(lookahead == BEGIN){
        match(BEGIN); flag(); width(); precision(); length(); 
        char spec = tokenValue;
        match(SPECIFIER);
        switch (spec)
        {
        case 'd':
            printed += printInteger(256);
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

static void setDefaults(){
    lenghtData = integer;
    flagData = 0;
    precisionData = 0;
    widthData = 0;
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