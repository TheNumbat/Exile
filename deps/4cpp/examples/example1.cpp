/*
 * Example of: cpp_lex_file
 * 4cpp lexer version 1.1
 *
 *  This is an example of the API level for quick and easy use.
 *  I have marked all of the 4cpp API calls with a comment.
 */

#include "../4cpp_lexer.h"

#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv){
    if (argc < 2){
        fprintf(stderr, "usage: %s <filename1> [filename2 ...]", argv[0]);
        exit(1);
    }
    for (int i = 1; i < argc; ++i){
        FILE *file = fopen(argv[i], "rb");
        
        if (file){
            
            fseek(file, 0, SEEK_END);
            int size = ftell(file);
            fseek(file, 0, SEEK_SET);
            char *data = (char*)malloc(size);
            fread(data, 1, size, file);
            fclose(file);
            
            // Initialize the token array with a capacity of 100
            //  (The exact capacity doesn't really matter unless you're
            //   hoping to avoid reallocation of the array for speed.)
            Cpp_Token_Array tokens = cpp_make_token_array(100);
            
            // Lex the file and store the results in 'tokens'
            cpp_lex_file(data, size, &tokens);
            
            // Iterate the index of each token.
            for (int i = 0; i < tokens.count; ++i){
                // Get the token from the array
                Cpp_Token token = tokens.tokens[i];
                // Check the token's type.
                if (token.type == CPP_TOKEN_COMMENT){
                    // Extract the token's lexeme and print it.
                    printf("%.*s\n", token.size, data + token.start);
                }
            }
            
            // Free the token array we don't need it any more
            //  (This can be skipped in a lot of cases)
            cpp_free_token_array(tokens);
        }
        else{
            fprintf(stderr, "could not open %s", argv[i]);
        }
    }
}