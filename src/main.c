#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "args.h"

/**
 * Stores location in a file.
 */
typedef struct Location {
    char *filename;

    int line;
    int column;
} Location;

/**
 * Stores token information and pointers to its neighboring tokens, creating a doubly-linked list.
 */
typedef struct Token {
    int is_identifier;
    int is_number;
    int is_comment;
    int is_directive;

    char *string;
    char operator;

    Location location;

    struct Token *prev;
    struct Token *next;
} Token;

/**
 * Stores information about doubly-linked token list.
 */
typedef struct TokenList {
    Token *front_token;
    Token *back_token;
} TokenList;

/**
 * Checks if a character is an appropriate first character for an identifier.
 *
 * @param ch The character to be checked.
 * @return 1 if it is, 0 otherwise.
 */
int is_identifier(char ch) {
    return isalpha(ch) || ch == '_' || ch == '$';
}

/**
 * Generates a new token.
 *
 * @param string Token's content.
 * @param end_location Token's final character's location in a file.
 * @return A newly generated token.
 */
Token *new_token(char *string, Location end_location) {
    Token *token = calloc(1, sizeof *token);

    token->string = string;
    token->location.line = end_location.line;
    token->location.column = end_location.column - (int) strlen(token->string);

    return token;
}

/**
 * Automatically sets a token's flags based on token's present information.
 *
 * @param token The token to which to set the flags to.
 */
void set_token_flags(Token *token) {
    token->is_identifier = is_identifier(token->string[0]);
    token->is_number = isdigit(token->string[0]);
    token->is_comment = token->string[0] == '/' && (token->string[1] == '/' || token->string[1] == '*');
    token->operator = (char) ((token->string[1] == '\0') ? token->string[0] : '\0');
    token->is_directive = token->prev && token->prev->operator == '#';
}

/**
 * Inserts a token at the end of a token list. Automatically sets the token's flags.
 *
 * @param list A token list into which to insert the token.
 * @param token A token to be inserted.
 */
void insert_token(TokenList *list, Token *token) {
    if (!list->front_token) {
        list->front_token = token;
    } else {
        list->back_token->next = token;
    }

    token->prev = list->back_token;
    list->back_token = token;

    set_token_flags(token);
}

/**
 * Deletes a token and frees the memory allocated to it.
 *
 * @param token A token to delete.
 */
void delete_token(Token *token) {
    if (!token) {
        return;
    }

    free(token->string);
    free(token);
}

/**
 * Deletes a token list and frees the memory allocated to it.
 *
 * @param token_list A token list to delete.
 */
void delete_token_list(TokenList *token_list) {
    Token *token = token_list->front_token;

    while (token->next) {
        token = token->next;
        delete_token(token->prev);
    }

    delete_token(token);
    free(token_list);
}

/**
 * Deletes a token from a given token list.
 *
 * @param token_list A token list from which to delete the token.
 * @param token A token to delete.
 * @return The next token in the token list.
 */
Token *delete_token_from_list(TokenList *token_list, Token *token) {
    if (!token) {
        return NULL;
    }

    Token *prev = token->prev;
    Token *next = token->next;

    if (prev) {
        prev->next = next;
    }
    if (next) {
        next->prev = prev;
    }
    if (token_list->front_token == token) {
        token_list->front_token = next;
    }
    if (token_list->back_token == token) {
        token_list->back_token = prev;
    }

    delete_token(token);

    return next;
}

/**
 * Appends a character at the end of a string.
 *
 * Safe to use with NULL (unset) strings.
 *
 * @param str A pointer to the string to be appended.
 * @param ch A character to append to the string.
 */
void append_char(char **str, char ch) {
    size_t length = (*str) ? strlen(*str) + 1 : 1;

    *str = realloc(*str, (length + 1) * (sizeof **str));

    if (!*str) {
        fprintf(stderr, "Could not allocate enough memory.");
        exit(EXIT_FAILURE);
    }

    (*str)[length - 1] = ch;
    (*str)[length] = '\0';
}

