#include "tokenizer.h"

#include <stdbool.h>
#include <stdio.h>

bool is_digit(char c) {
    return c >= '0' && c <= '9';
}

bool is_alphanumeric(char c) {
    return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9');
}

// TODO: hack to get things moving. this includes ($, ", ') which should be ignored or handled differently.
bool is_punctuation_start(char c) {
    return (c >= '!' && c <= '/') || (c >= ':' && c <= '?') || (c >= '[' && c <= '^') || (c >= '{' && c <= '~');
}

bool punctuation_can_be_doubled(char c) {
    // Omit equal sign; it will be captured in the can_be_assignment function.
    return c == '+' || c == '-' || c == '&' || c == '|' || c == ':';
}

// Can this character be followed by '=' and be a valid token?
bool punctuation_can_be_assignment(char c) {
    return c == '+' || c == '-' || c == '*' || c == '/' || c == '%' || c == '&' || c == '|' || c == '^' || c == '>' || c == '<' || c == '=' || c == '!';
}

const char* TokenTypeNames[] = {
    "",
    "PREPROCESSOR",
    "BLOCK_COMMENT",
    "LINE_COMMENT",
    "CHAR",
    "STRING",
    "IDENTIFIER",
    "CONST",
    "OPERATOR",
    "CONTINUATION",
    "NEWLINE",
    "UNKNOWN"
};

void buf_push(CharStar* cs, char c) {
    cs->buf[cs->buf_ptr++] = c;
    cs->buf_ptr %= BUF_SIZE;
}

const char* buf_finish(CharStar* cs) {
    cs->buf[cs->buf_ptr] = '\0';
    cs->buf_ptr = 0;
    return cs->buf;
}

CharStar global_cs;

void CharStar_init(CharStar* cs, const char* raw) {
    cs->line = 1;
    cs->pos = 0;
    cs->buf_ptr = 0;

    if (raw == NULL) {
        cs->done = true;
        return;
    }

    cs->done = false;
    cs->raw = raw;
    cs->cur = *cs->raw;

    if (cs->cur == '\0') {
        cs->next = '\0';
        cs->done = true;
    } else {
        cs->raw++;
    }

    cs->next = *cs->raw;

    if (cs->next != '\0') {
        cs->raw++;
    }

    //printf("init %c|%c|%s\n", cs->cur,cs->next,cs->raw);
}

void CharStar_seek(CharStar* cs) {
    if (cs->done) {
        return;
    }

    if (cs->cur == '\n') {
        cs->line++;
    }

    cs->pos++;
    cs->cur = cs->next;

    if (cs->cur == '\0') {
        cs->done = true;
        cs->next = '\0';
    } else {
        cs->next = *cs->raw++;
    }

    //printf("seek %c|%c|%s\n", cs->cur,cs->next,cs->raw);
}

bool CharStar_iter(CharStar* cs, char* cur, char* next) {
    if (cs->cur == '\0') {
        return false;
    }

    *cur = cs->cur;
    *next = cs->next;
    CharStar_seek(cs);
    return true;
}

void read_line_comment(CharStar *cs) {
    int line = cs->line;
    int pos = cs->pos;

    char cur;
    char next;

    while (CharStar_iter(cs, &cur, &next)) {
        // Skip trailing newline. Break before adding cur char to buffer.
        if (cur == '\n') {
            break;
        }

        buf_push(cs, cur);
    }

    cs->token.type = LINE_COMMENT;
    cs->token.line = line;
    cs->token.pos = pos;
    cs->token.value = buf_finish(cs);
}

typedef enum {
    START,
    BRACE,
    FIRST_STAR,
    WAIT,
    NEXT_STAR,
} BlockCommentState;

