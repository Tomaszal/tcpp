#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/* Token structure for file tokenization.
 * Stores token information and pointers to its neighboring tokens.
 *
 * More information on this process:
 * https://gcc.gnu.org/onlinedocs/cpp/Tokenization.html#Tokenization */
typedef struct Token {
    char *macro;
    int comment;
    int name;
    int number;

    struct Token *prev;
    struct Token *next;
} Token;

/* Doubly linked-list of tokens. */
typedef struct TokenList {
    Token *front_token;
    Token *back_token;
} TokenList;

/* Insert a token at the end of a token list. */
void insert_token(TokenList *list, Token *token) {
    if (!list->front_token) {
        list->front_token = token;
    } else {
        list->back_token->next = token;
    }

    token->prev = list->back_token;
    list->back_token = token;
}

/* Free the allocated memory of a token list and its tokens. */
void free_token_list(TokenList *list) {
    Token *token = list->front_token;

    while (token) {
        if (token->next) {
            token = token->next;
            free(token->prev);
        } else {
            free(token);
            free(list);
            break;
        }
    }
}

/* Append a character at the end of a string. */
void append_char(char *str, char ch) {
    size_t old_length = strlen(str);
    size_t new_length = old_length + 1;

    str = realloc(str, new_length * sizeof(*str));

    str[old_length] = ch;
    str[new_length] = '\0';
}

/* Peek a char in front of the current position in a file stream. */
char f_peek_char(FILE *ifs) {
    char ch;

    fseek(ifs, 1, SEEK_CUR + 1);
    ch = (char) fgetc(ifs);
    fseek(ifs, 1, SEEK_CUR - 1);

    return ch;
}

// TODO: Implement read file function.
/* Read file. */
void read_file() {
    TokenList *token_list = calloc(1, sizeof *token_list);

    for (int i = 1; i < 4; i++) {
        Token *token = calloc(1, sizeof *token);
        token->name = i;

        insert_token(token_list, token);
    }

    free_token_list(token_list);
}

int main(int argc, char **argv) {
    exit(EXIT_SUCCESS);
}
