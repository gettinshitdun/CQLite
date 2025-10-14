#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include "include/lexer.h"

static int is_identifier_char(char c) {
    return isalnum(c) || c == '_';
}

void lexer_init(Lexer* lexer, const char* input) {
    lexer->input = input;
    lexer->pos = 0;
    lexer->length = strlen(input);
}

static void skip_whitespace(Lexer* lexer) {
    while (lexer->pos < lexer->length &&
           isspace(lexer->input[lexer->pos])) {
        lexer->pos++;
    }
}

static char peek(Lexer* lexer) {
    if (lexer->pos >= lexer->length) return '\0';
    return lexer->input[lexer->pos];
}

static char advance(Lexer* lexer) {
    if (lexer->pos >= lexer->length) return '\0';
    return lexer->input[lexer->pos++];
}

static Token make_token(TokenType type, const char* start, size_t len) {
    Token t;
    t.type = type;
    t.lexeme = (char*)malloc(len + 1);
    memcpy(t.lexeme, start, len);
    t.lexeme[len] = '\0';
    return t;
}

static Token make_simple(TokenType type, const char* lex) {
    return make_token(type, lex, strlen(lex));
}

void lexer_free_token(Token* token) {
    if (token->lexeme) free(token->lexeme);
}

static TokenType keyword_type(const char* s) {
    if (strcasecmp(s, "create") == 0) return TK_CREATE;
    if (strcasecmp(s, "table") == 0) return TK_TABLE;
    if (strcasecmp(s, "insert") == 0) return TK_INSERT;
    if (strcasecmp(s, "into") == 0) return TK_INTO;
    if (strcasecmp(s, "values") == 0) return TK_VALUES;
    if (strcasecmp(s, "select") == 0) return TK_SELECT;
    if (strcasecmp(s, "from") == 0) return TK_FROM;
    if (strcasecmp(s, "where") == 0) return TK_WHERE;
    if (strcasecmp(s, "int") == 0) return TK_INT;
    if (strcasecmp(s, "text") == 0) return TK_TEXT;
    return TK_IDENTIFIER;
}

Token lexer_next_token(Lexer* lexer) {
    skip_whitespace(lexer);
    if (lexer->pos >= lexer->length) return make_simple(TK_EOF, "");

    char c = advance(lexer);

    // punctuation
    switch (c) {
        case '(': return make_simple(TK_LPAREN, "(");
        case ')': return make_simple(TK_RPAREN, ")");
        case ',': return make_simple(TK_COMMA, ",");
        case '*': return make_simple(TK_STAR, "*");
        case ';': return make_simple(TK_SEMI, ";");
        case '=': return make_simple(TK_EQ, "=");
        case '\'': {
            const char* start = &lexer->input[lexer->pos];
            while (peek(lexer) && peek(lexer) != '\'') advance(lexer);
            size_t len = &lexer->input[lexer->pos] - start;
            advance(lexer); // closing quote
            return make_token(TK_STRING_LITERAL, start, len);
        }
    }

    // number
    if (isdigit(c)) {
        const char* start = &lexer->input[lexer->pos - 1];
        while (isdigit(peek(lexer))) advance(lexer);
        size_t len = &lexer->input[lexer->pos] - start;
        return make_token(TK_INTEGER_LITERAL, start, len);
    }

    // identifier or keyword
    if (isalpha(c) || c == '_') {
        const char* start = &lexer->input[lexer->pos - 1];
        while (is_identifier_char(peek(lexer))) advance(lexer);
        size_t len = &lexer->input[lexer->pos] - start;
        char buffer[128];
        if (len >= sizeof(buffer)) len = sizeof(buffer) - 1;
        strncpy(buffer, start, len);
        buffer[len] = '\0';
        TokenType type = keyword_type(buffer);
        return make_token(type, start, len);
    }

    // unknown
    return make_simple(TK_UNKNOWN, (char[]){c, '\0'});
}
