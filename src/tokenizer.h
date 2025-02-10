#ifndef TOKENIZER_H
#define TOKENIZER_H

#include <stdbool.h>

typedef enum TokenType {
    PREPROCESSOR = 1,
    BLOCK_COMMENT,
    LINE_COMMENT,
    CHAR,
    STRING,
    IDENTIFIER,
    CONST,
    OPERATOR,
    CONTINUATION,
    NEWLINE,
    UNKNOWN
} TokenType;

typedef struct {
    TokenType type;
    int line;
    int pos;
    const char* value;
} Token;

#define BUF_SIZE 2048

typedef struct {
    bool done;
    const char* raw;
    char cur;
    char next;
    int line;
    int pos;
    Token token;
    char buf[BUF_SIZE];
    int buf_ptr;
} CharStar;

extern const char* TokenTypeNames[];
extern void CharStar_init(CharStar* cs, const char* raw);
extern bool CharStar_next(CharStar* cs);

#endif // TOKENIZER_H