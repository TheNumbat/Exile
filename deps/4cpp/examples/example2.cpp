/*
 * Example of: cpp_lex_step
 * 4cpp lexer version 1.1
 *
 *  This is an example of using cpp_lex_step to control allocation
 *  and input output chunking.  Allowing all work to happen in whatever
 *  memory you want and conforming to any limitations you want.
 *  In this case it will all work on the stack memory.
 *  I have marked all the 4cpp API calls with a comment.
 */

#define FCPP_FORBID_MALLOC
#include "../4cpp_lexer.h"

#include <stdio.h>

int main(int argc, char **argv){
    if (argc < 2){
        fprintf(stderr, "usage: %s <filename1> [filename2 ...]", argv[0]);
        exit(1);
    }
    for (int i = 1; i < argc; ++i){
        FILE *file = fopen(argv[i], "rb");
        
        if (file){
            char chunk[1024];
            int chunk_size = 1024;
            int read_pos = 0;
            int size;
            
            fseek(file, 0, SEEK_END);
            size = ftell(file);
            fseek(file, 0, SEEK_SET);
            
            // Create a token array with a capacity of 4.
            // Every time it gets full we will read the
            // tokens out, empty the array, and continue.
            Cpp_Token token_space[4];
            Cpp_Token_Array tokens;
            tokens.tokens = token_space;
            tokens.count = 0;
            tokens.max_count = 4;
            
            Cpp_Lex_Data S = cpp_lex_data_init();
            
            int finished = 0;
            int get_next_chunk = 1;
            int read_size = 0;
            
            while (!finished){
                
                if (get_next_chunk){
                    read_size = chunk_size;
                    if (read_pos + read_size > size){
                        read_size = size - read_pos;
                    }
                    
                    fseek(file, read_pos, SEEK_SET);
                    fread(chunk, 1, read_size, file);
                    read_pos += read_size;
                    
                    get_next_chunk = 0;
                }
                
                // Use the state we set up earlier, pass in the current chunk,
                // tell it the full size so we don't have to null terminate it,
                // and tell it to try to fill up the entire token array.
                Cpp_Lex_Result result =
                    cpp_lex_step(&S, chunk, read_size, size, &tokens, NO_OUT_LIMIT);
                
                // Iterate the index of each token.
                for (int i = 0; i < tokens.count; ++i){
                    // Get the token from the array
                    Cpp_Token token = tokens.tokens[i];
                    // Check the token's type.
                    if (token.type == CPP_TOKEN_COMMENT){
                        // Print the lexeme's position.  Since we're chunking
                        // the input, we can't just print the lexeme, we would
                        // have to do a little more work to get the position back.
                        printf("comment at: %d; length %d;\n", token.start, token.size);
                    }
                }
                // Set the array up to receive new tokens
                tokens.count = 0;
                
                switch (result){
                    // The whole file has been lexed.
                    case LexResult_Finished:
                    {
                        finished = 1;
                    }break;
                    
                    // We need to pass a new chunk to the lexer now.
                    // So tell the system to read the next chunk at
                    // the top of the next iteration.
                    case LexResult_NeedChunk:
                    {
                        get_next_chunk = 1;
                    }break;
                    
                    // We always dump the token array between steps.
                    // So nothing special needs to happen here.
                    case LexResult_NeedTokenMemory:
                    {}break;
                }
            }
            
            fclose(file);
        }
        else{
            fprintf(stderr, "could not open %s", argv[i]);
        }
    }
}