/**
 * Peaks the next character in a file stream.
 *
 * @param ifs The file from which to read.
 * @return The peeked character.
 */
char f_peek_char(FILE *ifs) {
    char ch = (char) fgetc(ifs);

    if (ch == EOF) {
        return EOF;
    }

    // Continued lines
    if (ch == '\\') {
        char peek1 = (char) fgetc(ifs);

        // LF & CR
        if (peek1 == '\n' || peek1 == '\r') {
            char peek2 = (char) fgetc(ifs);

            // CRLF
            if (peek2 == '\n' && peek1 == '\r') {
                ch = (char) fgetc(ifs);
                fseek(ifs, -1, SEEK_CUR);
            } else {
                ch = peek2;
            }

            fseek(ifs, -1, SEEK_CUR);
        }

        fseek(ifs, -1, SEEK_CUR);
    }

    fseek(ifs, -1, SEEK_CUR);

    return ch;
}

/**
 * Reads the next character from a file stream. Advances the current location accordingly.
 *
 * @param ifs A file from which to read.
 * @param ch A character pointer into which to read.
 * @param location The current location in a file.
 * @return 1 if successful, 0 otherwise.
 */
int f_read_char(FILE *ifs, char *ch, Location *location) {
    *ch = (char) fgetc(ifs);

    // Continued lines (lines ending with \)
    if (*ch == '\\' && (f_peek_char(ifs) == '\n' || f_peek_char(ifs) == '\r')) {
        char peek1 = (char) fgetc(ifs);

        // LF & CR
        if (peek1 == '\n' || peek1 == '\r') {
            char peek2 = (char) fgetc(ifs);

            // CRLF
            if (peek2 == '\n' && peek1 == '\r') {
                *ch = (char) fgetc(ifs);
            } else {
                *ch = peek2;
            }
        } else {
            fseek(ifs, -1, SEEK_CUR);
        }
    }

    // Convert CR & CRLF line endings to LF for consistency
    if (*ch == '\r') {
        // CR -> LF
        *ch = '\n';

        // Skip LF
        if (f_peek_char(ifs) == '\n') {
            fgetc(ifs);
        }
    }

    // Progress location
    if (*ch == '\n') {
        location->line++;
        location->column = 0;
    } else {
        location->column++;
    }

    return *ch != EOF;
}

/**
 * Reads a string between a START and an END character. Advances current file location accordingly.
 *
 * @param ifs A file from which to read.
 * @param start The initial character of the string scope.
 * @param end The final character of the string scope.
 * @param location The current location in a file.
 * @return A new string inside the specified scope (with the terminating characters).
 */
char *f_read_until(FILE *ifs, char start, char end, Location *location) {
    char *string = NULL;
    append_char(&string, start);

    char ch;
    do {
        if (!f_read_char(ifs, &ch, location)) {
            break;
        }

        append_char(&string, ch);
    } while (ch != end && ch != '\n');

    return string;
}

/**
 * Converts a token list into a string.
 *
 * TODO: finish stringify function (add actual string generation and return)
 *
 * @param token_list A token list to stringify.
 * @return A new string with contents of the token list.
 */
void stringify_token_list(TokenList *token_list) {
    Location location;
    location.line = 1;
    location.column = 0;

    for (Token *token = token_list->front_token; token; token = token->next) {
        while (token->location.line > location.line) {
            printf("\n");
            location.line++;
            location.column = 0;
        }

        while (token->location.column > location.column) {
            location.column++;
            printf(" ");
        }

        location.column += strlen(token->string);

        printf("%s", token->string);
    }
}

/**
 * Counts the number of non-empty lines in a token list.
 *
 * Multi-line tokens (continued lines, multi-line comments, etc.) are considered to be 1 line.
 *
 * @param token_list A token list to count.
 * @return The number of non-empty lines.
 */