void read_block_comment(CharStar *cs) {
    int line = cs->line;
    int pos = cs->pos;
    bool prev_was_star = false;

    char cur;
    char next;

    // Read the opening "/*" so we will not end early on the string "/*/".
    // Assumes cur, next will be '//', '*' as a requirement to enter this function.
    CharStar_iter(cs, &cur, &next);
    buf_push(cs, cur);
    // cur is now '*'. If next is '/0' we will not enter the loop.
    CharStar_iter(cs, &cur, &next);
    buf_push(cs, cur);

    while (CharStar_iter(cs, &cur, &next)) {
        buf_push(cs, cur);
        if (cur == '/' && prev_was_star) {
            break;
        }

        prev_was_star = (cur == '*');
    }

    cs->token.type = BLOCK_COMMENT;
    cs->token.line = line;
    cs->token.pos = pos;
    cs->token.value = buf_finish(cs);
}

void read_string(CharStar* cs) {
    int line = cs->line;
    int pos = cs->pos;
    bool started = false;
    int slashes = 0;

    char cur;
    char next;

    while (CharStar_iter(cs, &cur, &next)) {
        buf_push(cs, cur);

        if (cur == '"' && slashes % 2 == 0) {
            if (started) {
                break;
            }

            started = true;
        }

        // consecutive slashes for escape char
        if (cur == '\\') {
            slashes++;
        } else {
            slashes = 0;
        }
    }

    cs->token.type = STRING;
    cs->token.line = line;
    cs->token.pos = pos;
    cs->token.value = buf_finish(cs);
}

void read_numeric(CharStar* cs) {
    int line = cs->line;
    int pos = cs->pos;

    char cur;
    char next;

    while (CharStar_iter(cs, &cur, &next)) {
        if (cur != '.' && !is_alphanumeric(cur)) {
            break;
        }

        buf_push(cs, cur);
    }

    cs->token.type = CONST;
    cs->token.line = line;
    cs->token.pos = pos;
    cs->token.value = buf_finish(cs);
}

void read_punctuation(CharStar* cs) {
    int line = cs->line;
    int pos = cs->pos;

    char cur;
    char next;
    CharStar_iter(cs, &cur, &next);

    buf_push(cs, cur);

    // Left or right shift
    if ((cur == '>' || cur == '<') && cur == next) {
        CharStar_iter(cs, &cur, &next);
        buf_push(cs, cur);
        // Check for assignment
        if (next == '=') {
            CharStar_iter(cs, &cur, &next);
            buf_push(cs, cur);
        }
    }
    // Doubled-up characters (e.g. ++ or --)
    // or characters that can precede the equal sign
    else if ((punctuation_can_be_doubled(cur) && cur == next) || (punctuation_can_be_assignment(cur) && next == '=')) {
        CharStar_iter(cs, &cur, &next);
        buf_push(cs, cur);
    }
    // Pointer to struct member
    else if (cur == '-' && next == '>') {
        CharStar_iter(cs, &cur, &next);
        buf_push(cs, cur);
    }

    // TODO: ... is a special case because we have no mechanism (yet) to read '..' and rewind the ptr.

    cs->token.type = OPERATOR;
    cs->token.line = line;
    cs->token.pos = pos;
    cs->token.value = buf_finish(cs);
}

bool CharStar_next(CharStar* cs) {
    while (cs->cur != '\0') {
        // Ignore whitespace
        if (cs->cur == ' ' || cs->cur == '\t') {
            CharStar_seek(cs);
            continue;
        }

        if(cs->cur == '/') {
            if (cs->next == '/') {
                read_line_comment(cs);
                return true;
            }
            if (cs->next == '*') {
                read_block_comment(cs);
                return true;
            }
            // Fall through for '/' division symbol
        }

        if (cs->cur == '"') {
            read_string(cs);
            return true;
        }
        // Numeric test must come before punctuation test so we capture numbers like '.5'.
        else if (is_digit(cs->cur) || (cs->cur == '.' && is_digit(cs->next))) {
            read_numeric(cs);
            return true;
        }
        else if (is_punctuation_start(cs->cur)) {
            read_punctuation(cs);
            return true;
        }

        CharStar_seek(cs);
    }

    // TODO: partial token here?
    return false; // no token found
}