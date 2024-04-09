#include <stdint.h>
#include <stdarg.h>
#define VID_MEM (uint16_t*)0x00000000000B8000

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

Token lookahead;
uint64_t tokenValue;
int index;
const char* string;


char lengths[] = {'h', 'l', 'j', 'z', 't', 'L', 0};
char flags[] = {'-', '+', ' ', '#', '0', 0};
char specifiers[] = {'d', 'i', 'u', 'o', 'x', 'X', 'f', 'F', 'e', 'E', 'g', 'G', 'a', 'A', 'c', 's', 'p', 'n', '%', 0};
char numbers[] = "0123456789";

static void print(char character, uint8_t color, uint64_t index){
    uint16_t c = ((uint16_t)color << 8) | character;
    uint16_t* dest = VID_MEM;
    dest[index] = c;
}

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
        //Error
    }
}

void text(){
    if(lookahead == CHAR){
        char c = tokenValue;
        match(CHAR);
        print(c, 0x0F, index);
        text();
    }
    else{
        //empty
    }
}

void flag(){
    if(lookahead == FLAG){
        match(FLAG); flag();
    }else{
        //empty
    }
}

void width(){
    if(lookahead == NUMBER){
        match(NUMBER);
    }
}

void precision(){
    if(lookahead == DOT){
        match(DOT); match(NUMBER);
    }
}

void length(){
    if(lookahead == LENGTH){
        match(LENGTH); length();//Change this
    }
}

void format(){
    if(lookahead == BEGIN){
        match(BEGIN); flag(); width(); precision(); length(); 
        char spec = tokenValue;
        match(SPECIFIER);
    }
}

void list(){
    if(lookahead == CHAR || lookahead == BEGIN){
        text(); format(); list();
    }
}

void parse(const char* text){
    index = 0;
    string = text;
    list(); match(END);
}



uint64_t printf(const char* format){
    uint64_t i = 0;
    parse(format);
    
    return i;
}