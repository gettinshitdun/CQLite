// lexer.h
#ifndef CQLITE_LEXER_H
#define CQLITE_LEXER_H

#include "token.h"

typedef struct {
    const char* input;
    size_t pos;
    size_t length;
} Lexer;

void lexer_init(Lexer* lexer, const char* input);
Token lexer_next_token(Lexer* lexer);
void lexer_free_token(Token* token);

#endif