int count_non_empty_lines(TokenList *token_list) {
    int num = 0;

    Location location;
    location.line = 0;

    for (Token *token = token_list->front_token; token; token = token->next) {
        if (token->location.line > location.line) {
            location.line = token->location.line;
            num++;
        }
    }

    return num;
}

/**
 * Counts the number of comments in a token list.
 *
 * @param token_list A token list to count.
 * @return The number of comments.
 */
int count_comments(TokenList *token_list) {
    int num = 0;

    for (Token *token = token_list->front_token; token; token = token->next) {
        if (token->is_comment) {
            num++;
        }
    }

    return num;
}

/**
 * Deletes all comments from a token list.
 *
 * @param token_list A token list from which to delete comments.
 */
void delete_comments(TokenList *token_list) {
    Token *token = token_list->front_token;

    while (token) {
        if (token->is_comment) {
            token = delete_token_from_list(token_list, token);
        } else {
            token = token->next;
        }
    }
}

/**
 * Reads a given "*.c" file and generates a token list by tokenizing it.
 *
 * More information on this process:
 * https://gcc.gnu.org/onlinedocs/cpp/Tokenization.html#Tokenization
 *
 * @param file_name The location of the file to tokenize.
 * @return A newly generated token list.
 */
TokenList *tokenize_file(char *file_name) {
    FILE *ifs = fopen(file_name, "r");

    if (!ifs) {
        fprintf(stderr, "Could not open %s.", file_name);
        exit(EXIT_FAILURE);
    }

    verbose_printf("Tokenizing file %s.\n", file_name);

    char ch;
    TokenList *token_list = calloc(1, sizeof *token_list);
    Location location;
    location.line = 1;
    location.column = 0;

    while (f_read_char(ifs, &ch, &location)) {
        if (isspace(ch)) {
            continue;
        }

        char *token_string = NULL;

        if (is_identifier(ch) || isdigit(ch)) {
            // Name or number

            do {
                append_char(&token_string, ch);
                ch = f_peek_char(ifs);
            } while ((is_identifier(ch) || isdigit(ch)) && f_read_char(ifs, &ch, &location));

        } else if (ch == '/' && f_peek_char(ifs) == '/') {
            // Single line comments (//)

            do {
                append_char(&token_string, ch);
                ch = f_peek_char(ifs);
            } while (ch != '\n' && f_read_char(ifs, &ch, &location));

        } else if (ch == '/' && f_peek_char(ifs) == '*') {
            // Multiline comments (/* */)

            do {
                append_char(&token_string, ch);
            } while (f_read_char(ifs, &ch, &location) && !(ch == '*' && f_peek_char(ifs) == '/'));

            append_char(&token_string, '*');
            append_char(&token_string, '/');

            fgetc(ifs);

        } else if (ch == '\"' || ch == '\'') {
            // String and char literals (" ')

            token_string = f_read_until(ifs, ch, ch, &location);

        } else if (ch == '<' &&
                   token_list->back_token &&
                   token_list->back_token->is_directive &&
                   strcmp(token_list->back_token->string, "include") == 0) {
            // Include (< >)

            token_string = f_read_until(ifs, '<', '>', &location);

        } else {
            append_char(&token_string, ch);
        }

        insert_token(token_list, new_token(token_string, location));
    }

    return token_list;
}

int main(int argc, char **argv) {
    // Parse program arguments
    args_parse(argc, argv);

    TokenList *token_list = tokenize_file(args->input_file);

    normal_printf("%d non-empty lines found.\n", count_non_empty_lines(token_list));
    normal_printf("%d comments found.\n", count_comments(token_list));

    if (!args->keep_comments) {
        delete_comments(token_list);
    }

    stringify_token_list(token_list);

    delete_token_list(token_list);

    exit(EXIT_SUCCESS);
}
