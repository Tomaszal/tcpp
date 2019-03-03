/**
 * Tomaszal's C preprocessor (TCPP)
 *
 * A C preprocessor implements the macro language used to transform C programs before they are compiled.
 *
 * This project is an attempt to recreate the essential parts of GCC's built in C preprocessor (CPP).
 *
 * <a href="https://gcc.gnu.org/onlinedocs/cpp/">GCC CPP documentation</a>
 *
 * @author Tomas Zaluckij
 * @date Last modified 2019-03-03
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "args.h"
#include "hashmap.h"

/**
 * Stores location in a file.
 */
typedef struct Location {
    char *file_name;

    int line;
    int column;
} Location;

/**
 * Checks if two given locations are on the same line.
 *
 * @param loc1 The first location.
 * @param loc2 The second location.
 * @return 1 if they are, 0 otherwise.
 */
int same_line(Location loc1, Location loc2) {
    return loc1.line == loc2.line && loc1.file_name == loc2.file_name;
}

/**
 * Stores token information and pointers to its neighboring tokens, creating a doubly-linked list.
 */
typedef struct Token {
    char *string;
    char operator;

    int is_identifier;
    int is_number;
    int is_comment;
    int is_directive;

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

    token->location = end_location;
    token->location.column -= (int) strlen(token->string);

    return token;
}

/**
 * Automatically sets a token's flags based on token's present information.
 *
 * @param token The token to which to set the flags to.
 */
