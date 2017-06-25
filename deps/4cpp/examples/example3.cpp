/*
 * Example of: cpp_relex_step
 * 4cpp lexer version 1.1
 *
 *  This is an example of using the relex system.  This system works with
 *  FCPP_FORBID_MALLOC defined as well, but for this example I will use
 *  cpp_lex_file to get the initial token stack.
 *  I have marked all of the 4cpp calls with a comment.
 */

#include "../4cpp_lexer.h"

#include <stdio.h>
#include <string.h>

static void
print_all_comments(Cpp_Token_Array tokens, char *data){
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
}

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
            char *data = (char*)malloc(size + 1024);
            fread(data, 1, size, file);
            fclose(file);
            
            // Initialize the token array with a capacity of 100
            //  (The exact capacity doesn't really matter unless you're
            //   hoping to avoid reallocation of the array for speed.)
            Cpp_Token_Array array = cpp_make_token_array(100);
            
            // Lex the file and store the results in 'array'
            cpp_lex_file(data, size, &array);
            
            printf("BEFORE EDIT:\n");
            print_all_comments(array, data);
            printf("\n");
            
            int start_position = 0;
            int end_position = 0;
            int line_count = 0;
            for (int i = 0; i < size; ++i){
                if (data[i] == '\n'){
                    ++line_count;
                    if (line_count == 20){
                        start_position = i+1;
                    }
                    if (line_count == 21){
                        end_position = i+1;
                        break;
                    }
                }
            }
            
            char write_value[] = "// Replace some code with this comment.\n// ????\n// Profit!\n";
            int range_len = end_position - start_position;
            int write_len = sizeof(write_value) - 1;
            int shift_amount = write_len - range_len;
            
            if (shift_amount != 0){
                memmove(data + end_position + shift_amount, data + end_position, size - end_position);
            }
            memcpy(data + start_position, write_value, write_len);
            
            // Get the minimum range of tokens the system thinks are likely to be effected.
            Cpp_Relex_Range range = cpp_get_relex_range(&array, start_position, end_position);
            
            // Set the initial capacity to be the number of tokens in the range + a little tolerance.
            // For common edits there is usually a good chance that the new number of tokens is not
            // much greater than the original number, more tolerance prepares the system to handle
            // larger edits, but also means it takes longer to shortcircuit the relex operation
            // if it's going to do a "long relex" that would be better as a normal lex.
            int relex_capacity = range.end_token_index - range.start_token_index + 50;
            Cpp_Token_Array relex_array = cpp_make_token_array(relex_capacity);
            
            // Initialize a relex state.
            // Since we are handling this all in one big chunk the second stage of initialization
            // is not needed.
            Cpp_Relex_Data relex_state = cpp_relex_init(&array, start_position, end_position, shift_amount);
            
            int looping = 1;
            for(;looping;){
                Cpp_Lex_Result lex_result = cpp_relex_step(&relex_state,
                                                           data, size, size,
                                                           &array, &relex_array);
                
                switch (lex_result){
                    case LexResult_NeedChunk:
                    // If the file exists in separate chunks this indicates that
                    // the next chunk in the file should be passed to cpp_relex_step.
                    Assert(!"This is suppose to work in just one chunk!"); break;
                    
                    case LexResult_NeedTokenMemory:
                    {
                        // Here we could have decided to expand relex_array so it could
                        // hold new tokens, and then call cpp_relex_step again, but sometimes
                        // it's good to know when to cut your losses.
                        printf("The edit was too big!\n");
                        
                        // cpp_relex_abort ends the relex process.  It is only necessary if you
                        // want the array token to be restored to it's original state.  If you're
                        // okay with the array just having garbage in it, you don't have to do
                        // anything to end the relex.
                        cpp_relex_abort(&relex_state, &array);
                        
                        looping = 0;
                    }break;
                    
                    case LexResult_Finished:
                    {
                        // Find out how many tokens array needs to be able to store if we
                        // complete the edit.  If it's not enough expand the array accordingly.
                        // (Or you could just give up on the relex.)
                        int new_count = cpp_relex_get_new_count(&relex_state, array.count, &relex_array);
                        if (new_count > array.max_count){
                            cpp_resize_token_array(&array, 2*new_count);
                        }
                        
                        // Now we can complete the operation with a call to cpp_relex_complete.
                        // Before this call is made the array does not actually contain the new tokens.
                        cpp_relex_complete(&relex_state, &array, &relex_array);
                        
                        // Free the relex_array, we don't need it any more
                        cpp_free_token_array(relex_array);
                        
                        printf("AFTER EDIT:\n");
                        print_all_comments(array, data);
                        printf("\n");
                        
                        looping = 0;
                    }break;
                }
            }
            
            // Free the token array we don't need it any more
            //  (This can be skipped in a lot of cases)
            cpp_free_token_array(array);
        }
        else{
            fprintf(stderr, "could not open %s", argv[i]);
        }
    }
}