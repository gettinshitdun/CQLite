// token.h
#ifndef CQLITE_TOKEN_H
#define CQLITE_TOKEN_H

typedef enum {
    TK_EOF = 0,
    TK_CREATE,
    TK_TABLE,
    TK_INSERT,
    TK_INTO,
    TK_VALUES,
    TK_SELECT,
    TK_FROM,
    TK_WHERE,
    TK_INT,
    TK_TEXT,
    TK_IDENTIFIER,
    TK_INTEGER_LITERAL,
    TK_STRING_LITERAL,
    TK_STAR,
    TK_COMMA,
    TK_LPAREN,
    TK_RPAREN,
    TK_SEMI,
    TK_EQ,
    TK_UNKNOWN
} TokenType;

typedef struct {
    TokenType type;
    char* lexeme;
} Token;

#endif