void set_token_flags(Token *token) {
    token->operator = (char) ((token->string[1] == '\0') ? token->string[0] : '\0');

    token->is_identifier = is_identifier(token->string[0]);
    token->is_number = isdigit(token->string[0]);
    token->is_comment = token->string[0] == '/' && (token->string[1] == '/' || token->string[1] == '*');
    token->is_directive = token->prev && token->prev->operator == '#';

    if (token->is_directive) {
        if (token->prev->prev && same_line(token->prev->location, token->prev->prev->location)) {
            token->is_directive = 0;
        }
    }
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
 * Writes a token list to a file.
 *
 * @param token_list A token list to write.
 * @param file_name The location of the file to write to.
 */
void write_token_list_to_file(TokenList *token_list, char *file_name) {
    FILE *ofs = fopen(file_name, "w");

    if (!ofs) {
        fprintf(stderr, "Could not open or create file %s.\n", file_name);
        return;
    }

    verbose_printf("Writing tokens to %s.\n", file_name);

    Location location = {NULL, 0, 0};

    for (Token *token = token_list->front_token; token; token = token->next) {
        if (token->location.file_name != location.file_name) {
            if (location.line != 0) {
                fprintf(ofs, "\n");
            }

            location = token->location;
        }

        while (token->location.line > location.line) {
            location.line++;
            location.column = 0;

            fprintf(ofs, "\n");
        }

        while (token->location.column > location.column) {
            location.column++;

            fprintf(ofs, " ");
        }

        if (token->string) {
            location.column += strlen(token->string);

            fprintf(ofs, "%s", token->string);
        }
    }

    fprintf(ofs, "\n");
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
    int current_line = 0;

    for (Token *token = token_list->front_token; token; token = token->next) {
        if (token->location.line > current_line) {
            current_line = token->location.line;
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
        fprintf(stderr, "Could not open file %s.\n", file_name);
        exit(EXIT_FAILURE);
    }

    verbose_printf("Tokenizing file %s.\n", file_name);

    TokenList *token_list = calloc(1, sizeof *token_list);
    Location location = {file_name, 1, 0};
    char ch;

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

/**
 * Preprocesses a list of raw tokens.
 *
 * TODO: Inclusion of header files (~40%)
 *  > Search for files in other folders (how GCC does it)
 *  > Implement system file inclusion
 * TODO: Macro expansions (~15)
 * TODO: Conditional compilation
 * TODO: Line control
 * TODO: Other directives
 *
 * @param token_list A token list to preprocess.
 * @param file_vector Pointer to an array of open file names.
 */
void preprocess_token_list(TokenList *token_list, char ***file_vector) {
    if (!token_list->front_token) {
        return;
    }

    verbose_printf("Preprocessing file %s.\n", token_list->front_token->location.file_name);

    // Get the location of the current file for searching user header files
    char *file_location = malloc((strlen(token_list->front_token->location.file_name) + 1) * sizeof *file_location);
    strcpy(file_location, token_list->front_token->location.file_name);

    for (char *file_name = &file_location[strlen(file_location)];
         file_name != file_location; file_name -= sizeof *file_name) {
        if (*file_name == '/') {
            file_name[1] = '\0';
            break;
        }
    }

    HashMap *define_map = new_hash_map(0xffff, 0xb5c236b5);

    for (Token *token = token_list->front_token; token; token = token->next) {
        if (token->is_directive) {
            if (strcmp(token->string, "include") == 0) {
                // Include statement

                token = token->next;

                char *file_name = NULL;

                // Check if it is a user include
                if (token->string[0] == '"') {
                    file_name = malloc((strlen(file_location) + strlen(&token->string[1])) * sizeof *file_name);
                    strcpy(file_name, file_location);
                    strncat(file_name, &token->string[1], strlen(&token->string[1]) - 1);
                }

                // TODO: check if it is a system include

                if (!file_name) {
                    fprintf(stderr, "Could not find '%s'.\n", token->string);
                    continue;
                }

                // Add file path to the file vector
                int file_vector_size = 0;
                while ((*file_vector)[++file_vector_size]) {}

                *file_vector = realloc(*file_vector, (file_vector_size + 2) * sizeof **file_vector);
                (*file_vector)[file_vector_size] = file_name;
                (*file_vector)[file_vector_size + 1] = NULL;

                // Generate a raw token list from the file
                TokenList *temp_token_list = tokenize_file((*file_vector)[file_vector_size]);

                // Insert a spacer token after include
                Token *spacer = calloc(1, sizeof *spacer);

                spacer->location.file_name = token->location.file_name;
                spacer->location.line = token->location.line + 1;

                spacer->prev = token;
                spacer->next = token->next;

                if (token->next) {
                    token->next->prev = spacer;
                }

                token->next = spacer;

                // Delete include statement tokens
                token = token->prev->prev;
                for (int i = 0; i < 3; i++) {
                    token = delete_token_from_list(token_list, token);
                }

                // Connect newly generated token list to the main token list
                if (token->prev) {
                    token->prev->next = temp_token_list->front_token;
                    temp_token_list->front_token->prev = token->prev;
                } else {
                    token_list->front_token = temp_token_list->front_token;
                }

                token->prev = temp_token_list->back_token;
                temp_token_list->back_token->next = token;

                free(temp_token_list);

            } else if (strcmp(token->string, "define") == 0) {
                // Define statement

                Token *start_token = token->prev;
                token = token->next->next;

                // Generate define's value
                char *string = malloc(sizeof *string);
                *string = '\0';

                Location line = token->location;
                while (same_line(token->location, line)) {
                    string = realloc(string, (strlen(string) + strlen(token->string) + 1) * sizeof *string);
                    strcat(string, token->string);
                    token = delete_token_from_list(token_list, token);
                }

                // Map define's name to it's value
                hash_map_insert_key(define_map, start_token->next->next->string, string);

                // Delete define statement tokens
                token = start_token;
                for (int i = 0; i < 3; i++) {
                    token = delete_token_from_list(token_list, token);
                }
            }

        } else {

            // Check if current token is a defined key
            char *define = (char *) hash_map_get_key(define_map, token->string);
            if (define) {
                // Calculate how much defined value shifted the line horizontally
                int spacing_fix = (int) strlen(token->string) - (int) strlen(define);

                // Replace current token's string with the defined value
                free(token->string);
                token->string = malloc(strlen(define) * sizeof token->string);
                strcpy(token->string, define);

                // Shift all tokens in the current line
                Token *temp_token = token;
                while ((temp_token = temp_token->next) && same_line(token->location, temp_token->location)) {
                    temp_token->location.column -= spacing_fix;
                }
            }
        }
    }

    delete_hash_map(define_map);
    free(file_location);
}

int main(int argc, char **argv) {
    // Parse program arguments
    args_parse(argc, argv);

    // Create a file name vector to store accessed files between functions
    char **file_vector = malloc(2 * sizeof *file_vector);
    file_vector[0] = args->input_file;
    file_vector[1] = NULL;

    // Generate a new raw token list from the input
    TokenList *token_list = tokenize_file(file_vector[0]);

    // Print information about the input file
    normal_printf("%d non-empty lines found.\n", count_non_empty_lines(token_list));
    normal_printf("%d comments found.\n", count_comments(token_list));

    // Delete the comments unless 'keep_comments' is set
    if (!args->keep_comments) {
        delete_comments(token_list);
    }

    // Preprocess the raw token list and write it to the output file
    preprocess_token_list(token_list, &file_vector);
    write_token_list_to_file(token_list, args->output_file);

    // Free allocated memory
    delete_token_list(token_list);
    for (int i = 1; file_vector[i]; i++) {
        free(file_vector[i]);
    }

    // Successfully exit the program
    exit(EXIT_SUCCESS);
}
